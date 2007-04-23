#include "SignalManager.hh"

using namespace std;


class NullCallback : public Callback {
public:	
		virtual void call( void* Data ) { };
		virtual ~NullCallback() { };
};



SignalManager::SignalManager()
{
	// TODO: Not quite happy with having to use heap here.
	
	m_Handlers   = new Callback*[ SIGRTMAX ];
	m_OldActions = new struct sigaction[ SIGRTMAX ];
}

SignalManager::~SignalManager()
{
	// Check all Signal Slots.
	for ( int i = 0; i < SIGRTMAX; i++ )
	{
		// If there is still a registered signal in any one of them,
		// unregister it
		if ( m_Handlers[ i ] != 0 )
		{
			unregisterSignalHandler( i );
		}
	}
	
	delete m_Handlers;
	delete m_OldActions;
}


SignalManager* SignalManager::getInstance()
{
	static SignalManager Instance;
	
	return &Instance;
}


ASAAC_ReturnStatus SignalManager::registerSignalHandler( int Signal, Callback& Handler )
{
	// If Signal number is invalid, throw Exception
	if (( Signal < 0 ) || ( Signal >= SIGRTMAX )) throw OSException( LOCATION );
	
	// If another handler has already been registered for this signal, return with error
	if ( m_Handlers[ Signal ] != 0 ) return ASAAC_ERROR;
	
	
	struct sigaction ThisAction;
	
	// Set Handler field
	m_Handlers[ Signal ] = &Handler;
	
	ThisAction.sa_sigaction = SignalManager::InternalSignalHandler;
	ThisAction.sa_flags     = SA_SIGINFO;
	
	sigemptyset( &ThisAction.sa_mask );
	
	// register Handler Wrapper for the signal
	if ( oal_sigaction( Signal, &ThisAction, &(m_OldActions[ Signal ]) ) != 0 )
	{
		// on error, reset handler field
		m_Handlers[ Signal ] = 0;
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus SignalManager::unregisterSignalHandler( int Signal )
{
	// If Signal number is invalid, throw Exception
	if (( Signal < 0 ) || ( Signal >= SIGRTMAX )) throw OSException( LOCATION );

	if ( m_Handlers[ Signal ] == 0 ) return ASAAC_ERROR;
	
	// Set signal handler to old, saved value of the signal
	oal_sigaction( Signal, &(m_OldActions[ Signal ]), 0 );
	
	// reset handler field
	m_Handlers[ Signal ] = 0;
	
	return ASAAC_SUCCESS;
}
	

ASAAC_ReturnStatus SignalManager::raiseSignal( ASAAC_PublicId ProcessId, int Signal, int Value )
{
	union sigval SignalValue;
	
	SignalValue.sival_int = Value;
	
	return ( oal_sigqueue( ProcessId, Signal, SignalValue ) == 0 ) ? ASAAC_SUCCESS : ASAAC_ERROR;
}


ASAAC_TimedReturnStatus SignalManager::waitForSignal( int Signal, int& Value, const ASAAC_TimeInterval& Timeout )
{
	// Null Callback to disable default signal action of the process waiting for a signal
	static NullCallback ThisNullCallback;
	
	sigset_t ThisSigSet;
	siginfo_t ThisSigInfo;

	// Convert Timeout to format required for function call	
	timespec TimeSpecTimeout;

	TimeSpecTimeout.tv_sec  = Timeout.sec;
	TimeSpecTimeout.tv_nsec = Timeout.nsec;

	// Only wait for the indicated signal
	oal_sigemptyset( &ThisSigSet );
	oal_sigaddset( &ThisSigSet, Signal );

	// Register Null Handler. Returns ASAAC_ERROR if there is already a handler installed.
	ASAAC_ReturnStatus ChangedSignalHandler = registerSignalHandler( Signal, ThisNullCallback );
	
	int iError = 0;
	
	if ((Timeout.sec == TimeInfinity.sec) && (Timeout.nsec == TimeInfinity.nsec))
		iError = oal_sigwaitinfo( &ThisSigSet, &ThisSigInfo );
	else iError = oal_sigtimedwait( &ThisSigSet, &ThisSigInfo, &TimeSpecTimeout );

	// unregister Null Handler, if it was applied before.
	if ( ChangedSignalHandler == ASAAC_SUCCESS ) unregisterSignalHandler( Signal );
	
	if (( iError == -1 ) && ( errno = EAGAIN ))
	{
		return ASAAC_TM_TIMEOUT;
	}
	
	if ( iError <= 0 )
	{
		return ASAAC_TM_ERROR;
	}

	Value = ThisSigInfo.si_value.sival_int;
	
	return ASAAC_TM_SUCCESS;
}


void SignalManager::InternalSignalHandler( int Signal, siginfo_t* SignalInfo, void* Context )
{
	// This function merely serves as a wrapper call to decouple the Callback structure
	// from the posix data structure
	
	SignalManager* ThisInstance = SignalManager::getInstance();
	
	if ( ThisInstance->m_Handlers[ Signal ] == 0 ) return;
	
	ThisInstance->m_Handlers[ Signal ]->call( &(SignalInfo->si_value) );
	
	return;
}

