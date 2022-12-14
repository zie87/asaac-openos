#include "PCSClient.hh"
#include "PCSMessages.hh"

#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"
#include "ProcessManagement/Process.hh"
#include "ProcessManagement/Thread.hh"

namespace ASAAC
{

	namespace PCS
	{
	
		Client::Client()
		{
			m_RequestVc = 0;
			m_ReplyVc   = 1;
			m_Timeout   = OS_COMPLEX_COMMAND_TIMEOUT;

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


		void Client::setTimeout(ASAAC_TimeInterval Timeout)
		{
			m_Timeout = Timeout;
		}

		
		ASAAC_TimeInterval Client::getTimeout()
		{
			return m_Timeout;
		}

		
		void Client::establishCommunication()
		{
			try
			{
				ProcessManager *PM = ProcessManager::getInstance();
				ThreadManager *TM = ThreadManager::getInstance();
			
				Process *P = PM->getCurrentProcess();
				Thread *T = TM->getCurrentThread();
			
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
				
				throw;
			}
		}	


        void Client::configurePCS()
        {
            try
            {
                m_TwoWayCommunication.attachServer();
            }
            catch (ASAAC_Exception &e)
            {
                e.addPath("Configuring PCS failed");
                
                throw;
            }
        }                        


		void Client::sendRequest( 
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

			CommunicationManager::getInstance()->sendMessage( getRequestVc(), int_timeout, &Message, max_size);
		}


		void Client::receiveReply(
			const ASAAC_PCS_FunctionId FunctionId,
			const unsigned long TransferId, 
			ASAAC_PCS_MessageParameter &Reply,
			ASAAC_Time Timeout )
		{
			size_t max_size = sizeof(ASAAC_PCS_Message);
			long unsigned size = 0;
			ASAAC_PCS_Message Message;

			do
			{
				ASAAC_TimeInterval int_timeout = TimeStamp( Timeout ).asaac_Interval();
				
				CommunicationManager::getInstance()->receiveMessage( getReplyVc(), int_timeout, max_size, &Message, size);
			}				
			while (Message.transfer_id != TransferId);
			
			if (size < max_size)
				throw OSException("Message too small for PCS message record", LOCATION);
			
			if ((Message.unique_message_id.direction      != ASAAC_PCS_Reply) ||
				(Message.unique_message_id.function_id    != FunctionId)      ||
			    (Message.message_parameter._d.direction   != ASAAC_PCS_Reply) ||
			    (Message.message_parameter._d.function_id != FunctionId))
				throw OSException("Message parameter not correct", LOCATION);
				
			Reply = Message.message_parameter;
		}


		void Client::sendAndReceive( 
		    const ASAAC_PCS_FunctionId FunctionId,
		    ASAAC_PCS_MessageParameter &Param)
		{
			sendAndReceive(FunctionId, Param, TimeStamp(m_Timeout).asaac_Time() );
		}

		void Client::sendAndReceive( 
		    const ASAAC_PCS_FunctionId FunctionId,
		    ASAAC_PCS_MessageParameter &Param,
		    ASAAC_Time Timeout)
	    {
			establishCommunication();

			unsigned long TransferId;
			
			sendRequest( FunctionId, TransferId, Param, Timeout );
			receiveReply( FunctionId, TransferId, Param, Timeout);		
	    }	
		
		
		void Client::configureInterface( const ASAAC_InterfaceData& if_config )
		{
			ASAAC_PCS_MessageParameter Param;			
			Param._u.request_configure_interface.if_config = if_config;
			
			sendAndReceive( ASAAC_PCS_ConfigureInterface, Param);
			
			if (Param._u.reply_configure_interface.result == ASAAC_ERROR)
				throw OSException("configureInterface: PCS replied an error", LOCATION);
		}

			
		void Client::createTransferConnection( const ASAAC_TcDescription& tc_description )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_create_connection.tc_description = tc_description;
			
			sendAndReceive( ASAAC_PCS_CreateTransferConnection, Param);

			if (Param._u.reply_create_connection.result == ASAAC_ERROR)
				throw OSException("createTransferConnection: PCS replied an error", LOCATION);
		}


		void Client::getTransferConnectionDescription( ASAAC_PublicId tc_id, ASAAC_TcDescription& tc_description )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_tc_description.tc_id = tc_id;
			
			sendAndReceive( ASAAC_PCS_GetTransferConnectionDescription, Param);

			if (Param._u.reply_tc_description.result == ASAAC_ERROR)
				throw OSException("getTransferConnectionDescription: PCS replied an error", LOCATION);

			tc_description = Param._u.reply_tc_description.tc_description;
		}
		
		
		void Client::destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descriptor )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_destroy_connection.tc_id = tc_id;
			Param._u.request_destroy_connection.network_descriptor = network_descriptor;
			
			sendAndReceive( ASAAC_PCS_DestroyTransferConnection, Param);
			
			if (Param._u.reply_destroy_connection.result == ASAAC_ERROR)
				throw OSException("destroyTransferConnection: PCS replied an error", LOCATION);
		}

			
		void Client::getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_descriptor, ASAAC_NetworkPortStatus& status )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_network_status.network_descriptor = network_descriptor;
			
			sendAndReceive( ASAAC_PCS_GetNetworkPortStatus, Param);
			
			if (Param._u.reply_network_status.result == ASAAC_ERROR)
				throw OSException("getNetworkPortStatus: PCS replied an error", LOCATION);

			status = Param._u.reply_network_status.status;
		}

			
	    void Client::attachTransferConnectionToVirtualChannel(  const ASAAC_VcDescription vc_description, ASAAC_PublicId tc_id, ASAAC_Bool is_data_representation )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_attach_channel.vc_description = vc_description;
			Param._u.request_attach_channel.tc_id = tc_id;
			Param._u.request_attach_channel.is_data_representation = is_data_representation;
			
			sendAndReceive( ASAAC_PCS_AttachTransferConnectionToVirtualChannel, Param);
			
			if (Param._u.reply_attach_channel.result == ASAAC_ERROR)
				throw OSException("attachTransferConnectionToVirtualChannel: PCS replied an error", LOCATION);
		}

		
		void Client::detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id )
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_detach_channel.tc_id = tc_id;
			Param._u.request_detach_channel.vc_id = vc_id;
			
			sendAndReceive( ASAAC_PCS_DetachTransferConnectionFromVirtualChannel, Param);

			if (Param._u.reply_detach_channel.result == ASAAC_ERROR)
				throw OSException("detachTransferConnectionFromVirtualChannel: PCS replied an error", LOCATION);
		}
		
		void Client::getPMData( const unsigned long max_msg_length, const ASAAC_Time timeout, const ASAAC_PublicId sm_send_vc_id, ASAAC_PublicId &vc_id)
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_pm_data.max_msg_length = max_msg_length;
			Param._u.request_pm_data.timeout        = timeout;
			Param._u.request_pm_data.sm_send_vc_id  = sm_send_vc_id;
			
			sendAndReceive( ASAAC_PCS_GetPMData, Param, timeout);

			if (Param._u.reply_pm_data.result == ASAAC_TM_ERROR)
				throw OSException("getPMData: PCS replied an error", LOCATION);
			
			if (Param._u.reply_pm_data.result == ASAAC_TM_TIMEOUT)
				throw TimeoutException(LOCATION);

			vc_id = Param._u.reply_pm_data.vc_id;
		}
		
		void Client::returnPMData( const ASAAC_PublicId vc_id, const ASAAC_PublicId sm_receive_vc_id, const ASAAC_ReturnStatus sm_return_status)
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_return_pm_data.vc_id            = vc_id;
			Param._u.request_return_pm_data.sm_receive_vc_id = sm_receive_vc_id;
			Param._u.request_return_pm_data.sm_return_status = sm_return_status;
			
			sendAndReceive( ASAAC_PCS_ReturnPMData, Param);

			if (Param._u.reply_return_pm_data.result == ASAAC_ERROR)
				throw OSException("returnPMData: PCS replied an error", LOCATION);
		}
		
		//SBS: synchronize with GSM via PCS server/client
		void Client::listenAttachedChannels()
		{
			ASAAC_PCS_MessageParameter Param;
			
			Param._u.request_listen.result = ASAAC_SUCCESS;
			
			sendAndReceive( ASAAC_PCS_ListenAttachedChannels, Param);

			if (Param._u.reply_listen.result == ASAAC_ERROR)
				throw OSException("listenAttachedChannels: PCS replied an error", LOCATION);	}
		
	}
}
