#include "AllocatedArea.hh"

#include "OpenOSObject.hh"

using namespace std;


AllocatedArea::AllocatedArea() : m_IsInitialized(false)
{
}


void AllocatedArea::initialize( Allocator* ParentAllocator, unsigned long Size )
{
	if ( m_IsInitialized )
		throw OSException("Double Initialisation", LOCATION);

	m_IsInitialized = true;

	try
	{
		m_ParentAllocator = ParentAllocator;
		m_MemorySize      = Size;
		m_UsedMemory      = 0;
		
		m_BaseAddress = static_cast<char*>(ParentAllocator->allocate( Size ));
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing AllocatedArea", LOCATION);
		
		deinitialize();
		
		throw;
	}
}


void AllocatedArea::deinitialize()
{
	if (m_IsInitialized == false)
		return; 
	
	try
	{
		m_ParentAllocator->free( m_BaseAddress );
		m_MemorySize = 0;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing AllocatedArea", LOCATION);
		
		e.raiseError();
	}

	m_IsInitialized = false;
}
	

void AllocatedArea::reset()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_UsedMemory = 0;
}


ASAAC_Address AllocatedArea::allocate( long Size )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	long ActualSize = getDWordSize ( Size );

	// if new total memory size exceeds memory reserved in allocator, throw Exception
	if ( long(ActualSize + m_UsedMemory ) > m_MemorySize ) 
        throw ResourceException("AllocatedArea is out of memory", LOCATION);
	
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


unsigned long AllocatedArea::getSize() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_MemorySize;
}


unsigned long AllocatedArea::getFreeMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return ( m_MemorySize - m_UsedMemory );
}


unsigned long AllocatedArea::getUsedMemory() const
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
