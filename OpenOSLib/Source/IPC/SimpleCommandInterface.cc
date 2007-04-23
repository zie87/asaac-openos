#include "SimpleCommandInterface.hh"

#include "Exceptions/Exceptions.hh"

#include "IPC/ProtectedScope.hh"

#include "ProcessManagement/ProcessManager.hh"

using namespace std;


string CommandNames[13] = 
{ 
	"CMD_NULL_COMMAND", 
	"CMD_GET_PID", 
	"CMD_CREATE_PROCESS", 
	"CMD_RUN_PROCESS",
	"CMD_STOP_PROCESS", 
	"CMD_DESTROY_PROCESS", 
	"CMD_ATTACH_VC",
	"CMD_DETACH_VC", 
	"CMD_TERM_PROCESS", 
	"CMD_TERM_ENTITY", 
	"CMD_FLUSH_SESSION", 
	"CMD_ACTIVATE_ERROR_HANDLER", 
	"CMD_INVOKE_OS_SCOPE" 
};

SimpleCommandInterface::SimpleCommandInterface() : m_HandlerThreadRunning(false), m_IsInitialized(false)
{
}

SimpleCommandInterface::~SimpleCommandInterface()
{
	deinitialize();
}


void SimpleCommandInterface::initialize( Allocator* ThisAllocator, bool IsMaster, bool IsServer )
{
	if ( m_IsInitialized ) throw DoubleInitializationException(LOCATION);
	m_IsInitialized = true;
	
	m_CommandSemaphore.initialize( ThisAllocator, IsMaster );
	m_AdministrationSemaphore.initialize( ThisAllocator, IsMaster );

	m_SendReceiveEvent.initialize( ThisAllocator, IsMaster );
	
	if ( IsMaster )
	{
		m_SendReceiveEvent.resetEvent();
	}
	
	m_CommandData.initialize( ThisAllocator );

	for ( unsigned int Index = 0; Index < OS_MAX_COMMAND_HANDLERS; Index ++ )
	{
		m_Handler[ Index ].Identifier = 0;
	}

	m_IsMaster = IsMaster;
	m_IsServer = IsServer;
	
}


void SimpleCommandInterface::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	stopHandlerThread();
	
	m_CommandData.deinitialize();
	m_SendReceiveEvent.deinitialize();
	m_CommandSemaphore.deinitialize();
	m_AdministrationSemaphore.deinitialize();
	
	m_IsInitialized = false;
}
	

ASAAC_ReturnStatus SimpleCommandInterface::handleOneCommand( unsigned long& CommandIdentifier )
{
	ASAAC_ReturnStatus Status = ASAAC_SUCCESS;
	
	m_SendReceiveEvent.waitForEvent();
	
	if ( m_CommandData->Identifier != 0 )
	{
		ProtectedScope Access( "handle a command in SimpleCommandInterface", m_AdministrationSemaphore );
		
		CommandIdentifier = m_CommandData->Identifier;
		
		if ( m_CommandData->Identifier < 8 )
		{
#ifdef DEBUG_SCI
			cout << "Got Command: " << m_CommandData->Identifier << " (" << CommandNames[ m_CommandData->Identifier ] << ")" << endl;
#endif
		}
		
		CommandHandlerMapping *ThisCommand = getCommandHandler( m_CommandData->Identifier );
		
		if ( ThisCommand == 0 )
		{
			m_CommandData->Identifier = INVALID_HANDLER;
			Status = ASAAC_ERROR;
		}
		else
		{
			try
			{
				ThisCommand->Handler( m_CommandData->Buffer );
			}
			catch (ASAAC_Exception &e)
			{
				e.logMessage();
				Status = ASAAC_ERROR;
			}
			catch (...)
			{
				Status = ASAAC_ERROR;
			}
		}
#ifdef DEBUG_SCI		
		cout << "Returning : " << *(ASAAC_ReturnStatus*)m_CommandData->Buffer << endl;
#endif
	}

	m_SendReceiveEvent.resetEvent();
	return Status;
}
		

void* SimpleCommandInterface::HandlerThread(void* Param)
{
	SimpleCommandInterface *m_This = static_cast<SimpleCommandInterface*>(Param);

	unsigned long Command;

	for (;;)
	{
		m_This->handleOneCommand( Command );

#ifdef DEBUG_SCI
		cout << "Handler Thread received command: " << Command << endl;
#endif
		
		if ( Command == TERMINATE_HANDLER ) break;
	}				
		
	return 0;
}


void SimpleCommandInterface::TerminationHandler( CommandBuffer Buffer )
{
	oal_thread_exit(0);
}


void SimpleCommandInterface::startHandlerThread()
{
	if ( m_HandlerThreadRunning ) 
		return;
	
	addCommandHandler( TERMINATE_HANDLER, SimpleCommandInterface::TerminationHandler); 
	
	oal_thread_create( &m_HandlerThread, 0, SimpleCommandInterface::HandlerThread, this );
	
	m_HandlerThreadRunning = true;
}


void SimpleCommandInterface::stopHandlerThread()
{
	if ( m_HandlerThreadRunning == false) 
		return;
	
	ProtectedScope Access( "Stopping the handler thread in SimpleCommandInterface", m_CommandSemaphore );
	
	m_CommandData->Identifier = TERMINATE_HANDLER;
	
	m_SendReceiveEvent.setEvent();
	
	oal_thread_join( m_HandlerThread, 0 );
	
	removeCommandHandler( TERMINATE_HANDLER ); 
}


SimpleCommandInterface::CommandHandlerMapping* SimpleCommandInterface::getCommandHandler( unsigned long CommandIdentifier )
{
	for ( unsigned int Index = 0; Index < OS_MAX_COMMAND_HANDLERS; Index ++ )
	{
		if ( m_Handler[ Index ].Identifier == CommandIdentifier ) return &(m_Handler[ Index ]);
	}
	
	return 0;
}


ASAAC_ReturnStatus SimpleCommandInterface::addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler )
{
	ProtectedScope Access( "Adding a command handler to SimpleCommandInterface", m_AdministrationSemaphore );
	
	if ( CommandIdentifier >= INVALID_HANDLER ) 
		return ASAAC_ERROR; // Invalid Handler

	if ( CommandIdentifier == 0 ) 
		return ASAAC_ERROR; // Invalid Handler

	if ( getCommandHandler( CommandIdentifier ) != 0 ) 
		return ASAAC_ERROR; // Command already exists
	
	CommandHandlerMapping *FreeHandler = getCommandHandler( 0 );
	
	if ( FreeHandler == 0 ) 
		return ASAAC_ERROR;  // No free Handler slots

	FreeHandler->Identifier = CommandIdentifier;
	FreeHandler->Handler	= Handler;	
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus SimpleCommandInterface::removeCommandHandler( unsigned long CommandIdentifier )
{
	ProtectedScope Access( "Removing a command handler from SimpleCommandInterface", m_AdministrationSemaphore );
	
	CommandHandlerMapping *ThisHandler = getCommandHandler( CommandIdentifier );
	
	if ( ThisHandler == 0 ) 
		return ASAAC_ERROR;
	
	ThisHandler->Identifier = 0;
	ThisHandler->Handler	= 0;
	
	return ASAAC_SUCCESS;
}



ASAAC_TimedReturnStatus SimpleCommandInterface::sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	ASAAC_TimedReturnStatus WaitResult;
	
	CharacterSequence ErrorString;
	
	try
	{
		ProtectedScope Access( "Sending a command with SimpleCommandInterface", m_CommandSemaphore, Timeout, Cancelable );
		
		if ( Access.timedOut() )
			return ASAAC_TM_TIMEOUT;
		
		m_CommandData->Identifier = CommandIdentifier;
	
		memcpy( m_CommandData->Buffer, Buffer, OS_SIZE_OF_SIMPLE_COMMANDBUFFER );
	
		m_SendReceiveEvent.setEvent();
	
		WaitResult = m_SendReceiveEvent.waitForEventReset( Timeout );
	
		if ( WaitResult == ASAAC_TM_TIMEOUT ) 
			throw OSException( (ErrorString << "Timeout sending command: " << getCommandString(CommandIdentifier)).c_str(), LOCATION);
	
		if ( WaitResult == ASAAC_TM_ERROR ) 
			throw OSException( (ErrorString << "Error sending a command: " << getCommandString(CommandIdentifier)).c_str(), LOCATION);
	
		if ( m_CommandData->Identifier != CommandIdentifier )
		{ 
			WaitResult = ASAAC_TM_ERROR;
			throw OSException( (ErrorString << "Error receiving reply: false CommandIdentifier (sent: " 
				<< getCommandString(CommandIdentifier) << ", received: " << getCommandString(m_CommandData->Identifier) << ")").c_str(), LOCATION);
		}
		
		memcpy( Buffer, m_CommandData->Buffer, OS_SIZE_OF_SIMPLE_COMMANDBUFFER );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error sending a command with SimpleCommandInterface", LOCATION);
		e.logMessage();
		return WaitResult;
	}
	catch (...)
	{
		FatalException("Error sending a command via SimpleCommandInterface", LOCATION);
		return ASAAC_TM_ERROR;
	}
	
	return ASAAC_TM_SUCCESS;
}


size_t SimpleCommandInterface::predictSize()
{
	return ( 2 * Semaphore::predictSize() +
			 Event::predictSize() +
			 Shared<CommandData>::predictSize() );
}


ASAAC_CharacterSequence SimpleCommandInterface::getCommandString( unsigned long CommandIdentifier )
{
	static CharacterSequence Result;
	
	if ((CommandIdentifier > 0) && (CommandIdentifier < 13)) //TODO: replace this magic number
		Result = CommandNames[ CommandIdentifier ];
	else Result = "<Unknown Command>";
	
	return Result.asaac_str(); 
}


void SimpleCommandInterface::EmptyHandler( CommandBuffer Buffer )
{
	return;
}
