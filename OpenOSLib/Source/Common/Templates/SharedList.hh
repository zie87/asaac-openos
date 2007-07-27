#ifndef SHAREDLIST_HH_
#define SHAREDLIST_HH_

#include "OpenOSIncludes.hh"
#include "Exceptions/Exceptions.hh"

#include "Shared.hh"

#include "IPC/Semaphore.hh"
#include "IPC/ProtectedScope.hh"


using namespace std;


// *************************************************************************************************
//                                   D E C L A R A T I O N
// *************************************************************************************************


//! list of elements of type T that reside in a memory area allocated via an Allocator

/*! a SharedList stores a number of elements in an Allocator -governed memory area in a 
 *  thread-safe way, using semaphores.
 * 
 * One instance accessing the SharedList must initialize the SharedMemory 
 */

template <class T> class SharedList
{
	
public:
	//! constructor
	SharedList();
	
	//! initialization method
	void initialize( Allocator* ThisAllocator, const bool IsMaster = false, const unsigned long Size = 0 );
	/*!< \param[in] ThisAllocator Reference to Allocator to use for the allocation of control and data
	 *                            structures of the queue
	 *   \param[in] IsMaster      Flag indicating wether the calling instance shall be the master instance,
	 *                            and whether the longernal data structures shall be initialized, rather
	 *                            than assumed to be initialized by another master connecting.
	 *                       	
	 *   \param[in] Size		  Numer of cells in the queue. Applicable only for master instance.
	 * 
	 *   If the control and data structures required for a list with the given settings could not
	 *   be allocated, the function returns with an AllocationException.
	 */


	//! add data into a free position in list
	long add( const T Value, const ASAAC_Time& Timeout = TimeInfinity );
	
	//! insert data at position Index in list
	long insert( const long Index, const T Value, const ASAAC_Time& Timeout = TimeInfinity );
	
	//! remove data for a given index
	void remove( const long Index, const ASAAC_Time& Timeout = TimeInfinity );

	//! get capacity
	unsigned long getSize() const;

	//! get number of cells in list
	unsigned long getCount() const;
	
	//! check for empty state of the queue
	bool isEmpty() const;

	virtual ~SharedList(); 
	
	virtual void deinitialize();

	//! predict size in Allocator required by control and data structures of the queue
	static size_t predictSize( unsigned long Size );

	typedef struct {
		unsigned long Size; 	// Total Size of Buffer (non-resizeable!)
		unsigned long Count;	// Number of Cells in Buffer
	} GlobalData;

	T &operator[]( const long Index );

protected:
	Shared<GlobalData>  	m_GlobalData;	// Interprocess Global Queue Data
	Shared<T> 				m_Buffer;		// Interprocess Global Queue Buffer
	bool					m_IsInitialized;
	
	Semaphore               m_Semaphore; 

private:
	// probit implicit copying and assignment
	SharedList( const SharedList<T>& Value );
	SharedList<T>& operator=( const SharedList<T>& Value );
};



template <class T> SharedList<T>::SharedList() : m_IsInitialized( false )
{
}



template <class T> SharedList<T>::~SharedList()
{
}



template <class T> size_t SharedList<T>::predictSize( unsigned long Size )
{
	return ( Shared<T>::predictSize( Size ) +
			 Shared<GlobalData>::predictSize() +
			 Semaphore::predictSize() );
}



template <class T> void SharedList<T>::initialize( Allocator* ThisAllocator, 
							 									  const bool IsMaster, 
							 									  const unsigned long Size )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException(LOCATION);

	m_IsInitialized = true;
	
	try 
	{
		// Initialize Global Data and Buffer.
		m_GlobalData.initialize( ThisAllocator, 1 );
		
		//Instantiate Semaphore
		m_Semaphore.initialize( ThisAllocator, IsMaster, 1 );

		// Only Master initializes global data. Slaves will use this data
		if ( IsMaster )
		{
			m_GlobalData->Size = Size;
			m_GlobalData->Count = 0;
		
		}

		m_Buffer.initialize( ThisAllocator, m_GlobalData->Size );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing SharedList", LOCATION);
		
		deinitialize();
		
		throw;
	}
}



template <class T> void SharedList<T>::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	// release list buffer
	m_Buffer.deinitialize();

	// release semaphores
	m_Semaphore.deinitialize();
	
	// release shared data
	m_GlobalData.deinitialize();
	
	m_IsInitialized = false;
}



template <class T> inline long SharedList<T>::add( const T Value, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	ProtectedScope Access( "Add an element into SharedList", m_Semaphore, Timeout );

	if (m_GlobalData->Count == m_GlobalData->Size)
		throw OSException("SharedList if full", LOCATION);
	
	long Index = m_GlobalData->Count;
	
	m_Buffer[ Index ] = Value;
	
	m_GlobalData->Count++;
	
	return Index;
}



template <class T> inline long SharedList<T>::insert( const long Index, const T Value, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	ProtectedScope Access( "Add an element into SharedList", m_Semaphore, Timeout );

	if (m_GlobalData->Count == m_GlobalData->Size)
		throw OSException("SharedList if full", LOCATION);
	
	if ( (Index < 0) || (Index > (long)m_GlobalData->Count) )
		throw OSException("Index is out of range", LOCATION);
	
	void *Target = &m_Buffer[Index+1];
	void *Source = &m_Buffer[Index];
	long Size = (m_GlobalData->Count-Index) * sizeof(T);

	memmove( Target, Source, Size );

	m_Buffer[ Index ] = Value;
	
	m_GlobalData->Count++;	
	
	return Index;
}



template <class T> inline void SharedList<T>::remove( const long Index, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	ProtectedScope Access( "Remove an element from SharedList", m_Semaphore, Timeout );

	if (( Index < 0 ) || ( Index >= (long)m_GlobalData->Count ))
		throw OSException("Index is out of range", LOCATION);

	void *Target = &m_Buffer[Index];
	void *Source = &m_Buffer[Index+1];
	long Size = (m_GlobalData->Count-Index-1) * sizeof(T);

	memmove( Target, Source, Size );
	
	m_GlobalData->Count--;
}



template <class T> inline unsigned long SharedList<T>::getSize() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return m_GlobalData->Size; 
}



template <class T> inline unsigned long SharedList<T>::getCount() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return m_GlobalData->Count; 
}

	

template <class T> inline bool SharedList<T>::isEmpty() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return ( m_GlobalData->Count == 0 );
}



template <class T> inline T& SharedList<T>::operator[]( const long Index )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return m_Buffer[ Index ];
}


#endif /*SHAREDLIST_HH_*/
