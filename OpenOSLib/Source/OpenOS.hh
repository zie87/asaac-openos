#ifndef OPENOS_HH_
#define OPENOS_HH_

#include "OpenOSIncludes.hh"

#include "Exceptions/Exceptions.hh"
#include "Interfaces/OpenOS-Application.hh"

#include "Common/Shared.hh"
#include "Allocator/SharedMemory.hh"
#include "IPC/SimpleCommandInterface.hh"


typedef void* Address;


class ProcessManager;


class OpenOS
{
public:
	static OpenOS *getInstance();
	~OpenOS();
	
	void initializeRemote( );
	void initializeEntity( ASAAC_PublicId CpuId, bool &FlushSession );
	void initializeProcessStarter( ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId );
	void initializeProcess( ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId );
	
	void deinitialize();

	bool isInitialized();

	ASAAC_ReturnStatus destroyAllEntities();
	ASAAC_TimedReturnStatus sendCommand( ASAAC_PublicId CpuId, unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );
	
	SessionId getSessionId();
	OpenOSContext getContext();
	long getCpuIndex( ASAAC_PublicId CpuId );
	
    bool isSMOSProcess(ASAAC_PublicId ProcessId);
    bool isAPOSProcess(ASAAC_PublicId ProcessId);

	static size_t predictSize();
	
private:
	OpenOS();

	void initialize( bool &IsFirstSession );
	
	void initializeSystem( bool IsMaster, ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId = OS_UNUSED_ID);
	void deinitializeSystem();
	
	friend class ProcessManager;
	
	void registerCpu( ASAAC_PublicId cpu_id );
	ASAAC_ReturnStatus unregisterCpu( ASAAC_PublicId cpu_id );	

	void flushSession( bool IsFirstSession );	
	void flushLocalSession( SessionId NewSessionId );
	
	void initializeMutex();
	void acquireMutex();
	void releaseMutex();
	
	union CommandData {		
		CommandBuffer ReturnBuffer;
		SessionId NewSessionId;
		ASAAC_ReturnStatus  Return;
	};
	
	typedef struct
	{ 
		unsigned long	turn;
		bool			flag[OS_MAX_NUMBER_OF_PROCESSES * (OS_MAX_NUMBER_OF_THREADS + 1)];
	} MutexData;
	
	bool 					m_IsInitialized;
	
	OpenOSContext			m_Context;
	
	SharedMemory			m_Allocator;
	
	Shared<ASAAC_PublicId>	m_CpuId;
	
	Shared<MutexData>		m_MutexData;
	
	SharedMemory 			m_CpuAllocator;
	
public:

	static void FlushSessionHandler( CommandBuffer Buffer );
	
};

#endif /*OPENOS_HH_*/
