#include "OpenOSObject.hh"

#include "ProcessManagement/ThreadManager.hh"


ASAAC_ReturnStatus ASAAC_APOS_sleep(const ASAAC_TimeInterval* timeout)
{
	try
	{
		ThreadManager::getInstance()->sleep( *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::sleep", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_sleepUntil(const ASAAC_Time* absolute_local_time)
{
	try
	{
		ThreadManager::getInstance()->sleepUntil( *absolute_local_time );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::sleepUntil", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_getMyThreadId(ASAAC_PublicId* thread_id)
{
	try
	{
		ThreadManager::getInstance()->getMyThreadId( *thread_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getMyThreadId", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_startThread(const ASAAC_PublicId thread_id)
{
	try
	{
		ThreadManager::getInstance()->startThread( thread_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::startThread", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_suspendSelf()
{
	try
	{
		ThreadManager::getInstance()->suspendSelf( );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::suspendSelf", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_stopThread(const ASAAC_PublicId thread_id)
{
	try
	{
		ThreadManager::getInstance()->stopThread( thread_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::stopThread", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


void ASAAC_APOS_terminateSelf()
{
	try
	{
		ThreadManager::getInstance()->terminateSelf( );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::terminateSelf", LOCATION);
        e.raiseError();
    }
}
	

ASAAC_ReturnStatus ASAAC_APOS_lockThreadPreemption(unsigned long* lock_level)
{
	try
	{
		ThreadManager::getInstance()->lockThreadPreemption( *lock_level );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::lockThreadPreemption", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_unlockThreadPreemption(unsigned long* lock_level)
{
	try
	{
		ThreadManager::getInstance()->unlockThreadPreemption( *lock_level );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::unlockThreadPreemption", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}
	

ASAAC_ReturnStatus ASAAC_APOS_getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status)
{
	try
	{
		ThreadManager::getInstance()->getThreadStatus( thread_id, *thread_status );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getThreadStatus", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

