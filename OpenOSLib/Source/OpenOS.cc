#include "OpenOS.hh"

#include "Common/Shared.hh"
#include "Allocator/SharedMemory.hh"

#include "Managers/AllocatorManager.hh"
#include "Managers/FileManager.hh"
#include "Managers/FileNameGenerator.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "Communication/CommunicationManager.hh"

#include "FaultManagement/FaultManager.hh"
#include "FaultManagement/LoggingManager.hh"
#include "FaultManagement/ErrorHandler.hh"

OpenOS::OpenOS()
{
	m_IsInitialized = false;
	m_Context = OS_Undefined;
}


OpenOS::~OpenOS()
{
}


OpenOS *OpenOS::getInstance()
{
	static OpenOS Instance;
	return &Instance;
}

	
size_t OpenOS::predictSize()
{
	size_t CumulativeSize = 0;
	
	// m_CpuId
	CumulativeSize += Shared<ASAAC_PublicId>::predictSize(OS_MAX_NUMBER_OF_CPU);
	
	// m_MutexData  
	CumulativeSize += Shared<MutexData>::predictSize();

	// m_Semaphore  
	//CumulativeSize += Semaphore::predictSize();

	// ProcessManager  
	CumulativeSize += ProcessManager::predictSize();

	// CommunicationManager  
	CumulativeSize += CommunicationManager::predictSize();

	return CumulativeSize;	
}	


void OpenOS::initializeRemote( )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);

	m_Context = OS_Remote;
	
	//TODO: initialize remote environment
}


void OpenOS::initializeEntity( ASAAC_PublicId CpuId, bool &FlushSession )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);

	m_Context = OS_Entity;
				
	bool IsFirstSession;
	initialize( IsFirstSession );
	
	if (( FlushSession ) || ( IsFirstSession ))
	{
		flushSession( IsFirstSession );
		initializeSystem( true, CpuId );
	}
	else
	{
		initializeSystem( false, CpuId );
	}

    //Register CommandHandler for OS Level (for entities only)
    ProcessManager::getInstance()->addCommandHandler(CMD_FLUSH_SESSION, OpenOS::FlushSessionHandler);
}


void OpenOS::initializeProcessStarter( ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);

	m_Context = OS_Starter;

	bool IsFirstSession = false;
	initialize( IsFirstSession );
	
	if (IsFirstSession == true)
		throw FatalException("OpenOS::initialize() forced a flush event. Flushing the session by process starter is permitted.", LOCATION);
	
	initializeSystem( false, CpuId, ProcessId );
}


void OpenOS::initializeProcess( ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);

	m_Context = OS_Process;

	bool IsFirstSession = false;
	initialize( IsFirstSession );
	
	if (IsFirstSession == true)
		throw FatalException("OpenOS::initialize() forced a flush event. Flushing the session by client processes is permitted.", LOCATION);
	
	initializeSystem( false, CpuId, ProcessId );
}

	
void OpenOS::initialize( bool &IsFirstSession )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);
		
	try
	{
		IsFirstSession = true;
		 
		try
		{
			//Searching for an already created OpenOS object...
			m_Allocator.initialize(FileNameGenerator::getOpenOSName(), false, predictSize(), false, false);
			
			//If memory found, this is an advise of existing other objects
			//Otherwise m_Allocator.initialize would throw an exception
			IsFirstSession = false;			
		}
		catch ( ASAAC_Exception &e)
		{
			//Do nothing with the exception here, because this is not a real error
			m_Allocator.initialize(FileNameGenerator::getOpenOSName(), true, predictSize(), true, false);			
		}
		
		m_CpuId.initialize(&m_Allocator, OS_MAX_NUMBER_OF_CPU);
		
		m_MutexData.initialize(&m_Allocator);
		
		//initialize random generator
		unsigned int seed = (unsigned int)(TimeStamp::Now().nsec());
		srand(seed);

		m_IsInitialized = true;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Initializing of OpenOS object failed", LOCATION);
		
		m_IsInitialized = true;
		
		deinitialize();
		
		throw;		
	}
	catch ( ... )
	{
		m_IsInitialized = true;
		
		deinitialize();
		
		throw OSException("Initializing of OpenOS object failed", LOCATION);
	}	
}


void OpenOS::initializeSystem( bool IsMaster, ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId )
{
	//Initialize ProcessManager with dedicated rights
	if (m_Context == OS_Entity)
	{	
		ProcessManager::getInstance()->initializeEntityProcess( IsMaster, &m_Allocator, CpuId);
	}

	if ( (m_Context == OS_Starter) || (m_Context == OS_Process) )
	{	
		//TODO: LOCAL memory doesn't work for APOS processes. why?
		//if ( isSMOSProcess(ProcessId) )	
			ProcessManager::getInstance()->initializeClientProcess( &m_Allocator, CpuId, ProcessId, SHARED );
		//else ProcessManager::getInstance()->initializeClientProcess( &m_Allocator, CpuId, ProcessId, LOCAL );
	}
	
	//Initialize CommunicatinManager
	CommunicationManager::getInstance()->initialize( IsMaster, &m_Allocator );
		
	// Initialize Fault Manager and Logging Manager
    // These two must be initialized before the ProcessStarter is
    // called for the first time, in order to allow it to connect
    // to the error and log queues
    FaultManager::getInstance()->initialize(IsMaster);
    LoggingManager::getInstance()->initialize(IsMaster);
    
    //Initialize Error Handler
    ErrorHandler::getInstance()->initialize();		
}


void OpenOS::deinitializeSystem()
{
	CommunicationManager::getInstance()->deinitialize();
	    
	ProcessManager::getInstance()->deinitialize();

	ErrorHandler::getInstance()->deinitialize();
    
	LoggingManager::getInstance()->deinitialize();
	FaultManager::getInstance()->deinitialize();
}


void OpenOS::deinitialize()
{
	if (m_IsInitialized == false)
		return;

	try
	{
		
		deinitializeSystem();

		if ( m_CpuAllocator.isInitialized() )
			m_CpuAllocator.deinitialize();

		m_MutexData.deinitialize();
		m_CpuId.deinitialize();
		
		m_Allocator.deinitialize();
		
		FileManager::getInstance()->closeAllFiles();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing OpenOS", LOCATION);
		e.logMessage();
	}
	catch ( ... )
	{
		FatalException("Error deinitializing OpenOS", LOCATION).logMessage();
	}	

	m_IsInitialized = false;	
}


bool OpenOS::isInitialized()
{
	return m_IsInitialized;
}


ASAAC_ReturnStatus OpenOS::destroyAllEntities()
{
	ASAAC_ReturnStatus Status = ASAAC_SUCCESS;
	
	try
	{
		for (long Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
		{
			if (m_CpuId[Index] == OS_UNUSED_ID)
				continue;
	
			if (m_CpuId[Index] == ProcessManager::getInstance()->getCpuId())
				continue;
			
			CommandData d;
			
			if ( sendCommand( m_CpuId[Index], CMD_TERM_ENTITY, d.ReturnBuffer, TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time(), false ) == ASAAC_TM_ERROR )		
				Status = ASAAC_ERROR;
				
			if (d.Return == ASAAC_ERROR)
				Status = ASAAC_ERROR;
		}	

		if (ProcessManager::getInstance()->destroyEntity() == ASAAC_ERROR)
			Status = ASAAC_ERROR;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error destroying complete OpenOS", LOCATION);
		e.logMessage();
		
		Status = ASAAC_ERROR;
	}
	
	return Status;
}


ASAAC_TimedReturnStatus OpenOS::sendCommand( ASAAC_PublicId CpuId, unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	ASAAC_TimedReturnStatus Result = ASAAC_TM_ERROR;
	
	//To use an allocator here as a member variable is a bug fix
	//in conjunction with AllocatorManager::reallocateAllObject
	if ( m_CpuAllocator.isInitialized() )
		m_CpuAllocator.deinitialize();

	SimpleCommandInterface CommandInterface;

	CharSeq ErrorMessage;
	ErrorMessage << "Error sending a command to ProcessManager (CpuId = " << CpuId << ")";
	
	try
	{
		m_CpuAllocator.initialize( 
			FileNameGenerator::getProcessManagerName( CpuId ),
			false,
			ProcessManager::predictInternalSize(),
			false,
			false );
				
		CommandInterface.initialize( &m_CpuAllocator, false, false );
		
		Result = CommandInterface.sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
	}
	catch (ASAAC_Exception &e)
	{
		CommandInterface.deinitialize();
		
		e.addPath(ErrorMessage.c_str());
		e.logMessage();
		
		return ASAAC_TM_ERROR;
	}
	catch (...)
	{
		CommandInterface.deinitialize();

		FatalException( ErrorMessage.c_str(), LOCATION ).logMessage();
		
		return ASAAC_TM_ERROR;
	}

	CommandInterface.deinitialize();

	return Result;
}


SessionId OpenOS::getSessionId()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
		
	return m_Allocator.getSessionId();
}


OpenOSContext OpenOS::getContext()
{
	return m_Context;
}


long OpenOS::getCpuIndex( ASAAC_PublicId CpuId )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	for ( unsigned int Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index ++ )
	{
		if ( m_CpuId[ Index ] == CpuId )
			return Index;
	}
			 
	return -1;	
}


bool OpenOS::isSMOSProcess(ASAAC_PublicId ProcessId)
{
    return ( (ProcessId >= OS_PROCESSID_SMOS) && (ProcessId <= OS_PROCESSID_SMOS_MAX) );
}


bool OpenOS::isAPOSProcess(ASAAC_PublicId ProcessId)
{
    return ( (ProcessId >= OS_PROCESSID_APOS) && (ProcessId <= OS_PROCESSID_APOS_MAX) );
}


void OpenOS::registerCpu(ASAAC_PublicId cpu_id)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	long Index;
	for (Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
	{
		if (m_CpuId[Index] == cpu_id)
			throw OSException("Cpu is already registered", LOCATION);

		if (m_CpuId[Index] == OS_UNUSED_ID)
			break;
	}
	
	if (Index == OS_MAX_NUMBER_OF_CPU)
		throw OSException("Maximum number of cpu reached", LOCATION);
		
	m_CpuId[Index] = cpu_id;
}


ASAAC_ReturnStatus OpenOS::unregisterCpu(ASAAC_PublicId cpu_id)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	long Index;
	for (Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
	{
		if (m_CpuId[Index] == cpu_id)
			break;
	}
	
	if (Index == OS_MAX_NUMBER_OF_CPU)
	{
		OSException("Cpu is not registered", LOCATION).logMessage();
		return ASAAC_ERROR;
	}
		
	m_CpuId[Index] = OS_UNUSED_ID;
		
	return ASAAC_SUCCESS;
}


void OpenOS::flushSession(bool IsFirstSession)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	if ( IsFirstSession )
	{
		for (long Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
		{
			m_CpuId[Index] = OS_UNUSED_ID;
		}
	}
	
	//generate a new session id
	SessionId NewSessionId;
	
	// be sure to get a new session id
	do 
	{
		NewSessionId = rand();
	} 
	while (NewSessionId == m_Allocator.getSessionId());
	 
	for (long Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
	{
		if (m_CpuId[Index] == OS_UNUSED_ID)
			continue;
		
		CommandData d;		
		ASAAC_TimedReturnStatus Status;
		
		//First we just send a simple command to determine the processes liveliness
		Status = sendCommand(m_CpuId[Index], CMD_GET_PID, d.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time(), false);

		if ( (Status == ASAAC_TM_TIMEOUT) || (Status == ASAAC_TM_ERROR) )
		{ 
			m_CpuId[Index] = OS_UNUSED_ID;
			continue;
		}

		//If susseeded, we send our real approach
		d.NewSessionId = NewSessionId;
		Status = sendCommand(m_CpuId[Index], CMD_FLUSH_SESSION, d.ReturnBuffer, TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time(), false);
		
		if ( (Status == ASAAC_TM_TIMEOUT) || (Status == ASAAC_TM_ERROR) )
		{ 
			m_CpuId[Index] = OS_UNUSED_ID;
			continue;
		}
			
		if (d.Return == ASAAC_ERROR)
			m_CpuId[Index] = OS_UNUSED_ID; //TODO: what else shall we do?
	}
	
	initializeMutex();
	
	flushLocalSession( NewSessionId );
}


void OpenOS::flushLocalSession( SessionId NewSessionId )
{
	if ( ProcessManager::getInstance()->isInitialized() )
		ProcessManager::getInstance()->destroyAllClientProcesses();
		
	if ( CommunicationManager::getInstance()->isInitialized() )
		CommunicationManager::getInstance()->destroyAllGlobalVirtualChannels();
	
	AllocatorManager::getInstance()->reallocateAllObjects( NewSessionId );
}


void OpenOS::initializeMutex()
{
	unsigned long numberOfFlags = OS_MAX_NUMBER_OF_PROCESSES * (OS_MAX_NUMBER_OF_THREADS + 1);
	
	for (unsigned long i = 0; i < numberOfFlags; i++)
		m_MutexData->flag[i] = false;
}


void OpenOS::acquireMutex()
{
	Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	Thread *ThisThread = ProcessManager::getInstance()->getCurrentThread();

	ASAAC_PublicId process_id = 0;
	ASAAC_PublicId thread_id = OS_UNUSED_ID;
	
	if (ThisProcess != 0)
		if (ThisProcess->isInitialized())
			process_id = ThisProcess->getId();

	if (ThisThread != 0)
		if (ThisThread->isInitialized())
			thread_id = ThisThread->getId();

	unsigned long processIdx = 0;
	unsigned long threadIdx = 0;

	if (ThisProcess != 0)
	{
		processIdx = ProcessManager::getInstance()->getProcessIndex(ThisProcess->getId());
		
		if (ThisProcess->isInitialized())
			threadIdx = ThisProcess->getThreadIndex(thread_id);
	}

	unsigned long thisIdx = processIdx * (OS_MAX_NUMBER_OF_THREADS+1) + threadIdx;
	unsigned long numberOfFlags = OS_MAX_NUMBER_OF_PROCESSES * (OS_MAX_NUMBER_OF_THREADS + 1);	

	//begin of Peterson algorithm to entry a global safety state
	m_MutexData->flag[thisIdx] = true;

	for (unsigned long i = 0; i < numberOfFlags; i++)
	{ 
		if (i == thisIdx) continue;
		
		m_MutexData->turn = i;
		while ((m_MutexData->turn == i) && (m_MutexData->flag[i])) { sched_yield(); }
	}		
}


void OpenOS::releaseMutex()
{
	Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	Thread *ThisThread = ProcessManager::getInstance()->getCurrentThread();

	ASAAC_PublicId process_id = 0;
	ASAAC_PublicId thread_id = OS_UNUSED_ID;
	
	if (ThisProcess != 0)
		if (ThisProcess->isInitialized())
			process_id = ThisProcess->getId();

	if (ThisThread != 0)
		if (ThisThread->isInitialized())
			thread_id = ThisThread->getId();

	unsigned long processIdx = 0;
	unsigned long threadIdx = 0;

	if (ThisProcess != 0)
	{
		processIdx = ProcessManager::getInstance()->getProcessIndex(ThisProcess->getId());
		
		if (ThisProcess->isInitialized())
			threadIdx = ThisProcess->getThreadIndex(thread_id);
	}

	unsigned long thisIdx = processIdx * (OS_MAX_NUMBER_OF_THREADS+1) + threadIdx;

	m_MutexData->flag[thisIdx] = false;
}



// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************


void OpenOS::FlushSessionHandler( CommandBuffer Buffer )
{
	volatile CommandData *d = (CommandData*)Buffer;
	
	try
	{
		OpenOS::getInstance()->flushLocalSession(d->NewSessionId);
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error flushing local session", LOCATION);
		e.logMessage();
		
		d->Return = ASAAC_ERROR; 
	}
	catch (...)
	{
		FatalException("Critical Error flushing local session", LOCATION).logMessage();
		
		d->Return = ASAAC_ERROR; 
	}
	
	d->Return = ASAAC_SUCCESS; 
	
	return;
}

