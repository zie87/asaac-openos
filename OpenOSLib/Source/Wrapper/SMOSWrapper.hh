#ifndef SMOSWRAPPER_HH_
#define SMOSWRAPPER_HH_

#include "OpenOSIncludes.hh"

ASAAC_ReturnStatus createProcess(
	const char *name, 
	ASAAC_PublicId pid, 
	ASAAC_PublicId cpu_id = 0,
	ASAAC_AccessType access_type = ASAAC_LOCAL_ACCESS, 
	ASAAC_PublicId vc_sending = OS_OLI_CLIENT_VC_REQUEST, 
	ASAAC_PublicId vc_receiving = OS_OLI_CLIENT_VC_REPLY);

ASAAC_ReturnStatus createThread(
	const char *name, 
	ASAAC_PublicId pid,
	ASAAC_PublicId id);

ASAAC_ReturnStatus startProcess(ASAAC_PublicId pid);
    
ASAAC_ReturnStatus stopProcess(ASAAC_PublicId pid);

ASAAC_ReturnStatus createVirtualChannel(
	ASAAC_PublicId global_vc_id,
	ASAAC_VirtualChannelType vc_type = ASAAC_Application_Header_VC,
	unsigned long max_msg_length = 1024,
	unsigned long max_number_of_buffers = 40,
	unsigned long max_number_of_threads_attached = 4);
        
ASAAC_ReturnStatus createVcMapping(
	ASAAC_PublicId global_pid,
	ASAAC_PublicId local_vc_id,
	ASAAC_PublicId global_vc_id,
	ASAAC_Bool is_reading,
	unsigned long buffer_size = 1024,
    unsigned long number_of_message_buffers = 10);


#endif /*SMOSWRAPPER_HH_*/
