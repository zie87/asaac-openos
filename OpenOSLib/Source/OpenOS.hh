#ifndef OPENOS_HH_
#define OPENOS_HH_

#include "OpenOSIncludes.hh"

#include "Exceptions/Exceptions.hh"
#include "Interfaces/OpenOS-Application.hh"

#include "Common/Templates/Shared.hh"
#include "Allocator/SharedMemory.hh"
#include "IPC/SimpleCommandInterface.hh"


typedef void* Address;


class ProcessManager;


class OpenOS
{
public:
	static OpenOS *getInstance();
	~OpenOS();
	
	void initialize( LocalActivityState State );
	void initialize( LocalActivityState State,  bool Flush, ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId = OS_UNUSED_ID );

	void switchState( LocalActivityState State,  ASAAC_PublicId ProcessId );
	
	void deinitialize();

	bool isInitialized();

	void flushSession();	

	ASAAC_ReturnStatus       destroyAllEntities();
	ASAAC_TimedReturnStatus  sendCommand( ASAAC_PublicId CpuId, unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );
	
	SessionId 			getSessionId();
	LocalActivityState  getActivityState();
	long 				getCpuIndex( ASAAC_PublicId CpuId );
	
	static size_t predictSize();
	
private:
	OpenOS();

	void initializeThisObject();
	void deinitializeThisObject();
	
	void initializeGlobalObjects( ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId = OS_UNUSED_ID);
	void deinitializeGlobalObjects();
	
	friend class ProcessManager;
	
	void registerCpu( ASAAC_PublicId cpu_id );
	ASAAC_ReturnStatus unregisterCpu( ASAAC_PublicId cpu_id );	

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
	
	bool					m_IsMaster;
	
	LocalActivityState		m_ActivityState;
	
	SharedMemory			m_Allocator;
	
	Shared<ASAAC_PublicId>	m_CpuId;
	
	Shared<MutexData>		m_MutexData;
	
	SharedMemory 			m_CpuAllocator;
	
public:

	static void FlushSessionHandler( CommandBuffer Buffer );
	
};

#endif /*OPENOS_HH_*/
