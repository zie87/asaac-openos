#include "ASAAC_Exception.hh"
#include "FaultManagement/ErrorHandler.hh"
#include "ProcessManagement/ProcessManager.hh"
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
    return getMessage();
} 


void ASAAC_Exception::initialize()
{
	m_PathSize = 0;
	
	TimeManager::getAbsoluteLocalTime(m_Time);
	m_ProcessId = OS_UNUSED_ID;
	m_ThreadId = OS_UNUSED_ID;

	ProcessManager *PM = ProcessManager::getInstance();
	Process *P = PM->getCurrentProcess();
	if (P != 0)
	{
	 	if ( P->isInitialized() )
	 	{
	 		m_ProcessId = P->getId();
	 	}
	}

	Thread *T = PM->getCurrentThread();
	if (T != 0)
	{
	 	if ( T->isInitialized() )
	 	{
	 		m_ThreadId = T->getId();
	 	}
	}	
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


const char * ASAAC_Exception::getMessage() const
{
	static CharacterSequence Message;
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


const char * ASAAC_Exception::getMessageItem(unsigned short index) const
{
	if (index >= m_PathSize)
		return EmptyString;
		
	static CharacterSequence Message;
	Message.erase();
	
	ExceptionPathType Path = m_Path[index];	
		
	if ( Path.message.size > 0 )
	{
		Message << CharSeq( " \"") << Path.message << CharSeq( "\"");
	}

#ifdef DEBUG		
	if ( Path.function.size > 0 )
	{
		Message << CharSeq(" in function '") << Path.function << CharSeq( "'");
	}
	
	if ( Path.line > 0 )
	{
		Message << CharSeq(" on line ") << Path.line;
	}	
#endif
	
	return Message.c_str();
}


void ASAAC_Exception::printMessage() const
{
	cerr << getMessage() << endl;
}


void ASAAC_Exception::logMessage( ASAAC_LogMessageType message_type ) const
{
    ASAAC_CharacterSequence log_message = CharSeq(getMessage()).asaac_str(); 

#ifdef DEBUG
    printMessage();
#endif

    ErrorHandler* eh = ErrorHandler::getInstance();

    eh->logMessage(
        log_message, 
        message_type);
}


void ASAAC_Exception::raiseError( const bool do_throw ) const
{
	if ( do_throw == true )
	{
	/*	cout << "test" << endl;
		throw;
		throw *this;*/
	}

#ifdef DEBUG
    printMessage();
#endif

    ASAAC_CharacterSequence error_message = CharSeq(getMessage()).asaac_str(); 

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

