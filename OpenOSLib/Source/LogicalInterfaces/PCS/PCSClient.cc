#include "PCSClient.hh"
#include "PCSMessages.hh"

#include "OpenOS.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/Process.hh"
#include "ProcessManagement/Thread.hh"

namespace ASAAC
{

	namespace PCS
	{
	
		Client::Client()
		{
			m_IsInitialized = false;
			
			m_RequestVc = 0;
			m_ReplyVc = 1;
			m_TimeOut = OS_COMPLEX_COMMAND_TIMEOUT;

			m_TwoWayCommunication.setServerConfiguration(
				OS_PROCESSID_PCS, 			//process_id
				1, 							//thread_id
				OS_PCS_SERVER_VC_REPLY, 	//request_local_vc_id
				OS_PCS_SERVER_VC_REQUEST);	//reply_local_vc_id
							 
			m_TwoWayCommunication.setRequestGlobalVc(
				OS_PCS_GLOBAL_VC_REQUEST, 	//vc_id
				sizeof(ASAAC_PCS_Message));	//msg_length
				
			m_TwoWayCommunication.setReplyGlobalVc(
				OS_PCS_GLOBAL_VC_REPLY, 	//vc_id
				sizeof(ASAAC_PCS_Message));	//msg_length
		}

		
		Client::~Client()
		{
		}


		size_t	Client::predictSize()
		{
			size_t CumulativeSize = 0;
			
			// m_Semaphore  
			CumulativeSize += Semaphore::predictSize();
				
			return CumulativeSize;
		}


		void Client::initialize( bool IsMaster, Allocator *a )
		{
			m_Allocator.initialize( a, predictSize() );
			m_Semaphore.initialize( &m_Allocator, IsMaster );
			m_IsInitialized = true;
		}
		
		
		void Client::deinitialize()
		{
			m_IsInitialized = false;
			m_Semaphore.deinitialize();
			m_Allocator.deinitialize();
		}
		
		
		void Client::setRequestVc(ASAAC_PublicId RequestVc)
		{
			m_RequestVc = RequestVc;
		}


		ASAAC_PublicId Client::getRequestVc()
		{
			return m_RequestVc;
		}

		
		void Client::setReplyVc(ASAAC_PublicId ReplyVc)
		{
			m_ReplyVc = ReplyVc;
		}

				
		ASAAC_PublicId Client::getReplyVc()
		{
			return m_ReplyVc;
		}		


		void Client::setTimeOut(ASAAC_TimeInterval TimeOut)
		{
			m_TimeOut = TimeOut;
		}

		
		ASAAC_TimeInterval Client::getTimeOut()
		{
			return m_TimeOut;
		}

		
		ASAAC_ReturnStatus Client::establishCommunication()
		{
			try
			{
				ProcessManager *PM = ProcessManager::getInstance();
			
				Process *P = PM->getCurrentProcess();
				if (P == 0)
					throw FatalException("Current Process not found", LOCATION);
			
				Thread *T = PM->getCurrentThread();
				if (T == 0)
					throw FatalException("Current Thread not found", LOCATION);
			
				ASAAC_PublicId currentProcessId = P->getId();
				ASAAC_PublicId currentThreadId = T->getId();
				
				m_TwoWayCommunication.setClientConfiguration( 
					currentProcessId, 	//process_id
					currentThreadId,	//thread_id
					getRequestVc(), 	//request_local_vc_id
					getReplyVc());		//reply_local_vc_id
								
				m_TwoWayCommunication.attach();
			}
			catch (ASAAC_Exception &e)
			{
				e.addPath("Establishing communication channel to PCS failed");
				e.logMessage();
				return ASAAC_ERROR;
			}
			
			return ASAAC_SUCCESS;
		}	


        ASAAC_ReturnStatus Client::configurePCS()
        {
            try
            {
                m_TwoWayCommunication.attachServer();
            }
            catch (ASAAC_Exception &e)
            {
                e.addPath("Configuring PCS failed");
                e.logMessage();
                return ASAAC_ERROR;
            }
            
            return ASAAC_SUCCESS;
        }                        


		ASAAC_TimedReturnStatus Client::sendRequest( 
			const ASAAC_PCS_FunctionId FunctionId,
			unsigned long &TransferId,
			const ASAAC_PCS_MessageParameter Request,
			ASAAC_Time Timeout )
		{
			size_t max_size = sizeof(ASAAC_PCS_Message);
			TransferId = rand();		
			ASAAC_PCS_Message Message;

			Message.unique_message_id.function_id    = FunctionId;
			Message.unique_message_id.direction      = ASAAC_PCS_Request;
			Message.transfer_id 		 			 = TransferId;
			Message.message_parameter._d.function_id = FunctionId; 
			Message.message_parameter._d.direction   = ASAAC_PCS_Request; 
			Message.message_parameter._u = Request._u;

			ASAAC_TimeInterval int_timeout = TimeStamp( Timeout ).asaac_Interval();

			return ASAAC_APOS_sendMessage( getRequestVc(), &int_timeout, &Message, max_size);
		}


		ASAAC_TimedReturnStatus Client::receiveReply(
			const ASAAC_PCS_FunctionId FunctionId,
			const unsigned long TransferId, 
			ASAAC_PCS_MessageParameter &Reply,
			ASAAC_Time Timeout )
		{
			size_t max_size = sizeof(ASAAC_PCS_Message);
			long unsigned size = 0;
			ASAAC_TimedReturnStatus status;
			ASAAC_PCS_Message Message;

			do
			{
				ASAAC_TimeInterval int_timeout = TimeStamp( Timeout ).asaac_Interval();
				
				status = ASAAC_APOS_receiveMessage( getReplyVc(), &int_timeout, max_size, &Message, &size);
				
				if (status != ASAAC_TM_SUCCESS)
					return status;
			}				
			while (Message.transfer_id != TransferId);
			
			if (size < max_size)
				return status;
			
			if ((Message.unique_message_id.direction      != ASAAC_PCS_Reply) ||
				(Message.unique_message_id.function_id    != FunctionId)      ||
			    (Message.message_parameter._d.direction   != ASAAC_PCS_Reply) ||
			    (Message.message_parameter._d.function_id != FunctionId))
				return ASAAC_TM_ERROR;
				
			Reply = Message.message_parameter;
			return ASAAC_TM_SUCCESS;
		}


		ASAAC_ReturnStatus Client::sendAndReceive( 
		    const ASAAC_PCS_FunctionId FunctionId,
		    ASAAC_PCS_MessageParameter &Param)
		{
			return sendAndReceive(FunctionId, Param, TimeStamp(m_TimeOut).asaac_Time() );
		}

		ASAAC_ReturnStatus Client::sendAndReceive( 
		    const ASAAC_PCS_FunctionId FunctionId,
		    ASAAC_PCS_MessageParameter &Param,
		    ASAAC_Time Timeout)
	    {
			if (m_IsInitialized == false) 
				throw UninitializedObjectException(LOCATION);
				
			if ( establishCommunication() == ASAAC_ERROR )
				return ASAAC_ERROR;

			unsigned long TransferId;
			
			if (sendRequest( FunctionId, TransferId, Param, Timeout ) != ASAAC_TM_SUCCESS)
				return ASAAC_ERROR;

			if (receiveReply( FunctionId, TransferId, Param, Timeout) != ASAAC_TM_SUCCESS)
				return ASAAC_ERROR;
				
			return ASAAC_SUCCESS;
	    }	
		
		
		ASAAC_ReturnStatus Client::configureInterface( const ASAAC_InterfaceData& if_config )
		{
			ASAAC_PCS_MessageParameter Param;			
			Param._u.request_configure_interface.if_config = if_config;
			
			if (sendAndReceive( ASAAC_PCS_ConfigureInterface, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;
			
			return Param._u.reply_configure_interface.result;
		}

			
		ASAAC_ReturnStatus Client::createTransferConnection( const ASAAC_TcDescription& tc_description )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_create_connection.tc_description = tc_description;
			
			if (sendAndReceive( ASAAC_PCS_CreateTransferConnection, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;

			return Param._u.reply_create_connection.result;
		}


		ASAAC_ReturnStatus Client::getTransferConnectionDescription( ASAAC_PublicId tc_id, ASAAC_TcDescription& tc_description )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_tc_description.tc_id = tc_id;
			
			if (sendAndReceive( ASAAC_PCS_GetTransferConnectionDescription, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;

			tc_description = Param._u.reply_tc_description.tc_description;
			return Param._u.reply_tc_description.result;
		}
		
		
		ASAAC_ReturnStatus Client::destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descriptor )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_destroy_connection.tc_id = tc_id;
			Param._u.request_destroy_connection.network_descriptor = network_descriptor;
			
			if (sendAndReceive( ASAAC_PCS_DestroyTransferConnection, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;
			
			return Param._u.reply_destroy_connection.result;
		}

			
		ASAAC_ReturnStatus Client::getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_descriptor, ASAAC_NetworkPortStatus& status )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_network_status.network_descriptor = network_descriptor;
			
			if (sendAndReceive( ASAAC_PCS_GetNetworkPortStatus, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;
			
			status = Param._u.reply_network_status.status;
			return Param._u.reply_network_status.result;
		}

			
		ASAAC_ReturnStatus Client::attachTransferConnectionToVirtualChannel(  const ASAAC_VcDescription vc_description, ASAAC_PublicId tc_id, ASAAC_Bool is_data_representation )
		{
			ASAAC_PCS_MessageParameter Param;
			
			//Param._u.request_attach_channel.mapping = mapping;
			Param._u.request_attach_channel.vc_description = vc_description;
			Param._u.request_attach_channel.tc_id = tc_id;
			Param._u.request_attach_channel.is_data_representation = is_data_representation;
			
			if (sendAndReceive( ASAAC_PCS_AttachTransferConnectionToVirtualChannel, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;
			
			return Param._u.reply_attach_channel.result;
		}

		
		ASAAC_ReturnStatus Client::detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_detach_channel.tc_id = tc_id;
			Param._u.request_detach_channel.vc_id = vc_id;
			
			if (sendAndReceive( ASAAC_PCS_DetachTransferConnectionFromVirtualChannel, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;

			return Param._u.reply_detach_channel.result;
		}
		
		ASAAC_TimedReturnStatus Client::getPMData( const unsigned long max_msg_length, const ASAAC_Time timeout, const ASAAC_PublicId sm_send_vc_id, ASAAC_PublicId &vc_id)
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_pm_data.max_msg_length = max_msg_length;
			Param._u.request_pm_data.timeout        = timeout;
			Param._u.request_pm_data.sm_send_vc_id  = sm_send_vc_id;
			
			if (sendAndReceive( ASAAC_PCS_GetPMData, Param, timeout) != ASAAC_SUCCESS)
				return ASAAC_TM_ERROR;

			vc_id = Param._u.reply_pm_data.vc_id;
			return Param._u.reply_pm_data.result;
		}
		
		ASAAC_ReturnStatus Client::returnPMData( const ASAAC_PublicId vc_id, const ASAAC_PublicId sm_receive_vc_id, const ASAAC_ReturnStatus sm_return_status)
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_return_pm_data.vc_id            = vc_id;
			Param._u.request_return_pm_data.sm_receive_vc_id = sm_receive_vc_id;
			Param._u.request_return_pm_data.sm_return_status = sm_return_status;
			
			if (sendAndReceive( ASAAC_PCS_ReturnPMData, Param) != ASAAC_SUCCESS)
				return ASAAC_ERROR;

			return Param._u.reply_return_pm_data.result;
		}
		
		
	}
}
