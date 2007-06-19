#ifndef SHARED_HH_
#define SHARED_HH_

#include "OpenOSIncludes.hh"
#include "Exceptions/Exceptions.hh"

#include "Allocator/Allocator.hh"

// *************************************************************************************************
//                                   D E C L A R A T I O N
// *************************************************************************************************


//!< data object that can be allocated via an Allocator

template <class T> class Shared
{
	
protected:
	Allocator*		     m_Allocator;
	T*				     m_Global;
	bool				 m_IsInitialized;

public:

	//! default constructor, leaves object uninitialized
	Shared();	
	
	//! initializing constructor (see initialize())
	Shared( Allocator* ThisAllocator, unsigned long Count = 1 );

	//! initialization method, called by initializing constructor
	virtual void initialize( Allocator* Allocator, unsigned long Count = 1 );
	/*!< \param[in] Allocator Reference to Allocator that shall be used to allocate and free
	 *                        the memory required by the data provided by this Shared object
	 * 
	 *   \param[in] Count     Number of Objects of type T to be allocated and accessed via this
	 *                        Shared object.
	 * 
	 * If the memory provided by the Allocator is insufficient to store the data as requested,
	 * initialize() will throw an AllocationException.
	 */
	
	virtual ~Shared();
	
	//! explicit deinitialization, called by destructor
	virtual void deinitialize();
	
	//! explicit read access method for shared variable
	inline T& getValue() const;

	//! explicit write access method for shared variable
	inline void setValue( const T& Value );

	//! get location of data in virtual memory
	inline T* getLocation() const;
	
	
	//! implicit pointer-style access method to data
	inline T* operator->() const;
	/*!< This definition allows structures stored in this Shared object to
	 *   be accessed as if they were accessed through a pointer, enhancing
	 *   readability of code.
	 */

	//! implicit pointer-style access method to data
	inline T& operator*() const;
	/*!< This definition allows data stored in this Shared object to
	 *   be accessed as if they were accessed through a pointer, enhancing
	 *   readability of code.
	 */


	//! implicit array-style access method to data
	inline T& operator[]( long Index ) const;
	/*!< This definition allows array data stored in this Shared object to
	 *   be accessed as if they were accessed as a C array, enhancing
	 *   readability of code.
	 */
	
	//! get size of a single data object
	inline size_t getSize() const;
	
	//! predict total size required by all data objects inside this Shared instance
	static size_t predictSize(unsigned long Count = 1);
	
private:
	// disable implicit copying and assignment
	Shared( const Shared<T>& Value );
	Shared<T>& operator=(const Shared<T>& x);
	
	unsigned long m_Count;
};




template <class T> Shared<T>::Shared() : m_IsInitialized(false)
{ 
}


template <class T> Shared<T>::Shared( Allocator* ThisAllocator, unsigned long Count ) 
					 : m_IsInitialized(false)
{
	initialize( ThisAllocator, Count );
}


template <class T> void Shared<T>::initialize( Allocator* ThisAllocator, unsigned long Count ) 
{
	m_Count = Count;
	
	m_IsInitialized = true;
	
	try
	{        
		m_Global = (T*)ThisAllocator->allocate( sizeof( T ) * m_Count );
        //cout << "##############" << typeid( T ).name() << " count:" << Count << " address:" << m_Global << endl; 
	
		m_Allocator = ThisAllocator;
	}
	catch (...)
	{
		deinitialize();
	}
}


template <class T> Shared<T>::~Shared()
{
}


template <class T> void Shared<T>::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;

	try
	{		
		// inform allocator the allocated memory is no longer required
		m_Allocator->free((void*)(m_Global));
	
		// unlink from allocator
		m_Allocator = 0;
	}
	catch (...)
	{
	}
	
	m_IsInitialized = false;
}


template <class T> inline T& Shared<T>::getValue() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
		
	return *m_Global;
}


template <class T> inline void Shared<T>::setValue( const T& Value )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	*m_Global = Value;
}


template <class T> inline T* Shared<T>::getLocation() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_Global;
}


template <class T> inline T* Shared<T>::operator->() const
{ 
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_Global; 
}


template <class T> inline T& Shared<T>::operator*() const 
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return *m_Global;
}


template <class T> inline T& Shared<T>::operator[]( long Index ) const 
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_Global[Index]; 
}


template <class T> inline size_t Shared<T>::getSize() const
{
	return sizeof( T );
}


template <class T> size_t Shared<T>::predictSize(unsigned long Count)
{
	return getDWordSize( sizeof(T) * Count );
}



#endif /*SHAREABLE_HH_*/
