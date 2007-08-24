#include "Thread.hh"
#include "Process.hh"
#include "ProcessManager.hh"
#include "ThreadManager.hh"
#include "Managers/SignalManager.hh"

#include "FaultManagement/ErrorHandler.hh"

#include "IPC/ProtectedScope.hh"


Thread::Thread() : m_ParentProcess(0), m_IsInitialized(false), m_SuspendPending(false)
{	
}


Thread::~Thread()
{
}


size_t Thread::predictSize()
{
	return Shared<ThreadData>::predictSize();
}


void Thread::initialize( bool IsMaster, 
		                 const ASAAC_ThreadDescription& Description,
		                 Process* ParentProcess,
		                 Allocator* ThisAllocator )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );

	m_IsInitialized = true;

	try
	{				
		m_ProtectedScopeStackSize = 0;
		m_ThreadData.initialize( ThisAllocator );

		if ( IsMaster )
		{
			m_ThreadData->Description 			= Description;
			m_ThreadData->PosixThread 			= 0;
			m_ThreadData->Status				= ASAAC_DORMANT;
			m_ThreadData->SuspendLevel			= 0;
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
	if (m_IsInitialized == false)
		return;

	try
	{
		m_ThreadData.deinitialize();
		m_ParentProcess = NULL;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing Thread", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
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
			scope = m_ProtectedScopeStack[i]->getScope();
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

    try
    {
		CharacterSequence ErrorString;

		ProcessStatus ProcessState = m_ParentProcess->getState();
		
		if ( ProcessState == PROCESS_STOPPED )  
	        throw OSException("Process shall be in state PROCESS_STOPPED", LOCATION);
	
		if ( m_ThreadData->Status == ASAAC_DORMANT )  
	        throw OSException("Thread must not be in state ASAAC_DORMANT", LOCATION);

		int Policy = SCHED_RR;
	
		if ( SchedulingInfo.scheduling_info.priority < sched_get_priority_min(Policy) )
	        throw OSException( (ErrorString << "Priority is out of systems range (minimum is " << (long)sched_get_priority_min(Policy) << ")").c_str(), LOCATION);
		
		if ( SchedulingInfo.scheduling_info.priority > sched_get_priority_max(Policy) )
	        throw OSException( (ErrorString << "Priority is out of systems range (maximum is " << (long)sched_get_priority_max(Policy) << ")").c_str(), LOCATION);
		
		m_ThreadData->SchedulingInfo = SchedulingInfo;	    
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error setting scheduling parameters", LOCATION);
    	
    	throw;
    }
}


void Thread::update()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
	    if (m_ParentProcess->isServer() == false)
			throw OSException( "Cannot remote thread update", LOCATION);	
	    
	    int Policy = OS_POLICY;
			
		sched_param Parameter;
		Parameter.__sched_priority = 1;
		//Parameter.__sched_priority = m_ThreadData->SchedulingInfo.scheduling_info.priority;
		
#ifndef NO_SPORADIC_SERVER
// Posix standard says, that the following ss parameters are available, 
// if _POSIX_THREAD_SPORADIC_SERVER is defined	
#ifdef _POSIX_THREAD_SPORADIC_SERVER
		if (m_ThreadData->SchedulingInfo.scheduling_info.is_periodic == ASAAC_BOOL_TRUE)
			Policy = SCHED_SPORADIC;
	
		Parameter.sched_ss_low_priority = m_ThreadData->SchedulingInfo.scheduling_info.priority;
		Parameter.sched_ss_repl_period = TimeInterval(m_ThreadData->SchedulingInfo.scheduling_info.period).timespec_Interval();
		Parameter.sched_ss_init_budget = TimeInterval(m_ThreadData->SchedulingInfo.scheduling_info.period).timespec_Interval();
		Parameter.sched_ss_max_repl = m_ThreadData->SchedulingInfo.scheduling_info.max_repl;
#endif
#endif
		
		/*int Result = pthread_setschedparam( m_ThreadData->PosixThread, Policy, &Parameter );
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );*/
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error updating thread", LOCATION);
    	
    	throw;
    }
}


void Thread::start()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
		CharacterSequence ErrorString;
		
		if ( m_ThreadData->Status != ASAAC_DORMANT )
			throw OSException("Process is in state: DORMANT", LOCATION);	
	
		EntryPoint *ThisEntryPoint = m_ParentProcess->getEntryPoint( m_ThreadData->Description.entry_point );
	
		if ( ThisEntryPoint == NULL ) 
			throw OSException( (ErrorString << "EntryPoint not found: '" << m_ThreadData->Description.entry_point << "' ").c_str() , LOCATION);	
	
		int Result;

		sched_param Parameter;
		Parameter.__sched_priority = m_ThreadData->SchedulingInfo.scheduling_info.priority;
	
		Result = oal_thread_attr_init( &( m_ThreadData->PosixThreadAttributes ) );
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );
			
		Result = oal_thread_attr_setstacksize( &( m_ThreadData->PosixThreadAttributes ),
				            				      m_ThreadData->Description.stack_size );
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );
	    
	    Result = pthread_attr_setschedpolicy( &( m_ThreadData->PosixThreadAttributes ),
	                                          OS_POLICY );                           
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );

		Result = pthread_attr_setschedparam( &( m_ThreadData->PosixThreadAttributes ),
	                                         &Parameter );                           
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );
	    
		Result = oal_thread_create( &( m_ThreadData->PosixThread ),
						            &( m_ThreadData->PosixThreadAttributes ),
						            Thread::ThreadStartWrapper,
						            ( void* )( ThisEntryPoint->Address ) );
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );
		
		m_ThreadData->Status = ASAAC_RUNNING;

		update();
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error starting thread", LOCATION);
    	
    	throw;
    }
}


void Thread::stop()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
		if ( m_ThreadData->Status == ASAAC_DORMANT ) 
			throw OSException("Thread is in state ASAAC_DORMANT", LOCATION);
		
		int Result;
		
		// resume, so cancellation may be performed
		raiseSignal( OS_SIGNAL_RESUME );
		
		Result = oal_thread_cancel( m_ThreadData->PosixThread );
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );
	
		waitForTermination();
			
		m_ThreadData->Status = ASAAC_DORMANT;
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error stopping a thread", LOCATION);
    	
    	throw;
    }
}
	
	
void Thread::waitForTermination()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
		if ( isCurrentThread() ) 
			throw OSException("A thread cannot wait for its own termination", LOCATION);
		
		int Result = oal_thread_join( m_ThreadData->PosixThread, 0 );
		if (Result != 0)
			throw OSException( strerror(Result), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error waiting for termination", LOCATION);
    	
    	throw;
    }
}
	

void Thread::suspend()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	try
	{
		if ( m_ThreadData->Status  == ASAAC_DORMANT ) 
			throw OSException("Thread is in state 'DORAMANT'. It cannot be suspended", LOCATION);
		
		if ( isCurrentThread() ) 
			throw OSException("A thread cannot suspend itself by calling 'suspend'. Use 'suspendSelf' instead.", LOCATION);

		ASAAC_CharacterSequence scope;		
		if (!isCancelable(scope))
		{
			CharacterSequence LogMsg;
			LogMsg << "Thread with id (tid=" << getId() << ") shall be suspended. ";
			LogMsg << "Calling thread has to wait for action: '" << scope << "'";
			ErrorHandler::getInstance()->logMessage(LogMsg.asaac_str(), ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE);
		}
		
		ProtectedScope Access( "Suspending a thread", *(m_ParentProcess->getSemaphore()) );
	
		m_ThreadData->SuspendLevel ++;
	
		if ( m_ThreadData->SuspendLevel == 1 )
		{
			m_SuspendPending = true;
			
	 		raiseSignal( OS_SIGNAL_SUSPEND );

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

    try
    {
		if ( isCurrentThread() ) 
			throw OSException("A thread cannot resume itself", LOCATION);
		
		if ( m_ThreadData->Status == ASAAC_DORMANT ) 
			throw OSException("Thread is in state ASAAC_DORMANT", LOCATION);
		
		if ( m_ThreadData->SuspendLevel == 0 ) 
			throw OSException("Thread is not suspended", LOCATION);
		
		ProtectedScope Access( "Resuming a thread", *(m_ParentProcess->getSemaphore()) );
	
		m_ThreadData->SuspendLevel--;
	
		if ( m_ThreadData->SuspendLevel == 0 )
		{
				m_SuspendPending = true;
	
				raiseSignal( OS_SIGNAL_RESUME );
		
				while ( m_SuspendPending ) 
				{
					 sched_yield(); 
				}
		}
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error resuming a thread", LOCATION);
    	
    	throw;
    }
}
		

bool Thread::isSuspendPending()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return m_SuspendPending;
}


void Thread::suspendSelf()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
    	CharacterSequence ErrorString;
    	
		if ( isCurrentThread() == false ) 
			throw OSException("This thread object is not responsible for the current one", LOCATION);
	
		if ( m_ParentProcess->getLockLevel() > 0 )
			throw FatalException( (ErrorString << "Data Corrupted. Lock level of thread shall be equal to zero: " << m_ParentProcess->getLockLevel()).c_str(), LOCATION);
	
		m_ThreadData->SuspendLevel = 1;

		int iDummy;
		waitForSignal( OS_SIGNAL_RESUME, iDummy, TimeIntervalInfinity );
		
		m_SuspendPending = false; // resume() is waiting for this switch
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error suspending a thread itself", LOCATION);
    	
    	throw;
    }
}	
	

void Thread::terminateSelf()
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);

    try
    {
    	CharacterSequence ErrorString;
    	
		if ( isCurrentThread() == false ) 
			throw OSException("This thread object is not responsible for the current one", LOCATION);
		
		if ( m_ParentProcess->getLockLevel() > 0 )
			throw OSException( (ErrorString << "The lock level of the thread must be zero: " << m_ParentProcess->getLockLevel()).c_str(), LOCATION);

		m_ThreadData->Status = ASAAC_DORMANT;
	
		oal_thread_exit(0);
		throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error terminating self", LOCATION);
    	
    	throw;
    }
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

   	BlockingScope TimeoutScope();

	do 
	{
		iError = clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &TimeSpecTimeout, &TimeLeft );
		
		if ( iError == EINVAL ) 
			break;
		
	} while ( iError != 0 );
	
	if ( iError != 0 ) 
		throw OSException( strerror(errno), LOCATION ); 
}


void Thread::raiseSignal( int Signal, int Value )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    int Result = oal_thread_kill( m_ThreadData->PosixThread, Signal );
	if (Result != 0)
		throw OSException( strerror(Result), LOCATION );
}


void Thread::waitForSignal( int Signal, int& Value, const ASAAC_TimeInterval& Timeout )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    if ( isCurrentThread() == false ) 
		throw OSException("Only the thread itself can wait for its signals", LOCATION);

	SignalManager::getInstance()->waitForSignal( Signal, Value, Timeout );
}



// *******************************************************************************************
//                            S C O P E   H A N D L I N G
// *******************************************************************************************

void Thread::enterBlockingScope(BlockingScope &BlockingScopeObject)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    m_ThreadData->Status = ASAAC_WAITING;
}


void Thread::exitBlockingScope(BlockingScope &BlockingScopeObject)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    if ( getId() == BlockingScopeObject.getThreadId() )
		m_ThreadData->Status = ASAAC_RUNNING;
}


void Thread::enterProtectedScope(ProtectedScope &ProtectedScopeObject)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if (m_ProtectedScopeStackSize >= OS_MAX_NUMBER_OF_PROTECTEDSCOPES)
		throw FatalException("Maximum number of protected scopes in stack reached.", LOCATION);
		
	m_ProtectedScopeStack[m_ProtectedScopeStackSize] = &ProtectedScopeObject;
	m_ProtectedScopeStackSize++;
	
	if ( ProtectedScopeObject.isCancelable() == false )
	{
		int CancelState;
		oal_thread_setcancelstate( PTHREAD_CANCEL_DISABLE, &CancelState );
		
		ProtectedScopeObject.setCancelState(CancelState);
	}
}


void Thread::exitProtectedScope(ProtectedScope &ProtectedScopeObject)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	if (m_ProtectedScopeStackSize <= 0)
		throw FatalException("Exit a protected scope without entering.", LOCATION);
		
	m_ProtectedScopeStackSize--;
	
	if ( ProtectedScopeObject.isCancelable() == false )
	{
		oal_thread_setcancelstate( ProtectedScopeObject.getCancelState(), NULL );
		
		if ( ProtectedScopeObject.getCancelState() == PTHREAD_CANCEL_ENABLE )
		{
			oal_thread_testcancel();
		
			try
			{
				if ( (isSuspendPending() == true) && (ProtectedScopeObject.isSuspendPending() == false) )
				{
					raiseSignal( OS_SIGNAL_SUSPEND );
				}
			}
			catch ( ASAAC_Exception &e )
			{
				e.raiseError();
			}
		}
	}

}



// *******************************************************************************************
//                                W R A P P E R
// *******************************************************************************************

void* Thread::ThreadStartWrapper( void* RealAddress )
{
	void* Result = 0;

	try
	{
		// if the preemption level is > 0, this thread needs to wait for the release signal,
		// just like all threads that have been locked.
		
		Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();
		
		// Thread Id 0 ( Error Handler ) shall not be subject to suspension
		if (( ThisThread->getId() != 0 ) &&
			( ThisThread->m_ThreadData->SuspendLevel > 0 ))
		{
			// emulate a received 'suspend' call.
			// TODO: Analyse this call
			SuspendCallback( NULL );
		}
		
		EntryPointAddr ThisEntryPoint = EntryPointAddr(RealAddress);
		
		try
		{
			Result = ThisEntryPoint( 0 );
		}
		catch ( ASAAC_Exception &e )
		{
			e.addPath("Error in main loop of thread", LOCATION);

			ThisThread->m_ThreadData->Status = ASAAC_DORMANT;
			
			throw;
		}
	    catch ( exception &e )
	    {
			ThisThread->m_ThreadData->Status = ASAAC_DORMANT;
			
			throw FatalException(e.what(), LOCATION);
	    }
		
		ThisThread->m_ThreadData->Status = ASAAC_DORMANT;
	}
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Caught exception in wrapper method of thread", LOCATION);
    	e.raiseError();
    }
		
	return Result;
}



// *******************************************************************************************
//                              C A L L B A C K ' S
// *******************************************************************************************


void Thread::SuspendCallback( void* )
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

	Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();
	ThisThread->m_SuspendPending = false; // suspend() is waiting for this switch 

	ThisThread->waitForSignal( OS_SIGNAL_RESUME, iDummy, TimeIntervalInfinity );

	ThisThread->m_SuspendPending = false; // resume() is waiting for this switch
}

