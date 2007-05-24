#ifndef SIGNALMANAGER_HH_
#define SIGNALMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "AbstractInterfaces/Callback.hh"

#include "Exceptions/Exceptions.hh"


#define NUMBER_OF_SIGNALS 64


class SignalManager
{
public:
	
	static SignalManager* getInstance();
	
	virtual ~SignalManager();
	
	ASAAC_ReturnStatus registerSignalHandler( int Signal, Callback& Handler );
	ASAAC_ReturnStatus unregisterSignalHandler( int Signal );
	
	ASAAC_ReturnStatus raiseSignal( ASAAC_PublicId ProcessId, int Signal, int Value );
	ASAAC_TimedReturnStatus waitForSignal( int Signal, int& Value, const ASAAC_TimeInterval& Timeout = TimeIntervalInfinity );

private:

	SignalManager();
	static void InternalSignalHandler( int Signal, siginfo_t* SignalInfo, void* Context );

	Callback**			m_Handlers;
	struct sigaction*	m_OldActions;
	
};

#endif /*SIGNALMANAGER_HH_*/
