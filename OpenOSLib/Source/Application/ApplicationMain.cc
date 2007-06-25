#include "OpenOS.hh"

using namespace std;

#include "Interfaces/OpenOS-Application.hh"

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
		e.raiseError();
	}
	catch (...)
	{
		OSException( "Unknown exception in main loop of application.", LOCATION).raiseError();
	}

	return 0;	
}


