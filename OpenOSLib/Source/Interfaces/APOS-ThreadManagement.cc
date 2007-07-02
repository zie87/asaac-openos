#include "OpenOSObject.hh"

#include "ProcessManagement/ThreadManager.hh"


ASAAC_ReturnStatus ASAAC_APOS_sleep(const ASAAC_TimeInterval* timeout)
{
	return ThreadManager::getInstance()->sleep( *timeout );
}


ASAAC_ReturnStatus ASAAC_APOS_sleepUntil(const ASAAC_Time* absolute_local_time)
{
	return ThreadManager::getInstance()->sleepUntil( *absolute_local_time );
}


ASAAC_ReturnStatus ASAAC_APOS_getMyThreadId(ASAAC_PublicId* thread_id)
{
	return ThreadManager::getInstance()->getMyThreadId( *thread_id );
}


ASAAC_ReturnStatus ASAAC_APOS_startThread(const ASAAC_PublicId thread_id)
{
	return ThreadManager::getInstance()->startThread( thread_id );
}


ASAAC_ReturnStatus ASAAC_APOS_suspendSelf()
{
	return ThreadManager::getInstance()->suspendSelf( );
}


ASAAC_ReturnStatus ASAAC_APOS_stopThread(const ASAAC_PublicId thread_id)
{
	return ThreadManager::getInstance()->stopThread( thread_id );
}


void ASAAC_APOS_terminateSelf()
{
	return ThreadManager::getInstance()->terminateSelf( );
}
	

ASAAC_ReturnStatus ASAAC_APOS_lockThreadPreemption(unsigned long* lock_level)
{
	return ThreadManager::getInstance()->lockThreadPreemption( *lock_level );
}


ASAAC_ReturnStatus ASAAC_APOS_unlockThreadPreemption(unsigned long* lock_level)
{
	return ThreadManager::getInstance()->unlockThreadPreemption( *lock_level );
}
	

ASAAC_ReturnStatus ASAAC_APOS_getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status)
{
	return ThreadManager::getInstance()->getThreadStatus( thread_id, *thread_status );
}

