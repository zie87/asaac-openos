#ifndef ASAAC_PCS_CLIENT_HH_
#define ASAAC_PCS_CLIENT_HH_

#include "OpenOSIncludes.hh"
#include "PCSMessages.hh"
#include "Communication/TwoWayCommunication.hh"

using namespace std;


namespace ASAAC
{
	namespace PCS
	{
		class Client
		{
		private:
			ASAAC_PublicId 		m_RequestVc;
			ASAAC_PublicId 		m_ReplyVc;
			ASAAC_TimeInterval 	m_Timeout;

			TwoWayCommunication m_TwoWayCommunication;
			
		protected:			
			void sendRequest( 
				const ASAAC_PCS_FunctionId FunctionId,
				unsigned long &TransferId,
				const ASAAC_PCS_MessageParameter Request,
				ASAAC_Time Timeout );

			void receiveReply(
				const ASAAC_PCS_FunctionId FunctionId,
				const unsigned long TransferId, 
				ASAAC_PCS_MessageParameter &Reply,
				ASAAC_Time Timeout);
				
			void sendAndReceive( 
			    const ASAAC_PCS_FunctionId FunctionId,
			    ASAAC_PCS_MessageParameter &Param);
			    	
			void sendAndReceive( 
			    const ASAAC_PCS_FunctionId FunctionId,
			    ASAAC_PCS_MessageParameter &Param,
			    ASAAC_Time Timeout);
			    	
		public:
			Client();
			virtual ~Client();

			void setRequestVc(ASAAC_PublicId RequestVc);
			ASAAC_PublicId getRequestVc();

			void setReplyVc(ASAAC_PublicId ReplyVc);
			ASAAC_PublicId getReplyVc();

			void setTimeout(ASAAC_TimeInterval Timeout);
			ASAAC_TimeInterval getTimeout();
			
            void establishCommunication();
            void configurePCS();                        
            
			void configureInterface( const ASAAC_InterfaceData& if_config );	
			void createTransferConnection( const ASAAC_TcDescription& tc_description );
			void getTransferConnectionDescription( ASAAC_PublicId tc_id, ASAAC_TcDescription& tc_description );
			void destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descriptor );	
			void getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_descriptor, ASAAC_NetworkPortStatus& status );	
			void attachTransferConnectionToVirtualChannel( const ASAAC_VcDescription vc_description, ASAAC_PublicId tc_id, ASAAC_Bool is_data_representation );
			void detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id );
			
			void getPMData( const unsigned long max_msg_length, const ASAAC_Time timeout, const ASAAC_PublicId sm_send_vc_id, ASAAC_PublicId &vc_id);
			void returnPMData( const ASAAC_PublicId vc_id, const ASAAC_PublicId sm_receive_vc_id, const ASAAC_ReturnStatus sm_return_status);
			
			//SBS: synchronize with GSM via PCS server/client
			void listenAttachedChannels();
		};
		
	} //namespace: PCS
} //namespace ASAAC

#endif /*CLIENT_HH_*/
