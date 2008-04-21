#include "ASAAC_Exception.hh"

#include "FaultManagement/ErrorHandler.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"

#include "Managers/TimeManager.hh"


const char *EmptyString = "";


ASAAC_Exception::ASAAC_Exception( ) 
{
	initialize();
}


ASAAC_Exception::ASAAC_Exception( const char* Message, const char* Function, long Line )
{
	initialize();
	addPath(Message, Function, Line);
}


ASAAC_Exception::~ASAAC_Exception() throw()
{
}


const char *ASAAC_Exception::what() const throw()
{
    return getErrorMessage();
} 


void ASAAC_Exception::initialize()
{
	m_PathSize = 0;
	m_Time = TimeZero;
	m_ProcessId = OS_UNUSED_ID;
	m_ThreadId = OS_UNUSED_ID;
	
	static bool Initializing = false;
	
	if (Initializing == true)
		return;
	
	Initializing = true;
	
	try
	{
		TimeManager::getAbsoluteLocalTime(m_Time);
		m_ProcessId = ProcessManager::getInstance()->getCurrentProcessId();
		m_ThreadId = ThreadManager::getInstance()->getCurrentThread()->getId();
	}
	catch (ASAAC_Exception &e)
	{
		// do nothing
	}

	Initializing = false;
}


void ASAAC_Exception::setProcessId( ASAAC_PublicId ProcessId )
{
	m_ProcessId = ProcessId;
}


void ASAAC_Exception::setThreadId( ASAAC_PublicId ThreadId )
{
	m_ThreadId = ThreadId;
}


ASAAC_PublicId ASAAC_Exception::getProcessId()
{
	return m_ProcessId;
}


ASAAC_PublicId ASAAC_Exception::getThreadId()
{
	return m_ThreadId;	
}


bool ASAAC_Exception::isTimeout() const
{
	return false;
}


bool ASAAC_Exception::isResource() const
{
	return false;
}


ASAAC_ErrorCode ASAAC_Exception::getErrorCode() const
{
	return 0;
}


ASAAC_ErrorType ASAAC_Exception::getErrorType() const
{
	return ASAAC_OS_ERROR;
}


ASAAC_PublicId ASAAC_Exception::getVcId() const
{
	return 0;
}


ASAAC_PublicId ASAAC_Exception::getTcId() const
{
	return 0;
}


ASAAC_NetworkDescriptor ASAAC_Exception::getNetwork() const
{
	const ASAAC_NetworkDescriptor nd = {0,0};
	return nd;
}


ASAAC_Address ASAAC_Exception::getLocation() const
{
	return 0;
}

static CharacterSequence Message;

const char * ASAAC_Exception::getErrorMessage() const
{
    CharacterSequence ErrorType;
        
    Message.erase();
        
    switch (getErrorType())
    {
        case ASAAC_APPLICATION_ERROR: ErrorType = "ASAAC_APPLICATION_ERROR"; break;
        case ASAAC_APOS_CLIENT_ERROR: ErrorType = "ASAAC_APOS_CLIENT_ERROR"; break;
        case ASAAC_RESOURCE_ERROR:    ErrorType = "ASAAC_RESOURCE_ERROR"; break;
        case ASAAC_OS_ERROR:          ErrorType = "ASAAC_OS_ERROR"; break;
        case ASAAC_SMOS_ERROR:        ErrorType = "ASAAC_SMOS_ERROR"; break;
        case ASAAC_SMBP_ERROR:        ErrorType = "ASAAC_SMBP_ERROR"; break;
        case ASAAC_PROCESSOR_ERROR:   ErrorType = "ASAAC_PROCESSOR_ERROR"; break;
        case ASAAC_HW_RESOURCE_ERROR: ErrorType = "ASAAC_HW_RESOURCE_ERROR"; break;
        case ASAAC_HW_FAILURE:        ErrorType = "ASAAC_HW_FAILURE"; break;
        case ASAAC_FATAL_ERROR:       ErrorType = "ASAAC_FATAL_ERROR"; break;
        default: Message = "Unknown Exception Type"; break;
    }

    Message << m_Time;

    if (m_ProcessId != OS_UNUSED_ID)
    {
        Message = Message + CharSeq(" PID:") + CharSeq(m_ProcessId);
    }

    if (m_ThreadId != OS_UNUSED_ID)
    {
        Message = Message + CharSeq(" TID:") + CharSeq(m_ThreadId);
    }
    
    Message << " " << ErrorType << ":";
    
    Message << getMessageItem(0);
    
    for (unsigned short Index = 1; Index < m_PathSize; Index++)
    {    
        Message.appendLineBreak();
        Message << ">>>>> continuing " << ErrorType << ":" << getMessageItem(Index);
    }
    
    return Message.c_str();
}

const char * ASAAC_Exception::getLoggingMessage( ASAAC_LogMessageType message_type ) const
{
    CharacterSequence LoggingType;
        
    Message.erase();
        
    switch (message_type)
    {
        case ASAAC_LOG_MESSAGE_TYPE_ERROR:          LoggingType << "ASAAC_LOG_MESSAGE_TYPE_ERROR"; break;
        case ASAAC_LOG_MESSAGE_TYPE_APPLICATION:    LoggingType << "ASAAC_LOG_MESSAGE_TYPE_APPLICATION"; break;
        case ASAAC_LOG_MESSAGE_TYPE_GSM:            LoggingType << "ASAAC_LOG_MESSAGE_TYPE_GSM"; break;
        case ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE:    LoggingType << "ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE"; break;
        default: LoggingType << "Unknown Logging Type";
    }

    Message << m_Time;

    if (m_ProcessId != OS_UNUSED_ID)
    {
        Message = Message + CharSeq(" PID:") + CharSeq(m_ProcessId);
    }

    if (m_ThreadId != OS_UNUSED_ID)
    {
        Message = Message + CharSeq(" TID:") + CharSeq(m_ThreadId);
    }
    
    Message << " " << LoggingType << ":";
    
    Message << getMessageItem(0);
    
    for (unsigned short Index = 1; Index < m_PathSize; Index++)
    {    
        Message.appendLineBreak();
        Message << ">>>>> continuing " << LoggingType << ":" << getMessageItem(Index);
    }
    
    return Message.c_str();
}

static CharacterSequence MessageItem;

const char * ASAAC_Exception::getMessageItem(unsigned short index) const
{
	if (index >= m_PathSize)
		return EmptyString;
		
	MessageItem.erase();
	
	ExceptionPathType Path = m_Path[index];	
		
	if ( Path.message.size > 0 )
	{
		MessageItem << CharSeq( " \"") << Path.message << CharSeq( "\"");
	}

#ifdef DEBUG		
	if ( Path.function.size > 0 )
	{
		MessageItem << CharSeq(" in function '") << Path.function << CharSeq( "'");
	}
	
	if ( Path.line > 0 )
	{
		MessageItem << CharSeq(" on line ") << Path.line;
	}	
#endif
	
	return MessageItem.c_str();
}


void ASAAC_Exception::printErrorMessage() const
{
	cerr << getErrorMessage() << endl << endl;
}

void ASAAC_Exception::printLoggingMessage( ASAAC_LogMessageType message_type ) const
{
    cout << getLoggingMessage( message_type ) << endl << endl;
}

void ASAAC_Exception::logMessage( ASAAC_LogMessageType message_type ) const
{
    ASAAC_CharacterSequence log_message = CharSeq(getErrorMessage()).asaac_str(); 

#ifdef DEBUG
    printLoggingMessage( message_type );
#endif

    ErrorHandler* eh = ErrorHandler::getInstance();

    eh->logMessage(
        log_message, 
        message_type);
}


void ASAAC_Exception::raiseError( ) const
{
	
#ifdef DEBUG
    printErrorMessage();
#endif

if ((isTimeout() == true) || (isResource() == true))
	return;

#ifdef DEBUG_APPLICATION
#ifndef DEBUG
    printErrorMessage();
#endif
#endif
    
    ASAAC_CharacterSequence error_message = CharSeq(getErrorMessage()).asaac_str(); 

    ErrorHandler* eh = ErrorHandler::getInstance();

    eh->raiseError(
        getErrorCode(),
        error_message, 
        getErrorType(), 
        getLocation(),
        getVcId(), 
        getTcId(),
        getNetwork());
}


void ASAAC_Exception::addPath(  const char* Message, const char* Function, long Line )
{
	if (m_PathSize == OS_MAX_NUMBER_OF_EXCEPTIONS)
		return;
	
	CharSeq csMessage = Message;
	CharSeq csFunction = Function;
		
	m_Path[m_PathSize].message = csMessage.asaac_str();
	m_Path[m_PathSize].function = csFunction.asaac_str();
	m_Path[m_PathSize].line = Line;
	
	m_PathSize++;
}

