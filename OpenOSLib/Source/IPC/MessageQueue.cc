#include "MessageQueue.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Managers/FileManager.hh"


MessageQueue::MessageQueue() : m_IsInitialized(false)
{
}

MessageQueue::~MessageQueue()
{

}


ASAAC_ReturnStatus MessageQueue::create( ASAAC_CharacterSequence Name, QueueDirection Direction, unsigned long QueueSize, unsigned long MessageSize )
{
	if ( m_IsInitialized ) 
		return ASAAC_ERROR;
	
	oal_mq_attr QueueAttribute;

	long iFlags;

	if ( Direction == CLIENTS_SEND )
	{	
		iFlags 	= O_WRONLY;
	}
	else
	{
		iFlags   = O_RDONLY;
	}


	if (QueueSize > 10) QueueSize=10;

	QueueAttribute.mq_flags   = 0;
	QueueAttribute.mq_maxmsg  = QueueSize;
	QueueAttribute.mq_msgsize = MessageSize;
	QueueAttribute.mq_curmsgs = 0;
	
	m_Name = Name;

	//if queue is still linked, unlink it first
	oal_mq_unlink( m_Name.c_str()); 
	
	m_QueueHandle = oal_mq_open(  m_Name.c_str(), 
				      O_RDWR | O_CREAT, 
				      S_IRUSR | S_IWUSR,
				      &QueueAttribute );
	
	if ( m_QueueHandle == -1 ) 
        throw OSException( strerror(errno), LOCATION );

	m_IsInitialized = true;
	m_IsMaster      = true;
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus MessageQueue::open( ASAAC_CharacterSequence Name, QueueDirection Direction )
{
	if ( m_IsInitialized ) 
		return ASAAC_ERROR;

	long iFlags;

	if ( Direction == CLIENTS_SEND )
	{	
		iFlags 	= O_WRONLY;
	}
	else
	{
		iFlags   = O_RDONLY;
	}
	//iFlags = O_RDWR;
	
	m_QueueHandle = FileManager::getInstance()->getFileHandle( Name, MESSAGE_QUEUE, iFlags );
							
	if ( m_QueueHandle < 0 ) 
		return ASAAC_ERROR;
	
	m_IsMaster = false;
	m_Name = Name;
	
	m_IsInitialized = true;
	
	return ASAAC_SUCCESS;
}


void MessageQueue::close()
{
	if (  m_IsInitialized == false  ) 
		return;
	
	m_IsInitialized = false;
	
	try
	{
		if ( m_IsMaster )
		{
			FileManager::getInstance()->removeFile( m_QueueHandle );
		}
		else 
		{
			FileManager::getInstance()->closeFile( m_QueueHandle );
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error closing MessageQueue", LOCATION);
		e.raiseError();
	}
}


ASAAC_ResourceReturnStatus MessageQueue::sendMessageNonblocking(ASAAC_Address BufferReference, unsigned long Size)
{
	ASAAC_TimedReturnStatus Status = sendMessage( BufferReference, Size, TimeStamp::Instant().asaac_Time() );
	
	if ( Status == ASAAC_TM_SUCCESS ) return ASAAC_RS_SUCCESS;
	if ( Status == ASAAC_TM_TIMEOUT ) return ASAAC_RS_RESOURCE;
	
	return ASAAC_RS_ERROR;
}


ASAAC_ResourceReturnStatus MessageQueue::receiveMessageNonblocking( ASAAC_Address BufferReference, 
														 unsigned long MaxSize, 
														 unsigned long& ActualSize )
{
	ASAAC_TimedReturnStatus Status = receiveMessage( BufferReference, MaxSize, ActualSize, TimeStamp::Instant().asaac_Time() );
	
	if ( Status == ASAAC_TM_SUCCESS ) return ASAAC_RS_SUCCESS;
	if ( Status == ASAAC_TM_TIMEOUT ) return ASAAC_RS_RESOURCE;
	
	return ASAAC_RS_ERROR;
}


ASAAC_TimedReturnStatus MessageQueue::sendMessage( ASAAC_Address BufferReference, 
  							   unsigned long Size, 
    							   const ASAAC_Time& Timeout )
{
	if (  m_IsInitialized == false  ) 
		return ASAAC_TM_ERROR;
	
	timespec TimeSpecTimeout;

	TimeSpecTimeout.tv_sec  = Timeout.sec;
	TimeSpecTimeout.tv_nsec = Timeout.nsec;

	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	do {
		if ( ThisThread != 0 ) 
			ThisThread->setState( ASAAC_WAITING );
		
		long Result = 0;
		
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			Result = oal_mq_send( m_QueueHandle, (const char*)BufferReference, Size, 1 );
		else Result = oal_mq_timedsend( m_QueueHandle, (const char*)BufferReference, Size, 1, &TimeSpecTimeout );

        ThisThread = ProcessManager::getInstance()->getCurrentThread();
		if ( ThisThread != 0 ) 
			ThisThread->setState( ASAAC_RUNNING );

		if ( Result == 0 ) 
			return ASAAC_TM_SUCCESS;

		if ( errno == ETIMEDOUT ) 
			return ASAAC_TM_TIMEOUT;
			
	} while ( errno == EINTR );
	
	return ASAAC_TM_ERROR;
}	


ASAAC_TimedReturnStatus MessageQueue::receiveMessage( ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout )
{
	if (  m_IsInitialized == false  ) 
		return ASAAC_TM_ERROR;
	
	timespec TimeSpecTimeout;

	TimeSpecTimeout.tv_sec  = Timeout.sec;
	TimeSpecTimeout.tv_nsec = Timeout.nsec;

	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	do {
        if ( ThisThread != 0 ) 
			ThisThread->setState( ASAAC_WAITING );
		
		unsigned Prio;
		
		size_t ReceivedSize;
		
		if (TimeStamp(Timeout).isInfinity())
			ReceivedSize = oal_mq_receive( m_QueueHandle, (char*)BufferReference, MaxSize, &Prio );
		else ReceivedSize = oal_mq_timedreceive( m_QueueHandle, (char*)BufferReference, MaxSize, &Prio, &TimeSpecTimeout );

		ActualSize = (unsigned long)ReceivedSize;

        ThisThread = ProcessManager::getInstance()->getCurrentThread();
		if ( ThisThread != 0 ) 
			ThisThread->setState( ASAAC_RUNNING );
	
		if ( ReceivedSize > 0 )  
			return ASAAC_TM_SUCCESS;

		if ( errno == ETIMEDOUT )  
			return ASAAC_TM_TIMEOUT;
		
	} while ( errno == EINTR );
	
	return ASAAC_TM_ERROR;
}	

