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
	m_CurrentCpuId = OS_UNUSED_ID;
	m_CurrentProcessId = OS_UNUSED_ID;
	
	m_CurrentProcessIndex = -1;
	
	m_IsMaster = false;
	m_IsInitialized = false;
		
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


void ProcessManager::initialize( bool IsServer, bool IsMaster, Allocator *ParentAllocator, ASAAC_PublicId CurrentCpuId, ASAAC_PublicId CurrentProcessId, MemoryLocation Location )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );

	try
	{
		m_IsInitialized = true;	

		m_CurrentCpuId = CurrentCpuId;
		
		m_IsServer = IsServer;
		m_IsMaster = IsMaster;

		// register handler to catch SIGCHLD and fetch exit result of child processes
		// to avoid zombies
		SignalManager::getInstance()->registerSignalHandler( SIGCHLD, SigChildCallback );

		if ( IsServer )
			OpenOS::getInstance()->registerCpu(CurrentCpuId);
		
		Allocator* UsedAllocator;
		if ( Location == SHARED ) //Applications with special rights (GSM, SM, ...)
		{
			m_SharedMemoryAllocator.initialize( 
				FileNameGenerator::getProcessManagerName( m_CurrentCpuId ),
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
			
		setCurrentProcess( CurrentProcessId );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing ProcessManager", LOCATION);
		
		throw;
	}

}


void ProcessManager::deinitialize()
{
	if ( m_IsInitialized == false )
		return;
		
	try
	{		
		SignalManager::getInstance()->unregisterSignalHandler( SIGCHLD );

		if ( m_IsServer )
		{
			OpenOS::getInstance()->unregisterCpu(m_CurrentCpuId);
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


Process* ProcessManager::allocateProcess( const ASAAC_ProcessDescription& Description, long &Index )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

	try
	{
		ProtectedScope Access( "Creating a process", m_Semaphore );

		bool IsMaster = true;
		 
		bool IsServer; 
		SimpleCommandInterface *CommandInterface;
		MemoryLocation Location;
		
		if ( Description.global_pid == OS_PROCESSID_MASTER )
		{
			IsServer = true;
			CommandInterface = &m_CommandInterface;
			Location = LOCAL;
			
			Index = getProcessIndex( Description.global_pid );

			if (Index == -1)	
				Index = getProcessIndex( OS_UNUSED_ID );
		}
		else
		{
			IsServer = false;
			CommandInterface = NULL;
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
		
		m_ProcessObject[Index].initialize( IsServer, IsMaster, Description, Location, CommandInterface );
			
		return &m_ProcessObject[Index];
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error while creating a process");
		
		throw;
	}
}


Process* ProcessManager::getProcess( ASAAC_PublicId ProcessId, const bool do_throw )
{
	if (m_IsInitialized == false) 
	{
		if (do_throw == true)
			throw UninitializedObjectException(LOCATION);
		else return NULL;
	}
	
	Process* Object = NULL;
	
	try
	{
		CharacterSequence ErrorString;

		if ((ProcessId < OS_PROCESSID_MIN) || (ProcessId > OS_PROCESSID_MAX))
			throw OSException( (ErrorString << "ProcessId is out of range (" << CharSeq(ProcessId) << ")").c_str(), LOCATION);

		signed long Index = getProcessIndex( ProcessId );
		
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
				Object->initialize( false, false, Description, SHARED, NULL );
			}	
		}
		else
		{
			Object->initialize( false, false, Description, SHARED, NULL );
		}
	}
	catch (ASAAC_Exception &e)
	{
		CharacterSequence ErrorString;
		ErrorString << "Error configuring process object (pid = " << CharSeq(ProcessId) << ")";
		e.addPath( ErrorString.c_str(), LOCATION);

		if (do_throw)
		{
			throw e;
		}
		else
		{
			e.raiseError();
			return NULL;
		}
	}
	
	return Object;
}


void ProcessManager::createProcess( const ASAAC_ProcessDescription& Description )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		if (m_IsServer)
		{
			if (Description.cpu_id == this->getCurrentCpuId())
			{
				long Index;
				Process* NewProcess = allocateProcess(Description, Index);
			
				if ( NewProcess == NULL ) 
					throw OSException("Process object has not been created", LOCATION);
		
				NewProcess->launch();
			}
			else //different CPU from current one
			{
				CommandData d;		
				d.Data.Description = Description;
				
				if (d.Data.Description.access_type == ASAAC_OLI_ACCESS)
					d.Data.Timeout = TimeStamp(d.Data.Description.timeout).asaac_Time();
				else d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();
				
				OpenOS::getInstance()->sendCommand( Description.cpu_id, CMD_CREATE_PROCESS, d.ReturnBuffer, d.Data.Timeout );
			}
		}
		else //not server
		{
			CommandData d;
	
			d.Data.Description = Description;
			
			if (d.Data.Description.access_type == ASAAC_OLI_ACCESS)
				d.Data.Timeout = TimeStamp(d.Data.Description.timeout).asaac_Time();
			else d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();
	
			sendCommand( CMD_CREATE_PROCESS, d.ReturnBuffer, d.Data.Timeout );
			
			Process* CreatedProcess = getProcess( Description.global_pid );
			
			if (CreatedProcess == NULL)
				throw OSException("Created process could not be located", LOCATION);
				
			CreatedProcess->refreshPosixId();
					
			if ( d.Return == ASAAC_TM_ERROR )
				throw OSException("process entity reported an error while creating a process", LOCATION);

			if ( d.Return == ASAAC_TM_TIMEOUT )
				throw TimeoutException("process entity reported an timeout event while creating a process", LOCATION);
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error creating a process", LOCATION);
		
		throw;
	}
}


void ProcessManager::destroyProcess( const ASAAC_PublicId& ProcessId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( ProcessId == OS_PROCESSID_MASTER )
		{
			OpenOS::getInstance()->destroyAllEntities();
		}
		else
		{
		
			CharacterSequence ErrorString;
		
			if ( m_IsServer )
			{		
				Process* P = getProcess( ProcessId );
				
				if ( P == NULL ) 
					throw OSException("Process not found", LOCATION);
				
				if (P->getProcessDescription().cpu_id == this->getCurrentCpuId())
				{
					P->destroy();
				}
				else //different CPU from current one
				{
					CommandData d;		
					d.Data.Description.global_pid = ProcessId;
					d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();
					
					OpenOS::getInstance()->sendCommand( P->getProcessDescription().cpu_id, CMD_DESTROY_PROCESS, d.ReturnBuffer, d.Data.Timeout );
				}
			}
			else //not server
			{
				CommandData d;
		
				d.Data.Description.global_pid = ProcessId;
				d.Data.Timeout = TimeStamp(OS_COMPLEX_COMMAND_TIMEOUT).asaac_Time();
		
				sendCommand( CMD_DESTROY_PROCESS, d.ReturnBuffer, d.Data.Timeout );
			}
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error destroying client process", LOCATION);
		
		throw;
	}
}


void ProcessManager::runProcess(const ASAAC_PublicId process_id)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	Process* TargetProcess = ProcessManager::getInstance()->getProcess( process_id );

	if ( TargetProcess == NULL ) 
		throw OSException("Process object could not be found", LOCATION);

	TargetProcess->run();
}


void ProcessManager::stopProcess(const ASAAC_PublicId process_id)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	Process* TargetProcess = ProcessManager::getInstance()->getProcess( process_id );

	if ( TargetProcess == NULL ) 
		throw OSException("Process object could not be found", LOCATION);

	return TargetProcess->stop();
}


void ProcessManager::releaseProcess( ASAAC_PublicId ProcessId )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

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
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
	{
		if ( m_ProcessObject[ Index ].isInitialized() ) 
			m_ProcessObject[ Index ].deinitialize();
	}
	
	m_CurrentProcessIndex = -1;
}
 
    
void ProcessManager::releaseAllClientProcesses()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
	{
		if ( (Index != m_CurrentProcessIndex) && (m_ProcessObject[ Index ].isInitialized()) ) 
			m_ProcessObject[ Index ].deinitialize();
	}
}
    

void ProcessManager::setCurrentProcess( ASAAC_PublicId ProcessId )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		Process *P = getCurrentProcess( false );
		
		if (P != NULL)
			P->setServer( false );
		
		m_CurrentProcessIndex = getProcessIndex( ProcessId );
		
		if ( m_CurrentProcessIndex == -1 )
		{
			ASAAC_ProcessDescription Description;
			Description.global_pid = ProcessId;
			P = allocateProcess(Description, m_CurrentProcessIndex);
		}
		else P = getProcess( ProcessId, m_CurrentProcessIndex );
	
		P->setServer( true );
		
		m_CurrentProcessId = ProcessId;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error setting the current process", LOCATION);
		
		throw;
	}
}


Process* ProcessManager::getCurrentProcess( const bool do_throw )
{
	if (m_IsInitialized == false)
	{
		if (do_throw == true)
			throw UninitializedObjectException(LOCATION);
		else return NULL;
	}
	
	if ( (m_CurrentProcessIndex < 0) || (m_CurrentProcessIndex >= OS_MAX_NUMBER_OF_PROCESSES) )	
	{
		if (do_throw == true)
			throw OSException("CurrentProcess object not available", LOCATION);
		else return NULL;
	}
		
	return (&m_ProcessObject[m_CurrentProcessIndex]);
}


void 	 ProcessManager::addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.addCommandHandler( CommandIdentifier, Handler );
}


void 	 ProcessManager::removeCommandHandler( unsigned long CommandIdentifier )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.removeCommandHandler( CommandIdentifier );
}


void 	 ProcessManager::removeAllCommandHandler()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.removeAllCommandHandler();
}


void ProcessManager::sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
}


void ProcessManager::sendCommandNonblocking( unsigned long CommandIdentifier, CommandBuffer Buffer )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.sendCommandNonblocking( CommandIdentifier, Buffer );
}


void ProcessManager::handleOneCommand( unsigned long& CommandIdentifier )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface.handleOneCommand( CommandIdentifier );
}


ASAAC_PublicId ProcessManager::getCurrentCpuId()
{
	return m_CurrentCpuId;
}


ASAAC_PublicId ProcessManager::getCurrentProcessId()
{
	return m_CurrentProcessId;
}


ProcessManager* ProcessManager::getInstance()
{
	static ProcessManager Instance;
	
	return &Instance;
}


void ProcessManager::destroyAllClientProcesses()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	if (m_IsServer)
	{
        for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_PROCESSES; Index ++ )
        {
            if (m_ProcessObject[ Index ].isInitialized())
            {
                if ( (m_ProcessObject[ Index ].getProcessDescription().cpu_id == this->getCurrentCpuId()) && 
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
                if ( (m_ProcessObject[ Index ].getProcessDescription().cpu_id == this->getCurrentCpuId()) && 
                     (m_ProcessObject[ Index ].getId() != getCurrentProcess()->getId()) ) 
                {   
                    m_ProcessObject[ Index ].destroy();
                    m_ProcessObject[ Index ].deinitialize();
                }
            }
        }
	}
}


void ProcessManager::destroyEntity()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	if (m_IsServer)
	{
		destroyAllClientProcesses();
		
		if ( getCurrentProcess()->isOSScope() == false)
			getCurrentProcess()->destroy();
	}
	else
	{
		CommandData d;
		
		sendCommandNonblocking( CMD_TERM_ENTITY, d.ReturnBuffer );
	}
}



// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************

void ProcessManager::CreateProcessHandler( CommandBuffer Buffer )
{
	CommandData *d = (CommandData*)Buffer;
	
	try
	{
		//recalculate timeout
		d->Data.Description.timeout = TimeStamp(d->Data.Timeout).asaac_Interval();
		
		ProcessManager::getInstance()->createProcess(d->Data.Description);
		
		d->Return = ASAAC_TM_SUCCESS; 	
	}
	catch ( ASAAC_Exception &e )
	{
		e.raiseError();
		
		d->Return = e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR; 		
	}
}


void ProcessManager::DestroyProcessHandler( CommandBuffer Buffer )
{
	CommandData *d = (CommandData*)Buffer;
	
	try
	{
		ProcessManager::getInstance()->destroyProcess(d->Data.Description.global_pid);
		
		d->Return = ASAAC_TM_SUCCESS;
	}
	catch ( ASAAC_Exception &e )
	{		
		d->Return = e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
		
		e.raiseError();
	}
}


void ProcessManager::DestroyEntityHandler( CommandBuffer Buffer )
{
	volatile CommandData *d = (CommandData*)Buffer;
	
	try
	{
		ProcessManager::getInstance()->destroyEntity();
		
		d->Return = ASAAC_TM_SUCCESS;
	}
	catch ( ASAAC_Exception &e )
	{		
		d->Return = e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;

		e.raiseError();
	}
}	
