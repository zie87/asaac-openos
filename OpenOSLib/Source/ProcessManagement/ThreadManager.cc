#include "OpenOSObject.hh"

#include "ProcessManagement/ThreadManager.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "Managers/SignalManager.hh"
#include "IPC/BlockingScope.hh"


class ThreadSuspendCallback : public Callback {
public:
		virtual void call ( void* Data )
		{
			Thread::SuspendCallback( Data );
		}
		
		virtual ~ThreadSuspendCallback() { };
};
	
static ThreadSuspendCallback	SuspendCallback;


ThreadManager::ThreadManager()
{
}		


ThreadManager::~ThreadManager()
{
}


ThreadManager* ThreadManager::getInstance()
{
	static ThreadManager Instance;
	
	return &Instance;	
}

	
void ThreadManager::initialize()
{
	try
	{
		m_IsInitialized = true;
	
		SignalManager::getInstance()->registerSignalHandler( OS_SIGNAL_SUSPEND, SuspendCallback );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing ThreadManager", LOCATION);
		
		deinitialize();
		
		throw;
	}
}


void ThreadManager::deinitialize()
{
	try
	{
		SignalManager::getInstance()->unregisterSignalHandler( OS_SIGNAL_SUSPEND );		
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing ThreadManager", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
}


bool ThreadManager::isInitialized()
{
	return m_IsInitialized;
}


void ThreadManager::sleep(const ASAAC_TimeInterval timeout)
{
	getCurrentThread()->sleep( timeout );
}


void ThreadManager::sleepUntil(const ASAAC_Time absolute_local_time)
{
	getCurrentThread()->sleepUntil( absolute_local_time );
}


void ThreadManager::getMyThreadId(ASAAC_PublicId thread_id)
{
	thread_id = getCurrentThread()->getId();
}


void ThreadManager::startThread(const ASAAC_PublicId thread_id)
{
	ProcessManager::getInstance()->getCurrentProcess()->getThread( thread_id )->start();
}


void ThreadManager::suspendSelf()
{
	getCurrentThread()->suspendSelf();
}


void ThreadManager::stopThread(const ASAAC_PublicId thread_id)
{
	getThread( thread_id )->stop();
}

	
void ThreadManager::terminateSelf()
{
	getCurrentThread()->terminateSelf();
}

	
void ThreadManager::lockThreadPreemption(unsigned long lock_level)
{
	ProcessManager::getInstance()->getCurrentProcess()->lockThreadPreemption( lock_level );
}


void ThreadManager::unlockThreadPreemption(unsigned long lock_level)
{
	ProcessManager::getInstance()->getCurrentProcess()->unlockThreadPreemption( lock_level );
}

	
void ThreadManager::getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus thread_status)
{
	getThread( thread_id )->getState( thread_status );
}


void ThreadManager::createThread(const ASAAC_ThreadDescription thread_desc)
{
	ProcessManager::getInstance()->getProcess( thread_desc.global_pid )->createThread( thread_desc );
}


void ThreadManager::setSchedulingParameters(const ASAAC_ThreadSchedulingInfo thread_scheduling_info)
{
	getThread(thread_scheduling_info.global_pid, thread_scheduling_info.thread_id )->setSchedulingParameters( thread_scheduling_info );
}


void ThreadManager::getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus &thread_status)	
{
	getThread( process_id, thread_id )->getState( thread_status );
}


Thread* ThreadManager::getCurrentThread( const bool do_throw )
{
	if (m_IsInitialized == false)
	{
		if (do_throw == true)
			throw UninitializedObjectException(LOCATION);
		else return NULL;
	}

	Process *P = ProcessManager::getInstance()->getCurrentProcess( do_throw );
	
	if ( P == NULL )
		return NULL;
	
	return P->getCurrentThread( do_throw );
}


Thread* ThreadManager::getThread( ASAAC_PublicId ThreadId, const bool do_throw )
{
	Process *P = ProcessManager::getInstance()->getCurrentProcess( do_throw );
	
	if (P == NULL)
		return NULL;
	
	return P->getThread(ThreadId);
}


Thread* ThreadManager::getThread( ASAAC_PublicId ProcessId, ASAAC_PublicId ThreadId, const bool do_throw )
{
	Process *P = ProcessManager::getInstance()->getProcess( do_throw );
	
	if (P == NULL)
		return NULL;
	
	return P->getThread(ThreadId);
}
