#include "ProcessManagement/ThreadManager.hh"
#include "ProcessManagement/ProcessManager.hh"

#include "OpenOS.hh"



ThreadManager* ThreadManager::getInstance()
{
	static ThreadManager Instance;
	
	return &Instance;	
}


ThreadManager::~ThreadManager()
{
}

	
ThreadManager::ThreadManager()
{
}		


ASAAC_ReturnStatus ThreadManager::sleep(const ASAAC_TimeInterval timeout)
{
	ASAAC_Time t = TimeStamp(timeout).asaac_Time();
	return ASAAC_APOS_sleepUntil( &t );
}


ASAAC_ReturnStatus ThreadManager::sleepUntil(const ASAAC_Time absolute_local_time)
{
	timespec TimeSpecTimeout = TimeStamp(absolute_local_time).timespec_Time();
	timespec TimeLeft;

	int iError = 0;

	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_WAITING );	

	do {
	
		iError = clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &TimeSpecTimeout, &TimeLeft );
		
		if ( iError == EINVAL ) 
			break;
		
	} while ( iError != 0 );

    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) 
		ThisThread->setState( ASAAC_RUNNING );	
	
	return ( iError == 0 ) ? ASAAC_SUCCESS : ASAAC_ERROR;
}


ASAAC_ReturnStatus ThreadManager::getMyThreadId(ASAAC_PublicId thread_id)
{
	Thread*	ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread == 0 ) 
		return ASAAC_ERROR;
	
	thread_id = ThisThread->getId();
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ThreadManager::startThread(const ASAAC_PublicId thread_id)
{
	try
	{
		Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
		
		if ( ThisProcess == 0 ) 
			throw OSException("'Current process' not found.", LOCATION);
		
		Thread* ThisThread = ThisProcess->getThread( thread_id );
		
		if ( ThisThread == 0 ) 
			throw OSException("Thread not found.", LOCATION);
			
		if (ThisThread->start() == ASAAC_ERROR)
			throw OSException("Thread could not be started.", LOCATION);
			
	}
	catch (ASAAC_Exception &e)
	{
		e.logMessage();
		
		return ASAAC_ERROR;
	}
		
	return ASAAC_SUCCESS
	;
}


ASAAC_ReturnStatus ThreadManager::suspendSelf()
{
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread == 0 ) 
		return ASAAC_ERROR;
	
	return ThisThread->suspendSelf();
}


ASAAC_ReturnStatus ThreadManager::stopThread(const ASAAC_PublicId thread_id)
{
	Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	
	if ( ThisProcess == 0 ) 
		return ASAAC_ERROR;
	
	Thread* ThisThread = ThisProcess->getThread( thread_id );
	
	if ( ThisThread == 0 ) 
		return ASAAC_ERROR;

	return ThisThread->stop();
}

	
void ThreadManager::terminateSelf()
{
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread == 0 ) 
		throw ASAAC_Exception( "Cannot find thread", LOCATION );
	
	ThisThread->terminate();
}

	
ASAAC_ReturnStatus ThreadManager::lockThreadPreemption(unsigned long lock_level)
{
	Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	
	if ( ThisProcess == 0 ) 
		return ASAAC_ERROR;
	
	return ThisProcess->lockThreadPreemption( lock_level );
}


ASAAC_ReturnStatus ThreadManager::unlockThreadPreemption(unsigned long lock_level)
{
	Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	
	if ( ThisProcess == 0 ) 
		return ASAAC_ERROR;
	
	return ThisProcess->unlockThreadPreemption( lock_level );
}

	
ASAAC_ReturnStatus ThreadManager::getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus thread_status)
{
	Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	
	if ( ThisProcess == 0 ) 
		return ASAAC_ERROR;
	
	Thread* ThisThread = ThisProcess->getThread( thread_id );
	
	if ( ThisThread == 0 ) 
		return ASAAC_ERROR;

	return ThisThread->getState( thread_status );
}


ASAAC_ReturnStatus ThreadManager::createThread(const ASAAC_ThreadDescription thread_desc)
{
	Process* TargetProcess = ProcessManager::getInstance()->getProcess( thread_desc.global_pid );
	
	if ( TargetProcess == 0 ) return ASAAC_ERROR;

	return TargetProcess->createThread( thread_desc );
}


ASAAC_ReturnStatus ThreadManager::setSchedulingParameters(const ASAAC_ThreadSchedulingInfo thread_scheduling_info)
{
	Process* TargetProcess = ProcessManager::getInstance()->getProcess( thread_scheduling_info.global_pid );
	
	if ( TargetProcess == 0 ) 
		return ASAAC_ERROR;
	
	Thread* TargetThread = TargetProcess->getThread( thread_scheduling_info.thread_id );
	
	if ( TargetThread == 0 ) 
		return ASAAC_ERROR;

	return TargetThread->setSchedulingParameters( thread_scheduling_info );
}


ASAAC_ReturnStatus ThreadManager::getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus &thread_status)	
{
	Process* TargetProcess = ProcessManager::getInstance()->getProcess( process_id );
	
	if ( TargetProcess == 0 ) 
		return ASAAC_ERROR;
	
	Thread* TargetThread = TargetProcess->getThread( thread_id );
	
	if ( TargetThread == 0 ) 
		return ASAAC_ERROR;

	return TargetThread->getState( thread_status );
}


