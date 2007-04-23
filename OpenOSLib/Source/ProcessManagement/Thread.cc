#include "Thread.hh"
#include "Process.hh"
#include "ProcessManager.hh"

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
				// ProtectedScope will take care of calling this interrupt
				// again once the thread cancellation has been reset
				return;
			}
	
			Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();

			if ( ThisThread != 0 ) 
				ThisThread->setSuspendPending( false );
			
			SignalManager::getInstance()->waitForSignal( ASAAC_SIGNAL_RESUME, iDummy, TimeIntervalInfinity );
	
			if ( ThisThread != 0 ) 
				ThisThread->setSuspendPending( false );
		}
		
		virtual ~ThreadSuspendCallback() { };
};




class ThreadKillCallback : public Callback {
public:	
		virtual void call( void* Data ) 
		{ 
			Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
			
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
		SignalManager::getInstance()->registerSignalHandler( ASAAC_SIGNAL_KILL,    KillCallback );
		
		// register signal for suspension of threads
		SignalManager::getInstance()->registerSignalHandler( ASAAC_SIGNAL_SUSPEND, SuspendCallback );
		
		// register signal to catch superfluous 'resume' signals, to
		// avoid program abortion
		SignalManager::getInstance()->registerSignalHandler( ASAAC_SIGNAL_RESUME,  ResumeCallback );
		
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


ASAAC_ReturnStatus Thread::assign( const ASAAC_ThreadDescription& Description )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	// If there exists no matching entry point, return error
//	if ( m_ParentProcess->getEntryPoint( Description.entry_point ) == 0 ) return ASAAC_ERROR;
// ---> EntryPoint is not known at this time
	
	m_ThreadData->Description 			= Description;
	m_ThreadData->PosixThread 			= 0;
	m_ThreadData->Status				= ASAAC_DORMANT;
	m_ThreadData->SuspendLevel			= 0;
	
	return ASAAC_SUCCESS;
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
	
	for (unsigned int i = 0; i < m_ProtectedScopeStackSize; i++)
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


ASAAC_ReturnStatus Thread::setState( ASAAC_ThreadStatus State )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	m_ThreadData->Status = State;
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Thread::getState( ASAAC_ThreadStatus& State )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	State = m_ThreadData->Status;
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Thread::setSchedulingParameters( const ASAAC_ThreadSchedulingInfo& SchedulingInfo )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	ProcessStatus ProcessState = m_ParentProcess->getState();
	
	if ( ProcessState == PROCESS_RUNNING )  
        return ASAAC_ERROR;

	m_ThreadData->SchedulingInfo = SchedulingInfo;
		
	if (( ProcessState == PROCESS_STOPPED ) && ( m_ThreadData->Status != ASAAC_DORMANT ))
	{
		// TODO: Set scheduling parameters
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Thread::start()
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
		  
	return ASAAC_SUCCESS;
}


void* Thread::ThreadStartWrapper( void* RealAddress )
{
	// TODO: Maybe add 'dormant' state to thread, if createThread is changed to
	//        actually CREATE the thread
	
	// if the preemption level is > 0, this thread needs to wait for the release signal,
	// just like all threads that have been locked.
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	// Thread Id 0 ( Error Handler ) shall not be subject to suspension
	if (( ThisThread->getId() != 0 ) &&
		( ThisThread->m_ThreadData->SuspendLevel > 0 ))
	{
		// emulate a received 'suspend' call.
		SuspendCallback.call( 0 );
	}
	
	EntryPointAddr ThisEntryPoint = EntryPointAddr(RealAddress);
	
	void* result = 0;
	
	try
	{
		result = ThisEntryPoint( 0 );
	}
    catch ( ASAAC_Exception& e )
    {
        e.logMessage();
    }
	
	ThisThread = ProcessManager::getInstance()->getCurrentThread();
    if ( ThisThread != 0 ) 
        ThisThread->setState(ASAAC_DORMANT);
	
	return result;
}



ASAAC_ReturnStatus Thread::stop()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( m_ThreadData->Status == ASAAC_DORMANT ) 
        return ASAAC_ERROR;
	
	oal_thread_cancel( m_ThreadData->PosixThread );

	// resume, so cancellation may be performed
	oal_thread_kill( m_ThreadData->PosixThread, ASAAC_SIGNAL_RESUME );
	
	waitForTermination();
		
	m_ThreadData->Status = ASAAC_DORMANT;
	
	return ASAAC_SUCCESS;
}
	
	
ASAAC_ReturnStatus	Thread::waitForTermination()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() ) 
        return ASAAC_ERROR;
	
	oal_thread_join( m_ThreadData->PosixThread, 0 );
	
	return ASAAC_SUCCESS;
}
	

ASAAC_ReturnStatus Thread::suspend()
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
	 		oal_thread_kill( m_ThreadData->PosixThread, ASAAC_SIGNAL_SUSPEND );
	 		
	 		while ( m_SuspendPending ) 
	 		{ 
	 			sched_yield(); 
	 		}
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.logMessage();
		
		return ASAAC_ERROR;
	}
		
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Thread::resume()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() ) 
		return ASAAC_ERROR;
	
	ProtectedScope Access( "Resuming a thread", *(m_ParentProcess->getSemaphore()) );

	if ( m_ThreadData->SuspendLevel == 0 ) 
		return ASAAC_ERROR;
	
	m_ThreadData->SuspendLevel--;

	if ( m_ThreadData->Status == ASAAC_DORMANT ) 
		return ASAAC_ERROR;
	
	if ( m_ThreadData->SuspendLevel == 0 )
	{
			m_SuspendPending = true;

			oal_thread_kill( m_ThreadData->PosixThread, ASAAC_SIGNAL_RESUME );
	
			while ( m_SuspendPending ) 
			{
				 sched_yield(); 
			}
	}
		
	return ASAAC_SUCCESS;
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


ASAAC_ReturnStatus Thread::suspendSelf()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( m_ParentProcess->getLockLevel() > 0 )
	{
		return ASAAC_ERROR;
	}
	
	// Todo: What is a "release condition" ? It's specified in 11.4.1.5 of STANAG, but nowhere else.
	
	oal_sched_yield();
	
	return ASAAC_SUCCESS;
}	
	

ASAAC_ReturnStatus Thread::terminateSelf()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if (! isCurrentThread() ) 
        return ASAAC_ERROR;
	
//	if ( m_ParentProcess->getLockLevel() > 0 ) return ASAAC_ERROR;
	
	m_ThreadData->Status = ASAAC_DORMANT;
	
	oal_thread_exit(0);
}


ASAAC_ReturnStatus Thread::terminate()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if ( isCurrentThread() ) 
        return ASAAC_ERROR;

	if (m_ThreadData->Status == ASAAC_DORMANT)
		return ASAAC_ERROR;
		
	m_ThreadData->Status = ASAAC_DORMANT;
	
	oal_thread_kill( m_ThreadData->PosixThread, ASAAC_SIGNAL_KILL );
	return ASAAC_SUCCESS;
}



size_t Thread::predictSize()
{
	return Shared<ThreadData>::predictSize();
}

