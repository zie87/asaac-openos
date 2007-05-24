#include "OpenOS.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_createProcess(const ASAAC_ProcessDescription* process_desc)
{
	return ProcessManager::getInstance()->createClientProcess( *process_desc );
}


ASAAC_ReturnStatus ASAAC_SMOS_createThread(const ASAAC_ThreadDescription* thread_desc)
{
	return ThreadManager::getInstance()->createThread( *thread_desc );
}


ASAAC_ReturnStatus ASAAC_SMOS_runProcess(const ASAAC_PublicId process_id)
{
	return ProcessManager::getInstance()->runProcess( process_id );
}
	

ASAAC_ReturnStatus ASAAC_SMOS_stopProcess(const ASAAC_PublicId process_id)
{
	return ProcessManager::getInstance()->stopProcess( process_id );
}
	

ASAAC_ReturnStatus ASAAC_SMOS_destroyProcess(const ASAAC_PublicId process_id)
{
	return ProcessManager::getInstance()->destroyClientProcess( process_id );
}


ASAAC_ReturnStatus ASAAC_SMOS_setSchedulingParameters(const ASAAC_ThreadSchedulingInfo* thread_scheduling_info)
{
	return ThreadManager::getInstance()->setSchedulingParameters( *thread_scheduling_info );
}


ASAAC_ReturnStatus ASAAC_SMOS_getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status)
{
	return ThreadManager::getInstance()->getThreadState( process_id, thread_id, *thread_status );
}


