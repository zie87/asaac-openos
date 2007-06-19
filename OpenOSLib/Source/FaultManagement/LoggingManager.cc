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
			FM->createFile( CharSeq(LogFileNames[ Index ]).asaac_str(), ASAAC_RW, 0 );
			
			const ASAAC_UseOption UseOption = {ASAAC_READWRITE, ASAAC_SHARE};
			
			if (FM->openFile( CharSeq(LogFileNames[ Index ]).asaac_str(), UseOption, m_LogFileDescriptors[Index]) == ASAAC_ERROR);
				throw OSException( LogFileNames[ Index ], LOCATION );
		}
		
		m_LoggingQueue.initialize( IsMaster, OS_LOGGING_QUEUE, CLIENTS_RECEIVE, OS_MAX_ERROR_QUEUESIZE, sizeof( LogReportData ) );
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
	
	oal_off_t SeekResult;
	
	if ( log_id == 0 )
	{
		SeekResult = oal_lseek( m_LogFileDescriptors[ message_type ], sizeof( ASAAC_CharacterSequence ), SEEK_END );
	}
	else
	{
		SeekResult = oal_lseek( m_LogFileDescriptors[ message_type ], log_id * sizeof( ASAAC_CharacterSequence ), SEEK_SET );
	}
	
	if ( SeekResult == -1 ) 
		return ASAAC_RS_RESOURCE;
	
	size_t BytesRead = oal_read( m_LogFileDescriptors[ message_type ], &log_message, sizeof( ASAAC_CharacterSequence ) );
	
	if ( BytesRead != sizeof( ASAAC_CharacterSequence ) ) 
		return ASAAC_RS_ERROR;
	
	return ASAAC_RS_SUCCESS;
}
								
								
ASAAC_ResourceReturnStatus	LoggingManager::writeLog( ASAAC_LogMessageType message_type,
								 unsigned long log_id,
								 const ASAAC_CharacterSequence& log_message )
{
	if ( m_IsInitialized == false ) 
		return ASAAC_RS_RESOURCE;
	
	oal_off_t SeekResult;
	
	if ( log_id == 0 )
	{
		SeekResult = oal_lseek( m_LogFileDescriptors[ message_type ], 0, SEEK_END );
	}
	else
	{
		SeekResult = oal_lseek( m_LogFileDescriptors[ message_type ], log_id * sizeof( ASAAC_CharacterSequence ), SEEK_SET );
	}
	
	if ( SeekResult == -1 ) return ASAAC_RS_RESOURCE;
	
	long Index;
	
	switch(message_type)
	{
		case ASAAC_LOG_MESSAGE_TYPE_ERROR:       Index = 0; break;
  		case ASAAC_LOG_MESSAGE_TYPE_APPLICATION: Index = 1; break;
  		case ASAAC_LOG_MESSAGE_TYPE_GSM: 		 Index = 2; break;
  		case ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE: Index = 3; break;
  		default: Index = 0; break;
	}
	
	const char * msg = CharacterSequence(log_message).c_str();
	size_t BytesWritten = oal_write( m_LogFileDescriptors[ Index ], msg, log_message.size );
	
	char cr = 13; //Carriage Return
	oal_write( m_LogFileDescriptors[ Index ], &cr, 1 );
	
	if ( BytesWritten != sizeof( ASAAC_CharacterSequence ) ) return ASAAC_RS_ERROR;
	
	return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus	 LoggingManager::getLogReport( ASAAC_CharacterSequence& log_message,
								 			  ASAAC_LogMessageType& message_type,
								 			  ASAAC_PublicId& ProcessId,
								 			  const ASAAC_TimeInterval& Timeout )
{
	if ( m_IsInitialized == false ) 
		return ASAAC_TM_ERROR;
	
	unsigned long ActualSize;
	
	LogReportData ThisLogReport;
	
	ASAAC_TimedReturnStatus Result = m_LoggingQueue.receiveMessage( &ThisLogReport, sizeof( LogReportData ), ActualSize, TimeStamp( Timeout ).asaac_Time() );
	
	if ( Result == ASAAC_TM_TIMEOUT ) 
		return ASAAC_TM_TIMEOUT;
	
	if ( ActualSize != sizeof( LogReportData ) ) 
		return ASAAC_TM_ERROR;

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
	
	return Result;
}
