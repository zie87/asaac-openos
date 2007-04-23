#include "SMOSWrapper.hh"
#include "OpenOS.hh"


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
    Description.timeout = TimeInterval(5, Minutes).asaac_Interval();

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
	unsigned int max_msg_length,
	unsigned int max_number_of_buffers,
	unsigned int max_number_of_threads_attached)
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
    return ASAAC_SMOS_attachChannelToProcessOrThread( &RequestMapping );
}
