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


void MessageQueue::sendMessageNonblocking(ASAAC_Address BufferReference, unsigned long Size)
{
	try
	{
		sendMessage( BufferReference, Size, TimeStamp::Instant().asaac_Time() );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error sending message nonblocking", LOCATION);
			
		throw;
	}
}


void MessageQueue::receiveMessageNonblocking( ASAAC_Address BufferReference, 
														 unsigned long MaxSize, 
														 unsigned long& ActualSize )
{
	try
	{
		receiveMessage( BufferReference, MaxSize, ActualSize, TimeStamp::Instant().asaac_Time() );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error receiving message nonblocking", LOCATION);
			
		throw;
	}
}


void MessageQueue::sendMessage( ASAAC_Address BufferReference, 
  							   unsigned long Size, 
    							   const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{	
		unsigned long Written;
	
		FileManager::getInstance()->writeFile( m_QueueHandle, BufferReference, Size, Written, TimeStamp(Timeout).asaac_Interval() );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error sending a message", LOCATION);
		
        throw;		
	}
}	


void MessageQueue::receiveMessage( const ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
		
	try
	{	
		long Read;
		
		FileManager::getInstance()->readFile( m_QueueHandle, BufferReference, MaxSize, Read, TimeStamp(Timeout).asaac_Interval() );
		
		ActualSize = Read;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error receiving a message", LOCATION);

        throw;
	}
}	

