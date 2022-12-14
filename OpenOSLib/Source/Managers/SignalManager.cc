#include "SignalManager.hh"

#include "OpenOSObject.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"
#include "IPC/BlockingScope.hh"


class NullCallback : public Callback {
public:	
		virtual void call( void* Data ) { };
		virtual ~NullCallback() { };
};



SignalManager::SignalManager()
{
	m_IsInitialized = false;
}

SignalManager::~SignalManager()
{
}

size_t	SignalManager::predictSize()
{
	size_t CumulativeSize = 0;
	
	// m_ProcessId
	CumulativeSize +=  SharedMap<int, SignalData>::predictSize(OS_MAX_NUMBER_OF_SIGNALS);
	
	return CumulativeSize;
}

void SignalManager::initialize()
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );

	try
	{
		m_IsInitialized = true;
		
		m_Allocator.initialize(predictSize());
		m_Signals.initialize(&m_Allocator, true, OS_MAX_NUMBER_OF_SIGNALS);
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing SignalManager", LOCATION);
		
		deinitialize();
		
		throw;
	}
}


void SignalManager::deinitialize()
{
	if (m_IsInitialized == false)
		return;
		
	try
	{
		while (m_Signals.getCount() > 0)
			unregisterSignalHandler( m_Signals.idOf(0) );
	
		m_Signals.deinitialize();
		m_Allocator.deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing SignalManager", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
}


SignalManager* SignalManager::getInstance()
{
	static SignalManager Instance;
	
	return &Instance;
}


void SignalManager::registerSignalHandler( int Signal, Callback& Handler )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	try
    {
		// If Signal number is invalid, throw Exception
		if (( Signal < 0 ) || ( Signal > SIGRTMAX )) 
			throw OSException( "Signal value is not valid", LOCATION );

		// If another handler has already been registered for this signal, return with error
		if ( m_Signals.indexOf(Signal) != -1 ) 
			throw OSException( "A handler for this signal is already registered", LOCATION );		
		
		struct sigaction ThisAction;
		
		// Set Handler field
		SignalData Data;
		Data.Handler = &Handler;
		
		ThisAction.sa_sigaction = SignalManager::InternalSignalHandler;
		ThisAction.sa_flags     = SA_SIGINFO;
		
		sigemptyset( &ThisAction.sa_mask );
		
		// register Handler Wrapper for the signal
		if ( oal_sigaction( Signal, &ThisAction, &(Data.OldAction) ) != 0 )
			throw OSException( strerror(errno), LOCATION );
		
		try
		{
			m_Signals.add( Signal, Data );
		}
		catch ( ASAAC_Exception &e )
		{
			oal_sigaction( Signal, &(Data.OldAction), 0 );
			
			throw;
		}
    }
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error registering signal handler", LOCATION);
		
		throw;
	}
}


void SignalManager::unregisterSignalHandler( int Signal )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
	    // If Signal number is invalid, throw Exception
		if (( Signal < 0 ) || ( Signal > SIGRTMAX )) 
			throw OSException( "Signal value is not valid", LOCATION );
	
		int Index = m_Signals.indexOf(Signal);
		if ( Index == -1 ) 
			throw OSException( "A handler for this signal is not registered", LOCATION );		

		// Set signal handler to old, saved value of the signal
		if (oal_sigaction( Signal, &(m_Signals[Index].OldAction), 0 ) != 0)
			throw OSException( strerror(errno), LOCATION );

		m_Signals.remove( Signal );
    }
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error unregistering signal handler", LOCATION);
		
		throw;
	}
}
	

void SignalManager::raiseSignalToProcess( ASAAC_PublicId ProcessId, int Signal, int Value )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
	    union sigval SignalValue;
		
		SignalValue.sival_int = Value;
		
		pid_t PosixId = ProcessManager::getInstance()->getProcess(ProcessId)->getPosixId();
		
		if ( oal_sigqueue( PosixId, Signal, SignalValue ) != 0 )
			throw OSException( strerror(errno), LOCATION );
    }
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error raising signal to process", LOCATION);
		
		throw;
	}		
}


void SignalManager::raiseSignalToThread( ASAAC_PublicId ThreadId, int Signal, int Value )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
    	ThreadManager::getInstance()->getThread( ThreadId )->raiseSignal( Signal, Value );
    }
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error raising signal to thread", LOCATION);
		
		throw;
	}		
}


void SignalManager::waitForSignal( int Signal, siginfo_t &SignalInfo, const ASAAC_TimeInterval& Timeout )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    try
    {
       	BlockingScope TimeoutScope();

       	// Null Callback to disable default signal action of the process waiting for a signal
		static NullCallback ThisNullCallback;
		
		sigset_t ThisSigSet;
		siginfo_t ThisSigInfo;
	
		// Convert Timeout to format required for function call
		TimeStamp t(Timeout);
	
		// Only wait for the indicated signal
		oal_sigemptyset( &ThisSigSet );
		oal_sigaddset( &ThisSigSet, Signal );
	
		// Register Null Handler. 
		int Index = m_Signals.indexOf(Signal);
		
		if (Index == -1)
			registerSignalHandler( Signal, ThisNullCallback );
		
		long iError = 0;
		
		do
		{
			timespec TimeSpecTimeout = TimeInterval(t.asaac_Interval()).timespec_Interval();

			if ( TimeInterval(Timeout).isInfinity() )
				iError = oal_sigwaitinfo( &ThisSigSet, &ThisSigInfo );
			else iError = oal_sigtimedwait( &ThisSigSet, &ThisSigInfo, &TimeSpecTimeout );
		}
		while ((iError == -1) && (errno == EINTR));

		// unregister Null Handler, if it was applied before.
		if ( Index == -1 ) 
			unregisterSignalHandler( Signal );
		
		if ( iError == -1 )
		{
			if ( errno == EAGAIN )
				throw TimeoutException( LOCATION );
			else throw OSException( strerror(errno), LOCATION );
		}
	
		SignalInfo = ThisSigInfo;
    }
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error waiting for signal", LOCATION);
		
		throw;
	}		
}


void SignalManager::InternalSignalHandler( int Signal, siginfo_t* SignalInfo, void* Context )
{
    try
    {
		// This function merely serves as a wrapper call to decouple the Callback structure
		// from the posix data structure
		
		SignalManager* ThisInstance = SignalManager::getInstance();
		
		int Index = ThisInstance->m_Signals.indexOf( Signal );
		
		if ( Index == -1 ) 
			return;
		
		if (ThisInstance->m_Signals[Index].Handler == NULL )
			return;
		
		ThisInstance->m_Signals[Index].Handler->call( SignalInfo );
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath("Error executing callback function", LOCATION);
    	
    	e.raiseError();
    }
    
	return;
}

