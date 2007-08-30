#include "ErrorHandler.hh"

#include "FaultManagement/LogReportData.hh"

#include "Managers/TimeManager.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"
#include "ProcessManagement/Process.hh"
#include "ProcessManagement/Thread.hh"

#include "Exceptions/Exceptions.hh"


ErrorHandler::ErrorHandler() : m_IsInitialized(false)
{
	
}

ErrorHandler::~ErrorHandler()
{

}

ErrorHandler* ErrorHandler::getInstance()
{
	static ErrorHandler ThisErrorHandler;
	
	return &ThisErrorHandler;
}


void ErrorHandler::initialize()
{
	if ( m_IsInitialized ) 
		return;

	try
	{
		m_IsInitialized = true;	

		Process* CurrentProcess = ProcessManager::getInstance()->getCurrentProcess();

		if ( CurrentProcess == NULL ) 
			throw OSException("ProcessManager needs to be initialized before ErrorHandler.", LOCATION );
		
		m_HaveNewErrorInformation = false;
		m_HaveErrorInformation = false;
		
		CurrentProcess->addCommandHandler( CMD_ACTIVATE_ERROR_HANDLER, ErrorHandler::ActivateErrorHandler );

		m_ErrorMessageQueue.initialize( false, OS_ERROR_QUEUE, CLIENTS_SEND );			
		m_LoggingMessageQueue.initialize( false, OS_LOGGING_QUEUE, CLIENTS_SEND );
		
		m_LocalAllocator.initialize( Trigger::predictSize() );
		m_ErrorHandlerTrigger.initialize( &m_LocalAllocator, true );
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error initializing ErrorHandler", LOCATION);
		deinitialize();
		
		throw;
	}
		
}


void ErrorHandler::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	m_IsInitialized = false;
	
	try
	{
		m_ErrorHandlerTrigger.deinitialize();
		m_LocalAllocator.deinitialize();
		
		//m_ErrorMessageQueue.close();
		//m_LoggingMessageQueue.close();
		
		ProcessManager::getInstance()->getCurrentProcess()->removeCommandHandler( CMD_ACTIVATE_ERROR_HANDLER );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error deinitializing ErrorHandler", LOCATION);
		e.raiseError();
	}
}


ASAAC_ReturnStatus	ErrorHandler::raiseError( 
	ASAAC_ErrorCode error_code, 
	const ASAAC_CharacterSequence& error_message, 
	ASAAC_ErrorType error_type, 
	ASAAC_Address location,
	ASAAC_PublicId vc_id, 
	ASAAC_PublicId tc_id,
	ASAAC_NetworkDescriptor network)
{
	if ( m_IsInitialized == false ) 
		return ASAAC_ERROR;

	static bool Executing = false;
	if ( Executing == true ) return ASAAC_ERROR;
	Executing = true;
		
	try
	{
		ASAAC_Time absolute_local_time = TimeZero;
		ASAAC_Time absolute_global_time = TimeZero;
		ASAAC_Time relative_local_time = TimeZero;
		
		TimeManager::getAbsoluteLocalTime( absolute_local_time );
		TimeManager::getAbsoluteGlobalTime( absolute_global_time );
		TimeManager::getRelativeLocalTime( relative_local_time );
	
		ASAAC_PublicId process_id = ProcessManager::getInstance()->getCurrentProcessId();
		ASAAC_PublicId thread_id = ThreadManager::getInstance()->getCurrentThreadId();
		
		m_ErrorInformation.error_code           = error_code;
		m_ErrorInformation.error_message        = error_message;
		m_ErrorInformation.absolute_global_time = absolute_global_time;
		m_ErrorInformation.absolute_local_time  = absolute_local_time;
		m_ErrorInformation.relative_local_time  = relative_local_time;
		m_ErrorInformation.error_type           = error_type;
		m_ErrorInformation.process_id		    = process_id;
		m_ErrorInformation.thread_id		    = thread_id;
	
		m_ErrorInformation.cfm_id               = 0; //TODO: where is this information stored?
		m_ErrorInformation.pe_id                = 0; //TODO: where is this information stored?
		
		m_ErrorInformation.vc_id                = vc_id;
		m_ErrorInformation.tc_id                = tc_id;
		m_ErrorInformation.network              = network;
		m_ErrorInformation.location             = location;
		
		m_HaveNewErrorInformation				= true;
		m_HaveErrorInformation					= true;
			
		// TODO: check whether timeout should be set to TimeInstant rather (non-blocking) (jbm)
		// TODO: In case of timeout try to send the information later. Store it in a local queue. 
		// TODO: In case of non initialisation store it in a queue too.
		//m_ErrorMessageQueue.sendMessage( &m_ErrorInformation, sizeof( m_ErrorInformation ), TimeInfinity );
		m_ErrorMessageQueue.sendMessage( &m_ErrorInformation, sizeof( m_ErrorInformation ), TimeStamp::Instant().asaac_Time() );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error couldn't be raised", LOCATION);
		
		e.raiseError();
		
		Executing = false;
		return ASAAC_ERROR;
	}
	
	Executing = false;
	return ASAAC_SUCCESS;
}

				
ASAAC_ReturnStatus ErrorHandler::getErrorInformation( ASAAC_PublicId& faulty_thread_id,
											ASAAC_ErrorType& error_type,
											ASAAC_ErrorCode& error_code,
											ASAAC_CharacterSequence& error_message )
{
	if (m_HaveErrorInformation == false) 
		return ASAAC_ERROR;
	
	faulty_thread_id = m_ErrorInformation.thread_id;
	error_type       = m_ErrorInformation.error_type;
	error_code       = m_ErrorInformation.error_code;
	error_message    = m_ErrorInformation.error_message;
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ErrorHandler::getDebugErrorInformation( 
										ASAAC_ErrorType& error_type,
										ASAAC_ErrorCode& error_code,
										ASAAC_CharacterSequence& error_message )
{
	if (m_HaveNewErrorInformation == false)
		return ASAAC_ERROR;
	
	error_type       = m_ErrorInformation.error_type;
	error_code       = m_ErrorInformation.error_code;
	error_message    = m_ErrorInformation.error_message;
	
	m_HaveNewErrorInformation = false;
	
	return ASAAC_SUCCESS;
}										


void ErrorHandler::terminateErrorHandler( ASAAC_ReturnStatus return_status )
{
	try
	{
		//TODO: This shall be a callback by the end/termination of an error handler thread
		
		m_ErrorHandlerReturnStatus = return_status;
		
		m_ErrorHandlerTrigger.trigger();
		
		ThreadManager::getInstance()->getCurrentThread()->terminateSelf();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error terminating error handler thread", LOCATION);
		
		throw;
	}
}


ASAAC_ReturnStatus ErrorHandler::logMessage( const ASAAC_CharacterSequence& log_message, ASAAC_LogMessageType message_type )
{
	static bool Executing = false;
	
	if ( Executing == true ) 
		return ASAAC_ERROR;
	
	Executing = true;
	
	try
	{	
		LogReportData ThisReportData;
		ThisReportData.time = TimeStamp::Now().asaac_Time();	
	
		ASAAC_PublicId authentication_code = 0;
		ASAAC_PublicId process_id = ProcessManager::getInstance()->getCurrentProcessId();
		ASAAC_PublicId thread_id = ThreadManager::getInstance()->getCurrentThreadId();

		Process *ThisProcess = NULL;
		NO_EXCEPTION( ThisProcess = ProcessManager::getInstance()->getCurrentProcess() );
		
		if (ThisProcess != NULL)
			authentication_code = ThisProcess->getAuthenticationCode();
	
		ThisReportData.log_message = log_message;
		ThisReportData.message_type = message_type;
	
		ThisReportData.process_authentication_code = authentication_code;
		ThisReportData.process_id = process_id;
		ThisReportData.thread_id = thread_id;
	
#ifdef DEBUG
		CharSeq csTime = TimeStamp::Now().asaac_Time();
		CharSeq csMessageType = "";
		CharSeq csProcess = "";
		CharSeq csThread = "";
		CharSeq csMessage = log_message;
		
		if (process_id != OS_UNUSED_ID)
			csProcess << " PID:" << CharSeq(process_id);
		
		if (thread_id != OS_UNUSED_ID)
			csThread << " TID:" << CharSeq(thread_id);
		
		switch (message_type)
		{
			case ASAAC_LOG_MESSAGE_TYPE_ERROR: 			csMessageType << " ASAAC_LOG_MESSAGE_TYPE_ERROR"; break;
	  		case ASAAC_LOG_MESSAGE_TYPE_APPLICATION: 	csMessageType << " ASAAC_LOG_MESSAGE_TYPE_APPLICATION"; break;
	  		case ASAAC_LOG_MESSAGE_TYPE_GSM: 			csMessageType << " ASAAC_LOG_MESSAGE_TYPE_GSM"; break;
	  		case ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE: 	csMessageType << " ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE"; break;
	  		default: csMessageType << " Unknown Message Type";
		}
		cout << csTime.c_str() << csProcess.c_str() << csThread.c_str() << csMessageType.c_str() << ": '" << csMessage.c_str() << "'" << endl;
#endif
	
		if ( m_IsInitialized == false ) 
			throw UninitializedObjectException( LOCATION );
	
		// TODO: check whether timeout should be set to TimeInstant rather (non-blocking)
		// TODO: In case of timeout try to send the information later. Store it in a local queue. 
		//m_LoggingMessageQueue.sendMessage( &ThisReportData, sizeof( LogReportData ), TimeInfinity );
		m_LoggingMessageQueue.sendMessage( &ThisReportData, sizeof( LogReportData ), TimeStamp::Instant().asaac_Time() );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Message couldn't be logged", LOCATION);
		
		e.raiseError();
		
		Executing = false;
		return ASAAC_ERROR;
	}
	
	Executing = false;		
	return ASAAC_SUCCESS;
}
	


// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************

	
void ErrorHandler::ActivateErrorHandler( CommandBuffer Buffer )
{
	try
	{
		union DataType {
			struct {
				ASAAC_ErrorInfo		error_info;
				ASAAC_TimeInterval	Timeout;
			} send;
			
			ASAAC_ReturnStatus	Result;
		};
		
		DataType* Data = (DataType*)Buffer;
		
		ErrorHandler* This = ErrorHandler::getInstance();
		
		This->m_ErrorInformation = Data->send.error_info;
		This->m_HaveNewErrorInformation = true;
		This->m_HaveErrorInformation = true;
		
		Data->Result = ASAAC_ERROR;
		
		Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
		
		if ( ThisProcess == NULL ) 
			return;
		
		Thread* ErrorThread = ThisProcess->getThread( 0 );
		
		if ( ErrorThread == NULL ) 
		{
			cout << "No error handler thread found." << endl;
			return;
		}
	
		unsigned long CurrentTriggerState = This->m_ErrorHandlerTrigger.getTriggerState();
	
		ErrorThread->start();
		
		This->m_ErrorHandlerTrigger.waitForTrigger( CurrentTriggerState, Data->send.Timeout );
		
		Data->Result = This->m_ErrorHandlerReturnStatus;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error activating error handler", LOCATION);
		
		e.raiseError();
	}
}


