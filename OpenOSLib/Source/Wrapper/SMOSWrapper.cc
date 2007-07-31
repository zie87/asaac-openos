#include "SMOSWrapper.hh"
#include "OpenOSObject.hh"


ASAAC_ReturnStatus createProcess(
	const char *name, 
	ASAAC_PublicId pid, 
	ASAAC_PublicId cpu_id,
	ASAAC_AccessType access_type, 
	ASAAC_PublicId vc_sending, 
	ASAAC_PublicId vc_receiving)
{
    ASAAC_ProcessDescription Description;
    Description.global_pid = pid;
    Description.cpu_id = cpu_id;
    Description.programme_file_name = CharSeq(name).asaac_str();
    Description.access_type = access_type;
    Description.access_info._u.oli_channel.vc_sending = vc_sending;
    Description.access_info._u.oli_channel.vc_receiving = vc_receiving;
    Description.access_info._u.oli_channel.fragment_size = sizeof(ASAAC_OctetSequence);
    Description.timeout = TimeInterval(1, Minutes).asaac_Interval();

    ASAAC_TimedReturnStatus Status = ASAAC_SMOS_createProcess( &Description );

	return (Status==ASAAC_TM_SUCCESS)?createThread( "MainThread", pid, 1 ):ASAAC_ERROR;	
}


ASAAC_ReturnStatus createThread(
	const char *name, 
	ASAAC_PublicId pid,
	ASAAC_PublicId id)
{
    ASAAC_ThreadDescription ThreadDesc;
    ThreadDesc.global_pid = pid;
    ThreadDesc.thread_id  = id;
    ThreadDesc.stack_size  = 65536;
    ThreadDesc.entry_point = CharacterSequence(name).asaac_str();

	return ASAAC_SMOS_createThread( &ThreadDesc );	
}


ASAAC_ReturnStatus startProcess(ASAAC_PublicId pid)
{
	return ASAAC_SMOS_runProcess(pid);
}

    
ASAAC_ReturnStatus stopProcess(ASAAC_PublicId pid)
{
	return ASAAC_SMOS_destroyProcess(pid);
}


ASAAC_ReturnStatus createVirtualChannel(
	ASAAC_PublicId global_vc_id,
	ASAAC_VirtualChannelType vc_type,
	unsigned long max_msg_length,
	unsigned long max_number_of_buffers,
	unsigned long max_number_of_threads_attached)
{
	ASAAC_VcDescription Vc;
    Vc.global_vc_id					  = global_vc_id;
    Vc.max_msg_length				  = max_msg_length;
    Vc.max_number_of_buffers		  = max_number_of_buffers;
    Vc.max_number_of_threads_attached = max_number_of_threads_attached;
    Vc.vc_type = vc_type;
    Vc.is_typed_message 			  = ASAAC_BOOL_FALSE;
    return ASAAC_SMOS_createVirtualChannel( &Vc );
}

        
ASAAC_ReturnStatus createVcMapping(
	ASAAC_PublicId global_pid,
	ASAAC_PublicId local_vc_id,
	ASAAC_PublicId global_vc_id,
	ASAAC_Bool is_reading,
    unsigned long buffer_size,
    unsigned long number_of_message_buffers)
{
    ASAAC_VcMappingDescription RequestMapping;
    RequestMapping.global_pid			     = global_pid;
    RequestMapping.local_vc_id				 = local_vc_id;
    RequestMapping.global_vc_id				 = global_vc_id;
    RequestMapping.buffer_size				 = buffer_size;
    RequestMapping.number_of_message_buffers = number_of_message_buffers;
    RequestMapping.is_reading				 = is_reading;
    RequestMapping.is_lifo_queue			 = ASAAC_BOOL_FALSE;
    RequestMapping.is_refusing_queue		 = ASAAC_BOOL_TRUE;
    RequestMapping.local_thread_id           = 1;
    RequestMapping.Priority					 = 0;
    return ASAAC_SMOS_attachChannelToProcessOrThread( &RequestMapping );
}


ASAAC_ReturnStatus createTransferConnection(
	ASAAC_PublicId tc_id,
	ASAAC_PublicId network,
	ASAAC_PublicId port,
	ASAAC_Bool is_receiver
)
{
	ASAAC_TcDescription Description;
	Description.tc_id = tc_id;
	Description.network_descr.network = network;
	Description.network_descr.port = port;
	Description.is_receiver = is_receiver;
	Description.is_msg_transfer = ASAAC_BOOL_TRUE;
	Description.is_fragmented = ASAAC_BOOL_FALSE;
	Description.security_rating.classification_level = ASAAC_UNCLASSIFIED;
	Description.security_rating.security_category = ASAAC_LEVEL_1;
	Description.cpu_id = 0;
	Description.conf_data_size = 0;
	
	return ASAAC_SMOS_createTransferConnection(&Description);
}


ASAAC_ReturnStatus attachTransferConnectionToVirtualChannel(
	ASAAC_PublicId global_vc_id,
	ASAAC_PublicId tc_id,
	ASAAC_Bool is_data_representation)
{
	ASAAC_VcToTcMappingDescription Description;
	Description.global_vc_id = global_vc_id;
	Description.tc_id = tc_id;
	Description.is_data_representation = is_data_representation;
	
	return ASAAC_SMOS_attachTransferConnectionToVirtualChannel(&Description);
}
