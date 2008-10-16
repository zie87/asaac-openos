#include "OpenOSObject.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_createProcess(const ASAAC_ProcessDescription* process_desc)
{
    ASAAC_TimedReturnStatus result;
    
	try
	{
		ProcessManager::getInstance()->createProcess( *process_desc );
		
		result = ASAAC_TM_SUCCESS;

	    LOG_SERVICE1(*process_desc, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::createProcess", LOCATION);
	    e.raiseError();
	    
	    result = e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
	}
	
	return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_createThread(const ASAAC_ThreadDescription* thread_desc)
{
    ASAAC_ReturnStatus result;

    try
	{
		ThreadManager::getInstance()->createThread( *thread_desc );

		result = ASAAC_SUCCESS;

	    LOG_SERVICE1(*thread_desc, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::createThread", LOCATION);
	    e.raiseError();
	    
	    result = ASAAC_ERROR;
	}
	
	return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_runProcess(const ASAAC_PublicId process_id)
{
    ASAAC_ReturnStatus result;

    try
	{
		ProcessManager::getInstance()->runProcess( process_id );

        result = ASAAC_SUCCESS;
        
        LOG_SERVICE1(process_id, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::runProcess", LOCATION);
	    e.raiseError();
	    
	    result = ASAAC_ERROR;
	}

    return result;
}
	

ASAAC_ReturnStatus ASAAC_SMOS_stopProcess(const ASAAC_PublicId process_id)
{
    ASAAC_ReturnStatus result;

    try
	{
		ProcessManager::getInstance()->stopProcess( process_id );

        result = ASAAC_SUCCESS;
        
        LOG_SERVICE1(process_id, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::stopProcess", LOCATION);
	    e.raiseError();
	    
	    result = ASAAC_ERROR;
	}

    return result;
}
	

ASAAC_ReturnStatus ASAAC_SMOS_destroyProcess(const ASAAC_PublicId process_id)
{
    ASAAC_ReturnStatus result;

    try
	{
		ProcessManager::getInstance()->destroyProcess( process_id );

        result = ASAAC_SUCCESS;

        LOG_SERVICE1(process_id, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::destroyProcess", LOCATION);
	    e.raiseError();
	    
	    result = ASAAC_ERROR;
	}
	
    return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_setSchedulingParameters(const ASAAC_ThreadSchedulingInfo* thread_scheduling_info)
{
    ASAAC_ReturnStatus result;

    try
	{
		ThreadManager::getInstance()->setSchedulingParameters( *thread_scheduling_info );

        result = ASAAC_SUCCESS;
        
        LOG_SERVICE1(*thread_scheduling_info, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::setSchedulingParameters", LOCATION);
	    e.raiseError();
	    
	    result = ASAAC_ERROR;
	}

    return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status)
{
    ASAAC_ReturnStatus result;

    try
	{
		ThreadManager::getInstance()->getThreadState( process_id, thread_id, *thread_status );

        result = ASAAC_SUCCESS;
        
        LOG_SERVICE3(process_id, thread_id, *thread_status, result);
	}
	catch ( ASAAC_Exception &e )
	{
	    e.addPath("SMOS::getThreadState", LOCATION);
	    e.raiseError();
	    
	    result = ASAAC_ERROR;
	}
	
    return result;
}


