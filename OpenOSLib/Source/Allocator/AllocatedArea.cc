#include "AllocatedArea.hh"

#include "OpenOS.hh"

using namespace std;


AllocatedArea::AllocatedArea( Allocator* ParentAllocator, unsigned int Size ) : m_IsInitialized(false)
{
	initialize( ParentAllocator, Size );
}


AllocatedArea::AllocatedArea() : m_IsInitialized(false)
{
}


void AllocatedArea::initialize( Allocator* ParentAllocator, unsigned int Size )
{
	if ( m_IsInitialized )
	{
		throw OSException("Double Initialisation", LOCATION);
	}
	
	m_ParentAllocator = ParentAllocator;
	m_MemorySize      = Size;
	m_UsedMemory      = 0;
	
	try {
		m_BaseAddress = static_cast<char*>(ParentAllocator->allocate( Size ));
	}
	catch ( ASAAC_Exception& E )
	{
		m_MemorySize			= 0;
		m_ParentAllocator		= 0;	

		throw;
	}
	
	m_IsInitialized = true;
}


void AllocatedArea::deinitialize()
{
	if (m_IsInitialized == false)
		return; 
	
	m_ParentAllocator->free( m_BaseAddress );

	m_IsInitialized = false;
}
	

void AllocatedArea::reset()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_UsedMemory = 0;
}


ASAAC_Address AllocatedArea::allocate( int Size )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	int ActualSize = getDWordSize ( Size );

	// if new total memory size exceeds memory reserved in allocator, throw Exception
	if ( int(ActualSize + m_UsedMemory ) > m_MemorySize ) throw ResourceException();
	
	// get pointer to current memory slot
	void* Pointer = static_cast<void*>(&m_BaseAddress[ m_UsedMemory ]); 
	
	// update memory statistics/pointer to next memory slot
	m_UsedMemory += ActualSize;
	
	return Pointer;
}


void AllocatedArea::free(ASAAC_Address AllocatedAddress)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
}


unsigned int AllocatedArea::getSize() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_MemorySize;
}


unsigned int AllocatedArea::getFreeMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return ( m_MemorySize - m_UsedMemory );
}


unsigned int AllocatedArea::getUsedMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_UsedMemory;
}


unsigned long AllocatedArea::getAllocationCounter() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_ParentAllocator->getAllocationCounter();
}


AllocatedArea::~AllocatedArea()
{
	//deinitialize();
}
