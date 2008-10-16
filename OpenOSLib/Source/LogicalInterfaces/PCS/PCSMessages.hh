#ifndef ASAAC_PCS_MESSAGES_HH_
#define ASAAC_PCS_MESSAGES_HH_

namespace ASAAC
{
	namespace PCS
	{		
		typedef enum {
			ASAAC_PCS_ConfigureInterface,
			ASAAC_PCS_CreateTransferConnection,
			ASAAC_PCS_GetTransferConnectionDescription,
			ASAAC_PCS_DestroyTransferConnection,
			ASAAC_PCS_GetNetworkPortStatus,
			ASAAC_PCS_AttachTransferConnectionToVirtualChannel,
			ASAAC_PCS_DetachTransferConnectionFromVirtualChannel,
			ASAAC_PCS_GetPMData,
			ASAAC_PCS_ReturnPMData,
			ASAAC_PCS_ListenAttachedChannels, //SBS: synchronize with GSM via PCS server/client
			ASAAC_PCS_FunctionId_MAX
		} ASAAC_PCS_FunctionId;		
	
		typedef enum {
			ASAAC_PCS_Request,
			ASAAC_PCS_Reply
		} ASAAC_PCS_Direction;		
	
		typedef struct {
			ASAAC_PCS_FunctionId function_id;
			ASAAC_PCS_Direction direction;
		} ASAAC_PCS_MessageId;

		typedef struct ASAAC_PCS_RequestConfigureInterfaceData {
			ASAAC_InterfaceData if_config;
		};
		 						
		typedef struct ASAAC_PCS_ReplyConfigureInterfaceData { 	
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestCreateTransferConnectionData {
			ASAAC_TcDescription tc_description; 					
		};
		
		typedef struct ASAAC_PCS_ReplyCreateTransferConnectionData {			
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestGetTransferConnectionDescriptionData {
			ASAAC_PublicId tc_id; 					
		};
		
		typedef struct ASAAC_PCS_ReplyGetTransferConnectionDescriptionData {			
			ASAAC_TcDescription tc_description; 					
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestDestroyTransferConnectionData {
			ASAAC_PublicId tc_id;
			ASAAC_NetworkDescriptor network_descriptor; 					
		};
		
		typedef struct ASAAC_PCS_ReplyDestroyTransferConnectionData {				
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestGetNetworkPortStatusData {
			ASAAC_NetworkDescriptor network_descriptor;					
		};
		
		typedef struct ASAAC_PCS_ReplyGetNetworkPortStatusData {				
			ASAAC_NetworkPortStatus status;
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestAttachTransferConnectionToVirtualChannelData {
			ASAAC_VcDescription vc_description;
			ASAAC_PublicId tc_id; 	
			ASAAC_Bool is_data_representation;
		};
		
		typedef struct ASAAC_PCS_ReplyAttachTransferConnectionToVirtualChannelData {
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestDetachTransferConnectionFromVirtualChannelData {
			ASAAC_PublicId vc_id; 
			ASAAC_PublicId tc_id;
		};
		
		typedef struct ASAAC_PCS_ReplyDetachTransferConnectionFromVirtualChannelData {
			ASAAC_ReturnStatus result;						
		};
		
		typedef struct ASAAC_PCS_RequestGetPMData {
			unsigned long max_msg_length; 
			ASAAC_Time timeout;
			ASAAC_PublicId sm_send_vc_id; //Where PCS shall sendBuffer
		};

		typedef struct ASAAC_PCS_ReplyGetPMData {
			ASAAC_PublicId vc_id;
			ASAAC_TimedReturnStatus result; 
		};
		
		typedef struct ASAAC_PCS_RequestReturnPMData {
			ASAAC_PublicId vc_id; 
			ASAAC_PublicId sm_receive_vc_id; //Where PCS shall receiveBuffer
			ASAAC_ReturnStatus sm_return_status;		
		};

		typedef struct ASAAC_PCS_ReplyReturnPMData {
			ASAAC_ReturnStatus result;						
		};
		
		//SBS: synchronize with GSM via PCS server/client
		typedef struct ASAAC_PCS_RequestListenAttachedChannels {
			ASAAC_ReturnStatus result;						
		};
		//SBS: synchronize with GSM via PCS server/client
		typedef struct ASAAC_PCS_ReplyListenAttachedChannels {
			ASAAC_ReturnStatus result;				
		};
		
		typedef struct ASAAC_PCS_MessageParameter {
			ASAAC_PCS_MessageId _d;
			union {
				ASAAC_PCS_RequestConfigureInterfaceData 						request_configure_interface;
				ASAAC_PCS_ReplyConfigureInterfaceData 							reply_configure_interface;
				ASAAC_PCS_RequestCreateTransferConnectionData 					request_create_connection;
				ASAAC_PCS_ReplyCreateTransferConnectionData 					reply_create_connection;
				ASAAC_PCS_RequestGetTransferConnectionDescriptionData 			request_tc_description;
				ASAAC_PCS_ReplyGetTransferConnectionDescriptionData 			reply_tc_description;
				ASAAC_PCS_RequestDestroyTransferConnectionData 					request_destroy_connection;
				ASAAC_PCS_ReplyDestroyTransferConnectionData 					reply_destroy_connection;
				ASAAC_PCS_RequestGetNetworkPortStatusData 						request_network_status;
				ASAAC_PCS_ReplyGetNetworkPortStatusData 						reply_network_status;
				ASAAC_PCS_RequestAttachTransferConnectionToVirtualChannelData 	request_attach_channel;
				ASAAC_PCS_ReplyAttachTransferConnectionToVirtualChannelData 	reply_attach_channel;
				ASAAC_PCS_RequestDetachTransferConnectionFromVirtualChannelData request_detach_channel;
				ASAAC_PCS_ReplyDetachTransferConnectionFromVirtualChannelData 	reply_detach_channel;
				ASAAC_PCS_RequestGetPMData										request_pm_data;
				ASAAC_PCS_ReplyGetPMData										reply_pm_data;
				ASAAC_PCS_RequestReturnPMData									request_return_pm_data;
				ASAAC_PCS_ReplyReturnPMData										reply_return_pm_data;
				//SBS: synchronize with GSM via PCS server/client
				ASAAC_PCS_RequestListenAttachedChannels							request_listen;
				ASAAC_PCS_ReplyListenAttachedChannels							reply_listen;
			} _u;
		};
		
		typedef struct ASAAC_PCS_Message {
			unsigned long 				transfer_id;
			ASAAC_PCS_MessageId 		unique_message_id;
			ASAAC_PCS_MessageParameter 	message_parameter;
		};
		
	}
}

#endif /*MESSAGES_HH_*/
