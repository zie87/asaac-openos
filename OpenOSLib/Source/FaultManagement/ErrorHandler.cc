#include "ErrorHandler.hh"

#include "FaultManagement/LogReportData.hh"

#include "Managers/TimeManager.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/Process.hh"
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

	Process* CurrentProcess = ProcessManager::getInstance()->getCurrentProcess();

	if ( CurrentProcess == 0 ) 
		throw OSException("Process Manager needs to be initialized before ErrorHandler.", LOCATION );
	
	try
	{
		m_IsInitialized = true;	
		
		m_HaveNewErrorInformation = false;
		m_HaveErrorInformation = false;
		
		CurrentProcess->addCommandHandler( CMD_ACTIVATE_ERROR_HANDLER, ErrorHandler::activateErrorHandler );

		if (m_ErrorMessageQueue.open( OS_ERROR_QUEUE, CLIENTS_SEND ) == ASAAC_ERROR)
			throw OSException("Unable to connect to error queue", LOCATION);
			
		if (m_LoggingMessageQueue.open( OS_LOGGING_QUEUE, CLIENTS_SEND ) == ASAAC_ERROR)
			throw OSException("Unable to connect to logging queue", LOCATION);
		
		m_LocalAllocator.initialize( Trigger::predictSize() );
		m_ErrorHandlerTrigger.initialize( &m_LocalAllocator, true );
	}
	catch ( ASAAC_Exception& E )
	{
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
		
		Process *P = ProcessManager::getInstance()->getCurrentProcess();
		
		if (P != 0)		
			P->removeCommandHandler( CMD_ACTIVATE_ERROR_HANDLER );
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
	
	Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	Thread *ThisThread = ProcessManager::getInstance()->getCurrentThread();

		
	ASAAC_Time absolute_local_time;
	ASAAC_Time absolute_global_time;
	ASAAC_Time relative_local_time;
	
	if (TimeManager::getAbsoluteLocalTime( absolute_local_time ) == ASAAC_ERROR)
		absolute_local_time = TimeZero;

	if (TimeManager::getAbsoluteGlobalTime( absolute_global_time ) == ASAAC_ERROR)
		absolute_local_time = TimeZero;

	if (TimeManager::getRelativeLocalTime( relative_local_time ) == ASAAC_ERROR)
		absolute_local_time = TimeZero;


	ASAAC_PublicId process_id = 0;
	ASAAC_PublicId thread_id = OS_UNUSED_ID;
	
	if (ThisProcess != 0)
		if (ThisProcess->isInitialized())
			process_id = ThisProcess->getId();

	if (ThisThread != 0)
		if (ThisThread->isInitialized())
			thread_id = ThisThread->getId();

	
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
	ASAAC_TimedReturnStatus Result = ASAAC_TM_ERROR;
	//Result = m_ErrorMessageQueue.sendMessage( &m_ErrorInformation, sizeof( m_ErrorInformation ), TimeInfinity );
	Result = m_ErrorMessageQueue.sendMessage( &m_ErrorInformation, sizeof( m_ErrorInformation ), TimeStamp::Instant().asaac_Time() );

	return ( Result == ASAAC_TM_SUCCESS ? ASAAC_SUCCESS : ASAAC_ERROR );
}

				
ASAAC_ReturnStatus ErrorHandler::getErrorInformation( ASAAC_PublicId& faulty_thread_id,
											ASAAC_ErrorType& error_type,
											ASAAC_ErrorCode& error_code,
											ASAAC_CharacterSequence& error_message )
{
	if (m_HaveErrorInformation == false) 
		return ASAAC_ERROR;
	
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if (( ThisThread == 0 ) || ( ThisThread->getId() != 0 )) 
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
	m_ErrorHandlerReturnStatus = return_status;
	
	m_ErrorHandlerTrigger.trigger();
	
	ProcessManager::getInstance()->getCurrentThread()->terminateSelf();
}


ASAAC_ReturnStatus ErrorHandler::logMessage( const ASAAC_CharacterSequence& log_message, ASAAC_LogMessageType message_type )
{
	LogReportData ThisReportData;
	ThisReportData.time = TimeStamp::Now().asaac_Time();	

	Process* ThisProcess = ProcessManager::getInstance()->getCurrentProcess();
	Thread *ThisThread = ProcessManager::getInstance()->getCurrentThread();

	ASAAC_PublicId authentication_code = 0;
	ASAAC_PublicId process_id = OS_UNUSED_ID;
	ASAAC_PublicId thread_id = OS_UNUSED_ID;
	
	if (ThisProcess != 0)
		if (ThisProcess->isInitialized())
		{
			process_id = ThisProcess->getId();
			authentication_code = ThisProcess->getAuthenticationCode();
		}

	if (ThisThread != 0)
		if (ThisThread->isInitialized())
			thread_id = ThisThread->getId();
		
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
		return ASAAC_ERROR;

	// TODO: check whether timeout should be set to TimeInstant rather (non-blocking)
	// TODO: In case of timeout try to send the information later. Store it in a local queue. 
	ASAAC_TimedReturnStatus Result = ASAAC_TM_ERROR;
	//Result = m_LoggingMessageQueue.sendMessage( &ThisReportData, sizeof( LogReportData ), TimeInfinity );
	Result = m_LoggingMessageQueue.sendMessage( &ThisReportData, sizeof( LogReportData ), TimeStamp::Instant().asaac_Time() );
	
	return ( Result == ASAAC_TM_SUCCESS ? ASAAC_SUCCESS : ASAAC_ERROR );
}
	


// *******************************************************************************************
//                   C O M M A N D      H A N D L E R S
// *******************************************************************************************

	
void ErrorHandler::activateErrorHandler( CommandBuffer Buffer )
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
	if ( ThisProcess == 0 ) return;
	
	Thread* ErrorThread = ThisProcess->getThread( 0 );
	if ( ErrorThread == 0 ) 
	{
		cout << "No error handler thread found." << endl;
		return;
	}

	unsigned long CurrentTriggerState = This->m_ErrorHandlerTrigger.getTriggerState();

	ErrorThread->start();
	
	ASAAC_TimedReturnStatus ErrorHandlerResult = This->m_ErrorHandlerTrigger.waitForTrigger( CurrentTriggerState, Data->send.Timeout );

	if ( ErrorHandlerResult != ASAAC_TM_SUCCESS ) 
		return;
	
	Data->Result = This->m_ErrorHandlerReturnStatus;
}


