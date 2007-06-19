#include "OpenOS.hh"

using namespace std;

#include "Interfaces/OpenOS-Application.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/Process.hh"


int applicationMain( void )
{
	CharacterSequence Application;

	try
	{		
		OpenOS::getInstance()->initialize( LAS_PROCESS_RUNNING );
			
		Process* P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P == 0)
			throw FatalException("Current Process not available", LOCATION);

		Application << "    ASAAC Application (pid = " << CharSeq(P->getId()) << ") : "; 		

		//This call shall be implemented by application writer
		registerThreads();
		
		cout << Application << "started..." << endl;
	
	  	P->run();
	
		cout << Application << "stopped." << endl;
	
		OpenOS::getInstance()->deinitialize();
	}
	catch ( ASAAC_Exception& e )
	{
		e.raiseError();
	}
	catch (...)
	{
		CharSeq ErrorString;
		OSException( (ErrorString << Application << "Unknown Exception in MainLoop.").c_str(), LOCATION).raiseError();
	}

	return 0;	
}


