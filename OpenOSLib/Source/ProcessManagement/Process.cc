#include "Process.hh"

#include "OpenOS.hh"

#include "Allocator/SharedMemory.hh"
#include "Allocator/LocalMemory.hh"

#include "IPC/ProtectedScope.hh"

#include "Exceptions/Exceptions.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Managers/SignalManager.hh"
#include "Managers/TimeManager.hh"

#include "Managers/FileManager.hh"
#include "Managers/FileNameGenerator.hh"
#include "Managers/TimeManager.hh"

#include "FaultManagement/ErrorHandler.hh"

#include "Communication/CommunicationManager.hh"
#include "Communication/LocalVc.hh"

#include "LogicalInterfaces/asaac_oli.hh"

using namespace std;


const EntryPoint EmptyEntryPoint = { { 0, "" }, 0 };



Process::Process() :  m_IsInitialized( false ),  m_IsMaster(false), m_IsServer(false)
{
	m_PosixPid = 0;
	m_UseInternalCommandInterface = true;
	m_ActiveMainLoop = false;
}


Process::~Process()
{
}


void Process::initialize( bool IsServer, bool IsMaster, bool UseInternalCommandInterface, const ASAAC_ProcessDescription& Description, MemoryLocation Location )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );

	m_IsMaster = IsMaster;
	m_IsServer = IsServer;
	m_UseInternalCommandInterface = UseInternalCommandInterface;

	if ( Location == SHARED )
	{
		m_SharedAllocator.initialize( 
			FileNameGenerator::getProcessName( Description.global_pid ),
			IsMaster,
			predictSize() );
									
		m_Allocator = &m_SharedAllocator;
	}
	else
	{
		m_LocalAllocator.initialize( predictSize() );

		m_Allocator = &m_LocalAllocator;
	}
	
	m_ProcessData.initialize( m_Allocator );

	if ( IsMaster )
	{
		m_ProcessData->Status = PROCESS_DORMANT;
	}

	m_LocalVCs.initialize( m_Allocator, OS_MAX_NUMBER_OF_LOCALVCS );
		
	m_Semaphore.initialize( m_Allocator, IsMaster );
	
	m_InternalCommandInterface.initialize( m_Allocator, IsMaster );
	
	m_EntryPoints.initialize( m_Allocator, OS_MAX_NUMBER_OF_ENTRYPOINTS );
	
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		m_Threads[ Index ].initialize( m_Allocator, IsMaster, this );
	}
	
	if ( IsMaster )
	{
		m_ProcessData->Description        = Description;
		m_ProcessData->Status             = PROCESS_INITIALIZED;
		m_ProcessData->AuthenticationCode = lrand48();

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index ++ )
		{
			m_LocalVCs[ Index ].GlobalVcId   = OS_UNUSED_ID;
			m_LocalVCs[ Index ].LocalVcId    = OS_UNUSED_ID;
		}

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_ENTRYPOINTS; Index ++ )
		{
			m_EntryPoints[ Index ] = EmptyEntryPoint;
		}
	}
	
	setServer( IsServer );
	
	m_IsInitialized = true;
}



bool Process::isInitialized()
{
	return m_IsInitialized;
}


void Process::setServer( bool IsServer )
{
	if ( IsServer )
	{
		addCommandHandler( CMD_RUN_PROCESS,  Process::RunHandler  );
		addCommandHandler( CMD_GET_PID, Process::getPIDHandler );		
		addCommandHandler( CMD_STOP_PROCESS, Process::StopHandler );
		addCommandHandler( CMD_TERM_PROCESS, Process::DestroyHandler );
		addCommandHandler( CMD_ATTACH_VC, Process::attachLocalVcHandler );
		addCommandHandler( CMD_DETACH_VC, Process::detachLocalVcHandler );
		addCommandHandler( CMD_INVOKE_OS_SCOPE, Process::invokeOSScopeHandler );

        m_ProcessData->OSScopeSchedulingData.Policy = SCHED_RR;
        m_ProcessData->OSScopeSchedulingData.Parameter.__sched_priority = sched_get_priority_min(SCHED_RR) + 1;
        //pthread_setschedparam( pthread_self(), m_ProcessData->OSScopeSchedulingData.Policy, &m_ProcessData->OSScopeSchedulingData.Parameter );
	}
	else
	{
		removeAllCommandHandler();
	}
	
	m_IsServer = IsServer;
}


bool Process::isServer()
{
	return m_IsServer;
}


void Process::launch()
{
	CharacterSequence ErrorString;
	ASAAC_TimedReturnStatus Result = ASAAC_TM_SUCCESS;
	
	try
	{
		ASAAC_CharacterSequence ProcessPath;
		if (m_ProcessData->Description.access_type == ASAAC_OLI_ACCESS)
		{
			ProcessPath = FileNameGenerator::getLocalOliPath(m_ProcessData->Description.global_pid);
	
			ASAAC::OLI::Client OliClient;
			OliClient.setRequestVc( m_ProcessData->Description.access_info._u.oli_channel.vc_sending );
			OliClient.setReplyVc( m_ProcessData->Description.access_info._u.oli_channel.vc_receiving );
			OliClient.setSize( m_ProcessData->Description.access_info._u.oli_channel.fragment_size );
			OliClient.setTimeOut( m_ProcessData->Description.timeout );
			
			//OLI service successful?
			Result = OliClient.storeCompleteFile(ProcessPath, m_ProcessData->Description.programme_file_name);
			
			if (Result == ASAAC_TM_ERROR)
				throw OSException("Error executing OLI protocol", LOCATION);
	
			if (Result == ASAAC_TM_TIMEOUT)
				throw TimeoutException("Timeout executing OLI protocol", LOCATION);
		}
		else ProcessPath = FileNameGenerator::getAsaacPath(m_ProcessData->Description.programme_file_name);
		
		m_PosixPid = fork();

        if (m_PosixPid == -1)
            throw OSException( strerror(errno), LOCATION );                
        
        //Starter process
        if (m_PosixPid == 0)
        {
            try
            {            	
                OpenOS::getInstance()->switchState( false, LAS_PROCESS_INIT, getId() );

				// Enter main cycle of ProcessStarter
				unsigned long CommandId;
				for(;;)
				{					
					handleOneCommand( CommandId );
	
					if ( CommandId == CMD_TERM_PROCESS )
						throw OSException("Process reached signal to terminate", LOCATION);
					
					if ( CommandId == CMD_TERM_ENTITY )
						throw OSException("Process reached signal to terminate", LOCATION);
					
					if ( CommandId == CMD_RUN_PROCESS ) 
						break;
				} 
                    
		        // Load and execute the binary
		        FileManager::getInstance()->executeFile( ProcessPath, getAlias() );
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath("Exception in ProcessStarter", LOCATION); 
                e.raiseError();
                
                OpenOS::getInstance()->deinitialize();
                
                exit(-1);
            }
            catch (...)
            {
                OSException("Unknown Exception in ProcessStarter", LOCATION).raiseError();
                
                OpenOS::getInstance()->deinitialize();

                exit(-2);
            }
            
            OpenOS::getInstance()->deinitialize();

            exit(0);            
        }           
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error launching process", LOCATION);
		
		throw;
	}	
}


void Process::deinitialize()
{
	if (m_IsInitialized == false)
		return;
	
	m_IsInitialized = false;

	try
	{
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			m_Threads[ Index ].deinitialize();
		}
	
		m_EntryPoints.deinitialize();
		
		m_InternalCommandInterface.deinitialize();
		
		m_Semaphore.deinitialize();
		
		m_ProcessData.deinitialize();
		
		// Only one of the following two will have been initialized.
		// however, DEinitializing twice doesn't matter, and
		// we don't need to store WHICH one of the two was initialized
		// this way
		m_LocalAllocator.deinitialize();
		m_SharedAllocator.deinitialize();
		
		m_PosixPid = 0;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Deinitializing of Process failed");		
		throw;
	}
	
}


ASAAC_PublicId Process::getId()
{
	if ( m_IsInitialized == false ) 
		return OS_UNUSED_ID;
	
	return m_ProcessData->Description.global_pid;
}


ProcessAlias Process::getAlias()
{
	if ( (getId() >=0) && (getId() <= OS_PROCESSID_APOS_MAX) )
		return PROC_APOS;
		
	if ( (getId() > OS_PROCESSID_APOS_MAX) && (getId() <= OS_PROCESSID_SMOS_MAX) )
	{
		if ( getId() == OS_PROCESSID_GSM )
			return PROC_GSM;	
	
		if ( getId() == OS_PROCESSID_PCS )
			return PROC_PCS;	
			
		if ( getId() == OS_PROCESSID_OLI )
			return PROC_OLI;	
	
		if ( getId() == OS_PROCESSID_SM )
			return PROC_SM;
			
		return PROC_SMOS;
	}
	
	return PROC_UNDEFINED; 
}


ASAAC_ReturnStatus Process::refreshPosixPid()
{
	if ( m_IsServer )
	{
		m_PosixPid = getpid();
	}
	else
	{
		PIDCommandData Data;
		
		ASAAC_TimedReturnStatus Result = sendCommand( CMD_GET_PID, Data.Buffer, 
												      TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time());
	
		if ( Result == ASAAC_TM_SUCCESS ) 
	        m_PosixPid = Data.PosixPid;
	    else return ASAAC_ERROR;
	}	
	
	return ASAAC_SUCCESS;
}


signed long	Process::getThreadIndex( ASAAC_PublicId ThreadId )
{
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		if ( m_Threads[ Index ].getId() == ThreadId ) return Index;
	}
	
	return -1;	
}


Thread* Process::getThread( ASAAC_PublicId ThreadId )
{
	signed long	Index = getThreadIndex( ThreadId );
	
	if (Index != -1) 
		return &m_Threads[ Index ];
	
	return NULL;
}


Thread* Process::getCurrentThread()
{
	if ( m_IsInitialized == false ) 
		return NULL;
	
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		if ( m_Threads[ Index ].isCurrentThread() ) 
            return &(m_Threads[ Index ]);
	}
	
	return NULL;
}


ASAAC_ReturnStatus Process::addEntryPoint( ASAAC_CharacterSequence Name, EntryPointAddr Address )
{
	if ( getEntryPoint( Name ) != NULL ) 
		return ASAAC_ERROR;
		
	EntryPoint *NextFreeSlot = getEntryPoint( EmptyEntryPoint.Name );
	
	if ( NextFreeSlot == NULL ) 
		return ASAAC_ERROR;
	
	NextFreeSlot->Name    = Name;
	NextFreeSlot->Address = Address;
		
	return ASAAC_SUCCESS;
}


EntryPoint* Process::getEntryPoint( ASAAC_CharacterSequence Name )
{
	if ( m_IsInitialized == false ) 
		return NULL;
				 
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_ENTRYPOINTS; Index ++ )
	{
		if ( CharSeq(m_EntryPoints[ Index ].Name) != CharSeq(Name) ) continue;
		
		return &(m_EntryPoints[ Index ]);
	}
	
	return NULL;
}


Semaphore* Process::getSemaphore()
{
	return &(m_Semaphore);
}


ASAAC_ReturnStatus Process::lockThreadPreemption( unsigned long& LockLevel )
{
	{
		ProtectedScope Access( "Locking thread preemption", m_Semaphore );
		
		(m_ProcessData->LockLevel)++;
		
		LockLevel = m_ProcessData->LockLevel;
	}
	
	suspendAllThreads();	
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus 	 Process::addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler )
{
	if 	(m_UseInternalCommandInterface)
		return m_InternalCommandInterface.addCommandHandler( CommandIdentifier, Handler );
	return ProcessManager::getInstance()->addCommandHandler( CommandIdentifier, Handler );
}


ASAAC_ReturnStatus 	 Process::removeCommandHandler( unsigned long CommandIdentifier )
{
	if 	(m_UseInternalCommandInterface)
		return m_InternalCommandInterface.removeCommandHandler( CommandIdentifier );
	return ProcessManager::getInstance()->removeCommandHandler( CommandIdentifier );
}


ASAAC_ReturnStatus 	 	Process::removeAllCommandHandler()
{
	if 	(m_UseInternalCommandInterface)
		return m_InternalCommandInterface.removeAllCommandHandler();
	return ProcessManager::getInstance()->removeAllCommandHandler();
}


ASAAC_TimedReturnStatus Process::sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	if 	(m_UseInternalCommandInterface)
		return m_InternalCommandInterface.sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
	return ProcessManager::getInstance()->sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
}


void Process::sendCommandNonblocking( unsigned long CommandIdentifier, CommandBuffer Buffer )
{
	if 	(m_UseInternalCommandInterface)
		m_InternalCommandInterface.sendCommandNonblocking( CommandIdentifier, Buffer );
	ProcessManager::getInstance()->sendCommandNonblocking( CommandIdentifier, Buffer );
}


ASAAC_ReturnStatus Process::handleOneCommand( unsigned long& CommandIdentifier )
{
	if 	(m_UseInternalCommandInterface)
		return m_InternalCommandInterface.handleOneCommand( CommandIdentifier );
	return ProcessManager::getInstance()->handleOneCommand( CommandIdentifier );
}


ASAAC_ReturnStatus Process::unlockThreadPreemption( unsigned long& LockLevel )
{
	{
		ProtectedScope Access( "Unlocking thread preemption", m_Semaphore );
		
		(m_ProcessData->LockLevel)--;
		
		LockLevel = m_ProcessData->LockLevel;
	}
		
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		m_Threads[ Index ].resume();		
	}
	
	return ASAAC_SUCCESS;
}



unsigned long Process::getLockLevel()
{
	return m_ProcessData->LockLevel;
}


ProcessStatus Process::getState()
{
	return m_ProcessData->Status;
}


ASAAC_ReturnStatus Process::createThread( const ASAAC_ThreadDescription& Description )
{
	try
	{
		ProtectedScope Access( "Creating a thread", m_Semaphore );
	
		if ( m_ProcessData->Status != PROCESS_INITIALIZED ) 
			throw OSException("Process shall be in state PROCESS_INITIALIZED", LOCATION);
		
		if ( getThread( Description.thread_id ) != NULL ) 
			throw OSException("Thread with dedicated id already created", LOCATION);
		
		Thread* NextAvailableThread = getThread( OS_UNUSED_ID );
		
		if ( NextAvailableThread == NULL ) 
			throw OSException("Maximum number of threads reached", LOCATION);
		
		if (NextAvailableThread->assign( Description ) == ASAAC_ERROR)
			throw OSException("Thread couldn't be assigned", LOCATION);
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error creating thread", LOCATION);
		e.raiseError();
		
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Process::resumeAllThreads()
{
	try
	{
		Process *P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);
	
		if (P->getId() != this->getId())
			throw OSException("Calling process is not 'this' process.", LOCATION);
		
		ASAAC_ReturnStatus result = ASAAC_SUCCESS;
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			if (m_Threads[ Index ].getId() == OS_UNUSED_ID)
				continue;
				
			ASAAC_ThreadStatus state;
			m_Threads[ Index ].getState( state );
			
			if (state == ASAAC_DORMANT)
				continue;
			
			if (m_Threads[ Index ].resume() != ASAAC_SUCCESS)
				result = ASAAC_ERROR;
		}
		
		if (result == ASAAC_ERROR)
			throw OSException("An error occured while resuming a thread", LOCATION);
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();
		
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Process::suspendAllThreads()
{
	try
	{
		Process *P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);
	
		if (P->getId() != this->getId())
			throw OSException("Calling process is not 'this' process.", LOCATION);
		
		ASAAC_ReturnStatus result = ASAAC_SUCCESS;
		Thread *ThisThread = NULL;
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			if (m_Threads[ Index ].getId() == OS_UNUSED_ID)
				continue;
				
			ASAAC_ThreadStatus state;
			m_Threads[ Index ].getState( state );
			
			if (state == ASAAC_DORMANT)
				continue;
			
			if ( m_Threads[ Index ].isCurrentThread() )
			{
				ThisThread = &(m_Threads[ Index ]); 
				continue;
			}
			
			
			if (m_Threads[ Index ].suspend() != ASAAC_SUCCESS)
				result = ASAAC_ERROR;
		}
		
		if (ThisThread != NULL)
		{
			result = ThisThread->suspendSelf();
		}
		
		if (result == ASAAC_ERROR)
			throw OSException("An error occured while suspending a thread", LOCATION);
			
		m_ProcessData->Status = PROCESS_STOPPED;
		
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();
		
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Process::terminateAllThreads()
{
	try
	{
		Process *P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);
	
		if (P->getId() != this->getId())
			throw OSException("Calling process is not 'this' process.", LOCATION);
		
		ASAAC_ReturnStatus result = ASAAC_SUCCESS;
		Thread *ThisThread = NULL;
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			if (m_Threads[ Index ].getId() == OS_UNUSED_ID)
				continue;
				
			ASAAC_ThreadStatus state;
			m_Threads[ Index ].getState( state );
			
			if (state == ASAAC_DORMANT)
				continue;

			if ( m_Threads[ Index ].isCurrentThread() )
			{
				ThisThread = &(m_Threads[ Index ]); 
				continue;
			}
			
			if (m_Threads[ Index ].terminate() != ASAAC_SUCCESS)
				result = ASAAC_ERROR;
		}

		if (ThisThread != NULL)
		{
			result = ThisThread->terminateSelf();
		}
		
		if (result == ASAAC_ERROR)
			throw OSException("An error occured while terminating a thread", LOCATION);
			
		m_ProcessData->Status = PROCESS_STOPPED; 			
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();
		
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus Process::run()
{
	if ( ProcessManager::getInstance()->getCurrentProcess() == this )
	{
		if (m_IsServer == false )
			throw FatalException("Current process is not declared as server for communication", LOCATION);

		Thread* MainThread = getThread(1);
		
		if ( MainThread == NULL ) 
			return ASAAC_ERROR;

		if ( m_ProcessData->Status == PROCESS_DORMANT ) 
			return ASAAC_ERROR;
		
		if (m_ActiveMainLoop == true)
		{
			ASAAC_ReturnStatus result = ASAAC_ERROR;
			
			if ( m_ProcessData->Status == PROCESS_RUNNING ) 
				return ASAAC_ERROR;
	
			result = resumeAllThreads();
			
			m_ProcessData->Status = PROCESS_RUNNING;
			
			return result;
			
		}
		else
		{	
			m_ActiveMainLoop = true;
			
			try
			{
				m_ProcessData->Status = PROCESS_RUNNING;
				
				MainThread->start();
	
				unsigned long CommandId;
	
				for(;;)
				{
					handleOneCommand( CommandId );
	
					if ( CommandId == CMD_TERM_PROCESS ) break;
					if ( CommandId == CMD_TERM_ENTITY ) break;
				} 

				//TODO: Is this call still needed?
				if (m_IsMaster)
					ProcessManager::getInstance()->destroyEntity();
				
				terminateAllThreads();
				detachAndDestroyAllLocalVcs();
			}
			catch (ASAAC_Exception &e)
			{
                e.addPath("Error occured in main loop of this process", LOCATION);
                
                terminateAllThreads();
                detachAndDestroyAllLocalVcs();

				m_ActiveMainLoop = false;
				m_ProcessData->Status = PROCESS_DORMANT;
                
				throw;
			}			
				
			m_ActiveMainLoop = false;
			m_ProcessData->Status = PROCESS_DORMANT;

			return ASAAC_SUCCESS;
		}
				
		return ASAAC_ERROR;
	}
	else
	{
	    CommandData Data;
    
		bool RefreshPosixPid = ( getState() == PROCESS_INITIALIZED );
	
		if ( sendCommand( CMD_RUN_PROCESS, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() )  != ASAAC_TM_SUCCESS )
		{
			return ASAAC_ERROR;
		}
		
		if ( Data.Return == ASAAC_SUCCESS )
		{
			// wait for process to send its PID,
			// thus also signalling its readiness to
			// respond.
            if (RefreshPosixPid)
                refreshPosixPid();
		}
		
		return Data.Return;
	}
}


ASAAC_ReturnStatus Process::stop()
{
	ASAAC_ReturnStatus status = ASAAC_ERROR;
	
	try
	{
		if ( m_IsServer )
		{		
			if ( (m_ProcessData->Status) != PROCESS_RUNNING )
				throw OSException("Process is not in state 'RUNNING'", LOCATION);
			if (suspendAllThreads() != ASAAC_SUCCESS)
				throw OSException("An error occured while suspending all threads.", LOCATION);
		
			m_ProcessData->Status = PROCESS_STOPPED;
			
			status = ASAAC_SUCCESS;
		}
		else
		{
			CommandData Data;
			
			ASAAC_TimedReturnStatus snd_status;
			snd_status = sendCommand( CMD_STOP_PROCESS, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

			if ( snd_status == ASAAC_TM_TIMEOUT ) 
				throw OSException("Timeout while sending command 'stop' to process", LOCATION);
	
			if ( snd_status == ASAAC_TM_ERROR ) 
				throw OSException("Error while sending command 'stop' to process", LOCATION);
			
			status = Data.Return;
		}
	}
	catch ( ASAAC_Exception &e )
	{
        CharacterSequence ErrorString;
        
		e.addPath( (ErrorString << "An Error occured while stopping the process: " << CharSeq(getId())).c_str(), LOCATION);
		e.raiseError();
		status = ASAAC_ERROR;
	}
	catch (...)
	{
		status = ASAAC_ERROR;
	}
	
	return status;
}


ASAAC_ReturnStatus Process::attachLocalVc( ASAAC_PublicId GlobalVcId, ASAAC_PublicId LocalVcId )
{
	try
	{
		// process must be in STOPPED or INITIALIZED state
		ProcessStatus ThisState = getState();

		if (( ThisState != PROCESS_STOPPED ) && 
			( ThisState != PROCESS_INITIALIZED )) 
			throw OSException("Process must be in state 'STOPPED' or 'INITIALIZED'", LOCATION);

		if (isAttachedTo( LocalVcId ) == true )
			throw OSException("Local VC has already been attached to this process.", LOCATION);

		if (m_IsServer == true)
		{  					
			LocalVc* ThisLocalVc = CommunicationManager::getInstance()->getLocalVirtualChannel( getId(), GlobalVcId, LocalVcId );
	
			if ( ThisLocalVc == NULL ) 
				throw OSException("LocalVC could not be found in GlobalVC list", LOCATION);
	
            {
	            ProtectedScope Access( "Attaching a local vc", m_Semaphore );
	            
				long Index = getAttachedVirtualChannelIndex( OS_UNUSED_ID );			
				
				if ( Index == -1 ) 
					throw OSException("Maximum number of attached local VCs has been reached. No free slots.", LOCATION);
									
				m_LocalVCs[ Index ].GlobalVcId   = GlobalVcId;		
				m_LocalVCs[ Index ].LocalVcId    = LocalVcId;
            }		
		}	
		else		
		{
			VCCommandData Data;
		
			Data.VC.GlobalVcId = GlobalVcId;
			Data.VC.LocalVcId  = LocalVcId;
		
			ASAAC_TimedReturnStatus status;
			
			status = sendCommand( 
				CMD_ATTACH_VC, 
				Data.ReturnBuffer, 
				TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time());
			
			if ( status == ASAAC_TM_TIMEOUT ) 
				throw OSException("Timeout while sending command 'attachLocalVc' to process", LOCATION);
	
			if ( status == ASAAC_TM_ERROR ) 
				throw OSException("Error while sending command 'attachLocalVc' to process", LOCATION);
						
			if ( Data.Return == ASAAC_ERROR ) 
				throw OSException("Server replied an error", LOCATION);
		}
				
	}
	catch ( ASAAC_Exception &e)
	{
		CharacterSequence ErrorString;
		ErrorString << "LocalVc couldn't be attached to process. ProcessId:" << CharSeq(getId()) 
			<< " GlobalVcId:" << CharSeq(GlobalVcId) << " LocalVcId:" << CharSeq(LocalVcId);
		 
		e.addPath(ErrorString.c_str(), LOCATION);
		e.raiseError();
		
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS; 
}


ASAAC_ReturnStatus Process::detachAndDestroyAllLocalVcs()
{
	if (m_IsServer == false)
		return ASAAC_ERROR;
		
	ASAAC_ReturnStatus Result = ASAAC_SUCCESS;
	
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index ++ )
	{
		if ( m_LocalVCs[ Index ].GlobalVcId == OS_UNUSED_ID ) 
			continue;

		if ( m_LocalVCs[ Index ].LocalVcId == OS_UNUSED_ID ) 
			continue;
		
		LocalVc *LVc = CommunicationManager::getInstance()->getLocalVirtualChannel( getId(), m_LocalVCs[ Index ].GlobalVcId, m_LocalVCs[ Index ].LocalVcId );
		
		if (LVc == NULL)
			continue;
		
		if (LVc->remove() == ASAAC_ERROR)
			Result = ASAAC_ERROR;
		
		m_LocalVCs[ Index ].GlobalVcId   = OS_UNUSED_ID;
		m_LocalVCs[ Index ].LocalVcId    = OS_UNUSED_ID;
	}
	
	return Result;
}


ASAAC_ReturnStatus Process::detachLocalVc( ASAAC_PublicId LocalVcId )
{
	try
	{
		// process must be in STOPPED or INITIALIZED state
		ProcessStatus ThisState = getState();
		
		if (( ThisState != PROCESS_STOPPED ) && 
			( ThisState != PROCESS_INITIALIZED )) 
			throw OSException("Process must be in state 'STOPPED' or 'INITIALIZED'", LOCATION);
		
        
        if (m_IsServer == true)
        {
			long Index = getAttachedVirtualChannelIndex( LocalVcId );
			
			if ( Index == -1 )
				throw OSException("LocalVcId not found", LOCATION);
		
			//free the slot
			m_LocalVCs[ Index ].GlobalVcId   = OS_UNUSED_ID;
			m_LocalVCs[ Index ].LocalVcId    = OS_UNUSED_ID;
        }	
        else
		{
			VCCommandData Data;
			Data.VC.LocalVcId  = LocalVcId;
		
			ASAAC_TimedReturnStatus status;
			
			status = sendCommand( CMD_DETACH_VC, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

			if ( status == ASAAC_TM_TIMEOUT ) 
				throw OSException("Timeout while sending command 'attachLocalVc' to process", LOCATION);
	
			if ( status == ASAAC_TM_ERROR ) 
				throw OSException("Error while sending command 'attachLocalVc' to process", LOCATION);
						
			if ( Data.Return == ASAAC_ERROR ) 
				throw OSException("Server replied an error", LOCATION);
		}
	}
	catch ( ASAAC_Exception &e)
	{
		CharacterSequence ErrorString;
		ErrorString << "Error detaching local vc. ProcessId:" << CharSeq(getId()) << " LocalVcId:" << CharSeq(LocalVcId);
		e.addPath(ErrorString.c_str(), LOCATION);
		
		throw;
	}
	
	return ASAAC_SUCCESS;
}



size_t Process::predictSize()
{
	size_t CumulativeSize = 0;
	
	// m_ProcessData
	CumulativeSize +=  Shared<ProcessData>::predictSize();
	
	// m_RegisteredVCs
	CumulativeSize +=  Shared<VCData>::predictSize( OS_MAX_NUMBER_OF_LOCALVCS );
	
	// m_Semaphore
	CumulativeSize +=  Semaphore::predictSize();
	
	// m_Threads
	CumulativeSize +=  OS_MAX_NUMBER_OF_THREADS * Thread::predictSize();
				 
	// m_EntryPoints
	CumulativeSize +=  OS_MAX_NUMBER_OF_ENTRYPOINTS * Shared<EntryPoint>::predictSize();
	
	//m_InternalCommandInterface
	CumulativeSize +=  SimpleCommandInterface::predictSize();
	
	return CumulativeSize;
}


	
ASAAC_ReturnStatus Process::destroy()
{
	CommandData Data;
	
	try
	{
		//If process shall be destroyed from inside it's own thread
		//sendCommand will not return here.	
		ASAAC_TimedReturnStatus TerminationStatus = 
			sendCommand( 
				CMD_TERM_PROCESS, 
				Data.ReturnBuffer, 
				TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

		CharSeq ErrorString;
	
		if (TerminationStatus == ASAAC_TM_ERROR)
			OSException((ErrorString << "Termination command could not be send properly (PID=" << CharSeq(getId()) << ")").c_str(), LOCATION).raiseError();

		if (TerminationStatus == ASAAC_TM_TIMEOUT)
			OSException((ErrorString << "Termination command was not replied by application to be terminated (PID=" << CharSeq(getId())<<")").c_str(), LOCATION).raiseError();

		if (Data.Return == ASAAC_ERROR)
			OSException((ErrorString << "Application created an error while terminating itself. (PID=" << CharSeq(getId()) << ")").c_str(), LOCATION).raiseError();
		
	
		//If Process shall be killed from outside, kill it truely now...
		Process *P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);
				
		if (P->getId() != this->getId())
		{
			pid_t PosixPid = m_PosixPid;
			int Dummy;
			
			ASAAC_TimedReturnStatus SignalStatus = SignalManager::getInstance()->waitForSignal( SIGCHLD, Dummy, OS_SIMPLE_COMMAND_TIMEOUT  );
	
			if ( SignalStatus == ASAAC_TM_ERROR )
			{	
				OSException("Signal to assume processes readyness could not be sent.", LOCATION).raiseError();
			}

			if ( SignalStatus == ASAAC_TM_TIMEOUT )
			{	
				if ( PosixPid != 0 ) 
					oal_kill( PosixPid, SIGTERM );
				else OSException("Kill Signal cannot be send, because pid is unknown (PID=0).", LOCATION).raiseError();
			}
			
			if ( SignalStatus == ASAAC_TM_SUCCESS )
			{
				int Dummy;		
				oal_waitpid( (pid_t)-1, &Dummy, 0 );
			}
			
			//TODO: deinitialize here?
			deinitialize();
	
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();
		
		return ASAAC_ERROR;
	}
		
	return ASAAC_SUCCESS;
}


LocalVc* Process::getAttachedVirtualChannel( ASAAC_PublicId LocalVcId )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	LocalVc* LVc = NULL;
	try
	{
		CharacterSequence ErrorString;
		
		ProtectedScope Access( "Searching a local vc", m_Semaphore );

		long Index = getAttachedVirtualChannelIndex( LocalVcId );
		
		if (Index == -1)
			throw OSException( (ErrorString << "LocalVc (local_vc_id=" << CharSeq(LocalVcId) << ") not attached to process (pid=" << CharSeq(getId()) << ")").c_str(), LOCATION);
			
		LVc = CommunicationManager::getInstance()->getLocalVirtualChannel( getId(), m_LocalVCs[ Index ].GlobalVcId, m_LocalVCs[ Index ].LocalVcId ); 
	}
	catch ( ASAAC_Exception &e)
	{
		e.raiseError();
		
		return NULL;
	}
	
	return LVc;
}


long Process::getAttachedVirtualChannelIndex( ASAAC_PublicId LocalVcId )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

    long Index;
    for ( Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index ++ )
    {
        if ( m_LocalVCs[ Index ].LocalVcId == LocalVcId )
            break;
    }
    
    if (Index == OS_MAX_NUMBER_OF_LOCALVCS)
        return -1;
    
    return Index;
}


bool Process::isAttachedTo( ASAAC_PublicId LocalVcId )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);
    
    if ( getAttachedVirtualChannelIndex( LocalVcId ) == -1 )
        return false;
    
    return true;
}


unsigned long Process::getAuthenticationCode()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return ( m_ProcessData->AuthenticationCode );
}


ASAAC_ProcessDescription Process::getProcessDescription()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return ( m_ProcessData->Description );
}


bool Process::isOSScope()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return (getCurrentThread() == NULL);
}


ASAAC_ReturnStatus Process::invokeOSScope(OSScopeFunction foo, OSScopeCommandBuffer param)
{
	if ( isOSScope() )
	{
		return foo(param);
	}
	else
	{
		OSScopeCommandData Data;
		Data.Return = ASAAC_ERROR;
		
		try
		{
			Data.Scope.foo = foo;
			memcpy( Data.Scope.param, param, sizeof(OSScopeCommandBuffer) );
		
			ASAAC_TimedReturnStatus status;
			status = sendCommand( 
				CMD_INVOKE_OS_SCOPE, 
				Data.ReturnBuffer, 
				TimeStamp::Infinity().asaac_Time(),
				true);
					
			if ( status == ASAAC_TM_TIMEOUT ) 
				throw OSException("Timeout while executing OSScopeFunction", LOCATION);
		
			if ( status == ASAAC_TM_ERROR ) 
				throw OSException("Error while executing OSScopeFunction", LOCATION);
		}
		catch (ASAAC_Exception &e)
		{
			e.raiseError();
		}
					
		return Data.Return;
	} 
}


SchedulingData  Process::getOSScopeSchedulingData()
{
    return m_ProcessData->OSScopeSchedulingData;
}



// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************


void Process::RunHandler( CommandBuffer Buffer )
{
	volatile ASAAC_ReturnStatus* Return = (ASAAC_ReturnStatus*)( Buffer );

	try
	{
		Process* ThisInstance = ProcessManager::getInstance()->getCurrentProcess();
		
		if (ThisInstance == NULL)
			throw OSException("'Current process' is not available.", LOCATION);
			
		if ( ThisInstance->getState() == PROCESS_INITIALIZED )
			// PROCESS_INITIALIZED means Process is still under control of the
			// Process Starter. The process Starter handles the starting itself.
			// Hence, do nothing, but only return success.
			 *Return = ASAAC_SUCCESS;
			 
		else *Return = ThisInstance->run();
			
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();
		
		*Return = ASAAC_ERROR;
		return;
	}
	catch (...)
	{
		*Return = ASAAC_ERROR;

		return;
	}
	
	return;
}


void Process::StopHandler( CommandBuffer Buffer )
{
	volatile ASAAC_ReturnStatus* Return = (ASAAC_ReturnStatus*)( Buffer );

	try
	{
		Process* ThisInstance = ProcessManager::getInstance()->getCurrentProcess();
		
		if (ThisInstance == NULL)
			throw OSException("'Current process' is not available.", LOCATION);
			
		*Return = ThisInstance->stop();
			
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();
		
		*Return = ASAAC_ERROR;
		return;
	}
	catch (...)
	{
		*Return = ASAAC_ERROR;

		return;
	}
	
	return;
}



void Process::DestroyHandler( CommandBuffer Buffer )
{
	volatile ASAAC_ReturnStatus* Return = (ASAAC_ReturnStatus*)( Buffer );
		
	//Nothing to do	
		
	*Return = ASAAC_SUCCESS;
	return;
}



void Process::getPIDHandler( CommandBuffer Buffer )
{
	pid_t* Return = (pid_t*)( Buffer );

	*Return = getpid();		

	return;
}



void Process::attachLocalVcHandler( CommandBuffer Buffer )
{
	VCCommandData* Data = (VCCommandData*)Buffer; 

	// get current process	
	ProcessManager* PM = ProcessManager::getInstance();
	Process* ThisProcess = PM->getCurrentProcess();
		
	try
	{
		if (ThisProcess == NULL)
			throw FatalException("Current Process not found", LOCATION);

		if (ThisProcess->attachLocalVc( Data->VC.GlobalVcId, Data->VC.LocalVcId ) == ASAAC_ERROR)
			throw OSException("An error occured while attaching VC.", LOCATION);
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();

		Data->Return = ASAAC_ERROR;
		
		return;
	}
	catch (...)
	{
		Data->Return = ASAAC_ERROR;

		return;
	}

	Data->Return = ASAAC_SUCCESS;
	
	return;
}



void Process::detachLocalVcHandler( CommandBuffer Buffer )
{
	VCCommandData* Data = (VCCommandData*)Buffer; 

	// get current process	
	ProcessManager* PM = ProcessManager::getInstance();
	Process* ThisProcess = PM->getCurrentProcess();
		
	try
	{
		if (ThisProcess == NULL)
			throw FatalException("Current Process not found", LOCATION);

		if (ThisProcess->detachLocalVc( Data->VC.LocalVcId ) == ASAAC_ERROR)
			throw OSException("An error occured while detaching VC.", LOCATION);
	}
	catch (ASAAC_Exception &e)
	{
		e.raiseError();

		Data->Return = ASAAC_ERROR;
		
		return;
	}
	catch (...)
	{
		Data->Return = ASAAC_ERROR;

		return;
	}

	Data->Return = ASAAC_SUCCESS;
	
	return;
}

void Process::invokeOSScopeHandler( CommandBuffer Buffer )
{
	OSScopeCommandData* Data = (OSScopeCommandData*)Buffer; 

	Data->Return = Data->Scope.foo(Data->Scope.param);
	
	return;
}

