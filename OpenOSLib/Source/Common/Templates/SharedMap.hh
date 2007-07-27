#ifndef SHAREDMAP_HH_
#define SHAREDMAP_HH_

#include "OpenOSIncludes.hh"
#include "Exceptions/Exceptions.hh"

#include "SharedList.hh"

#include "IPC/Semaphore.hh"
#include "IPC/ProtectedScope.hh"


// *************************************************************************************************
//                                   D E C L A R A T I O N
// *************************************************************************************************


//! list of elements of type T that reside in a memory area allocated via an Allocator

/*! a SharedList stores a number of elements in an Allocator -governed memory area in a 
 *  thread-safe way, using semaphores.
 * 
 * One instance accessing the SharedList must initialize the SharedMemory 
 */

template <class TID, class T> class SharedMap
{
	
public:
	//! constructor
	SharedMap();
	
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
	long add( const TID Id,  const T Value, const ASAAC_Time& Timeout = TimeInfinity );
	
	//! remove data for a given index
	void remove( const TID Id, const ASAAC_Time& Timeout = TimeInfinity );

	long indexOf( const TID Id );

	TID idOf ( const long Index );

	//! get capacity
	unsigned long getSize() const;

	//! get number of cells in list
	unsigned long getCount() const;
	
	//! check for empty state of the queue
	bool isEmpty() const;

	virtual ~SharedMap(); 
	
	virtual void deinitialize();

	//! predict size in Allocator required by control and data structures of the queue
	static size_t predictSize( unsigned long Size );

	typedef struct {
		TID Id;
		T	Data;
	} MapData;

	T &operator[]( const long Index );

protected:
	SharedList<MapData>  	m_List;	
	bool					m_IsInitialized;

	Semaphore               m_Semaphore; 

private:
	// probit implicit copying and assignment
	SharedMap( const SharedMap<TID, T>& Value );
	SharedMap<TID, T>& operator=( const SharedMap<TID, T>& Value );
};



template <class TID, class T> SharedMap<TID, T>::SharedMap() : m_IsInitialized( false )
{
}



template <class TID, class T> SharedMap<TID, T>::~SharedMap()
{
}



template <class TID, class T> size_t SharedMap<TID, T>::predictSize( unsigned long Size )
{
	return ( SharedList<MapData>::predictSize(Size)+
	         Semaphore::predictSize() );
}



template <class TID, class T> void SharedMap<TID, T>::initialize( Allocator* ThisAllocator, 
							 									  const bool IsMaster, 
							 									  const unsigned long Size )
{
	try 
	{
		m_List.initialize( ThisAllocator, IsMaster, Size );
		
		//Instantiate Semaphore
		m_Semaphore.initialize( ThisAllocator, IsMaster, 1 );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing SharedMap", LOCATION);
		
		deinitialize();
		
		throw;
	}
}



template <class TID, class T> void SharedMap<TID, T>::deinitialize()
{
	m_List.deinitialize();
	
	m_Semaphore.deinitialize();
}



template <class TID, class T> inline long SharedMap<TID, T>::add( const TID Id, const T Value, const ASAAC_Time& Timeout )
{
	ProtectedScope Access( "Add an element into SharedMap", m_Semaphore, Timeout );

	long Index = 0;
	
	long l = 0;
	long u = m_List.getCount();
	
	while ( u != l )
	{
		Index = l + div(u-l, (long)2).quot;

		if (m_List[Index].Id == Id)
			throw OSException("Data with dedicated Id already exist", LOCATION);
	
		if (m_List[Index].Id < Id)
			l = Index+1;
		else u = Index;			
	}
	
	Index = l;
	
	const MapData Data =  {Id, Value};
	m_List.insert(Index, Data, Timeout);
	
	return Index;
}



template <class TID, class T> inline void SharedMap<TID, T>::remove( const TID Id, const ASAAC_Time& Timeout )
{
	try
	{
		m_List.remove(indexOf(Id), Timeout);
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Data with dedicated Id not found", LOCATION);
		
		throw;
	}
}



template <class TID, class T> inline unsigned long SharedMap<TID, T>::getSize() const
{
	return m_List.getSize();
}



template <class TID, class T> inline unsigned long SharedMap<TID, T>::getCount() const
{
	return m_List.getCount();
}

	

template <class TID, class T> inline bool SharedMap<TID, T>::isEmpty() const
{
	return m_List.isEmpty();
}



template <class TID, class T> inline T& SharedMap<TID, T>::operator[]( const long Index )
{
	return m_List[ Index ].Data;
}



template <class TID, class T> inline long SharedMap<TID, T>::indexOf( const TID Id )
{
	ProtectedScope Access( "Determine index of an element in SharedMap", m_Semaphore );

	long i;
	
	long l = 0;
	long u = m_List.getCount();
	
	while ( u != l )
	{
		i = l + div(u-l, (long)2).quot;

		if (m_List[i].Id == Id)
			return i;
	
		if (m_List[i].Id < Id)
			l = i+1;
		else u = i;			
	}
	
	return -1;
}



template <class TID, class T> inline TID SharedMap<TID, T>::idOf( const long Index )
{
	return m_List[Index].Id;
}



#endif /*SHAREDMAP_HH_*/
