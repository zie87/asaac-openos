#include "OpenOS.hh"

using namespace std;

#include "Interfaces/OpenOS-Application.hh"

#include "Managers/FileManager.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/Process.hh"


ASAAC_CharacterSequence MainThreadName = { 10, "MainThread" };


int applicationMain( char argc, char** argv )
{
	ASAAC_PublicId CpuId 		= OS_UNUSED_ID;
	ASAAC_PublicId ProcessId 	= OS_UNUSED_ID;
	unsigned long Handle 		= 0;
	CharacterSequence ShellName = "";
			
	CharacterSequence ErrorString;
	
	try
	{
	    if ( argc == 4 )
	    {		
			CpuId 		= CharSeq( argv[0] ).asaac_id();
			ProcessId 	= CharSeq( argv[1] ).asaac_id();
			Handle 		= CharSeq( argv[2] ).c_int();		
			ShellName 	= CharSeq( argv[3] );		
	    }
	    else 
	    	throw OSException( (ErrorString << "Count of parameters is not equal to four (argc=" << argc << ")").c_str(), LOCATION);
	}
    catch ( ASAAC_Exception& e )
    {
    	e.addPath("ASAAC Application started with wrong parameter", LOCATION);
    	e.logMessage();
    	return 1;
    }
    catch (...)
    {
    	OSException("ASAAC Application started with wrong parameter", LOCATION).logMessage();
    	return 2;
    }
	
	CharacterSequence Application;
	Application << "    ASAAC Application (pid = " << CharSeq(ProcessId) << ") : "; 
	
	CharacterSequence CouldNotStart;
	CouldNotStart << Application << "Application could not properly be started: ";

	try
	{		
		if (ShellName == "")
		{
			if ( FileManager::getInstance()->restoreStateByHandle( Handle ) != ASAAC_SUCCESS )
				throw OSException( (ErrorString << CouldNotStart << "File Manager could not load initial state.").c_str(), LOCATION);
		}
		else
		{
		    if ( FileManager::getInstance()->restoreStateByName( ShellName.asaac_str() ) != ASAAC_SUCCESS )
				throw OSException( (ErrorString << CouldNotStart << "File Manager could not load initial state.").c_str(), LOCATION);
				
		    FileManager::getInstance()->reopenFile();
		}	
	
		OpenOS::getInstance()->initializeProcess( CpuId, ProcessId );
			
		Process* P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P == 0)
			throw FatalException("Current Process not available", LOCATION);

		//This call shall be implemented by application writer
		registerThreads();
		
		cout << Application.c_str() << "started..." << endl;
	
	  	P->run();
	
		cout << Application.c_str() << "stopped." << endl;
	
		// Close all files in File Manager.
		//FileManager::getInstance()->closeAllFiles();
		
		OpenOS::getInstance()->deinitialize();
	}
	catch ( ASAAC_Exception& e )
	{
		e.logMessage();
	}
	catch (...)
	{
		CharSeq ErrorString;
		OSException( (ErrorString << Application.c_str() << "Unknown Exception in MainLoop.").c_str(), LOCATION).logMessage();
	}

	return 0;	
}


