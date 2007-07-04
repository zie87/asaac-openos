#include "ProcessManager.hh"

#include "OpenOSObject.hh"

#include "AbstractInterfaces/Callback.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/SharedMemory.hh"

#include "IPC/ProtectedScope.hh"

#include "Managers/FileNameGenerator.hh"
#include "Managers/SignalManager.hh"

#include "FaultManagement/ErrorHandler.hh"
#include "Exceptions/Exceptions.hh"

class ProcessSigChildCallback : public Callback {
	
	public:
	
		virtual void call ( void* Data )
		{
			int Dummy;
			
			pid_t* PID = (pid_t*)Data;
			
			cout << "PID = " << *PID << endl;
			
			oal_waitpid( (pid_t)-1, &Dummy, 0 );
		}
		
		virtual ~ProcessSigChildCallback() { };
};


static ProcessSigChildCallback SigChildCallback;

ProcessManager::ProcessManager()
{
	m_CpuId = OS_UNUSED_ID;
	m_CurrentProcessIndex = -1;
	m_IsMaster = false;
	m_IsInitialized = false;
	
	m_BufferedEntryPointCounter = 0;
		
	// register handler to catch SIGCHLD and fetch exit result of child processes
	// to avoid zombies
	SignalManager::getInstance()->registerSignalHandler( SIGCHLD, SigChildCallback );
	
}


size_t	ProcessManager::predictSize()
{
	size_t CumulativeSize = 0;
	
	// m_Semaphore
	CumulativeSize +=  Semaphore::predictSize();
	
	// m_ProcessId
	CumulativeSize +=  Shared<ASAAC_PublicId>::predictSize(OS_MAX_NUMBER_OF_PROCESSES);
	
	return CumulativeSize;
}


size_t	ProcessManager::predictInternalSize()
{
	size_t CumulativeSize = 0;
	
	// m_CommandInterface
	CumulativeSize += SimpleCommandInterface::predictSize();
	
	return CumulativeSize;
}


void ProcessManager::initialize( bool IsServer, bool IsMaster, Allocator *ParentAllocator, ASAAC_PublicId CpuId, MemoryLocation Location )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );

	if ( IsServer )
	{
		OpenOS::getInstance()->registerCpu(CpuId);
	}
	
	m_CpuId = CpuId;			
	
	m_IsServer = IsServer;
	m_IsMaster = IsMaster;
	
	Allocator* UsedAllocator;

	if ( Location == SHARED ) //Applications with special rights (GSM, SM, ...)
	{
		m_SharedMemoryAllocator.initialize( 
			FileNameGenerator::getProcessManagerName( m_CpuId ),
			IsServer,	// IsServer is correct here!
			predictInternalSize() );
									
		UsedAllocator = &m_SharedMemoryAllocator;
	}
	else //Normal Applications
	{
		m_LocalMemoryAllocator.initialize( predictSize() );

		UsedAllocator = &m_LocalMemoryAllocator;
	}
	
	//CommandInterface has to be the first object initialized here,
	//because memory is used in OpenOS object too
	m_CommandInterface.initialize( UsedAllocator, IsServer );
	
	//Semaphore is an global OpenOS object, therefore it has to allocate memory from parent allocator
	m_Semaphore.initialize( ParentAllocator, IsMaster );
	m_ProcessId.initialize( ParentAllocator, OS_MAX_NUMBER_OF_PROCESSES );
	
	if ( IsMaster )
	{
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
			m_ProcessId[ Index ] = OS_UNUSED_ID;
	}	

	if ( IsServer )
	{
		m_CommandInterface.addCommandHandler( CMD_CREATE_PROCESS,  ProcessManager::CreateProcessHandler );
		m_CommandInterface.addCommandHandler( CMD_DESTROY_PROCESS, ProcessManager::DestroyProcessHandler );
		m_CommandInterface.addCommandHandler( CMD_TERM_ENTITY, 	   ProcessManager::DestroyEntityHandler );		
	}		
	
	m_IsInitialized = true;	
}


void ProcessManager::deinitialize()
{
	if ( m_IsInitialized == false )
		return;
		
	try
	{		
		if ( m_IsServer )
		{
			OpenOS::getInstance()->unregisterCpu(m_CpuId);
		}

		releaseAllProcesses();					

		m_Semaphore.deinitialize();
		m_CommandInterface.deinitialize();
		
		m_LocalMemoryAllocator.deinitialize();
		m_SharedMemoryAllocator.deinitialize();
	}
	catch (ASAAC_Exception &e)
	{		
		e.addPath("Error deinitializing ProcessManager");
		e.raiseError();
	}
	
	m_IsInitialized = false;
}


bool ProcessManager::isInitialized()
{
	return m_IsInitialized;
}


ProcessManager::~ProcessManager()
{

}


void ProcessManager::handleBufferMemory()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	for (unsigned long Index = 0; Index < m_BufferedEntryPointCounter; Index++)
	{
		getCurrentProcess()->addEntryPoint(
			m_BufferedEntryPoints[Index].Name,
			m_BufferedEntryPoints[Index].Address);
	}
			
	m_BufferedEntryPointCounter = 0;
}		


ASAAC_ReturnStatus ProcessManager::addEntryPoint( ASAAC_CharacterSequence Name, EntryPointAddr Address )
{
	if (m_IsInitialized == false)
	{
		if (m_BufferedEntryPointCounter == OS_MAX_NUMBER_OF_ENTRYPOINTS)
			return ASAAC_ERROR;
		
		m_BufferedEntryPoints[m_BufferedEntryPointCounter].Name = Name;
		m_BufferedEntryPoints[m_BufferedEntryPointCounter].Address = Address;
		m_BufferedEntryPointCounter++;

		return ASAAC_SUCCESS;
	}
	else
	{
		return getCurrentProcess()->addEntryPoint(Name, Address);
	}
}


long ProcessManager::getProcessIndex( ASAAC_PublicId ProcessId )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
	
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
	{
		if ( m_ProcessId[ Index ] == ProcessId )
			return Index;
	}
			 
	return -1;	
}


Process* ProcessManager::createProcess( bool IsMasterProcess, const ASAAC_ProcessDescription& Description, long &Index )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

	try
	{
		ProtectedScope Access( "Creating a process", m_Semaphore );

		bool IsMaster = true;
		 
		bool IsServer; 
		bool UseInternalCommandInterface;
		MemoryLocation Location;
		
		if (IsMasterProcess)
		{
			IsServer = true;
			UseInternalCommandInterface = false;
			Location = LOCAL;
			
			Index = getProcessIndex( Description.global_pid );

			if (Index == -1)	
				Index = getProcessIndex( OS_UNUSED_ID );
		}
		else
		{
			IsServer = false;
			UseInternalCommandInterface = true;
			Location = SHARED;
					
			Index = getProcessIndex( Description.global_pid );
			
			if (Index != -1)
				throw OSException( (ErrorString << "Process is already created with dedicated id (" << CharSeq(Description.global_pid) << ")").c_str(), LOCATION);
	
			Index = getProcessIndex( OS_UNUSED_ID );
		}
		
		if ( Index == -1 ) 
			throw OSException("Maximum number of processes reached. No more free slots.", LOCATION);
			
		//Maybe an old object is still initialized	
		if ( m_ProcessObject[Index].isInitialized() )
			m_ProcessObject[Index].deinitialize();

		m_ProcessId[Index] = Description.global_pid;
		
		m_ProcessObject[Index].initialize( IsServer, IsMaster, UseInternalCommandInterface, Description, Location );
			
		return &m_ProcessObject[Index];
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error while creating a process");
		throw;
	}
}


Process* ProcessManager::getProcess( ASAAC_PublicId ProcessId, long &Index )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
	
	Process* Object = NULL;
	
	try
	{
		CharacterSequence ErrorString;

		if ((ProcessId < OS_PROCESSID_MIN) || (ProcessId > OS_PROCESSID_MAX))
			throw OSException( (ErrorString << "ProcessId is out of range (" << CharSeq(ProcessId) << ")").c_str(), LOCATION);

		Index = getProcessIndex( ProcessId );
		
		if ( Index == -1 ) 
			throw OSException("Process not found", LOCATION);
	
		Object = &m_ProcessObject[Index];
		
		ASAAC_ProcessDescription Description;
		Description.global_pid = ProcessId;
		
		if (Object->isInitialized())
		{
			if (Object->getId() != ProcessId)
			{
				Object->deinitialize();
				Object->initialize( false, false, true, Description, SHARED );
			}	
		}
		else
		{
			Object->initialize( false, false, true, Description, SHARED );
		}
	}
	catch (ASAAC_Exception &e)
	{
		CharacterSequence ErrorString;
		ErrorString << "Error configuring process object (pid = " << CharSeq(ProcessId) << ")";
		e.addPath( ErrorString.c_str(), LOCATION);
		e.raiseError();
		
		Index = -1;
		return NULL;
	}
	
	return Object;
}


Process* ProcessManager::getProcess( ASAAC_PublicId ProcessId )
{
	long dummy;
	return getProcess( ProcessId, dummy );
}


void ProcessManager::initializeEntityProcess(  bool IsMaster, Allocator *ParentAllocator, ASAAC_PublicId CpuId)
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );

	ASAAC_ProcessDescription Description;
	Description.global_pid = OS_PROCESSID_MASTER;
	
	try 
	{
		initialize( true, IsMaster, ParentAllocator, CpuId, SHARED );		

		Process* NewProcess = createProcess(true, Description, m_CurrentProcessIndex);
		
		if (NewProcess == NULL)
			throw OSException("Unable to create master process", LOCATION);			 

		m_IsInitialized = true;
		
		handleBufferMemory();
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error initializing master process", LOCATION);

		throw;
	}
}


void ProcessManager::initializeClientProcess( Allocator *ParentAllocator, ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId, MemoryLocation Location )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );
	
	try 
	{
		initialize( false, false, ParentAllocator, CpuId, Location );
		
		setCurrentProcess( ProcessId );
		
		handleBufferMemory();
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error initializing client process", LOCATION);
		
		throw;
	}
}


ASAAC_TimedReturnStatus ProcessManager::createClientProcess( const ASAAC_ProcessDescription& Description )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	if (m_IsServer)
	{
		if (Description.cpu_id == this->getCpuId())
		{
			try 
			{
				long Index;
				Process* NewProcess = createProcess(false, Description, Index);
			
				if ( NewProcess == NULL ) 
					throw OSException("Process object has not been created", LOCATION);
		
				NewProcess->launch();
				
				if ( NewProcess->refreshPosixPid() == ASAAC_ERROR ) 
					throw TimeoutException("Timeout requesting pid of client process", LOCATION);
			}
			catch ( ASAAC_Exception& e )
			{
				e.addPath("Error creating a process", LOCATION);
				e.raiseError();
				
				if ( e.isTimeout() )		
					return ASAAC_TM_TIMEOUT;
				else return ASAAC_TM_ERROR;
			}
		}
		else //different CPU from current one
		{
			ASAAC_TimedReturnStatus status = ASAAC_TM_ERROR;
			
			try
			{
				CommandData d;		
				d.Data.Description = Description;
				
				if (d.Data.Description.access_type == ASAAC_OLI_ACCESS)
					d.Data.Timeout = TimeStamp(d.Data.Description.timeout).asaac_Time();
				else d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();
				
				status = OpenOS::getInstance()->sendCommand(
					Description.cpu_id, 
					CMD_CREATE_PROCESS, 
					d.ReturnBuffer, 
					d.Data.Timeout );
			}
			catch ( ASAAC_Exception &e )
			{			
				CharacterSequence cs;
				
				cs << "createClientProcess(): ProcessManager for Description.cpu_id = " << Description.cpu_id << " was not found."; 
				e.addPath(cs.c_str());
					
				throw;
			}
			
			return status;			
		}
			
		return ASAAC_TM_SUCCESS;
	}
	else //not server
	{
		CommandData d;

		d.Data.Description = Description;
		
		if (d.Data.Description.access_type == ASAAC_OLI_ACCESS)
			d.Data.Timeout = TimeStamp(d.Data.Description.timeout).asaac_Time();
		else d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();

		ASAAC_TimedReturnStatus Result;
		Result = sendCommand( 
			CMD_CREATE_PROCESS, 
			d.ReturnBuffer, 
			d.Data.Timeout );
		
		if (Result != ASAAC_TM_SUCCESS)
			return Result;	

		if (d.Return != ASAAC_TM_SUCCESS)
			return d.Return;

		Process* CreatedProcess = getProcess( Description.global_pid );
		
		if (CreatedProcess == NULL)
			return ASAAC_TM_ERROR;
			
		if ( CreatedProcess->refreshPosixPid() == ASAAC_ERROR ) 
			return ASAAC_TM_TIMEOUT;
				
		return d.Return;
	}
}


ASAAC_ReturnStatus ProcessManager::destroyClientProcess( const ASAAC_PublicId& ProcessId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	if ( ProcessId == OS_PROCESSID_MASTER )
		return OpenOS::getInstance()->destroyAllEntities();

	CharacterSequence ErrorString;
	ASAAC_TimedReturnStatus Status = ASAAC_TM_ERROR;

	if ( m_IsServer )
	{		
		try
		{
			Process* P = getProcess( ProcessId );
			
			if ( P == NULL ) 
				throw OSException("Process not found", LOCATION);
			
			if (P->getProcessDescription().cpu_id == this->getCpuId())
			{
				P->destroy();
				
				Status = ASAAC_TM_SUCCESS;
			}
			else //different CPU from current one
			{
				
				CommandData d;		
				d.Data.Description.global_pid = ProcessId;
				d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();
				
				Status = OpenOS::getInstance()->sendCommand(
					P->getProcessDescription().cpu_id, 
					CMD_DESTROY_PROCESS, 
					d.ReturnBuffer, 
					d.Data.Timeout );
			}
		}
		catch (ASAAC_Exception &e)
		{
			e.addPath( (ErrorString << "Error destroying a process (pid = " << CharSeq(ProcessId) << ")").c_str(), LOCATION);
			e.raiseError();
			
			Status = ASAAC_TM_ERROR;
		}
	}
	else //not server
	{
		CommandData d;

		d.Data.Description.global_pid = ProcessId;
		d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();

		Status = sendCommand( 
			CMD_DESTROY_PROCESS, 
			d.ReturnBuffer, 
			d.Data.Timeout );
	}
	
	if (Status != ASAAC_TM_SUCCESS)
		return ASAAC_ERROR;	
			
	return ASAAC_SUCCESS;	
}


ASAAC_ReturnStatus ProcessManager::runProcess(const ASAAC_PublicId process_id)
{
	Process* TargetProcess = ProcessManager::getInstance()->getProcess( process_id );

	if ( TargetProcess == NULL ) 
		return ASAAC_ERROR;

	return  TargetProcess->run();
}


ASAAC_ReturnStatus ProcessManager::stopProcess(const ASAAC_PublicId process_id)
{
	Process* TargetProcess = ProcessManager::getInstance()->getProcess( process_id );

	if ( TargetProcess == NULL ) 
		return ASAAC_ERROR;

	return TargetProcess->stop();
}


void ProcessManager::releaseProcess( ASAAC_PublicId ProcessId )
{
    long Index = getProcessIndex( ProcessId );
    
    if (Index != -1)
    {
    	m_ProcessObject[Index].deinitialize();
	}
	
	if ( Index == m_CurrentProcessIndex )				
		m_CurrentProcessIndex = -1;
}
 
    
void ProcessManager::releaseAllProcesses()
{
	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
	{
		if ( m_ProcessObject[ Index ].isInitialized() ) 
			m_ProcessObject[ Index ].deinitialize();
	}
	
	m_CurrentProcessIndex = -1;
}
 
    
void ProcessManager::releaseAllClientProcesses()
{
	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
	{
		if ( (Index != m_CurrentProcessIndex) && (m_ProcessObject[ Index ].isInitialized()) ) 
			m_ProcessObject[ Index ].deinitialize();
	}
}
    

void ProcessManager::setCurrentProcess( ASAAC_PublicId ProcessId )
{
	Process *P = getProcess( ProcessId, m_CurrentProcessIndex);

	P->setServer( true );
}


Process* ProcessManager::getCurrentProcess()
{
	if (m_IsInitialized == false) 
		return NULL;
	
	if ( (m_CurrentProcessIndex < 0) || (m_CurrentProcessIndex >= OS_MAX_NUMBER_OF_PROCESSES) )	
		return NULL;
		
	return (&m_ProcessObject[m_CurrentProcessIndex]);
}


Thread* ProcessManager::getCurrentThread()
{
	if (m_IsInitialized == false) 
		return NULL;

	Process *P = getCurrentProcess();
	
	if (P != NULL)
	{
		if (P->isInitialized())
			return getCurrentProcess()->getCurrentThread();
		else return NULL;
	}
	else return NULL;
}


ASAAC_ReturnStatus 	 ProcessManager::addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_CommandInterface.addCommandHandler( CommandIdentifier, Handler );
}


ASAAC_ReturnStatus 	 ProcessManager::removeCommandHandler( unsigned long CommandIdentifier )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_CommandInterface.removeCommandHandler( CommandIdentifier );
}


ASAAC_ReturnStatus 	 ProcessManager::removeAllCommandHandler()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_CommandInterface.removeAllCommandHandler();
}


ASAAC_TimedReturnStatus ProcessManager::sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_CommandInterface.sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
}


void ProcessManager::sendCommandNonblocking( unsigned long CommandIdentifier, CommandBuffer Buffer )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.sendCommandNonblocking( CommandIdentifier, Buffer );
}


ASAAC_ReturnStatus ProcessManager::handleOneCommand( unsigned long& CommandIdentifier )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_CommandInterface.handleOneCommand( CommandIdentifier );
}


ASAAC_PublicId ProcessManager::getCpuId()
{
	return m_CpuId;
}


ASAAC_PublicId ProcessManager::getProcessId( ProcessAlias Alias )
{
	switch (Alias)
	{
		case PROC_APOS: for (unsigned long id = OS_PROCESSID_APOS; id <= OS_PROCESSID_APOS_MAX; id++) 
						{
							if (getProcessIndex(id) == -1)
								return id;
						} 
						break;
						
		case PROC_SMOS: for (unsigned long id = OS_PROCESSID_SMOS; id <= OS_PROCESSID_SMOS_MAX; id++) 
						{
							if (getProcessIndex(id) == -1)
								return id;
						} 
						break;
						
		case PROC_GSM:  return OS_PROCESSID_GSM; 
						break;
						
		case PROC_PCS:  return OS_PROCESSID_PCS; 
						break;
						
		case PROC_OLI:  return OS_PROCESSID_OLI; 
						break;
						
		case PROC_SM:   return OS_PROCESSID_SM; 
						break;
						
		default: return OS_UNUSED_ID;
	}

	return OS_UNUSED_ID;
}


ProcessManager* ProcessManager::getInstance()
{
	static ProcessManager Instance;
	
	return &Instance;
}


ASAAC_ReturnStatus ProcessManager::destroyAllClientProcesses()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	if (m_IsServer)
	{
        for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
        {
            if (m_ProcessObject[ Index ].isInitialized())
            {
                if ( (m_ProcessObject[ Index ].getProcessDescription().cpu_id == this->getCpuId()) && 
                     (m_ProcessObject[ Index ].getId() != getCurrentProcess()->getId()) &&
                     (m_ProcessObject[ Index ].getAlias() == PROC_APOS ) ) 
                {   
                    m_ProcessObject[ Index ].destroy();
                    m_ProcessObject[ Index ].deinitialize();
                }
            }
        }

        for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
        {
            if (m_ProcessObject[ Index ].isInitialized())
            {
                if ( (m_ProcessObject[ Index ].getProcessDescription().cpu_id == this->getCpuId()) && 
                     (m_ProcessObject[ Index ].getId() != getCurrentProcess()->getId()) ) 
                {   
                    m_ProcessObject[ Index ].destroy();
                    m_ProcessObject[ Index ].deinitialize();
                }
            }
        }
		
		return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;	
}


ASAAC_ReturnStatus ProcessManager::destroyEntity()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	if (m_IsServer)
	{
		destroyAllClientProcesses();
		
		if ( getCurrentProcess()->isOSScope() == false)
			getCurrentProcess()->destroy();
		
		return ASAAC_SUCCESS;
	}
	else
	{
		CommandData d;
		
		sendCommandNonblocking( CMD_TERM_ENTITY, d.ReturnBuffer );
		
		return ASAAC_SUCCESS;
	}
}



// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************

void ProcessManager::CreateProcessHandler( CommandBuffer Buffer )
{
	CommandData *d = (CommandData*)Buffer;
	
	//recalculate timeout
	d->Data.Description.timeout = TimeStamp(d->Data.Timeout).asaac_Interval();
	
	d->Return = ProcessManager::getInstance()->createClientProcess(d->Data.Description);
	
	return;
}


void ProcessManager::DestroyProcessHandler( CommandBuffer Buffer )
{
	CommandData *d = (CommandData*)Buffer;
	
	d->Return = (ProcessManager::getInstance()->destroyClientProcess(d->Data.Description.global_pid) == ASAAC_SUCCESS)?ASAAC_TM_SUCCESS:ASAAC_TM_ERROR;
	
	return;
}


void ProcessManager::DestroyEntityHandler( CommandBuffer Buffer )
{
	volatile CommandData *d = (CommandData*)Buffer;
	
	d->Return = (ProcessManager::getInstance()->destroyEntity() == ASAAC_SUCCESS)? ASAAC_TM_SUCCESS: ASAAC_TM_ERROR;
	
	return;
}	
