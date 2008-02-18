#include "LocalVc.hh"

#include "Communication/GlobalVc.hh"

#include "Managers/TimeManager.hh"

#include "Allocator/AllocatedArea.hh"

using namespace std;



LocalVc::LocalVc() : m_IsInitialized(false)
{
}



LocalVc::~LocalVc()
{
}



size_t LocalVc::predictSize( unsigned long MaximumBuffers )
{
	return ( Shared<ASAAC_VcMappingDescription>::predictSize() +
			 SharedCyclicQueue<unsigned long>::predictSize( MaximumBuffers ) ) ;
}



void LocalVc::initialize( bool IsMaster,
						  const ASAAC_VcMappingDescription& Description,
		                  GlobalVc* ParentGlobalVc,
		                  Allocator* ThisAllocator,
		                  Callback* OverwriteCallback  )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException(LOCATION);
		
	try
	{
		m_IsInitialized = true;

		m_ParentGlobalVc 	= ParentGlobalVc;
		m_IsMaster 			= IsMaster;
		m_OverwriteCallback = OverwriteCallback;
	
		m_Description.initialize( ThisAllocator );

		if (m_IsMaster == true)			
			*m_Description = Description;
		
		// Get number of Buffers for queue
		unsigned long ThisQueueSize = m_Description->number_of_message_buffers;
	
		// Evaluate BlockingType and Direction
		DirectionType ThisQueueDirection = m_Description->is_lifo_queue ? LIFO : FIFO;
		BlockingType  ThisQueueBlocking  = m_Description->is_refusing_queue ? BLOCKING : OVERWRITE_OLDEST;
		
		// Allocate and initialize BufferQueue in shared memory.
		m_Queue.initialize( ThisAllocator, // Memory allocator to determine position
						    m_IsMaster,		   // This function creates the object, so is the master.
						    ThisQueueSize, 	   // Size of the queue in cells
							ThisQueueDirection, 			 
							ThisQueueBlocking );
	
		m_Queue.setOverwriteCallback( m_OverwriteCallback );
	
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing local vc", LOCATION);
		
		deinitialize();
		
		throw;
	}
}



void LocalVc::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
    try
    {
		try 
		{
			for (;;)
			{
				//TODO: SharedCyclicQueue shall offer a mode, to block all other operations
				//and return all contents in queue
				
				unsigned long Buffer = m_Queue.pop( TimeStamp::Instant().asaac_Time() );
				m_ParentGlobalVc->releaseBuffer( Buffer );
			}
		}
		catch ( TimeoutException &e )
		{
		}
		
		m_Description.deinitialize();
		m_Queue.deinitialize();
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error deinitializing local vc", LOCATION);
        e.raiseError();
    }
    
	m_IsInitialized = false;
}



bool LocalVc::isInitialized()
{
	return m_IsInitialized;
}



void LocalVc::remove()
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);

	m_ParentGlobalVc->removeLocalVc( m_Description->global_pid, m_Description->local_vc_id );
}



ASAAC_PublicId LocalVc::getId()
{	
	if ( m_IsInitialized == false ) 
		return OS_UNUSED_ID;
	
	return m_Description->local_vc_id;
}



void LocalVc::sendMessageNonblocking(ASAAC_Address BufferReference, unsigned long Size)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

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



void LocalVc::receiveMessageNonblocking( ASAAC_Address BufferReference, 
														 unsigned long MaxSize, 
														 unsigned long& ActualSize )
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

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



void LocalVc::sendMessage(ASAAC_Address BufferReference, unsigned long Size, const ASAAC_Time& Timeout)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
		
	try
	{
		CharacterSequence ErrorString;
		
		if ( m_Description->is_reading ) 
			throw OSException("This is a reading local vc", LOCATION);
	
		if ( Size > m_Description->buffer_size ) 
			throw OSException( (ErrorString << "Size of message (" << Size << ") is bigger than the buffer of global vc (" <<  (unsigned long)m_Description->buffer_size << ")").c_str(), LOCATION);
	
		ASAAC_Address LockedBuffer;
	
		lockBuffer( LockedBuffer, Size, Timeout );
	
		memcpy( LockedBuffer, BufferReference, Size );
	
		sendBuffer( LockedBuffer, Size, Timeout );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error sending a message", LOCATION);
        
        throw;
	}
}



void LocalVc::receiveMessage( ASAAC_Address BufferReference, 
										   unsigned long MaxSize, 
										   unsigned long& ActualSize, 
										   const ASAAC_Time& Timeout )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		if ( m_Description->is_reading == false ) 
			throw OSException("This is a sending local vc", LOCATION);
	
		ASAAC_Address ReceivedBuffer;
		
		receiveBuffer( ReceivedBuffer, ActualSize, Timeout );
											  	  
		if ( ActualSize > MaxSize )
		{
			unlockBuffer( ReceivedBuffer );
			throw OSException("Received Message size is bigger than MaxSize parameter", LOCATION);
		}
	
		memcpy( BufferReference, ReceivedBuffer, ActualSize );
		
		unlockBuffer( ReceivedBuffer );
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error receiving a message", LOCATION);

        throw;
	}
}



void LocalVc::lockBuffer( ASAAC_Address& BufferReference, 
									   unsigned long Size, 
									   const ASAAC_Time& Timeout )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		CharacterSequence ErrorString;
		
		// return error if Size of buffer requested is too large
		if ( Size > m_Description->buffer_size ) 
			throw OSException( (ErrorString << "Chosen size (" << CharSeq(Size) << ") is bigger than global vcs buffer (" << CharSeq(m_Description->buffer_size) << ")").c_str(), LOCATION);
		
		// get Buffer from the 
		unsigned long BufferId = m_ParentGlobalVc->claimFreeBuffer();

		BufferReference = m_ParentGlobalVc->getBufferArea( BufferId );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error locking a buffer", LOCATION);
		
		throw;
	}
}



void LocalVc::sendBuffer( ASAAC_Address BufferReference, 
								  unsigned long Size,
								  const ASAAC_Time& Timeout )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		CharacterSequence ErrorString;
		
		if ( m_Description->is_reading ) 
			throw OSException("This is a reading local vc", LOCATION);
			
		if ( Size > m_Description->buffer_size )
			throw OSException( (ErrorString << "Chosen size (" << Size << ") is higher than local vcs buffer size (" << m_Description->buffer_size << ")").c_str(), LOCATION);
	
		// Get buffer number and Buffer Info field
		unsigned long BufferId = m_ParentGlobalVc->getBufferNumber( BufferReference );
		BufferInfo* ThisBufferInfo = m_ParentGlobalVc->getBufferInfo( BufferId );
		
		// Set actual message size in the Buffer Info field
		ThisBufferInfo->ContentLength = Size;

		m_ParentGlobalVc->sendBuffer( BufferId, Timeout );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error sending a buffer", LOCATION);
        
        throw;
	}
}



void LocalVc::receiveBuffer( ASAAC_Address& BufferReference, 
										  unsigned long& Size,
										  const ASAAC_Time& Timeout )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	try 
    {
		if ( m_Description->is_reading == false ) 
			throw OSException("This is a sending local vc", LOCATION);

		unsigned long BufferId = popBuffer( Timeout );
		BufferInfo* ThisBufferInfo = m_ParentGlobalVc->getBufferInfo( BufferId );
		
		Size = ThisBufferInfo->ContentLength;
		BufferReference = m_ParentGlobalVc->getBufferArea( BufferId );
	}
	catch( ASAAC_Exception& e )
	{
		e.addPath("Error receiving a buffer", LOCATION);
        
		throw;
	}
}



void LocalVc::unlockBuffer( ASAAC_Address BufferReference )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

    try
    {
		CharacterSequence ErrorString;

		unsigned long BufferId     = m_ParentGlobalVc->getBufferNumber( BufferReference );
		BufferInfo* ThisBufferInfo = m_ParentGlobalVc->getBufferInfo( BufferId );
		
		if(ThisBufferInfo->UsageCount <= 0)
			throw OSException( (ErrorString << "Buffer is not locked: " << BufferId).c_str(), LOCATION);
		
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << m_ParentGlobalVc->getId() << " : LocalVc::unlockBuffer()-> releaseBuffer(" << BufferId << ")" << endl;
#endif
		m_ParentGlobalVc->releaseBuffer( BufferId );
    }
    catch ( ASAAC_Exception &e)
    {
        e.addPath("Error unlocking a buffer", LOCATION);
        
        throw;
    }
}



void LocalVc::waitForAvailableData( const ASAAC_Time& Timeout )
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);
	
	m_Queue.waitForAvailableData( Timeout );
}



void LocalVc::pushBuffer( unsigned long BufferNumber, const ASAAC_Time& Timeout )
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);

	try 
	{	
        // Increase usage count of buffer
        m_ParentGlobalVc->claimBuffer( BufferNumber );
        
		m_Queue.push( BufferNumber, Timeout );
	}
	catch ( ASAAC_Exception& e )
	{
        e.addPath("Error queueing a buffer", LOCATION);
        
		//release Buffer, if transfer has been failed
		m_ParentGlobalVc->releaseBuffer( BufferNumber );
        
		throw;
	}
}
	

unsigned long LocalVc::popBuffer( const ASAAC_Time& Timeout )
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);

    return m_Queue.pop( Timeout );
}


void LocalVc::waitForFreeCells( const ASAAC_Time& Timeout )
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);
	
	m_Queue.waitForFreeCells( Timeout );
}


ASAAC_VcMappingDescription* LocalVc::getDescription()
{
    if ( m_IsInitialized == false ) 
        throw UninitializedObjectException(LOCATION);
	
	return m_Description.getLocation();
}



