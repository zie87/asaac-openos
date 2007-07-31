#ifndef SHAREDCYCLICQUEUE_HH_
#define SHAREDCYCLICQUEUE_HH_

#include "OpenOSIncludes.hh"
#include "Exceptions/Exceptions.hh"

#include "Shared.hh"
#include "AbstractInterfaces/Callback.hh"

#include "IPC/Semaphore.hh"
#include "IPC/ProtectedScope.hh"


using namespace std;


// *************************************************************************************************
//                                   D E C L A R A T I O N
// *************************************************************************************************


enum BlockingType  {BLOCKING, OVERWRITE_NEWEST, OVERWRITE_OLDEST, EXCEPTION };
enum DirectionType {FIFO,     LIFO        };


//! cyclic queue of elements of type T that reside in a memory area allocated via an Allocator

/*! a SharedCyclicQueue stores a number of elements in an Allocator -governed memory area in a 
 *  thread-safe way, using semaphores as counters for free and occupied cells.
 * 
 * One instance accessing the SharedCyclicQueue must initialize the 
 */

template <class T> class SharedCyclicQueue
{
	
public:
	//! initializing constructor (see initialize())
	SharedCyclicQueue( Allocator* ThisAllocator, 
					   bool IsMaster = false, 
					   unsigned long Size = 0, 
					   DirectionType Direction = FIFO, 
					   BlockingType Blocking = BLOCKING
					  );
					   
	//! non-initializing constructor
	SharedCyclicQueue();
	
	//! initialization method
	void initialize( Allocator* ThisAllocator, 
							 bool IsMaster = false, 
							 unsigned long Size = 0, 
							 DirectionType Direction = FIFO, 
							 BlockingType Blocking = BLOCKING
					);
	/*!< \param[in] ThisAllocator Reference to Allocator to use for the allocation of control and data
	 *                            structures of the queue
	 *   \param[in] IsMaster      Flag indicating wether the calling instance shall be the master instance,
	 *                            and whether the longernal data structures shall be initialized, rather
	 *                            than assumed to be initialized by another master connecting.
	 *                       	
	 *   \param[in] Size		  Numer of cells in the queue. Applicable only for master instance.
	 * 
	 *   \param[in] Direction     Input-Output behaviour of queue (LIFO/FIFO). Applicable only for master.
	 * 
	 *   \param[in] Blocking      Handling of writes to full queue. Applicable only for master.
	 * 
	 *   If the control and data structures required for a queue with the given settings could not
	 *   be allocated, the function returns with an AllocationException.
	 */


	//! set the handler called when overwriting of cells in the queue
	void setOverwriteCallback( Callback* OverwriteCallback );
	
	//! set the handler called for any update to the queue
	void setUpdateCallback( Callback* UpdateCallback );

	//! push data longo the next free cell of the queue
	void push( const T Value, const ASAAC_Time& Timeout = TimeZero );
	
	//! pull data from the next allocated cell in the queue
	T pop( const ASAAC_Time& Timeout = TimeZero );

	//! wait for free cells in the queue to become available
	void waitForFreeCells( const ASAAC_Time& Timeout = TimeInfinity );
	
	//! wait for cells in the queue to become allocated with data
	void waitForAvailableData( const ASAAC_Time& Timeout = TimeInfinity );

	//! get number of free cells in the queue
	unsigned long getFreeCells() const;
	
	//! check for empty state of the queue
	bool isEmpty() const;

	virtual ~SharedCyclicQueue(); 
	
	virtual void deinitialize();

	//! predict size in Allocator required by control and data structures of the queue
	static size_t predictSize( unsigned long Size );


	struct GlobalCyclicQueueData {
		unsigned long Size; 			// Total Size of Buffer (non-resizeable!)
		unsigned long Free;			// Number of free Cells in Buffer
		long NextWriteCell;			// Cell number longo which the next Write operation will commence
		long NextReadCell;			// Cell number which will be read from in next pop() call
		DirectionType Direction;	// FIFO or LIFO Queue?
		BlockingType Blocking;		// Blocking (wait for ClearToWrite/ClearToRead) or
									// Nonblocking (signal Exception when data unavailable)
	};

protected:
	Shared<GlobalCyclicQueueData>  m_GlobalData;	// Interprocess Global Queue Data
	Shared<T> 					   m_Buffer;		// Interprocess Global Queue Buffer
	bool						   m_IsInitialized;
	
	Semaphore					   m_WriteSemaphore;  // Counts writeable cells
	Semaphore					   m_ReadSemaphore;   // Counts readable cells
	Semaphore                      m_AccessSemaphore; // Limits to only ONE concurrent Queue access

	Callback*				   	   m_OverwriteCallback;
	Callback*				   	   m_UpdateCallback;
	
private:
	// probit implicit copying and assignment
	SharedCyclicQueue( const SharedCyclicQueue<T>& Value );
	SharedCyclicQueue<T>& operator=( const SharedCyclicQueue<T>& Value );
};


template <class T> SharedCyclicQueue<T>::SharedCyclicQueue(  Allocator* ThisAllocator, 
															 bool IsMaster, 
															 unsigned long Size, 
															 DirectionType Direction, 
															 BlockingType Blocking ) :
															 
															 m_IsInitialized(false),
															 m_OverwriteCallback( NULL ),
															 m_UpdateCallback( NULL )
{
	// Call the explicit initialization
	initialize( ThisAllocator, IsMaster, Size, Direction, Blocking );
}


template <class T> SharedCyclicQueue<T>::SharedCyclicQueue() : m_IsInitialized( false ),  m_OverwriteCallback( NULL ), m_UpdateCallback( NULL )
{
}


template <class T> SharedCyclicQueue<T>::~SharedCyclicQueue()
{
}


template <class T> void SharedCyclicQueue<T>::initialize( Allocator* ThisAllocator, 
							 									  bool IsMaster, 
							 									  unsigned long Size, 
							 									  DirectionType Direction, 
							 									  BlockingType Blocking )
{
	// Check for Double Initialization. If so, throw exception!
	if ( m_IsInitialized ) 
		throw DoubleInitializationException(LOCATION);

	m_IsInitialized = true;
	
	try {
		// Initialize Global Data and Buffer.
		m_GlobalData.initialize( ThisAllocator, 1 );
		
		//Instantiate Semaphore and Events
		// Their global Data shall reside in the already initialized IP m_GlobalData structure
	
		m_ReadSemaphore.initialize( ThisAllocator, IsMaster, 0 );      // no readable cells to begin with
		m_WriteSemaphore.initialize( ThisAllocator, IsMaster, Size );  // <Size> cells are writable to begin with
		m_AccessSemaphore.initialize( ThisAllocator, IsMaster, 1 );    // only ONE concurrent queue access


		// Only Master initializes global data. Slaves will use this data
		if ( IsMaster )
		{
			m_GlobalData->NextReadCell  = 0;
		
			m_GlobalData->NextWriteCell = 0;
			m_GlobalData->Size = Size;
			m_GlobalData->Free = Size;
		
			m_GlobalData->Direction = Direction;
			m_GlobalData->Blocking  = Blocking;
		}
		// After this point, no Parametric data shall be used, apart from
		// Allocator and IsMaster
		
		if ( ( ! IsMaster ) && ( Size != 0 ) )
		{
			m_Buffer.initialize( ThisAllocator, Size );
		}
		else
		{
			m_Buffer.initialize( ThisAllocator, m_GlobalData->Size );
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing SharedCyclicQueue", LOCATION);
        // If something goes amiss, clean up
		
		deinitialize();
		
		throw;
	}
}


template <class T>  void SharedCyclicQueue<T>::setOverwriteCallback( Callback* OverwriteCallback )
{
	if (( m_OverwriteCallback != NULL ) && ( OverwriteCallback != NULL ))
		throw OSException("Callback already set", LOCATION);
	
	m_OverwriteCallback = OverwriteCallback;
}




template <class T> void SharedCyclicQueue<T>::setUpdateCallback( Callback* UpdateCallback )
{
	if (( m_UpdateCallback != NULL ) && ( UpdateCallback != NULL )) 
		throw OSException("Callback already set", LOCATION);
	
	m_UpdateCallback = UpdateCallback;
}



template <class T> void SharedCyclicQueue<T>::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	// release queue buffer
	m_Buffer.deinitialize();

	m_OverwriteCallback = NULL;
	m_UpdateCallback    = NULL;

	// release semaphores
	m_AccessSemaphore.deinitialize();
	m_WriteSemaphore.deinitialize();
	m_ReadSemaphore.deinitialize();
	
	// release shared data
	m_GlobalData.deinitialize();
	
	m_IsInitialized = false;
}


template <class T> void SharedCyclicQueue<T>::push( const T Value, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	// If the queue is configured to be blocking, wait for free slots, with timeout
	if ( m_GlobalData->Blocking == BLOCKING )
	{
		m_WriteSemaphore.wait( Timeout );
	}

	// Get exclusive Queue access before doing anything
	// This will only be blocked for VERY VERY SHORT TIME
	// so, don't apply a timeout for this.
	ProtectedScope Access( "Pushing an element in SharedCyclicQueue", m_AccessSemaphore );

	// This cell will now be overwritten. Call Overwrite Callback if applicable.
	if (( m_GlobalData->Free == 0 ) && ( m_OverwriteCallback != NULL ))
	{
		m_OverwriteCallback->call( static_cast<void*>(&(m_Buffer[ m_GlobalData->NextWriteCell ])) );
	}
	

	// Write data longo buffer
	m_Buffer[m_GlobalData->NextWriteCell] = Value;
	
	
	// If a formerly empty cell was written longo,
	if ( m_GlobalData->Free > 0 )
	{
		m_GlobalData->Free--;    // decrease the number of free cells
		m_ReadSemaphore.post();  // and add one to the number of readable cells
	}


	// TODO: ADVANCE POINTERS
	if ( m_GlobalData->Direction == FIFO )
	{
		if ( m_GlobalData->Free > 0 ) // Normal Pointer advancement
		{
			m_GlobalData->NextWriteCell = ( m_GlobalData->NextWriteCell + 1 ) % m_GlobalData->Size;
		}
		else // Buffer FULL!
		{
			switch ( m_GlobalData->Blocking ) {
				
				case BLOCKING: // Blocking -> Normal Advancement is safe.
				case EXCEPTION:
					m_GlobalData->NextWriteCell = ( m_GlobalData->NextWriteCell + 1 ) % m_GlobalData->Size;
					break;
					
				case OVERWRITE_NEWEST:
					// Don't advance pointers at all!
					break;
					
				case OVERWRITE_OLDEST:
					m_GlobalData->NextWriteCell = ( m_GlobalData->NextWriteCell + 1 ) % m_GlobalData->Size;
					m_GlobalData->NextReadCell = m_GlobalData->NextWriteCell;
					break;
					
				}
		}
	}
	else // Direction == LIFO
	{
		if ( m_GlobalData->Free > 0 ) // Normal Pointer advancement
		{
			m_GlobalData->NextReadCell = m_GlobalData->NextWriteCell;
			m_GlobalData->NextWriteCell = ( m_GlobalData->NextWriteCell + 1 ) % m_GlobalData->Size;
		}
		else // Buffer FULL!
		{
			switch ( m_GlobalData->Blocking ) {
				
				case BLOCKING: // Blocking -> Normal pointer advancement.
				case EXCEPTION:
					m_GlobalData->NextReadCell = m_GlobalData->NextWriteCell;
					m_GlobalData->NextWriteCell = ( m_GlobalData->NextWriteCell + 1 ) % m_GlobalData->Size;
					
					break;
					
				case OVERWRITE_NEWEST:
					// Only advance read pointer, and only to current Cell
					m_GlobalData->NextReadCell = m_GlobalData->NextWriteCell;
					break;
					
				case OVERWRITE_OLDEST:
					// advance both normally, killing old cells
					m_GlobalData->NextReadCell = m_GlobalData->NextWriteCell;
					m_GlobalData->NextWriteCell = ( m_GlobalData->NextWriteCell + 1 ) % m_GlobalData->Size;
					break;
					
			}
		}
	}


	if ( m_UpdateCallback != NULL )
	{
		m_UpdateCallback->call( static_cast<void*>( &(*m_GlobalData) ));
	}
}


template <class T> T SharedCyclicQueue<T>::pop( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	// wait for data to be available or for timeout
	m_ReadSemaphore.wait( Timeout );

	// get exclusive queue access
	// This will only be blocked for VERY VERY SHORT TIME,
	// so don't use a timeout with this.
	ProtectedScope Access( "Popping an element from SharedCyclicQueue", m_AccessSemaphore );

	// Read Value from Queue Buffer
	T Value = m_Buffer[m_GlobalData->NextReadCell];
	
	// Increase number of Free Cells in Queue
	if ( m_GlobalData->Free < m_GlobalData->Size )
	{
		m_GlobalData->Free ++;

		// BLOCKING queues require the write semaphore. increase the counter
		if ( m_GlobalData->Blocking == BLOCKING ) 
		{
			m_WriteSemaphore.post();
		}
	}

	if ( m_GlobalData->Direction == FIFO )
	{
			m_GlobalData->NextReadCell = ( m_GlobalData->NextReadCell + 1 ) % m_GlobalData->Size;
	}
	else // Direction == LIFO
	{
			m_GlobalData->NextReadCell = ( m_GlobalData->NextReadCell + m_GlobalData->Size - 1 ) % m_GlobalData->Size;
			m_GlobalData->NextWriteCell = ( m_GlobalData->NextReadCell + 1 ) % m_GlobalData->Size;
	}
	

	if ( m_UpdateCallback != NULL )
	{
		m_UpdateCallback->call( static_cast<void*>( &(*m_GlobalData) ));
	}

	return Value;
}



template <class T> void SharedCyclicQueue<T>::waitForFreeCells( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	try
	{
		// If this queue is nonblocking, there are always 'free cells'.
		if ( m_GlobalData->Blocking == BLOCKING )
		{ 
			// Wait for WriteSemaphore
			m_WriteSemaphore.wait( Timeout );
			
			// and if you got it, release it rightaway.
			m_WriteSemaphore.post();
			
			// Note that, unless explicitely protected, another thread may
			// push longo the queue still after this call has been done,
			// thereby blocking it from writing again.
		}
	}
	catch ( ASAAC_Exception &e )
	{
        e.addPath("Error waiting for free cells", LOCATION);

        throw;
	}
}



template <class T> void SharedCyclicQueue<T>::waitForAvailableData( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	try
	{
		// Wait for ReadSemaphore
		m_ReadSemaphore.wait( Timeout );
		
		// and if you got it, release it rightaway.
		m_ReadSemaphore.post();
		
		// Note that, unless explicitely protected, another thread may
		// push longo the queue still after this call has been done,
		// thereby blocking it from writing again.	
	}
	catch ( ASAAC_Exception &e )
	{
        e.addPath("Error waiting for free cells", LOCATION);

        throw;
	}
}



template <class T> unsigned long SharedCyclicQueue<T>::getFreeCells() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	long iFree;

	iFree = m_GlobalData->Free;

	return iFree;
}


template <class T> bool SharedCyclicQueue<T>::isEmpty() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return ( m_GlobalData->Free == m_GlobalData->Size );
}


template <class T> size_t SharedCyclicQueue<T>::predictSize( unsigned long Size )
{
	return ( Shared<T>::predictSize( Size ) +
			 Shared<GlobalCyclicQueueData>::predictSize() +
			 3 * Semaphore::predictSize() );
}



#endif /*SHAREDCYCLICQUEUE_HH_*/
