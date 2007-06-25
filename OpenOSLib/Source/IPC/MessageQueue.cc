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
	
	unsigned long Written;
	
	return FileManager::getInstance()->writeFile( m_QueueHandle, BufferReference, Size, Written, TimeStamp(Timeout).asaac_Interval() );
}	


ASAAC_TimedReturnStatus MessageQueue::receiveMessage( const ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout )
{
	if (  m_IsInitialized == false  ) 
		return ASAAC_TM_ERROR;
		
	ASAAC_TimedReturnStatus Result;
	long Read;
	
	return FileManager::getInstance()->readFile( m_QueueHandle, BufferReference, MaxSize, Read, TimeStamp(Timeout).asaac_Interval() );
	
	ActualSize = Read;
	
	return Result;
}	

