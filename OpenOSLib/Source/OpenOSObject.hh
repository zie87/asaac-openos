#ifndef OPENOSOBJECT_HH_
#define OPENOSOBJECT_HH_

#include "OpenOSIncludes.hh"

#include "Exceptions/Exceptions.hh"

#include "Common/Templates/Shared.hh"
#include "Allocator/SharedMemory.hh"
#include "IPC/SimpleCommandInterface.hh"

class ProcessManager;

class OpenOS
{
public:
	static OpenOS *getInstance();
	~OpenOS();

	void initialize(bool IsMaster, LocalActivityState State);
	void initialize(bool IsMaster, LocalActivityState State,
			ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId = OS_UNUSED_ID);

	void switchState(bool IsMaster, LocalActivityState State,
			ASAAC_PublicId ProcessId);

	void deinitialize();

	bool isInitialized();

	void flushSession();

	void destroyAllEntities();
	void sendCommand(ASAAC_PublicId CpuId, unsigned long CommandIdentifier,
			CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity,
			bool Cancelable = false);

	SessionId getSessionId();
	LocalActivityState getActivityState();
	long getCpuIndex(ASAAC_PublicId CpuId);

	static size_t predictSize();

private:
	OpenOS();

	void initializeThisObject();
	void deinitializeThisObject();

	void initializeGlobalObjects(ASAAC_PublicId CpuId,
			ASAAC_PublicId ProcessId = OS_UNUSED_ID);
	void deinitializeGlobalObjects();

	friend class ProcessManager;

	void registerCpu(ASAAC_PublicId cpu_id);
	ASAAC_ReturnStatus unregisterCpu(ASAAC_PublicId cpu_id);

	void flushLocalSession(SessionId NewSessionId);

	void initializeMutex();
	void acquireMutex();
	void releaseMutex();

	typedef union 
	{
		CommandBuffer ReturnBuffer;
		SessionId NewSessionId;
		ASAAC_ReturnStatus Return;
	} CommandData;

	typedef struct
	{
		unsigned long turn;
		bool flag[OS_MAX_NUMBER_OF_PROCESSES * (OS_MAX_NUMBER_OF_THREADS + 1)];
	} MutexData;

	bool m_IsInitialized;

	bool m_IsMaster;

	LocalActivityState m_ActivityState;

	SharedMemory m_Allocator;

	Shared<ASAAC_PublicId> m_CpuId;

	Shared<MutexData> m_MutexData;

	SharedMemory m_CpuAllocator;

public:

	static void FlushSessionHandler(CommandBuffer Buffer);

};

#endif /*OPENOSOBJECT_HH_*/
