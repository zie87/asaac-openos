#include "OpenOSObject.hh"

#include "Common/Templates/Shared.hh"
#include "Allocator/SharedMemory.hh"

#include "Managers/AllocatorManager.hh"
#include "Managers/FileManager.hh"
#include "Managers/FileNameGenerator.hh"
#include "Managers/EventManager.hh"
#include "Managers/SemaphoreManager.hh"
#include "Managers/SignalManager.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"
#include "Communication/CommunicationManager.hh"

#include "FaultManagement/FaultManager.hh"
#include "FaultManagement/LoggingManager.hh"
#include "FaultManagement/ErrorHandler.hh"


OpenOS::OpenOS()
{
	m_IsInitialized = false;
	m_IsMaster = true;
	m_ActivityState = LAS_UNDEFINED;
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


void OpenOS::initialize( bool IsMaster, LocalActivityState State )
{
    CharacterSequence CpuId = getenv(OS_ENV_ID_CPU);
    
    if ( CpuId.empty() )
        throw OSException("CpuId not in environment", LOCATION);
    
    CharacterSequence ProcessId = getenv(OS_ENV_ID_PROCESS);
    
    if ( ProcessId.empty() )
        throw OSException("ProcessId not in environment", LOCATION);
        
    initialize( IsMaster, State, CpuId.asaac_id(), ProcessId.asaac_id() );
}


void OpenOS::initialize( bool IsMaster, LocalActivityState State, ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);

    setenv(OS_ENV_ID_CPU, CharSeq(CpuId).c_str(), 1);
    setenv(OS_ENV_ID_PROCESS, CharSeq(ProcessId).c_str(), 1);

	m_IsMaster = IsMaster;
	m_ActivityState = State;

	initializeThisObject();	
	
	if ( IsMaster == true )
		flushSession();
	
	initializeGlobalObjects( CpuId, ProcessId );
}


void OpenOS::switchState( bool IsMaster, LocalActivityState State,  ASAAC_PublicId ProcessId )
{
	try
	{
	    if ((State != LAS_PROCESS_INIT) || (m_ActivityState != LAS_ENTITY))
	        throw OSException("Transition is only implemented from entity state to process_init state", LOCATION);
	
		m_IsMaster = IsMaster;
	
	    setenv(OS_ENV_ID_PROCESS, CharSeq(ProcessId).c_str(), 1);

		AllocatorManager::getInstance()->reallocateAllObjects( getSessionId() );

	    CommunicationManager::getInstance()->releaseAllVirtualChannels();
	    
	    ProcessManager::getInstance()->setCurrentProcess(ProcessId);
	    ProcessManager::getInstance()->releaseAllClientProcesses();    
	
	    m_ActivityState = LAS_PROCESS_INIT;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error switching to new state", LOCATION);
		
		throw;
	}
}


void OpenOS::initializeThisObject()
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);
		
	try
	{
		m_IsMaster = true;
		 
		try
		{
			//Searching for an already created OpenOS object...
			m_Allocator.initialize(FileNameGenerator::getOpenOSName(), false, predictSize(), false, false);
			
			//If memory found, this is an advise of existing other objects
			//Otherwise m_Allocator.initialize would throw an exception
			m_IsMaster = false;			
		}
		catch ( ASAAC_Exception &e)
		{
			//Do nothing with the exception here, because this is not a real error
			m_Allocator.initialize(FileNameGenerator::getOpenOSName(), true, predictSize(), true, false);			
		}
		
		m_CpuId.initialize(&m_Allocator, OS_MAX_NUMBER_OF_CPU);
		
		m_MutexData.initialize(&m_Allocator);
		
		//initialize random generator
		unsigned long seed = (unsigned long)(TimeStamp::Now().nsec());
		srand(seed);

		m_IsInitialized = true;
		
		//After initialization shared objects must be initialized, if this process is master
		if ( m_IsMaster  == true )
			flushSession();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Initializing of OpenOS object failed", LOCATION);
		
		m_IsInitialized = true;
		
		deinitialize();
		
		throw;		
	}
}


void OpenOS::deinitializeThisObject()
{
	if (m_IsInitialized == false)
		return;

	try
	{
		m_CpuAllocator.deinitialize();
	
		m_MutexData.deinitialize();
		m_CpuId.deinitialize();
	
		m_Allocator.deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing OpenOS object", LOCATION);
		e.raiseError();
	}

	m_IsInitialized = false;	
}


void OpenOS::initializeGlobalObjects( ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId )
{
    //Initialize SemaphoreManager and EventManager
    SignalManager::getInstance()->initialize();
    SemaphoreManager::getInstance()->initialize();
    EventManager::getInstance()->initialize();

	//Initialize ProcessManager with dedicated rights
	ProcessManager *PM = ProcessManager::getInstance();
	switch ( m_ActivityState )
	{
		case LAS_UNDEFINED:			break;
		case LAS_ENTITY:			PM->initialize( true, m_IsMaster, &m_Allocator, CpuId, OS_PROCESSID_MASTER, SHARED);
								    PM->addCommandHandler(CMD_FLUSH_SESSION, OpenOS::FlushSessionHandler);
									break;
		case LAS_PROCESS_INIT:
		case LAS_PROCESS_RUNNING:	PM->initialize( false, false, &m_Allocator, CpuId, ProcessId, SHARED );
									break;
		case LAS_REMOTE:			break;
	}

	//Initialize ThreadManager
	ThreadManager::getInstance()->initialize();

	//Initialize CommunicatinManager
	CommunicationManager::getInstance()->initialize( m_IsMaster, &m_Allocator );

    //Initialize FaultManager and Logging Manager
    FaultManager::getInstance()->initialize( m_IsMaster );
    LoggingManager::getInstance()->initialize( m_IsMaster );
    
    //Initialize ErrorHandler
    ErrorHandler::getInstance()->initialize();
}


void OpenOS::deinitializeGlobalObjects()
{
	ErrorHandler::getInstance()->deinitialize();    
	LoggingManager::getInstance()->deinitialize();
	FaultManager::getInstance()->deinitialize();

	CommunicationManager::getInstance()->deinitialize();
	ThreadManager::getInstance()->deinitialize();

	ProcessManager::getInstance()->deinitialize();

	EventManager::getInstance()->deinitialize();
    SemaphoreManager::getInstance()->deinitialize();
    SignalManager::getInstance()->deinitialize();
}


void OpenOS::deinitialize()
{
	if (m_IsInitialized == false)
		return;

	try
	{
		deinitializeGlobalObjects();

		deinitializeThisObject();

		FileManager::getInstance()->closeAllFiles();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing OpenOS", LOCATION);
		e.raiseError();
	}
}


bool OpenOS::isInitialized()
{
	return m_IsInitialized;
}


void OpenOS::destroyAllEntities()
{
	try
	{
		for (long Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
		{
			if (m_CpuId[Index] == OS_UNUSED_ID)
				continue;
	
			if (m_CpuId[Index] == ProcessManager::getInstance()->getCurrentCpuId())
				continue;
			
			CommandData d;
			sendCommand( m_CpuId[Index], CMD_TERM_ENTITY, d.ReturnBuffer, TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time(), false );		
				
			if (d.Return == ASAAC_ERROR)
				throw OSException("Error returned by entity", LOCATION);
		}	

		ProcessManager::getInstance()->destroyEntity();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error destroying complete OpenOS", LOCATION);

		throw;
	}
}


void OpenOS::sendCommand( ASAAC_PublicId CpuId, unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
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
				
		CommandInterface.initialize( &m_CpuAllocator, false );
		
		CommandInterface.sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
	}
	catch (ASAAC_Exception &e)
	{
		CommandInterface.deinitialize();
		
		e.addPath(ErrorMessage.c_str());
		
		throw;
	}

	CommandInterface.deinitialize();
}


SessionId OpenOS::getSessionId()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
		
	return m_Allocator.getSessionId();
}


LocalActivityState OpenOS::getActivityState()
{
	return m_ActivityState;
}


long OpenOS::getCpuIndex( ASAAC_PublicId CpuId )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index ++ )
	{
		if ( m_CpuId[ Index ] == CpuId )
			return Index;
	}
			 
	return -1;	
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
		OSException("Cpu is not registered", LOCATION).raiseError();
		return ASAAC_ERROR;
	}
		
	m_CpuId[Index] = OS_UNUSED_ID;
		
	return ASAAC_SUCCESS;
}


void OpenOS::flushSession()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( m_IsMaster )
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
		 
		bool EntityFound = false; 
		 
		for (long Index = 0; Index < OS_MAX_NUMBER_OF_CPU; Index++)
		{
			if (m_CpuId[Index] == OS_UNUSED_ID)
				continue;
			
			CommandData d;		
			
			//First we just send a simple command to determine the processes liveliness and its PosixId
			try
			{
				sendCommand(m_CpuId[Index], CMD_GET_PID, d.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time(), false);
			}
			catch ( ASAAC_Exception &e )
			{
				m_CpuId[Index] = OS_UNUSED_ID;
				continue;
			}
	
			//If succeeded, we send our real approach
			d.NewSessionId = NewSessionId;
			sendCommand(m_CpuId[Index], CMD_FLUSH_SESSION, d.ReturnBuffer, TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time(), false);
				
			if (d.Return == ASAAC_ERROR)
			{
				m_CpuId[Index] = OS_UNUSED_ID;
			}
			else EntityFound = true;
		}
		
		// The found SharedMemory was an undeleted one from a former session
		if (( m_IsMaster == false ) && ( EntityFound == false ))
			m_IsMaster = true;
		
		initializeMutex();
		
		flushLocalSession( NewSessionId );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error flushing the session", LOCATION);
		
		throw;
	}
}


void OpenOS::flushLocalSession( SessionId NewSessionId )
{
	if ( ProcessManager::getInstance()->isInitialized() )
		ProcessManager::getInstance()->destroyAllClientProcesses();
		
	if ( CommunicationManager::getInstance()->isInitialized() )
		CommunicationManager::getInstance()->destroyAllVirtualChannels();
	
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
	Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess(false);
	Thread *ThisThread = ThreadManager::getInstance()->getCurrentThread(false);

	ASAAC_PublicId process_id = 0;
	ASAAC_PublicId thread_id = OS_UNUSED_ID;
	
	if (ThisProcess != NULL)
		if (ThisProcess->isInitialized())
			process_id = ThisProcess->getId();

	if (ThisThread != NULL)
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
	Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess(false);
	Thread *ThisThread = ThreadManager::getInstance()->getCurrentThread(false);

	ASAAC_PublicId process_id = 0;
	ASAAC_PublicId thread_id = OS_UNUSED_ID;
	
	if (ThisProcess != NULL)
		if (ThisProcess->isInitialized())
			process_id = ThisProcess->getId();

	if (ThisThread != NULL)
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
		e.raiseError();
		
		d->Return = ASAAC_ERROR; 
	}
	
	d->Return = ASAAC_SUCCESS; 
	
	return;
}

