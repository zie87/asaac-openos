#include "OpenOS.hh"

using namespace std;

#include "Application/ApplicationStart.h"

#include "Managers/FileManager.hh"

#include "FaultManagement/ErrorHandler.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/Process.hh"


int applicationStart( char argc, char** argv )
{
	CharacterSequence ProcessPath;
	ASAAC_PublicId    CpuId     = OS_UNUSED_ID;
	ASAAC_PublicId    ProcessId = OS_UNUSED_ID;
	int               Handle    = 0;

	CharacterSequence ErrorString;
	
	try
	{
		if ( argc != 3 )
			throw OSException( (ErrorString << "Count of parameters is not equal to three (argc=" << argc << ")").c_str(), LOCATION);
			
		ProcessPath = CharSeq( argv[0] );
		CpuId 	    = CharSeq( argv[1] ).asaac_id();
		ProcessId   = CharSeq( argv[2] ).asaac_id();
	}
    catch ( ASAAC_Exception& e )
    {
    	e.addPath("ProcessStarter started with wrong parameter", LOCATION);
    	e.logMessage();
    	return 1;
    }
    catch (...)
    {
    	OSException("ProcessStarter started with wrong parameter", LOCATION).logMessage();
    	return 2;
    }
	
	try 
	{		
		OpenOS::getInstance()->initializeProcessStarter( CpuId, ProcessId );
		
		Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

		if (ThisProcess == 0)
			throw FatalException("Current Process not available", LOCATION);
		
		// Wait for commands until CMD_RUN_PROCESS is encountered
			for (;;)
			{
				unsigned long CommandId;
	
				ThisProcess->handleOneCommand( CommandId );
				
#ifdef DEBUG_SCI
				cout << "  ProcessStarter: received command: " << CommandId << endl;
#endif			
				
				if ( CommandId == CMD_TERM_PROCESS ) 
					throw OSException("Process reached signal to terminate", LOCATION);
				
				if ( CommandId == CMD_RUN_PROCESS ) break;
			}


#ifdef DEBUG		
		// check if a .sh file exists, then execute the .sh file
		// FOR DEBUG PURPOSE ONLY!!!
		CharSeq ShFile = ProcessPath;
		ShFile += ".sh";

		FILE *fp;
		if ((fp = fopen (ShFile.c_str (), "r")) != NULL)
		{
			fclose (fp);
	  		ASAAC_CharacterSequence Filename;
	  		FileManager::getInstance()->saveState( Filename );

			CharacterSequence LogMsg;
			LogMsg << "Start ClientProcess by Shell Script (DEBUG MODE): " << ShFile;
			ErrorHandler::getInstance()->logMessage(LogMsg.asaac_str(), ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE);

			//Because this process ends immediately, cleanup local objects first 
			OpenOS::getInstance()->deinitialize();
			
			// Execute the application. First parametter is publicId of process. Second parameter is the
			// handle required to restore the FileManager
			static char CommandBuffer[255];
	
			CharSeq Command = ShFile << " " << CpuId << " " << ProcessId << " 0 " << Filename;
			Command.convertTo(CommandBuffer);
			
			char *SArgv[4];
			SArgv[0] = "/bin/tcsh";
			SArgv[1] = "-c";
			SArgv[2] = CommandBuffer;
			SArgv[3] = 0;
			
			execv ("/bin/tcsh", SArgv);
			
			throw OSException( strerror(errno), LOCATION );	
		}
#endif

        char* ExecArgs[5];
        char** ExecEnv = environ;
        
        char ProcessPathBuffer[512];
        char CpuIdBuffer[16];
        char ProcessIdBuffer[16];
        char HandleBuffer[16];
        char ShellNameBuffer[1] = "";

		// Now, save state of the File Manager,
		// prepare handles to be handed over to application		
		FileManager::getInstance()->saveState( Handle );
	
		CharSeq(Handle).convertTo(HandleBuffer);
	
		// Execute the application.             
        ProcessPath.convertTo( ProcessPathBuffer );
        CharSeq(CpuId).convertTo( CpuIdBuffer );
        CharSeq(ProcessId).convertTo( ProcessIdBuffer );
                    
        ExecArgs[0] = CpuIdBuffer;
        ExecArgs[1] = ProcessIdBuffer;
        ExecArgs[2] = HandleBuffer;
        ExecArgs[3] = ShellNameBuffer;
        ExecArgs[4] = 0;
        
        // For APOS processes drop all privileges. set uid and gid == nobody
        if (OpenOS::getInstance()->isSMOSProcess( ProcessId ) == false)
        {
            setuid( 65534 );
            setgid( 65534 );
        } 

        CharacterSequence LogMsg;
        LogMsg << "Start ClientProcess: " << ProcessPath;
        ErrorHandler::getInstance()->logMessage(LogMsg.asaac_str(), ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE);
            
        //Because this process ends immediately, cleanup local objects first 
        OpenOS::getInstance()->deinitialize();

		execve( ProcessPathBuffer, ExecArgs, ExecEnv);
		
		throw OSException( strerror(errno), LOCATION );
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error in ProcessStarter", LOCATION);	
		e.logMessage();
	}
	catch (...)
	{
		OSException("Unknown Exception in ProcessStarter", LOCATION).logMessage();
	}

	return 0;
}

