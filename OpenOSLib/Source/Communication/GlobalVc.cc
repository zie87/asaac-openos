#include "Communication/GlobalVc.hh"
#include "Communication/VcUpdateSignal.hh"

#include "IPC/ProtectedScope.hh"

#include "Allocator/SharedMemory.hh"
#include "Allocator/AllocatedArea.hh"

#include "ProcessManagement/ProcessManager.hh"

#include "Managers/FileNameGenerator.hh"


using namespace std;


GlobalVc::GlobalVc() : m_IsInitialized(false)
{
}


size_t GlobalVc::predictSize( const ASAAC_VcDescription& Description )
{
	size_t CumulativeSize = 0;
	
	// m_Description
	CumulativeSize += Shared<ASAAC_VcDescription>::predictSize();  
	
	// m_Status
	CumulativeSize += Shared<GlobalVcStatus>::predictSize(); 
	
	// m_AccessSemaphore, m_SendSemaphore
	CumulativeSize += 2 * Semaphore::predictSize();          

	unsigned long TotalNumberOfBuffers = (Description.max_number_of_buffers) *
										 (Description.max_number_of_threads_attached);

	// m_BufferInfo
	CumulativeSize += Shared<BufferInfo>::predictSize( TotalNumberOfBuffers ); 
						
	// Shared Buffer Info fields
	unsigned long TotalBufferSize = TotalNumberOfBuffers * ( Description.max_msg_length );

	// m_BufferData
	CumulativeSize += Shared<char>::predictSize( TotalBufferSize );

	// m_FreeBuffersQueue
	CumulativeSize += SharedCyclicQueue<unsigned long>::predictSize( TotalNumberOfBuffers ); 

	// m_LocalVcIndex
	CumulativeSize += Shared<LocalVcIndex>::predictSize( OS_MAX_NUMBER_OF_LOCALVCS );

	// m_LocalVcObject
	CumulativeSize += OS_MAX_NUMBER_OF_LOCALVCS * LocalVc::predictSize( Description.max_number_of_buffers );


	return CumulativeSize;
}


void GlobalVc::initialize(ASAAC_PublicId GlobalVc, bool IsMaster, const ASAAC_VcDescription& Data, SessionId p_SessionId )
{
	// Avoid duplicate initalization
	if ( m_IsInitialized ) 
		throw DoubleInitializationException(LOCATION);

	try
	{	
		m_IsInitialized = true;
		
		CharacterSequence ErrorString;
		
		if (IsMaster == true)
		{
			if ( Data.max_number_of_threads_attached < 2 ) 
				throw OSException(
					"The chosen max_number_of_threads_attached is not valid. "
					"It is lower than 2. "
					"A valid communication via VCs is only possible with at least two involved threads. ", LOCATION);
		
			/*if ( div((long)Data.max_number_of_buffers, (long)Data.max_number_of_threads_attached).quot == 0 )
			{ 
				CharSeq ErrorString;
				ErrorString << "The chosen max_number_of_buffers (" << Data.max_number_of_buffers << ") is not valid. ";
				ErrorString << "It is lower than the max_number_of_threads_attached (" << Data.max_number_of_threads_attached << "). ";
				ErrorString << "For every thread at least one buffer has to be reserved";
				throw OSException( ErrorString.c_str(), LOCATION );
			}*/
		}
		
		m_IsMaster = IsMaster;
		
		
		m_Allocator.initialize( FileNameGenerator::getGlobalVcName( GlobalVc ),
								IsMaster,
								predictSize( Data ) );
		
		// Initialize Shared Memory Description Datablock, and if Master, set data
		// This data needs to be initialized first, so the size of all later
		// structures can be computed by slaves and master in the same consistent way
		m_Description.initialize( &m_Allocator );
        
		if ( m_IsMaster )
		{
			*m_Description = Data;
		}
		else 
		{
			//If we have attached to a already created shared memory,
			//now check the consistency...
			if ( m_Allocator.getSize() < predictSize( *m_Description ) )
				throw FatalException( (ErrorString << "Size of SharedMemory (" << (signed long)m_Allocator.getSize() 
						<< ") is smaller then predicted (" << (signed long)predictSize( *m_Description ) << ")").c_str(), LOCATION); 	
		}
		
		// Initialize GlobalVC Status object
		m_Status.initialize( &m_Allocator );
        
		if ( m_IsMaster )
		{
			m_Status->NumberOfBuffers      	= 0;
			m_Status->NumberOfConnectedVCs 	= 0;
			m_Status->NumberOfConnectedTCs 	= 0;
			m_Status->SenderVcIndex   		= -1;
		}
		
		// Initialize SpinLocks used in management functions
		m_ActionSemaphore.initialize( &m_Allocator, m_IsMaster );
		m_AllocationSemaphore.initialize( &m_Allocator, m_IsMaster );
		
		// Next, allocate BufferInfo data and BufferData, depending
		// on the data in the shared Description
		
		// This number is usually A LOT too high, but we're rather safe than sorry.
		unsigned long TotalNumberOfBuffers = m_Description->max_number_of_buffers;
	
	
		m_BufferInfo.initialize( &m_Allocator, TotalNumberOfBuffers );
		
		// Allocate shared memory for buffer data
		unsigned long TotalBufferSize = TotalNumberOfBuffers * ( m_Description->max_msg_length );
		
		m_BufferData.initialize( &m_Allocator, TotalBufferSize );
	
		// Initialize FreeBuffers queue, inserting all buffers initially.
		// Buffer is FIFO and will throw an exception on over- and underrun.
		m_FreeBuffersQueue.initialize( &m_Allocator, m_IsMaster, TotalNumberOfBuffers, FIFO, EXCEPTION );
		
		if ( m_IsMaster )
		{
			for ( unsigned long Index = 0; Index < TotalNumberOfBuffers; Index++ )
			{
				// Buffer is free right now
				m_FreeBuffersQueue.push( Index, TimeInfinity );
				
				// Adjust Buffer Info to reflect the right values
				m_BufferInfo[ Index ].BufferLength  = m_Description->max_msg_length;
				m_BufferInfo[ Index ].ContentLength = 0;
				m_BufferInfo[ Index ].UsageCount    = 0;
			}
		}
		
		// Initialize Queues Callback (to be used for the LocalVc queues)
		m_GlobalVcQueueCallback.initialize( this );
	
		
		for ( unsigned long Index = 0; Index < m_Description->max_number_of_threads_attached; Index++ )
		{	
			// Initialize LocalVc Allocator		
			m_LocalVcAllocator[ Index ].initialize( &m_Allocator, LocalVc::predictSize(m_Description->max_number_of_buffers) );
		}

		m_LocalVcIndex.initialize( &m_Allocator, OS_MAX_NUMBER_OF_LOCALVCS );
		
		if ( m_IsMaster )
		{
			for ( unsigned long Index = 0; Index < m_Description->max_number_of_threads_attached; Index++ )
			{	
				// Initialize LocalVc Ids		
				m_LocalVcIndex[ Index ].ProcessId = OS_UNUSED_ID;
				m_LocalVcIndex[ Index ].LocalVcId = OS_UNUSED_ID;
			}			
		}
        
        m_SessionId = p_SessionId;
	}
	catch (ASAAC_Exception &e)
	{
		deinitialize();
		
		e.addPath("Error initializing GlobalVc object", LOCATION);
		
		throw;	
	}
}


GlobalVc::~GlobalVc()
{
}


void GlobalVc::deinitialize()
{
	if (m_IsInitialized == false ) 
		return;

	try
	{
		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index++ )
		{
			m_LocalVcObject[ Index ].deinitialize();
		}

		for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCALVCS; Index++ )
		{
			m_LocalVcAllocator[ Index ].deinitialize();
		}
		
		m_LocalVcIndex.deinitialize();
		
		m_FreeBuffersQueue.deinitialize();
	
		m_BufferData.deinitialize();
	
		m_ActionSemaphore.deinitialize();
		m_AllocationSemaphore.deinitialize();
		
		m_Status.deinitialize();
		m_Description.deinitialize();
		
		// release allocator reference
		m_Allocator.deinitialize();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error deinitializing GlobalVc", LOCATION);	
		e.raiseError();	
	}	

	m_IsInitialized = false;	
}


bool GlobalVc::isInitialized()
{
	return m_IsInitialized;
}


ASAAC_PublicId GlobalVc::getId()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);
	
	return m_Description->global_vc_id;
}


GlobalVcStatus GlobalVc::getStatus()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);
	
	return *m_Status;
}


ASAAC_VcDescription *GlobalVc::getDescription()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);
	
	return m_Description.getLocation();
}


SessionId GlobalVc::getSessionId()
{
    return m_SessionId;
}


// ******************************************************************************************************
//                                         LOCAL VC CONFIGURATION
// ******************************************************************************************************

void GlobalVc::createLocalVc( const ASAAC_VcMappingDescription& Description )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		// Now, we will handle some manipulation of LocalVC lists. Lock GlobalVc via
		// Semaphore while doing this.
		 ProtectedScope Access( "Creating a local vc", m_ActionSemaphore );

		// If all LocalVc Slots are occupied, return error
		if ( m_Description->max_number_of_threads_attached == m_Status->NumberOfConnectedVCs ) 
			throw OSException("All LocalVc Slots are occupied.", LOCATION);
	
		// if too many buffers requested, return error
		if ( m_Status->NumberOfBuffers + Description.number_of_message_buffers > m_Description->max_number_of_buffers )
			throw OSException("Not enough buffer memory in global VC assigned.", LOCATION);
	
		if (( Description.is_reading == false ) && ( m_Status->SenderVcIndex != -1 )) 
			throw OSException("Only one writing LocalVC per GlobalVC is allowed.", LOCATION);
	
		// If LocalVc already exists, abort with ASAAC_ERROR
		if ( getLocalVc( Description.global_pid, Description.local_vc_id ) != 0 ) 
			throw OSException("LocalVc already exists.", LOCATION);
	
		// Find vacant Local Vc Slot. On failure, abort with ASAAC_ERROR
		long NewLocalVcIndex = getLocalVcIndex( OS_UNUSED_ID, OS_UNUSED_ID );
	
		if ( NewLocalVcIndex == -1 ) 
			throw OSException("Maximum number of local vcs is reached.", LOCATION);
		LocalVc* NewLocalVc = &m_LocalVcObject[ NewLocalVcIndex ];
	
		// Find target Process. On failure, abort with ASAAC_ERROR
		Process* CurrentProcess = ProcessManager::getInstance()->getCurrentProcess();
		Process* TargetProcess = ProcessManager::getInstance()->getProcess( Description.global_pid );
		ProcessStatus ThisState = TargetProcess->getState();
	
		// Process must be in stopped or initialized state
		if ( ( ( ThisState != PROCESS_STOPPED ) && ( ThisState != PROCESS_INITIALIZED ) ) &&
			 ( CurrentProcess->getId() != TargetProcess->getId() ) ) //TODO: analyse exception due to a leak in the standard
			throw OSException("Process has to be in state 'STOPPED' or 'INITIALIZED'.", LOCATION);
		
		// Assign Local Vc to empty LocalVc Slot determined above	
		m_LocalVcAllocator[ NewLocalVcIndex ].reset();
		NewLocalVc->initialize( true, Description, this, &(m_LocalVcAllocator[ NewLocalVcIndex ]), &m_GlobalVcQueueCallback );

		m_LocalVcIndex[ NewLocalVcIndex ].ProcessId = Description.global_pid;
		m_LocalVcIndex[ NewLocalVcIndex ].LocalVcId = Description.local_vc_id;

		try
		{
			TargetProcess->attachLocalVc( m_Description->global_vc_id, Description.local_vc_id );
	
			if ( Description.is_reading == false )
				m_Status->SenderVcIndex = NewLocalVcIndex;
			
			m_Status->NumberOfConnectedVCs ++;
			m_Status->NumberOfBuffers += Description.number_of_message_buffers;
		}
		catch ( ASAAC_Exception &e )
		{
			NewLocalVc->deinitialize();

			m_LocalVcIndex[ NewLocalVcIndex ].ProcessId = OS_UNUSED_ID;
			m_LocalVcIndex[ NewLocalVcIndex ].LocalVcId = OS_UNUSED_ID;

			throw;
		}
		
	}	
	catch(ASAAC_Exception &e)
	{
		CharacterSequence ErrorString;
		ErrorString << "Error creating a local vc (local_vc_id:" << CharSeq(Description.local_vc_id) 
			<< " global_vc_id:" << CharSeq(Description.global_vc_id) << " plobal_pid:" << CharSeq(Description.global_pid) << ")";
			
		e.addPath(ErrorString.c_str(), LOCATION);
		
		throw;
	}
}

	  

void GlobalVc::removeLocalVc( ASAAC_PublicId ProcessId, ASAAC_PublicId LocalVcId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

    try
    {
		Process* TargetProcess = ProcessManager::getInstance()->getProcess( ProcessId );
		if (TargetProcess == NULL)
			throw OSException("Process not found.", LOCATION);
	
		ProcessStatus State = TargetProcess->getState();
		if ( ( State != PROCESS_INITIALIZED ) &&
			 ( State != PROCESS_STOPPED ) )
			 throw OSException("Process must be in state 'INITIALIZED' or 'STOPPED'." ,LOCATION);		

		ProtectedScope Access( "Removing a local vc", m_ActionSemaphore );
		
		long ThisLocalVcIndex = getLocalVcIndex( ProcessId, LocalVcId );
		
		if ( ThisLocalVcIndex == -1 ) 
			throw OSException("LocalVc index not valid." ,LOCATION);		
		
		m_LocalVcIndex[ ThisLocalVcIndex ].ProcessId = OS_UNUSED_ID;
		m_LocalVcIndex[ ThisLocalVcIndex ].LocalVcId = OS_UNUSED_ID;

		LocalVc* ThisLocalVc = &m_LocalVcObject[ ThisLocalVcIndex ];

		Process* LocalVcProcess = ProcessManager::getInstance()->getProcess( ProcessId );					
		LocalVcProcess->detachLocalVc( LocalVcId ); 
	
		m_Status->NumberOfBuffers -= ThisLocalVc->getDescription()->number_of_message_buffers;
		m_Status->NumberOfConnectedVCs --;
		
		ThisLocalVc->deinitialize();
		
		if ( ThisLocalVcIndex == m_Status->SenderVcIndex )
		{
			m_Status->SenderVcIndex = -1;
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error removing local vc", LOCATION);

		throw;
	}	
}
	


void GlobalVc::removeLocalVcsFromProcess( ASAAC_PublicId ProcessId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		for ( unsigned long Index = 0; Index < m_Description->max_number_of_threads_attached; Index++ )
		{
			if (m_LocalVcObject[ Index ].isInitialized() == false )
				continue;

			if ( m_LocalVcObject[ Index ].getDescription()->global_pid == ProcessId )
				removeLocalVc( ProcessId, m_LocalVcObject[ Index ].getId() ); 
		}
	}
	catch (ASAAC_Exception &e)
	{
        e.addPath("Error while removing local vcs from process", LOCATION);
        
		throw;
	}
}



void GlobalVc::removeAllLocalVcs( )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	try
	{
		for ( unsigned long Index = 0; Index < m_Description->max_number_of_threads_attached; Index++ )
		{
			if (m_LocalVcObject[ Index ].isInitialized() == false )
				continue;
			
			removeLocalVc( m_LocalVcObject[ Index ].getDescription()->global_pid, m_LocalVcObject[ Index ].getId() ); 
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error removing all local vcs", LOCATION);
		e.raiseError();
		
		throw;
	}
}



// ******************************************************************************************************
//                                         BUFFER ALLOCATION / INFORMATION
// ******************************************************************************************************

BufferInfo* GlobalVc::getBufferInfo( unsigned long Buffer ) const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	if ( Buffer >= m_Description->max_number_of_buffers ) 
		return 0;
	
	return &(m_BufferInfo[Buffer]);
}


ASAAC_Address GlobalVc::getBufferArea( unsigned long Buffer ) const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	if ( Buffer >= m_Description->max_number_of_buffers ) 
		return 0;
	
	unsigned long BufferUnit = m_Description->max_msg_length; 
	
	return static_cast<ASAAC_Address>( &(m_BufferData[ BufferUnit * Buffer ]) );
}


long GlobalVc::getBufferNumber( ASAAC_Address BufferLocation ) const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	if ( BufferLocation != 0 )
	{
		ldiv_t DivResult = div((long)BufferLocation - (long)m_BufferData.getLocation(), (long)m_Description->max_msg_length);
		
		if (DivResult.rem != 0)
			throw OSException("BufferLocation is not valid", LOCATION);
		
		// now verify the number;
		if ( getBufferArea( DivResult.quot ) == BufferLocation )
		{
			return DivResult.quot;
		}
	}
	
	return -1;
}


// ******************************************************************************************************
//                                         BUFFER COMMUNICATION
// ******************************************************************************************************

void GlobalVc::sendBuffer( unsigned long SourceBuffer, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

    try
    {
		// received a buffer from either a VC or a TC. 
		
		// get Info and Data blocks for said pushed Buffer
		BufferInfo* SourceInfo = getBufferInfo( SourceBuffer );
		
		// If Buffer is not a locked one, throw exception
		if ( SourceInfo->UsageCount <= 0 ) 
            throw OSException( "Buffer has not been locked" ,LOCATION );
	
		// If Content length is illegally high, throw exception
		if ( SourceInfo->ContentLength > m_Description->max_msg_length ) 
            throw OSException( "Content length is illegally high ( > max_msg_length )", LOCATION );
		
		// Now, we're working on internal VC tables. Protect that from mayhem
		// by use of the access semaphore!
		
		// Only one sending should occur at a time.
		// Since only one thread/process is supposed to be sending anyway,
		// this should not be a problem. But... better be safe than sorry.	
		//ProtectedScope Action( "sending a Buffer to local vc's ", m_ActionSemaphore, Timeout, true );
		ProtectedScope Action( "sending a Buffer to local vc's ", m_ActionSemaphore, Timeout, false );
		
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << " : Entry sendBuffer: " << SourceBuffer << endl << endl;
#endif
		 
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << " : sendBuffer -> waitForFreeCells: " << SourceBuffer << endl << endl;
#endif
		
		for ( long iVC = 0; iVC < (long)m_Description->max_number_of_threads_attached; iVC++ )
		{
			LocalVc* LocalVcObject = getLocalVcByIndex(iVC);
			
			if (( LocalVcObject->isInitialized() ) && ( iVC != m_Status->SenderVcIndex ))
				LocalVcObject->waitForFreeCells( Timeout );
		}
	
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << " : sendBuffer -> queueBuffer: " << SourceBuffer << endl << endl;
#endif
	
		// Loop over all connected VC's
		for ( long iVC = 0; iVC < (long)m_Description->max_number_of_threads_attached; iVC++ )
		{
			LocalVc* LocalVcObject = getLocalVcByIndex(iVC);
			
			if ((LocalVcObject->isInitialized()) && ( iVC != m_Status->SenderVcIndex ))
				LocalVcObject->pushBuffer( SourceBuffer );
				// We don't need to respect timeouts here any more. We've
				// already made sure all buffers should be available for writing.
		}
	
		// All list manipulation/information is done.
		// Free the semaphore again.
	
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << " : sendBuffer (releaseBuffer): " << SourceBuffer << endl << endl;
#endif
	
		// release SourceBuffer ONE time (for the sender).
		// now it is linked n times where n is the number of receivers linked.
		releaseBuffer( SourceBuffer );
		
		// Signal the VcUpdateSignal
		VcUpdateSignal::getInstance()->trigger();
	
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << " : Exit sendBuffer: " << SourceBuffer << endl << endl;
#endif		
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error while sending a buffer to receiving vcs", LOCATION);
        
        throw;        
    }
}


#ifdef DEBUG_BUFFER
#include <queue>
void GlobalVc::printMessageQueue()
{
	queue<ASAAC_PublicId> q;
	cout << "GVC-Id: " << getId() << " : Unused Buffers in cyclic queue" << endl;
	while (m_FreeBuffersQueue.isEmpty() == false)
	{
		ASAAC_PublicId id = m_FreeBuffersQueue.pop( TimeInfinity );
		cout << id << ":";
		q.push(id);
	}
	while (q.empty() == false)
	{
		m_FreeBuffersQueue.push(q.front());
		q.pop();
	}
	cout <<  endl << endl;
}
#endif


unsigned long GlobalVc::claimFreeBuffer()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	// This function should be thread-safe due to usage of the cyclic buffer.
	// No extra synchronization required.
	
    unsigned long BufferNumber;
    	
	try 
    {
	    if(m_FreeBuffersQueue.isEmpty())
	        throw ResourceException("FreeBuffersQueue is empty", LOCATION );
            
		BufferNumber = m_FreeBuffersQueue.pop( TimeInfinity );
		
		#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << ", getFreeBuffer():" << BufferNumber;
		cout << ", UsageCount: " << m_BufferInfo[BufferNumber].UsageCount << endl << endl;
		printMessageQueue();
		#endif		

		assert( m_BufferInfo[BufferNumber].UsageCount == 0 );
		// a freshly gotten Buffer HAS to have a undefined count and shall
		// be set to 1. 
		m_BufferInfo[BufferNumber].UsageCount = 1;
	}
	catch ( ResourceException& e )
	{
        e.addPath("Error returning a free buffer", LOCATION);
        
		throw;
	}
	
	return BufferNumber;
}


void GlobalVc::claimBuffer( unsigned long BufferNumber )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

    try
    {
		if ((BufferNumber >= m_Description->max_number_of_buffers) ||
		    (BufferNumber < 0))
		   throw ASAAC_Exception("BufferNumber out of range.", LOCATION );
	
		ProtectedScope Access( "Claiming a buffer", m_AllocationSemaphore );
	
		#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << ": claimBuffer(): m_BufferInfo[" << BufferNumber << "].UsageCount = " << m_BufferInfo[ BufferNumber ].UsageCount << endl;		
		#endif
	
		if (m_BufferInfo[BufferNumber].UsageCount == 0)
			throw ASAAC_Exception("A free buffer can only be claimed using claimFreeBuffer()", LOCATION);
	
		m_BufferInfo[BufferNumber].UsageCount ++;
	
		#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << getId() << ": UsageCount of Buffer " << BufferNumber << " is: " << m_BufferInfo[BufferNumber].UsageCount << endl;
		printMessageQueue();
		#endif
    }
    catch (ASAAC_Exception &e)
    {
        CharacterSequence ErrorString;
        
        e.addPath( (ErrorString << "Error claiming buffer: " << BufferNumber).c_str(), LOCATION);
        
        throw;
    }	
}


void GlobalVc::releaseBuffer( unsigned long BufferNumber )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	// This function is called while the Buffer could be shared amongst
	// several processes. UsageCount should be protected via semaphore
    
    try
    {
		if ((BufferNumber >= m_Description->max_number_of_buffers) ||
		    (BufferNumber < 0))
		   throw ASAAC_Exception("BufferNumber out of range.", LOCATION );
		   
		ProtectedScope Access( "Releasing a buffer", m_AllocationSemaphore );
	
		//if constrained is fulfilled, buffer is already released
		//return without doing anything
		if (m_BufferInfo[BufferNumber].UsageCount != 0)
        {		
			//assert( m_BufferInfo[BufferNumber].UsageCount > 0 );
			//if UsageCount is 0, this buffer was not used any time.
			if (m_BufferInfo[BufferNumber].UsageCount > 0)
				m_BufferInfo[BufferNumber].UsageCount --;
		
#ifdef DEBUG_BUFFER
			cout << "GVC-Id: " << getId() << ": releaseBuffer( " << BufferNumber << ") , UsageCount: " << m_BufferInfo[BufferNumber].UsageCount << endl << endl;
			printMessageQueue();
#endif
		
			//Buffer is free		
			if ( m_BufferInfo[BufferNumber].UsageCount == 0 )
				m_FreeBuffersQueue.push( BufferNumber, TimeInfinity );
        }
    }
    catch (ASAAC_Exception &e)
    {
        CharacterSequence ErrorString;
        
        e.addPath( (ErrorString << "Error releasing buffer: " << BufferNumber).c_str(), LOCATION);
        
        throw;
    }
}


LocalVc* GlobalVc::getLocalVc( ASAAC_PublicId ProcessId, ASAAC_PublicId LocalVcId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	long Index = getLocalVcIndex( ProcessId, LocalVcId );
	
	if ( Index < 0 ) 
		return NULL;
	
	return getLocalVcByIndex(Index);
}


long	GlobalVc::getLocalVcIndex( ASAAC_PublicId ProcessId, ASAAC_PublicId LocalVcId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	for ( unsigned long Index = 0; Index < m_Description->max_number_of_threads_attached; Index ++ )
	{
		if ( m_LocalVcIndex[ Index ].ProcessId != ProcessId ) 
			continue;
		
		if ( m_LocalVcIndex[ Index ].LocalVcId == LocalVcId ) 
			return Index;
	}

	return -1;
}


long	GlobalVc::getSendingLocalVcIndex()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	return m_Status->SenderVcIndex;	
}


LocalVc* GlobalVc::getLocalVcByIndex( long Index )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException(LOCATION);

	if ( (  Index < 0 ) || 
         ( (unsigned long)Index > m_Description->max_number_of_threads_attached ) ) 
		return NULL;
	
	LocalVc* LocalVcObject = &m_LocalVcObject[ Index ];
	
	if ((LocalVcObject->isInitialized() == false) && (m_LocalVcIndex[ Index ].ProcessId != OS_UNUSED_ID))
	{
		ASAAC_VcMappingDescription Description;
		m_LocalVcAllocator[Index].reset();
		LocalVcObject->initialize( false, Description, this, &m_LocalVcAllocator[Index], &m_GlobalVcQueueCallback );
	}
	
	return LocalVcObject;
}



					  


