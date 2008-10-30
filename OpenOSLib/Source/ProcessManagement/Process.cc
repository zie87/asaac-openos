#include "Process.hh"

#include "OpenOSObject.hh"

#include "Allocator/SharedMemory.hh"
#include "Allocator/LocalMemory.hh"

#include "IPC/ProtectedScope.hh"

#include "Exceptions/Exceptions.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Managers/SignalManager.hh"
#include "Managers/TimeManager.hh"

#include "Managers/AllocatorManager.hh"
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
	m_PosixId = 0;
	m_ActiveMainLoop = false;
	m_Destroying = false;
}


Process::~Process()
{
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

	// m_ThreadIndex
	CumulativeSize += Shared<ASAAC_PublicId>::predictSize( OS_MAX_NUMBER_OF_THREADS );

	// m_ThreadObject
	CumulativeSize +=  OS_MAX_NUMBER_OF_THREADS * Thread::predictSize();

	// m_EntryPoints
	CumulativeSize +=  OS_MAX_NUMBER_OF_THREADS * Shared<EntryPoint>::predictSize();

	//m_InternalCommandInterface
	CumulativeSize +=  SimpleCommandInterface::predictSize();

	return CumulativeSize;
}


void Process::initialize( bool IsServer, bool IsMaster, const ASAAC_ProcessDescription& Description, MemoryLocation Location, SimpleCommandInterface *CommandInterface )
{
	if ( m_IsInitialized )
		throw DoubleInitializationException( LOCATION );

	try
	{
		m_IsInitialized = true;

		m_IsMaster = IsMaster;

		if (CommandInterface == NULL)
			m_CommandInterface = &m_InternalCommandInterface;
		else m_CommandInterface = CommandInterface;

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
		m_LocalVCs.initialize( m_Allocator, OS_MAX_NUMBER_OF_LOCALVCS );
		m_Semaphore.initialize( m_Allocator, IsMaster );
		m_InternalCommandInterface.initialize( m_Allocator, IsMaster );
		m_EntryPoints.initialize( m_Allocator, OS_MAX_NUMBER_OF_THREADS );
		m_ThreadIndex.initialize( m_Allocator, OS_MAX_NUMBER_OF_THREADS );

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			m_ThreadAllocator[ Index ].initialize( m_Allocator, Thread::predictSize() );
		}

		if ( IsMaster )
		{
			m_ProcessData->Description        = Description;
			m_ProcessData->Status             = PROCESS_INITIALIZED;
			m_ProcessData->AuthenticationCode = lrand48();
			m_ProcessData->LockLevel		  = 0;

			for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
			{
				m_ThreadIndex[ Index ] = OS_UNUSED_ID;
			}

			for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index ++ )
			{
				m_LocalVCs[ Index ].GlobalVcId   = OS_UNUSED_ID;
				m_LocalVCs[ Index ].LocalVcId    = OS_UNUSED_ID;
			}

			for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index ++ )
			{
				m_EntryPoints[ Index ] = EmptyEntryPoint;
			}
		}

		setServer( IsServer );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing process object", LOCATION);

		deinitialize();

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
			m_ThreadObject[ Index ].deinitialize();
		}

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			m_ThreadAllocator[ Index ].deinitialize();
		}

		m_EntryPoints.deinitialize();
		m_InternalCommandInterface.deinitialize();
		m_Semaphore.deinitialize();
		m_ProcessData.deinitialize();
		m_ThreadIndex.deinitialize();

		// Only one of the following two will have been initialized.
		// however, DEinitializing twice doesn't matter, and
		// we don't need to store WHICH one of the two was initialized
		// this way
		m_LocalAllocator.deinitialize();
		m_SharedAllocator.deinitialize();

		m_IsMaster = false;
		m_IsServer = false;

		m_PosixId = 0;
		m_ActiveMainLoop = false;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Deinitializing of Process failed");

		e.raiseError();
	}

}


bool Process::isInitialized()
{
	return m_IsInitialized;
}


void Process::setServer( bool IsServer )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	if (m_IsServer == IsServer)
		return;

	if ( IsServer == true )
	{
		addCommandHandler( CMD_RUN_PROCESS,  Process::RunHandler  );
		addCommandHandler( CMD_GET_PID, Process::RequestPIDHandler );
		addCommandHandler( CMD_STOP_PROCESS, Process::StopHandler );
		addCommandHandler( CMD_TERM_PROCESS, Process::DestroyHandler );
		addCommandHandler( CMD_ATTACH_VC, Process::AttachLocalVcHandler );
		addCommandHandler( CMD_DETACH_VC, Process::DetachLocalVcHandler );
		addCommandHandler( CMD_INVOKE_OS_SCOPE, Process::InvokeOSScopeHandler );

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
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	return m_IsServer;
}


void Process::launch()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

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
			OliClient.storeCompleteFile(ProcessPath, m_ProcessData->Description.programme_file_name);
		}
		else ProcessPath = FileNameGenerator::getAsaacPath(m_ProcessData->Description.programme_file_name);

		m_PosixId = fork();

        if (m_PosixId == -1)
            throw OSException( strerror(errno), LOCATION );

        //Starter process
        if (m_PosixId == 0)
        {
            try
            {
            	m_PosixId = getpid();

                OpenOS::getInstance()->switchState( false, LAS_PROCESS_INIT, getId() );

            	struct sched_param param;
                ProcessAlias alias = getAlias();

                switch (alias)
                {
					case PROC_SMOS:
					case PROC_GSM:
					case PROC_SM:
		            	param.__sched_priority = sched_get_priority_max(SCHED_FIFO)-2;
		            	break;
					case PROC_PCS:
		            	param.__sched_priority = sched_get_priority_max(SCHED_FIFO)-0;
		            	break;
					case PROC_OLI:
		            	param.__sched_priority = sched_get_priority_max(SCHED_FIFO)-1;
		            	break;
					case PROC_UNDEFINED:
					case PROC_APOS:
					default:
		            	param.__sched_priority = sched_get_priority_min(SCHED_FIFO);
		            	break;
                }

            	int result = sched_setscheduler( m_PosixId, SCHED_FIFO, &param);

            	if (result != 0)
            		throw OSException( strerror(errno), LOCATION );

				// Enter main cycle of ProcessStarter
				for(;;)
				{
					unsigned long CommandId;
					handleOneCommand( CommandId );

					if ( CommandId == CMD_TERM_PROCESS )
						break;

					if ( CommandId == CMD_TERM_ENTITY )
						break;

					if ( CommandId == CMD_RUN_PROCESS )
						FileManager::getInstance()->executeFile( ProcessPath, getAlias() );
				}

                AllocatorManager::getInstance()->deallocateAllObjects();
                //OpenOS::getInstance()->deinitialize();
	            exit(OS_SIGNAL_SUCCESS);
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath("Exception in ProcessStarter", LOCATION);
                e.printErrorMessage();
                e.raiseError();

				//TODO: To be able to deinitialize all objects switchState shall
				//set Master flags of all objects to false and correct the dedicated Server flags

                AllocatorManager::getInstance()->deallocateAllObjects();
                //OpenOS::getInstance()->deinitialize();

                exit(OS_SIGNAL_ERROR);
            }
            catch (...)
            {
                OSException e("Unknown Exception in ProcessStarter", LOCATION);
                e.printErrorMessage();
                e.raiseError();

                AllocatorManager::getInstance()->deallocateAllObjects();
                //OpenOS::getInstance()->deinitialize();

                exit(OS_SIGNAL_FATALERROR);
            }
        }
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error launching process", LOCATION);

		throw;
	}
}


void Process::refreshPosixId()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	if ( m_IsServer )
	{
		m_PosixId = getpid();
	}
	else
	{
		PIDCommandData Data;

		sendCommand( CMD_GET_PID, Data.Buffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time());

        m_PosixId = Data.PosixPid;
	}
}


pid_t Process::getPosixId()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	return m_PosixId;
}


void Process::createThread( const ASAAC_ThreadDescription& Description )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		ProtectedScope Access( "Creating a thread", m_Semaphore );

		if ( m_ProcessData->Status != PROCESS_INITIALIZED )
			throw OSException("Process shall be in state PROCESS_INITIALIZED", LOCATION);

		if ( getThreadIndex( Description.thread_id ) != -1 )
			throw OSException("Thread with dedicated id already created", LOCATION);

		signed long Index = getThreadIndex( OS_UNUSED_ID );

		if ( Index == -1 )
			throw OSException("Maximum number of threads reached", LOCATION);

		m_ThreadAllocator[Index].reset();
		m_ThreadObject[ Index ].initialize( true, Description, this, &(m_ThreadAllocator[Index]) );
		m_ThreadIndex[ Index ] = Description.thread_id;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error creating thread", LOCATION);

		throw;
	}
}


signed long	Process::getThreadIndex( ASAAC_PublicId ThreadId )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		if ( m_ThreadIndex[ Index ] == ThreadId )
			return Index;
	}

	return -1;
}


Thread* Process::getThread( ASAAC_PublicId ThreadId )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	signed long	Index = getThreadIndex( ThreadId );

	if (Index == -1)
		throw OSException("Thread object could not be found", LOCATION);

	return getThreadByIndex(Index);
}


Thread* Process::getCurrentThread()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	unsigned long Index;

	for ( Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		Thread *ThreadObject = getThreadByIndex(Index);
		if ( ThreadObject->isInitialized() )
		{
			if ( m_ThreadObject[ Index ].isCurrentThread() )
	            break;
		}
	}

	if (Index == OS_MAX_NUMBER_OF_THREADS)
		throw OSException("Thread object could not be found", LOCATION);

	return getThreadByIndex(Index);
}


Thread*	Process::getThreadByIndex( unsigned long Index )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	if ( (  Index < 0 ) ||
         ( (unsigned long)Index > OS_MAX_NUMBER_OF_THREADS ) )
		throw OSException("Index is out of range",LOCATION);

	Thread* ThreadObject = &m_ThreadObject[ Index ];

	if ((ThreadObject->isInitialized() == false) && (m_ThreadIndex[ Index ] != OS_UNUSED_ID))
	{
		ASAAC_ThreadDescription Description;
		m_ThreadAllocator[Index].reset();
		ThreadObject->initialize( false, Description, this, &m_ThreadAllocator[Index] );
	}

	return ThreadObject;
}


void Process::resumeAllThreads()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		Process *P = ProcessManager::getInstance()->getCurrentProcess();

		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);

		if (P->getId() != this->getId())
			throw OSException("Calling process is not 'this' process.", LOCATION);

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			Thread* ThreadObject = getThreadByIndex(Index);

			if (ThreadObject->isInitialized() == false )
				continue;

			ASAAC_ThreadStatus state;
			ThreadObject->getState( state );

            if (state == ASAAC_DORMANT)
                continue;

            if (state == ASAAC_DORMANT)
                continue;

			ThreadObject->resume();
		}

		m_ProcessData->Status = PROCESS_RUNNING;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error resuming all threads", LOCATION);

		throw;
	}
}


void Process::suspendAllThreads()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		Process *P = ProcessManager::getInstance()->getCurrentProcess();

		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);

		if (P->getId() != this->getId())
			throw OSException("Calling process is not 'this' process.", LOCATION);

		Thread *ThisThread = NULL;
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			Thread* ThreadObject = getThreadByIndex(Index);

			if (ThreadObject->isInitialized() == false)
				continue;

			ASAAC_ThreadStatus state;
			ThreadObject->getState( state );

			if (state == ASAAC_DORMANT)
				continue;

			if ( ThreadObject->isCurrentThread() )
			{
				ThisThread = ThreadObject;
				continue;
			}
			ThreadObject->suspend();
		}

		m_ProcessData->Status = PROCESS_STOPPED;

		if (ThisThread != NULL)
			ThisThread->suspendSelf();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error suspending all threads", LOCATION);

		throw;
	}
}


void Process::terminateAllThreads()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		Process *P = ProcessManager::getInstance()->getCurrentProcess();

		if (P == NULL)
			throw FatalException("Current Process not found", LOCATION);

		if (P->getId() != this->getId())
			throw OSException("Calling process is not 'this' process.", LOCATION);

		Thread *ThisThread = NULL;
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
		{
			Thread* ThreadObject = getThreadByIndex(Index);

			if (ThreadObject->isInitialized() == false)
				continue;

			ASAAC_ThreadStatus state;
			ThreadObject->getState( state );

			if (state == ASAAC_DORMANT)
				continue;

			if ( ThreadObject->isCurrentThread() )
			{
				ThisThread = ThreadObject;
				continue;
			}

			ThreadObject->stop();
		}

		m_ProcessData->Status = PROCESS_STOPPED;

		if (ThisThread != NULL)
			ThisThread->terminateSelf();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error resuming all threads", LOCATION);

		throw;
	}
}


void Process::run()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( m_IsServer == true )
		{
			Thread* MainThread = getThread(1);

			if ( m_ProcessData->Status == PROCESS_DORMANT )
				throw OSException( "Process is in state dormant", LOCATION );

			if (m_ActiveMainLoop == true)
			{
				if ( m_ProcessData->Status == PROCESS_RUNNING )
					throw OSException( "Process is already running", LOCATION );

				resumeAllThreads();
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
					//if (m_IsMaster)
						//ProcessManager::getInstance()->destroyEntity();

					//terminateAllThreads();
					suspendAllThreads();

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
			}
		}
		else //m_IsServer == false
		{
		    CommandData Data;

			bool RefreshPosixId = ( getState() == PROCESS_INITIALIZED );

			sendCommand( CMD_RUN_PROCESS, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

			if ( Data.Return == ASAAC_SUCCESS )
			{
				// wait for process to send its PID,
				// thus also signalling its readiness to
				// respond.
	            if (RefreshPosixId)
	                refreshPosixId();
			}

			if (Data.Return == ASAAC_ERROR)
				throw OSException("Error returned by process itself", LOCATION);
		}
	}
	catch ( ASAAC_Exception &e )
	{
        CharacterSequence ErrorString;
        ErrorString << "Error running process " << CharSeq(getId());
		e.addPath( ErrorString.c_str(), LOCATION);

		throw;
	}
}


void Process::stop()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( m_IsServer )
		{
			if ( (m_ProcessData->Status) != PROCESS_RUNNING )
				throw OSException("Process is not in state 'RUNNING'", LOCATION);

			suspendAllThreads();
		}
		else
		{
			CommandData Data;
			sendCommand( CMD_STOP_PROCESS, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

			if (Data.Return == ASAAC_ERROR)
				throw OSException("Error returned by process itself", LOCATION);
		}
	}
	catch ( ASAAC_Exception &e )
	{
        CharacterSequence ErrorString;
        ErrorString << "Error stopping process " << CharSeq(getId());
		e.addPath( ErrorString.c_str(), LOCATION);

		throw;
	}
}


void Process::destroy()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CommandData Data;
	CharSeq ErrorString;

	try
	{
		try
		{
			refreshPosixId();

			m_Destroying = true;

			sendCommand( CMD_TERM_PROCESS, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

			if (Data.Return == ASAAC_ERROR)
				throw OSException((ErrorString << "Application created an error while terminating itself. (PID=" << CharSeq(getId()) << ")").c_str(), LOCATION);

			//TODO: here the process shall wait for the child signal, that process has been terminated
			//This must perhaps be synchronized with the signal callback function.

			if (m_IsMaster)
			{
				siginfo_t SignalInfo;
				SignalManager::getInstance()->waitForSignal( SIGCHLD, SignalInfo, OS_SIMPLE_COMMAND_TIMEOUT );
			}
		}
		catch (ASAAC_Exception &e)
		{
			if (ProcessManager::getInstance()->getCurrentProcess()->getId() != this->getId())
			{
				if ( m_PosixId != 0 )
				{
					oal_kill( m_PosixId, SIGTERM );
				}
				else
				{
					e.addPath("Kill Signal cannot be sent, because posix-id of process is unknown.", LOCATION);
					throw e;
				}
			}
			else throw e;
		}

		//TODO: deinitialize here?
		deinitialize();
	}
	catch (ASAAC_Exception &e)
	{
		m_Destroying = false;

		CharSeq ErrorString;
		e.addPath( (ErrorString << "Error destroying process " << getId()).c_str(), LOCATION);

		throw;
	}

	m_Destroying = false;
}


void Process::addEntryPoint( const ASAAC_CharacterSequence &Name, const EntryPointAddr Address )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( getEntryPointIndex( Name ) != -1 )
			throw OSException("EntryPoint with dedicated name is already registered", LOCATION);

		signed long Index = getEntryPointIndex( EmptyEntryPoint.Name );

		if ( Index == -1 )
			throw OSException("No more free slots", LOCATION);

		m_EntryPoints[ Index ].Name    = Name;
		m_EntryPoints[ Index ].Address = Address;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error adding an entry point to process", LOCATION);

		throw;
	}
}


signed long	Process::getEntryPointIndex( const ASAAC_CharacterSequence &Name )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_THREADS; Index++ )
	{
		if ( CharSeq(m_EntryPoints[ Index ].Name) == CharSeq(Name) )
			return Index;
	}

	return -1;
}


EntryPoint* Process::getEntryPoint( const ASAAC_CharacterSequence &Name )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CharSeq ErrorString;

	signed long	Index = getEntryPointIndex( Name );

	if (Index == -1)
		throw OSException( (ErrorString << "EntryPoint could not be found: " << Name).c_str(), LOCATION);

	return &m_EntryPoints[ Index ];
}


Semaphore* Process::getSemaphore()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	return &(m_Semaphore);
}


void Process::lockThreadPreemption( unsigned long& LockLevel )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		{
			ProtectedScope Access( "Locking thread preemption", m_Semaphore );

			(m_ProcessData->LockLevel)++;

			LockLevel = m_ProcessData->LockLevel;
		}

		suspendAllThreads();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error locking thread preemption", LOCATION);

		throw;
	}
}


void Process::unlockThreadPreemption( unsigned long& LockLevel )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		{
			ProtectedScope Access( "Unlocking thread preemption", m_Semaphore );

			(m_ProcessData->LockLevel)--;

			LockLevel = m_ProcessData->LockLevel;
		}

		resumeAllThreads();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error unlocking thread preemption", LOCATION);

		throw;
	}
}


unsigned long Process::getLockLevel()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	return m_ProcessData->LockLevel;
}


void 	 Process::addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface->addCommandHandler( CommandIdentifier, Handler );
}


void 	 Process::removeCommandHandler( unsigned long CommandIdentifier )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface->removeCommandHandler( CommandIdentifier );
}


void 	Process::removeAllCommandHandler()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface->removeAllCommandHandler();
}


void 	Process::sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface->sendCommand( CommandIdentifier, Buffer, Timeout, Cancelable );
}


void 	Process::sendCommandNonblocking( unsigned long CommandIdentifier, CommandBuffer Buffer )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface->sendCommandNonblocking( CommandIdentifier, Buffer );
}


void	Process::handleOneCommand( unsigned long& CommandIdentifier )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	m_CommandInterface->handleOneCommand( CommandIdentifier );
}


void Process::attachLocalVc( const ASAAC_PublicId GlobalVcId, const ASAAC_PublicId LocalVcId )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		// process must be in STOPPED or INITIALIZED state
		ProcessStatus ThisState = getState();

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

			sendCommand( CMD_ATTACH_VC, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time());

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

		throw;
	}
}


void Process::detachLocalVc( const ASAAC_PublicId LocalVcId )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		// process must be in STOPPED or INITIALIZED state
		ProcessStatus ThisState = getState();

		//if (( ThisState != PROCESS_STOPPED ) &&
			//( ThisState != PROCESS_INITIALIZED ))
			//throw OSException("Process must be in state 'STOPPED' or 'INITIALIZED'", LOCATION);


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

			sendCommand( CMD_DETACH_VC, Data.ReturnBuffer, TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

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
}


void Process::detachAndDestroyAllLocalVcs()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		if (m_IsServer == false)
			throw OSException("This is not the server object", LOCATION);

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index ++ )
		{
			if ( m_LocalVCs[ Index ].GlobalVcId == OS_UNUSED_ID )
				continue;

			if ( m_LocalVCs[ Index ].LocalVcId == OS_UNUSED_ID )
				continue;

			LocalVc *LVc = CommunicationManager::getInstance()->getLocalVirtualChannel( getId(), m_LocalVCs[ Index ].GlobalVcId, m_LocalVCs[ Index ].LocalVcId );

			if (LVc == NULL)
				continue;

			LVc->remove();

			m_LocalVCs[ Index ].GlobalVcId   = OS_UNUSED_ID;
			m_LocalVCs[ Index ].LocalVcId    = OS_UNUSED_ID;
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error detaching and destropying all local vcs from process", LOCATION);

		throw;
	}
}


long Process::getAttachedVirtualChannelIndex( const ASAAC_PublicId LocalVcId )
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


LocalVc* Process::getAttachedVirtualChannel( const ASAAC_PublicId LocalVcId )
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
		throw;
	}

	return LVc;
}


bool Process::isAttachedTo( const ASAAC_PublicId LocalVcId )
{
    if (m_IsInitialized == false)
        throw UninitializedObjectException(LOCATION);

    if ( getAttachedVirtualChannelIndex( LocalVcId ) == -1 )
        return false;

    return true;
}


void Process::invokeOSScope(OSScopeFunction foo, OSScopeCommandBuffer param)
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( isOSScope() )
		{
			foo(param);
		}
		else
		{
			OSScopeCommandData Data;
			Data.Return = ASAAC_ERROR;

			try
			{
				Data.Scope.foo = foo;
				memcpy( Data.Scope.param, param, sizeof(OSScopeCommandBuffer) );

				sendCommand( CMD_INVOKE_OS_SCOPE, Data.ReturnBuffer, TimeStamp::Infinity().asaac_Time(), true);
			}
			catch (ASAAC_Exception &e)
			{
				e.raiseError();
			}

			if (Data.Return == ASAAC_ERROR)
				throw OSException("OSScopeFunction reported an error", LOCATION);
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error occured invoking a function in os scope", LOCATION);

		throw;
	}
}


SchedulingData  Process::getOSScopeSchedulingData()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	return m_ProcessData->OSScopeSchedulingData;
}



bool Process::isOSScope()
{
    if (m_IsInitialized == false)
        throw UninitializedObjectException(LOCATION);

    Thread *CurrentThread = NULL;
    NO_EXCEPTION( CurrentThread = getCurrentThread() );

	return (CurrentThread == NULL);
}


bool Process::isCurrentProcess()
{
	return m_IsServer;
}


ASAAC_PublicId Process::getId()
{
	if ( m_IsInitialized == false )
		return OS_UNUSED_ID;

	return m_ProcessData->Description.global_pid;
}


ASAAC_PublicId Process::getId( ProcessAlias Alias )
{
	switch (Alias)
	{
		case PROC_APOS: for (unsigned long id = OS_PROCESSID_APOS; id <= OS_PROCESSID_APOS_MAX; id++)
						{
							if (ProcessManager::getInstance()->getProcessIndex(id) == -1)
								return id;
						}
						break;

		case PROC_SMOS: for (unsigned long id = OS_PROCESSID_SMOS; id <= OS_PROCESSID_SMOS_MAX; id++)
						{
							if (ProcessManager::getInstance()->getProcessIndex(id) == -1)
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


ProcessAlias Process::getAlias()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

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


ProcessStatus Process::getState()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	return m_ProcessData->Status;
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


// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************


void Process::RunHandler( CommandBuffer Buffer )
{
	volatile ASAAC_ReturnStatus* Return = (ASAAC_ReturnStatus*)( Buffer );

	try
	{
		Process* ThisInstance = ProcessManager::getInstance()->getCurrentProcess();

		if ( ThisInstance->getState() != PROCESS_INITIALIZED )
		{
			// PROCESS_INITIALIZED means Process is still under control of the
			// Process Starter. The process Starter handles the starting itself.
			// Hence, do nothing, but only return success.
			ThisInstance->run();
		}

		*Return = ASAAC_SUCCESS;
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

		ThisInstance->stop();
		*Return = ASAAC_SUCCESS;

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

	// Nothing to do. Termination signal will be handled in main loop of 'run()'
	*Return = ASAAC_SUCCESS;
	return;
}



void Process::RequestPIDHandler( CommandBuffer Buffer )
{
	pid_t* Return = (pid_t*)( Buffer );

	*Return = getpid();

	return;
}



void Process::AttachLocalVcHandler( CommandBuffer Buffer )
{
	VCCommandData* Data = (VCCommandData*)Buffer;

	try
	{
		// get current process
		Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

		ThisProcess->attachLocalVc( Data->VC.GlobalVcId, Data->VC.LocalVcId );
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



void Process::DetachLocalVcHandler( CommandBuffer Buffer )
{
	VCCommandData* Data = (VCCommandData*)Buffer;

	try
	{
		// get current process
		Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

		ThisProcess->detachLocalVc( Data->VC.LocalVcId );
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

void Process::InvokeOSScopeHandler( CommandBuffer Buffer )
{
	OSScopeCommandData* Data = (OSScopeCommandData*)Buffer;

	try
	{
		Data->Scope.foo(Data->Scope.param);
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Caught exception invoking a function in os scope", LOCATION);
		e.raiseError();

		Data->Return = ASAAC_ERROR;
	}

	Data->Return = ASAAC_SUCCESS;
}


// *******************************************************************************************
//                              C A L L B A C K ' S
// *******************************************************************************************

void Process::SigChildCallback( void* Data )
{
	siginfo_t* SignalInfo = (siginfo_t*)Data;

	//Process has exited and returned his signal
	if ((SignalInfo->si_value.sival_int >= SIGRTMIN) && (m_Destroying == false))
		deinitialize();

	//Return value is "SUCCESS", so no more information is needed
	if (SignalInfo->si_value.sival_int == OS_SIGNAL_SUCCESS)
		return;

	CharSeq ErrorString;
	ErrorString << oal_signal_description(SignalInfo->si_value.sival_int);


	if (SignalInfo->si_value.sival_int == OS_SIGNAL_SUCCESS)
		ErrorString = "OS_SIGNAL_SUCCESS";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_TIMEOUT)
		ErrorString = "OS_SIGNAL_TIMEOUT";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_RESOURCE)
		ErrorString = "OS_SIGNAL_RESOURCE";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_ERROR)
		ErrorString = "OS_SIGNAL_ERROR";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_FATALERROR)
		ErrorString = "OS_SIGNAL_FATALERROR";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_KILL)
		ErrorString = "OS_SIGNAL_KILL";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_SUSPEND)
		ErrorString = "OS_SIGNAL_SUSPEND";

	if (SignalInfo->si_value.sival_int == OS_SIGNAL_RESUME)
		ErrorString = "OS_SIGNAL_RESUME";


	if (SignalInfo->si_errno != 0)
		ErrorString << ": " << strerror(SignalInfo->si_errno);

	CharSeq FunctionString;
	FunctionString << "Address: " << CharSeq((unsigned long)SignalInfo->si_addr, hexadecimal);

	long Line = 0;

	FatalException e( ErrorString.c_str(), FunctionString.c_str(), Line);

	e.setProcessId( getId() );
	e.setThreadId( OS_UNUSED_ID );

	CharSeq ErrorPathString;
	ErrorPathString << "Signal received by parent process " << ProcessManager::getInstance()->getCurrentProcessId();
	e.addPath( ErrorPathString.c_str(), LOCATION);

	e.raiseError();

	//TODO: Is this call needed? Reason?
	int Dummy;
	oal_waitpid( (pid_t)-1, &Dummy, 0 );
}
