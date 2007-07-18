#include "LoggingManager.hh"

#include "FaultManagement/LogReportData.hh"

#include "Managers/FileManager.hh"
#include "Managers/TimeManager.hh"

#include "Exceptions/Exceptions.hh"

#include "ProcessManagement/ProcessManager.hh"

using namespace std;

LoggingManager::LoggingManager() : m_IsInitialized(false)
{
	
}

LoggingManager::~LoggingManager()
{

}


LoggingManager* LoggingManager::getInstance()
{
	static LoggingManager ThisManager;
	
	return &ThisManager;
}



void LoggingManager::initialize(bool IsMaster)
{
	static char LogFileNames[][20] = { "log.error", "log.application", "log.gsm", "log.maintenance" };

	if ( m_IsInitialized ) 
		return;
		
	m_IsInitialized = true;
	
	m_IsMaster = IsMaster;
	
	FileManager* FM = FileManager::getInstance();
	
	try 
	{
		for ( long Index = 0; Index < 4; Index ++ ) m_LogFileDescriptors[ Index ] = 0;
		
		for ( long Index = 0; Index < 4; Index ++ )
		{
			if ( IsMaster )
				FM->createFile( CharSeq(LogFileNames[ Index ]).asaac_str(), ASAAC_RW, 0 );
			
			const ASAAC_UseOption UseOption = {ASAAC_READWRITE, ASAAC_SHARE};
			
			FM->openFile( CharSeq(LogFileNames[ Index ]).asaac_str(), UseOption, m_LogFileDescriptors[Index]);
		}
		
		m_LoggingQueue.initialize( IsMaster, OS_LOGGING_QUEUE, CLIENTS_RECEIVE, OS_SIZE_OF_ERROR_QUEUE, sizeof( LogReportData ) );
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error initializing LoggingManager", LOCATION);
		e.raiseError();
		deinitialize();
	}
}

void LoggingManager::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	m_IsInitialized = false;
	
	try
	{
		m_LoggingQueue.deinitialize();
		
		FileManager* FM = FileManager::getInstance();
		
		for ( long Index = 0; Index < 4; Index ++ )
		{
			if ( m_LogFileDescriptors[ Index ] ) 
				FM->closeFile( m_LogFileDescriptors[ Index ] );
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error deinitializing LoggingManager", LOCATION);
		e.raiseError();
	}
}



ASAAC_ResourceReturnStatus	LoggingManager::readLog( ASAAC_LogMessageType message_type,
								unsigned long log_id,
								ASAAC_CharacterSequence& log_message )
{
	if ( m_IsInitialized == false ) 
		return ASAAC_RS_RESOURCE;

	try
	{
		FileManager* FM = FileManager::getInstance();
			
		unsigned long NewPosition;
		
		if ( log_id == 0 )
			FM->seekFile( m_LogFileDescriptors[ message_type ], ASAAC_END_OF_FILE, 0, NewPosition );
		else FM->seekFile( m_LogFileDescriptors[ message_type ], ASAAC_START_OF_FILE, log_id * ASAAC_OS_MAX_STRING_SIZE, NewPosition );
		
		long BytesRead;
		
		FM->readFile( m_LogFileDescriptors[ message_type ], log_message.data, 
				ASAAC_OS_MAX_STRING_SIZE, BytesRead, OS_SIMPLE_COMMAND_TIMEOUT );
		
		log_message.size = BytesRead;
		
		if ( BytesRead != ASAAC_OS_MAX_STRING_SIZE )
			throw OSException("Message is not complete", LOCATION);
	}
	catch (ASAAC_Exception &e)
	{
		log_message = CharSeq("").asaac_str();
		
        return e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;		
	}
	
	return ASAAC_RS_SUCCESS;
}
								
								
ASAAC_ResourceReturnStatus	LoggingManager::writeLog( ASAAC_LogMessageType message_type,
								 unsigned long log_id,
								 const ASAAC_CharacterSequence& log_message )
{
	if ( m_IsInitialized == false ) 
		return ASAAC_RS_RESOURCE;
	
	try
	{
		FileManager* FM = FileManager::getInstance();
		
		unsigned long NewPosition;
		
		if ( log_id == 0 )
			FM->seekFile( m_LogFileDescriptors[ message_type ], ASAAC_END_OF_FILE, 0, NewPosition );
		else FM->seekFile( m_LogFileDescriptors[ message_type ], ASAAC_START_OF_FILE, log_id * ASAAC_OS_MAX_STRING_SIZE, NewPosition );
		
		long Index;
		switch(message_type)
		{
			case ASAAC_LOG_MESSAGE_TYPE_ERROR:       Index = 0; break;
	  		case ASAAC_LOG_MESSAGE_TYPE_APPLICATION: Index = 1; break;
	  		case ASAAC_LOG_MESSAGE_TYPE_GSM: 		 Index = 2; break;
	  		case ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE: Index = 3; break;
	  		default: Index = 0; break;
		}
		
		ASAAC_CharacterSequence message = CharSeq(log_message).append( ASAAC_OS_MAX_STRING_SIZE-log_message.size-1, ' ' ).appendLineBreak().asaac_str(); 
		unsigned long BytesWritten;
		
		FM->writeFile(m_LogFileDescriptors[ Index ], message.data, ASAAC_OS_MAX_STRING_SIZE, 
				BytesWritten, OS_SIMPLE_COMMAND_TIMEOUT );
		
		if ( BytesWritten != ASAAC_OS_MAX_STRING_SIZE )
			throw OSException("Message is not complety written to device", LOCATION);
	}
	catch (ASAAC_Exception &e)
	{
        return e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;		
	}
	
	return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus	 LoggingManager::getLogReport( ASAAC_CharacterSequence& log_message,
								 			  ASAAC_LogMessageType& message_type,
								 			  ASAAC_PublicId& ProcessId,
								 			  const ASAAC_TimeInterval& Timeout )
{
	if ( m_IsInitialized == false ) 
		return ASAAC_TM_ERROR;
	
	try
	{
		unsigned long ActualSize;
		
		LogReportData ThisLogReport;
		
		m_LoggingQueue.receiveMessage( &ThisLogReport, sizeof( LogReportData ), ActualSize, TimeStamp( Timeout ).asaac_Time() );
		
		if ( ActualSize != sizeof( LogReportData ) ) 
			throw OSException("Size of retrieved message doesn't fit to size of LogReportData", LOCATION);
	
//  TODO: Currently it is not possible to check authentity of the master process, because this data is not located
//  in shared memory

//	Process* Sender = ProcessManager::getInstance()->getProcess( ThisLogReport.process_id );
//	
//	if ( Sender == 0 ) return ASAAC_TM_ERROR;
//	
//	// If Message sender has sent wrong authentication code, return ASAAC_TM_ERROR
//	if ( Sender->getAuthenticationCode() != ThisLogReport.process_authentication_code ) return ASAAC_TM_ERROR;
		
		log_message	= ThisLogReport.log_message;
		message_type = ThisLogReport.message_type;
		ProcessId    = ThisLogReport.process_id;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving log report", LOCATION);
		e.raiseError();
		
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
	}
	
	return ASAAC_TM_SUCCESS;
}
