#include "OpenOSObject.hh"
#include "OpenOSApplication.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/Process.hh"


int applicationMain( void )
{
	try
	{		
		OpenOS::getInstance()->initialize( false, LAS_PROCESS_RUNNING );
			
		//This call shall be implemented by application writer
		registerThreads();

	  	ProcessManager::getInstance()->getCurrentProcess()->run();
		OpenOS::getInstance()->deinitialize();
	}
	catch ( ASAAC_Exception& e )
	{
		OpenOS::getInstance()->deinitialize();

		e.addPath("Caught exception in main loop of application.", LOCATION);
		e.printErrorMessage();
		
		if (e.isTimeout())
			return OS_SIGNAL_TIMEOUT;
		
		if (e.isResource())
			return OS_SIGNAL_RESOURCE;
		
		return OS_SIGNAL_ERROR;
	}

	return OS_SIGNAL_SUCCESS;	
}


void registerThread(char * name, EntryPointAddr address)
{
	try
	{
		ASAAC_CharacterSequence threadName = CharacterSequence(name).asaac_str();
		ProcessManager::getInstance()->getCurrentProcess()->addEntryPoint( threadName, address );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error registering thread", LOCATION);
		e.raiseError();
	}
}


// *******************************************************************************************
//                   B U F F E R     H A N D L I N G
// *******************************************************************************************

typedef struct {
	char *  		Name;
	EntryPointAddr	Address;	
} BufferedEntryPoint;

static BufferedEntryPoint BufferedEntryPoints[OS_MAX_NUMBER_OF_THREADS];
static unsigned long 	  BufferedEntryPointCounter = 0;


char bufferThread(char * name, EntryPointAddr address)
{
	try
	{
		if (BufferedEntryPointCounter == OS_MAX_NUMBER_OF_THREADS)
			throw OSException("No more free slots", LOCATION);

		BufferedEntryPoints[BufferedEntryPointCounter].Name = name;
		BufferedEntryPoints[BufferedEntryPointCounter].Address = address;
		
		BufferedEntryPointCounter++;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error registering thread", LOCATION);
		e.printErrorMessage();
		
		return false;
	}
	
	return true;
}


void registerBufferedThreads()
{
	for (unsigned long Index = 0; Index < BufferedEntryPointCounter; Index++)
	{
		registerThread( BufferedEntryPoints[Index].Name, BufferedEntryPoints[Index].Address);
	}
}
