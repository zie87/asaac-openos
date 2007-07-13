#include "OpenOSObject.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_createProcess(const ASAAC_ProcessDescription* process_desc)
{
	try
	{
		ProcessManager::getInstance()->createProcess( *process_desc );
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::create", LOCATION);
	    e.raiseError();
	    
	    return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
	}
	
	return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_createThread(const ASAAC_ThreadDescription* thread_desc)
{
	return ThreadManager::getInstance()->createThread( *thread_desc );
}


ASAAC_ReturnStatus ASAAC_SMOS_runProcess(const ASAAC_PublicId process_id)
{
	try
	{
		ProcessManager::getInstance()->runProcess( process_id );
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::runProcess", LOCATION);
	    e.raiseError();
	    
	    return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}
	

ASAAC_ReturnStatus ASAAC_SMOS_stopProcess(const ASAAC_PublicId process_id)
{
	try
	{
		ProcessManager::getInstance()->stopProcess( process_id );
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::stopProcess", LOCATION);
	    e.raiseError();
	    
	    return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}
	

ASAAC_ReturnStatus ASAAC_SMOS_destroyProcess(const ASAAC_PublicId process_id)
{
	try
	{
		ProcessManager::getInstance()->destroyProcess( process_id );
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::destroyProcess", LOCATION);
	    e.raiseError();
	    
	    return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_setSchedulingParameters(const ASAAC_ThreadSchedulingInfo* thread_scheduling_info)
{
	return ThreadManager::getInstance()->setSchedulingParameters( *thread_scheduling_info );
}


ASAAC_ReturnStatus ASAAC_SMOS_getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status)
{
	return ThreadManager::getInstance()->getThreadState( process_id, thread_id, *thread_status );
}


