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
		e.addPath("Caught exception in main loop of application.", LOCATION);
		e.printMessage();
		
		return 1;
	}
	catch (...)
	{
		FatalException( "Caught exception in main loop of application.", LOCATION).printMessage();
		
		return 2;
	}

	return 0;	
}


bool registerThread(char * name, EntryPointAddr address)
{
	try
	{
		ASAAC_CharacterSequence threadName = CharacterSequence(name).asaac_str();
		ProcessManager::getInstance()->addEntryPoint( threadName, address );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error registering thread", LOCATION);
		e.raiseError();
		
		return false;
	}
	
	return true;
}
