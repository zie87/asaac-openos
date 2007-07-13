#include "Thread.hh"
#include "Process.hh"
#include "ProcessManager.hh"
#include "ThreadManager.hh"

#include "Managers/SignalManager.hh"

#include "IPC/ProtectedScope.hh"

#include "FaultManagement/ErrorHandler.hh"

//TODO: For what is this needed?
static bool SuspendStatus;

class ThreadSuspendCallback : public Callback {

public:
		virtual void call ( void* Data )
		{
			int iDummy;

			// get current cancel state
			// if current thread is not supposed to be cancelled,
			// it is not supposed to be suspended, either,
			// because it could hold important OS resources
			oal_thread_setcancelstate( PTHREAD_CANCEL_DISABLE, &iDummy );
			oal_thread_setcancelstate( iDummy, 0 );
			if ( iDummy == PTHREAD_CANCEL_DISABLE )
			{
				// ProtectedScope will take care of calling this longerrupt
				// again once the thread cancellation has been reset
				return;
			}
	
			Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread(false);

			if ( ThisThread != NULL ) 
				ThisThread->setSuspendPending( false );
			
			SignalManager::getInstance()->waitForSignal( OS_SIGNAL_RESUME, iDummy, TimeIntervalInfinity );
	
			if ( ThisThread != NULL ) 
				ThisThread->setSuspendPending( false );
		}
		
		virtual ~ThreadSuspendCallback() { };
};




class ThreadKillCallback : public Callback {
public:	
		virtual void call( void* Data ) 
		{ 
			Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();
			
			if ( ThisThread != 0 ) 
				ThisThread->terminateSelf(); 
		}
		
		virtual ~ThreadKillCallback() { };
};





class ThreadResumeCallback : public Callback {
public:	
		virtual void call( void* Data ) { SuspendStatus = false; };
		virtual ~ThreadResumeCallback() { };
};


static ThreadKillCallback		KillCallback;
static ThreadSuspendCallback	SuspendCallback;
static ThreadResumeCallback		ResumeCallback;


Thread::Thread() : m_ParentProcess(0), m_IsInitialized(false), m_SuspendPending(false)
{
	static bool SignalManagerInitialized = false;
	m_ProtectedScopeStackSize = 0;

	if ( SignalManagerInitialized == false )
	{
		// register signal for termination of threads
		SignalManager::getInstance()->registerSignalHandler( OS_SIGNAL_KILL,    KillCallback );
		
		// register signal for suspension of threads
		SignalManager::getInstance()->registerSignalHandler( OS_SIGNAL_SUSPEND, SuspendCallback );
		
		// register signal to catch superfluous 'resume' signals, to
		// avoid program abortion
		SignalManager::getInstance()->registerSignalHandler( OS_SIGNAL_RESUME,  ResumeCallback );
		
		SignalManagerInitialized = true;
	}
	
}


Thread::~Thread()
{
}


void Thread::initialize( Allocator* ThisAllocator, bool IsMaster, Process* ParentProcess )
{
	assert( m_IsInitialized == false );
	m_IsInitialized = true;

	try
	{		
		m_ThreadData.initialize( ThisAllocator );
	
		if ( IsMaster )
		{
			m_ThreadData->Description.thread_id = OS_UNUSED_ID;
		}
	
		m_ParentProcess = ParentProcess;
	}
	catch ( ASAAC_Exception& E )
	{
		deinitialize();
		
		throw;
	}
}


void Thread::deinitialize()
{
	m_ThreadData.deinitialize();
	m_ParentProcess = NULL;
	
	m_IsInitialized = false;
}


void Thread::enterProtectedScope(ProtectedScope *ps)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if (m_ProtectedScopeStackSize >= OS_MAX_NUMBER_OF_PROTECTEDSCOPES)
		throw FatalException("Maximum number of protected scopes in stack reached.", LOCATION);
		
	m_ProtectedScopeStack[m_ProtectedScopeStackSize] = ps;
	m_ProtectedScopeStackSize++;
}


void Thread::exitProtectedScope(ProtectedScope *ps)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if (m_ProtectedScopeStackSize <= 0)
		throw FatalException("Exit a protected scope without entering.", LOCATION);
		
	m_ProtectedScopeStackSize--;
}


void Thread::assign( const ASAAC_ThreadDescription& Description )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	// If there exists no matching entry point, return error
//	if ( m_ParentProcess->getEntryPoint( Description.entry_polong ) == 0 ) return ASAAC_ERROR;
// ---> EntryPolong is not known at this time
	
	m_ThreadData->Description 			= Description;
	m_ThreadData->PosixThread 			= 0;
	m_ThreadData->Status				= ASAAC_DORMANT;
	m_ThreadData->SuspendLevel			= 0;
}

	
bool Thread::isInitialized()
{
	return m_IsInitialized;
}

	
bool Thread::isCurrentThread()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return ( oal_thread_equal( oal_thread_self(), m_ThreadData->PosixThread ) != 0 );
}


bool Thread::isCancelable( ASAAC_CharacterSequence &scope )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	bool cancelable = true;
	
	for (unsigned long i = 0; i < m_ProtectedScopeStackSize; i++)
	{
		if (!m_ProtectedScopeStack[i]->isCancelable())
		{
			cancelable = false;
			scope = m_ProtectedScopeStack[i]->scope();
		}
	}
	
	return cancelable;
}	


ASAAC_PublicId Thread::getId()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return m_ThreadData->Description.thread_id;
}


void Thread::setState( ASAAC_ThreadStatus State )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	m_ThreadData->Status = State;
}


void Thread::getState( ASAAC_ThreadStatus& State )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	State = m_ThreadData->Status;
}


void Thread::setSchedulingParameters( const ASAAC_ThreadSchedulingInfo& SchedulingInfo )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	ProcessStatus ProcessState = m_ParentProcess->getState();
	
	if ( ProcessState == PROCESS_RUNNING )  
        throw OSException("Process is in state PROCESS_RUNNING", LOCATION);

	m_ThreadData->SchedulingInfo = SchedulingInfo;
		
	if (( ProcessState == PROCESS_STOPPED ) && ( m_ThreadData->Status != ASAAC_DORMANT ))
	{
		// TODO: Set scheduling parameters
	}
}


void Thread::start()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;
	
	if ( m_ThreadData->Status != ASAAC_DORMANT )
		throw OSException("Process is in state: DORMANT", LOCATION);	

	EntryPoint *ThisEntryPoint = m_ParentProcess->getEntryPoint( m_ThreadData->Description.entry_point );

	if ( ThisEntryPoint == 0 ) 
		throw OSException( (ErrorString << "EntryPoint not found: '" << m_ThreadData->Description.entry_point << "' ").c_str() , LOCATION);	

	oal_thread_attr_init( &( m_ThreadData->PosixThreadAttributes ) );
	
	oal_thread_attr_setstacksize( &( m_ThreadData->PosixThreadAttributes ),
							      m_ThreadData->Description.stack_size );
    
    SchedulingData Scheduling = m_ParentProcess->getOSScopeSchedulingData();
    Scheduling.Parameter.__sched_priority--;
    
    /*pthread_attr_setschedparam( &( m_ThreadData->PosixThreadAttributes ),
                                &Scheduling.Parameter );                           

    pthread_attr_setschedpolicy( &( m_ThreadData->PosixThreadAttributes ),
                                Scheduling.Policy );                           
    */
	oal_thread_create( &( m_ThreadData->PosixThread ),
					   &( m_ThreadData->PosixThreadAttributes ),
					      Thread::ThreadStartWrapper,
					    ( void* )( ThisEntryPoint->Address )
				  );
	
	m_ThreadData->Status = ASAAC_RUNNING;
}


void* Thread::ThreadStartWrapper( void* RealAddress )
{
	void* Result = 0;

	try
	{
		// TODO: Maybe add 'dormant' state to thread, if createThread is changed to
		//        actually CREATE the thread
		
		// if the preemption level is > 0, this thread needs to wait for the release signal,
		// just like all threads that have been locked.
		
		Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();
		
		// Thread Id 0 ( Error Handler ) shall not be subject to suspension
		if (( ThisThread->getId() != 0 ) &&
			( ThisThread->m_ThreadData->SuspendLevel > 0 ))
		{
			// emulate a received 'suspend' call.
			SuspendCallback.call( 0 );
		}
		
		EntryPointAddr ThisEntryPoint = EntryPointAddr(RealAddress);
			
		Result = ThisEntryPoint( 0 );

		ThisThread = ThreadManager::getInstance()->getCurrentThread();

		ThisThread->setState(ASAAC_DORMANT);
	}
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Caught exception in main loop of thread", LOCATION);
    	e.raiseError();
    }
    catch ( ... )
    {
        OSException("Caught exception in main loop of thread", LOCATION).raiseError();
    }
		
	return Result;
}



void Thread::stop()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( m_ThreadData->Status == ASAAC_DORMANT ) 
		throw OSException("Thread is in state ASAAC_DORMANT", LOCATION);
	
	oal_thread_cancel( m_ThreadData->PosixThread );

	// resume, so cancellation may be performed
	oal_thread_kill( m_ThreadData->PosixThread, OS_SIGNAL_RESUME );
	
	waitForTermination();
		
	m_ThreadData->Status = ASAAC_DORMANT;
}
	
	
void Thread::waitForTermination()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() ) 
		throw OSException("A thread cannot wait for its own termination", LOCATION);
	
	oal_thread_join( m_ThreadData->PosixThread, 0 );
}
	

void Thread::suspend()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	try
	{
		ASAAC_CharacterSequence scope;
		
		if (!isCancelable(scope))
		{
			CharacterSequence LogMsg;
			LogMsg << "Thread with id (tid=" << getId() << ") shall be suspended. ";
			LogMsg << "Calling thread has to wait for action: '" << scope << "'";
			ErrorHandler::getInstance()->logMessage(LogMsg.asaac_str(), ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE);
		}
		
		if ( isCurrentThread() ) 
			throw OSException("A thread cannot suspend itself by calling 'suspend'. Use 'suspendSelf' instead.", LOCATION);
		
		ProtectedScope Access( "Suspending a thread", *(m_ParentProcess->getSemaphore()) );
	
		m_ThreadData->SuspendLevel ++;
	
		if ( m_ThreadData->Status  == ASAAC_DORMANT ) 
			throw OSException("Thread is in state 'DORAMANT'. It cannot be suspended", LOCATION);
		
		if ( m_ThreadData->SuspendLevel == 1 )
		{
			m_SuspendPending = true;
	 		oal_thread_kill( m_ThreadData->PosixThread, OS_SIGNAL_SUSPEND );
	 		
	 		while ( m_SuspendPending ) 
	 		{ 
	 			sched_yield(); 
	 		}
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Eror suspending a thread", LOCATION);
		
		throw;
	}
}


void Thread::resume()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() ) 
		throw OSException("A thread cannot resume itself", LOCATION);
	
	ProtectedScope Access( "Resuming a thread", *(m_ParentProcess->getSemaphore()) );

	if ( m_ThreadData->SuspendLevel == 0 ) 
		throw OSException("Suspend level is zero", LOCATION);
	
	m_ThreadData->SuspendLevel--;

	if ( m_ThreadData->Status == ASAAC_DORMANT ) 
		throw OSException("Thread is in state ASAAC_DORMANT", LOCATION);
	
	if ( m_ThreadData->SuspendLevel == 0 )
	{
			m_SuspendPending = true;

			oal_thread_kill( m_ThreadData->PosixThread, OS_SIGNAL_RESUME );
	
			while ( m_SuspendPending ) 
			{
				 sched_yield(); 
			}
	}
}
		

bool Thread::isSuspendPending()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return m_SuspendPending;
}


void Thread::setSuspendPending( bool Value )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	m_SuspendPending = Value;
}


void Thread::suspendSelf()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() == false ) 
		throw OSException("This thread object is not responsible for the current one", LOCATION);

	if ( m_ParentProcess->getLockLevel() > 0 )
		throw OSException("The lock level of the thread must be zero", LOCATION);
	
	// Todo: What is a "release condition" ? It's specified in 11.4.1.5 of STANAG, but nowhere else.
	
	oal_sched_yield();
}	
	

void Thread::terminateSelf()
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() == false ) 
		throw OSException("This thread object is not responsible for the current one", LOCATION);
	
//	if ( m_ParentProcess->getLockLevel() > 0 ) return ASAAC_ERROR;
	
	m_ThreadData->Status = ASAAC_DORMANT;
	
	oal_thread_exit(0);
}


void Thread::terminate()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() ) 
		throw OSException("A thread cannot terminate itself using this function", LOCATION);

	if (m_ThreadData->Status == ASAAC_DORMANT)
		throw OSException("The thread is in state ASAAC_DORMANT", LOCATION);
		
	m_ThreadData->Status = ASAAC_DORMANT;
	
	oal_thread_kill( m_ThreadData->PosixThread, OS_SIGNAL_KILL );
}


void Thread::sleep(const ASAAC_TimeInterval timeout)
{
    sleepUntil( TimeStamp(timeout).asaac_Time() );
}


void Thread::sleepUntil(const ASAAC_Time absolute_local_time)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() == false ) 
		throw OSException("This thread object is not responsible for the current one", LOCATION);

	timespec TimeSpecTimeout = TimeStamp(absolute_local_time).timespec_Time();
	timespec TimeLeft;

	long iError = 0;

	setState( ASAAC_WAITING );

	do 
	{
		iError = clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &TimeSpecTimeout, &TimeLeft );
		
		if ( iError == EINVAL ) 
			break;
		
	} while ( iError != 0 );
	
	setState( ASAAC_RUNNING );
	
	if ( iError != 0 ) 
		throw OSException( strerror(errno), LOCATION ); 
}


size_t Thread::predictSize()
{
	return Shared<ThreadData>::predictSize();
}

