#ifndef OBJECTPOOL_HH_
#define OBJECTPOOL_HH_

#include "OpenOSIncludes.hh"
#include "Exceptions/Exceptions.hh"

#include "Common/SharedCyclicQueue.hh"
#include "Allocator/LocalMemory.hh"

//!< pool of objects to be dynamically claimed and released

/*!< an ObjectPool provides a central storage of a given
 * number of instances of a given object type that can be claimed and released at
 * run time.
 * 
 * Instead of returning an error on claiming unavailable instances, the calling
 * instance can decide to wait for an instance of the object to become available
 * within a given timeout.
 * 
 * In contrast to a fully dynamical instantiation of objects, claiming objects
 * from an ObjectPool does not poise the heap, and leaves the program's memory
 * usage constant.
 */

template <class T, unsigned int N> class ObjectPool {
	
	protected:
		SharedCyclicQueue<T*>  m_FreeObjectPointers;
		T					  m_Objects[N];
		LocalMemory			  m_Allocator;
	
	public:
		ObjectPool();
		
		~ObjectPool();
		
		T*   acquireObject( const ASAAC_TimeInterval& Timeout );
		//!< claim an object from the ObjectPool
		/*!< \param[in] Timeout Point of time to wait until if no instances are instantly
		 *                      available, before returning 0.
		 * 
		 *   \returns   Pointer to an instance of the requested object. 0, if no instance
		 *              could be acquired within the time specified by Timeout.
		 */
		
		void releaseObject( T* Object );
		//!< return object to ObjectPool
		/*!< \param[in] Object Pointer to instance to be returned to the ObjectPool
		 */
		
};


template <class T, unsigned int N> ObjectPool<T,N>::ObjectPool()
{
	try
	{
		m_Allocator.initialize( SharedCyclicQueue<T*>::predictSize( N ) );
	
		m_FreeObjectPointers.initialize( &m_Allocator, true, N );
	
	
		// All objects are free initially
		for ( unsigned int i = 0; i < N; i ++ )
		{
			m_FreeObjectPointers.push( &(m_Objects[i]) );
		}
	}
	catch ( ASAAC_Exception& E )
	{
		// if something goes amiss, clean up
		m_FreeObjectPointers.deinitialize();
		m_Allocator.deinitialize();

		throw;
	}
}


template <class T, unsigned int N> ObjectPool<T,N>::~ObjectPool()
{
	m_FreeObjectPointers.deinitialize();
	m_Allocator.deinitialize();
}


template <class T, unsigned int N> T* ObjectPool<T,N>::acquireObject( const ASAAC_TimeInterval& Timeout )
{
	try {
		return m_FreeObjectPointers.pop( TimeStamp(Timeout).asaac_Time() );
	}
	catch ( ResourceException &E )
	{
		return 0;
	}
}


template <class T, unsigned int N> void ObjectPool<T,N>::releaseObject( T* Object )
{
	if ( Object != 0 )
	{
		m_FreeObjectPointers.push( Object );
	}
}



#endif /*OBJECTPOOL_HH_*/
