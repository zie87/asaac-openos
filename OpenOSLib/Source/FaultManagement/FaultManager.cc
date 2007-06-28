#include "FaultManager.hh"

#include "Exceptions/Exceptions.hh"
#include "ProcessManagement/ProcessManager.hh"

FaultManager::FaultManager() : m_IsInitialized(false)
{
	
}

FaultManager::~FaultManager()
{

}


FaultManager* FaultManager::getInstance()
{
	static FaultManager ThisFaultManager;
	
	return &ThisFaultManager;
}


void FaultManager::initialize(bool IsMaster)
{
	if ( m_IsInitialized ) 
		return;

	m_IsMaster = IsMaster;

	m_IsInitialized = true;

	try
	{
		m_ErrorMessageQueue.initialize( IsMaster, OS_ERROR_QUEUE, CLIENTS_RECEIVE, OS_MAX_ERROR_QUEUESIZE, sizeof( ASAAC_ErrorInfo ) );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error initializing FaultManager", LOCATION);
		e.raiseError();
		deinitialize();
	}
	
	return;
}


void FaultManager::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	try
	{
		m_ErrorMessageQueue.deinitialize();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error deinitializing FaultManager", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
}


ASAAC_TimedReturnStatus		FaultManager::getError( ASAAC_ErrorInfo& error_info, const ASAAC_TimeInterval& time_out )
{
	if ( m_IsInitialized == false ) 
		return ASAAC_TM_ERROR;

	try
	{
		unsigned long ActualSize;
		ASAAC_TimedReturnStatus Result;
		
		ASAAC_Time timeOut = TimeStamp(time_out).asaac_Time();
	
		m_ErrorMessageQueue.receiveMessage( &error_info, sizeof( ASAAC_ErrorInfo ), ActualSize, timeOut );
		
		if ( ActualSize != sizeof( ASAAC_ErrorInfo ) ) 
			throw OSException("", LOCATION);
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving error", LOCATION);
		e.raiseError();
				
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
	}
	
	return ASAAC_TM_SUCCESS;
}

	
ASAAC_TimedReturnStatus		FaultManager::activateErrorHandler( ASAAC_PublicId process_id, 
													ASAAC_PublicId faulty_thread_id,
													ASAAC_ErrorType error_type,
													ASAAC_ErrorCode error_code,
													const ASAAC_CharacterSequence& error_message,
													ASAAC_ReturnStatus& error_handler_status,
													const ASAAC_TimeInterval& Timeout )
{
	Process* TargetProcess = ProcessManager::getInstance()->getProcess( process_id );
	
	if ( TargetProcess == 0 ) 
		return ASAAC_TM_ERROR;
	
	// if no error handler thread is defined, return error
	Thread* ErrorHandler = TargetProcess->getThread( 0 );
	if ( ErrorHandler == 0 )
	{
		cout << "Client has no error handler defined..." << endl;
		return ASAAC_TM_ERROR;
	}

	// if faulty thread does not exist, return error
	if ( TargetProcess->getThread( faulty_thread_id ) == 0 ) 
		return ASAAC_TM_ERROR;

	if (( error_type > ASAAC_FATAL_ERROR ) ||
	    ( error_type < ASAAC_APPLICATION_ERROR ))
	    return ASAAC_TM_ERROR;
	
	union {
		struct  {
			ASAAC_ErrorInfo	error;
			ASAAC_TimeInterval timeout;
		} send;
		
		ASAAC_ReturnStatus	Result;
		CommandBuffer Buffer;
	} Data;
	
	
	Data.send.error.process_id = process_id;
	Data.send.error.thread_id  = faulty_thread_id;
	Data.send.error.error_type = error_type;
	Data.send.error.error_code = error_code;
	Data.send.error.error_message = error_message;
	
	Data.send.timeout = Timeout;
	
	ASAAC_TimedReturnStatus CommandResult = TargetProcess->sendCommand(CMD_ACTIVATE_ERROR_HANDLER, Data.Buffer, TimeStamp(Timeout).asaac_Time() );
	
	error_handler_status = Data.Result;
	
	return CommandResult;
}

