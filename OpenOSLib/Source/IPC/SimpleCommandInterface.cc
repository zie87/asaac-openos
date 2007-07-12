#include "SimpleCommandInterface.hh"

#include "Exceptions/Exceptions.hh"

#include "IPC/ProtectedScope.hh"

#include "ProcessManagement/ProcessManager.hh"

using namespace std;

#define SIZE_OF_COMMAND_NAME_ARRAY 13

string CommandNames[SIZE_OF_COMMAND_NAME_ARRAY] = 
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


void SimpleCommandInterface::initialize( Allocator* ThisAllocator, bool IsMaster )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException(LOCATION);
	
	m_IsInitialized = true;
	
	m_CommandSemaphore.initialize( ThisAllocator, IsMaster );
	m_AdministrationSemaphore.initialize( ThisAllocator, IsMaster );

	m_SendReceiveEvent.initialize( ThisAllocator, IsMaster );
	
	if ( IsMaster )
	{
		m_SendReceiveEvent.resetEvent();
	}
	
	m_CommandData.initialize( ThisAllocator );

	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_COMMAND_HANDLERS; Index ++ )
	{
		m_Handler[ Index ].Identifier = OS_UNUSED_ID;
		m_Handler[ Index ].Handler = NULL;
	}

	m_IsMaster = IsMaster;
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
	

void SimpleCommandInterface::handleOneCommand( ASAAC_PublicId& CommandIdentifier )
{
	try
	{
		m_SendReceiveEvent.waitForEvent();
		
		if ( m_CommandData->Identifier != OS_UNUSED_ID )
		{
			ProtectedScope Access( "handle a command in SimpleCommandInterface", m_AdministrationSemaphore );
			
			CommandIdentifier = m_CommandData->Identifier;
			
#ifdef DEBUG_SCI
			cout << "Process " << ProcessManager::getInstance()->getCurrentProcess()->getId() << " got Command: " << m_CommandData->Identifier << " (" << CharSeq(getCommandString(m_CommandData->Identifier)) << ")" << endl;
#endif
			
			CommandHandlerMapping *ThisCommand = getCommandHandler( m_CommandData->Identifier );
			
			if ( ThisCommand == NULL )
				m_CommandData->Identifier = INVALID_HANDLER;
			else ThisCommand->Handler( m_CommandData->Buffer );
			
#ifdef DEBUG_SCI		
			cout << "Returning Data: " << *(ASAAC_ReturnStatus*)m_CommandData->Buffer << endl;
			cout << "Returning Command: " << m_CommandData->Identifier << " (" << CharSeq(getCommandString(m_CommandData->Identifier)) << ")" << endl;
#endif
		}

		m_SendReceiveEvent.resetEvent();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error handling a command", LOCATION);

		m_SendReceiveEvent.resetEvent();
		
		throw;
	}
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
		
		if ( Command == TERMINATE_HANDLER ) 
			break;
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
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_COMMAND_HANDLERS; Index ++ )
	{
		if ( m_Handler[ Index ].Identifier == CommandIdentifier ) 
			return &(m_Handler[ Index ]);
	}
	
	return NULL;
}


void SimpleCommandInterface::addCommandHandler( ASAAC_PublicId CommandIdentifier, CommandHandler Handler )
{
	try
	{
		ProtectedScope Access( "Adding a command handler to SimpleCommandInterface", m_AdministrationSemaphore );
		
		if ( Handler == NULL ) 
			throw OSException("Invalid Handler (NULL)", LOCATION);
	
		if ( CommandIdentifier >= INVALID_HANDLER ) 
			throw OSException("Invalid Handler", LOCATION);
	
		if ( CommandIdentifier == OS_UNUSED_ID ) 
			throw OSException("Invalid Handler", LOCATION);
	
		if ( getCommandHandler( CommandIdentifier ) != 0 ) 
			throw OSException("Command already exists", LOCATION);
		
		CommandHandlerMapping *FreeHandler = getCommandHandler( OS_UNUSED_ID );
		
		if ( FreeHandler == NULL ) 
			throw OSException("No free Handler slots", LOCATION);
	
		FreeHandler->Identifier = CommandIdentifier;
		FreeHandler->Handler	= Handler;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error adding command handler", LOCATION);
		
		throw;
	}
}


void SimpleCommandInterface::removeCommandHandler( ASAAC_PublicId CommandIdentifier )
{
	cout << "##################" << endl;
	
	try
	{
		ProtectedScope Access( "Removing a command handler from SimpleCommandInterface", m_AdministrationSemaphore );
		
		CommandHandlerMapping *ThisHandler = getCommandHandler( CommandIdentifier );
		
		if ( ThisHandler == NULL ) 
			throw OSException("CommandHandler with dedicated CommandIdentifier not found", LOCATION);
		
		ThisHandler->Identifier = OS_UNUSED_ID;
		ThisHandler->Handler	= NULL;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error removing command handler", LOCATION);
		
		throw;
	}
}


void SimpleCommandInterface::removeAllCommandHandler()
{
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_COMMAND_HANDLERS; Index ++ )
	{
		m_Handler[ Index ].Identifier = OS_UNUSED_ID;
		m_Handler[ Index ].Handler = NULL;
	}
}


void SimpleCommandInterface::sendCommandNonblocking( ASAAC_PublicId CommandIdentifier, CommandBuffer Buffer )
{
	try
	{
		ProtectedScope Access( "Sending a command with SimpleCommandInterface", m_CommandSemaphore );
		
		m_SendReceiveEvent.waitForEventReset( TimeStamp(OS_SIMPLE_COMMAND_TIMEOUT).asaac_Time() );

		m_CommandData->Identifier = CommandIdentifier;
	
		memcpy( m_CommandData->Buffer, Buffer, OS_SIZE_OF_SIMPLE_COMMANDBUFFER );
	
		m_SendReceiveEvent.setEvent();	
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error sending a command with SimpleCommandInterface", LOCATION);

		throw;
	}
}


void SimpleCommandInterface::sendCommand( ASAAC_PublicId CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout, bool Cancelable )
{
	CharacterSequence ErrorString;
	
	try
	{
		ProtectedScope Access( "Sending a command with SimpleCommandInterface", m_CommandSemaphore, Timeout, Cancelable );

		m_SendReceiveEvent.waitForEventReset( Timeout );
		
		m_CommandData->Identifier = CommandIdentifier;
		memcpy( m_CommandData->Buffer, Buffer, OS_SIZE_OF_SIMPLE_COMMANDBUFFER );
	
		m_SendReceiveEvent.setEvent();
	
		m_SendReceiveEvent.waitForEventReset( Timeout );
	
#ifdef DEBUG_SCI		
		cout << "Returned Data: " << *(ASAAC_ReturnStatus*)m_CommandData->Buffer << endl;
		cout << "Returned Command: " << m_CommandData->Identifier << " (" << CharSeq(getCommandString(m_CommandData->Identifier)) << ")" << endl;
#endif
		if ( m_CommandData->Identifier == INVALID_HANDLER )
			throw OSException("Receiving process returned it couldn't handle the message", LOCATION);

		if ( m_CommandData->Identifier != CommandIdentifier )
			throw OSException( (ErrorString << "Error receiving reply: false CommandIdentifier (sent: " 
				<< getCommandString(CommandIdentifier) << ", received: " << getCommandString(m_CommandData->Identifier) << ")").c_str(), LOCATION);
		
		memcpy( Buffer, m_CommandData->Buffer, OS_SIZE_OF_SIMPLE_COMMANDBUFFER );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error sending a command with SimpleCommandInterface", LOCATION);
		
		throw;
	}
}


size_t SimpleCommandInterface::predictSize()
{
	return ( 2 * Semaphore::predictSize() +
			 Event::predictSize() +
			 Shared<CommandData>::predictSize() );
}


ASAAC_CharacterSequence SimpleCommandInterface::getCommandString( ASAAC_PublicId CommandIdentifier )
{
	static CharacterSequence Result;
	
	if ((CommandIdentifier > 0) && (CommandIdentifier < SIZE_OF_COMMAND_NAME_ARRAY)) 
		Result = CommandNames[ CommandIdentifier ];
	else Result = "<Unknown Command>";
	
	return Result.asaac_str(); 
}


void SimpleCommandInterface::EmptyHandler( CommandBuffer Buffer )
{
	return;
}
