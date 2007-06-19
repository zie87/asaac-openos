#include "MessageQueue.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Managers/FileManager.hh"


MessageQueue::MessageQueue() : m_IsInitialized(false)
{
}

MessageQueue::~MessageQueue()
{

}


void MessageQueue::initialize( const bool IsMaster, const ASAAC_CharacterSequence Name, const QueueDirection Direction, const unsigned long QueueSize, const unsigned long MessageSize )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException(LOCATION);

	try
	{
		m_IsInitialized = true;
		
		m_IsMaster = IsMaster;
		m_Name = Name;

		if (m_IsMaster)
			FileManager::getInstance()->createMessageQueue( m_Name, ASAAC_RW, QueueSize, MessageSize );
	
		const ASAAC_UseOption UseOption = {ASAAC_READWRITE, ASAAC_SHARE};
		FileManager::getInstance()->openMessageQueue( m_Name, UseOption, m_QueueHandle );
	}
	catch ( ASAAC_Exception &e )
	{
		deinitialize();
		
		e.addPath("Error initializing MessageQueue", LOCATION);
		
		throw;
	}
}


void MessageQueue::deinitialize()
{
	if (  m_IsInitialized == false  ) 
		return;
	
	m_IsInitialized = false;
	
	try
	{
		FileManager::getInstance()->closeFile( m_QueueHandle );

		if ( m_IsMaster )
			FileManager::getInstance()->deleteMessageQueue( m_Name, ASAAC_NORMAL, TimeIntervalInstant );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error deinitializing MessageQueue", LOCATION);
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

