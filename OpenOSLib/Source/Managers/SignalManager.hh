#ifndef SIGNALMANAGER_HH_
#define SIGNALMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "AbstractInterfaces/Callback.hh"
#include "Allocator/LocalMemory.hh"
#include "Common/Templates/SharedMap.hh"

class SignalManager
{
public:
	static SignalManager* getInstance();
	virtual ~SignalManager();
	
	static size_t	predictSize();
	
	void initialize();
	void deinitialize();
	
	void registerSignalHandler( int Signal, Callback& Handler );
	void unregisterSignalHandler( int Signal );
	
	void raiseSignalToProcess( ASAAC_PublicId ProcessId, int Signal, int Value );
	void raiseSignalToThread( ASAAC_PublicId ThreadId, int Signal, int Value );
	void waitForSignal( int Signal, int& Value, const ASAAC_TimeInterval& Timeout = TimeIntervalInfinity );

private:
	bool m_IsInitialized;
	
	SignalManager();
	static void InternalSignalHandler( int Signal, siginfo_t* SignalInfo, void* Context );

	typedef struct {
		Callback*			Handler;
		struct sigaction	OldAction;
	} SignalData;
	
	LocalMemory 				m_Allocator;
	SharedMap<int, SignalData>  m_Signals;
};

#endif /*SIGNALMANAGER_HH_*/
