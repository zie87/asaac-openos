#include "OpenOS.hh"
#include "OpenOSConstants.hh"
#include "Wrapper/SMOSWrapper.hh"
#include "Common/Aid.h"

#include <iostream>

using namespace std;

ASAAC_APPLICATION

const ASAAC_TimeInterval TimeIntervalInfinity = {2147483647, 999999999};

ASAAC_THREAD( MainThread )
{
	cout << "GSM MainThread has been started..." << endl;		

	ASAAC_APOS_startThread(2);
	ASAAC_APOS_startThread(3);
	
	//Creating two example processes communicating with each other...
	createProcess( "SendHelloWorldApplication/Debug/SendHelloWorldApplication", 1, 0, ASAAC_LOCAL_ACCESS);
	createProcess( "ReceiveHelloWorldApplication/Debug/ReceiveHelloWorldApplication", 2, 0, ASAAC_LOCAL_ACCESS);

	createVirtualChannel(1);

	createVcMapping( 1, 1, 1, ASAAC_BOOL_FALSE );
	createVcMapping( 2, 1, 1, ASAAC_BOOL_TRUE );
	
	startProcess(1);
	startProcess(2);
	
	cout << "Hello World system is now running for 10 seconds"; fflush(stdout);
	for(int i = 0; i < 5; ++i)
	{
		cout << "."; sleep(1); fflush(stdout);		
	}	
	cout << endl;

	cout << "GSM Application stops now..." << endl;		
	ASAAC_SMOS_destroyProcess(OS_PROCESSID_MASTER);

	return 0;
}



ASAAC_THREAD( ErrorHandlerThread )
{
	cout << "GSM ErrorHandlerThread has been started..." << endl;		

	for (;;)
	{	
		ASAAC_ErrorInfo info;
		ASAAC_TimeInterval timeout = TimeIntervalInfinity;
		ASAAC_TimedReturnStatus status;
  
		status = ASAAC_SMOS_getError(&info, &timeout);

		if (status == ASAAC_TM_SUCCESS)
		{
			ASAAC_SMOS_writeLog(ASAAC_LOG_MESSAGE_TYPE_ERROR, 0, &info.error_message);
		}
	}

    return 0;    
};



ASAAC_THREAD( LoggingHandlerThread )
{
	cout << "GSM LoggingHandlerThread has been started..." << endl;		

	for (;;)
	{	
		ASAAC_CharacterSequence log_message; 
		ASAAC_LogMessageType message_type; 
		ASAAC_PublicId process_id; 
		ASAAC_TimeInterval timeout = TimeIntervalInfinity;
		ASAAC_TimedReturnStatus status;
		
		status = ASAAC_SMOS_getLogReport(&log_message, &message_type, &process_id, &timeout);

		if (status == ASAAC_TM_SUCCESS)
		{
			ASAAC_SMOS_writeLog(message_type, 0, &log_message);
		}
	}

    return 0;    
};

