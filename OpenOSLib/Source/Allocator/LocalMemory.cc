//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : LocalMemory.cc
//  @ Date : 12.04.2006
//  @ Author : 
//
//

#include "LocalMemory.hh"
#include "Exceptions/Exceptions.hh"

LocalMemory::LocalMemory() : m_IsInitialized(false)
{
}


void LocalMemory::initialize( long Size )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );
	
	m_IsInitialized = true;

	try
	{
		m_BaseAddress = 0;
		m_UsedMemory = 0;
		
		m_BaseAddress = new char[Size];
		
		if ( m_BaseAddress == NULL ) 
			throw OSException( "Allocation Error" );
	
		m_MemorySize = Size;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing LocalMemory", LOCATION);

		deinitialize();

		throw;
	}	
}


void LocalMemory::deinitialize()
{
	if (m_IsInitialized == false)
		return; 
	
	try
	{
		if ( m_BaseAddress != 0 )
		{
			delete[] m_BaseAddress;
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing LocalMemory", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
}


LocalMemory::~LocalMemory()
{
	deinitialize();
}


ASAAC_Address LocalMemory::allocate( long Size )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
		
	// If new total memory exceeds allocatable size, throw Exception
	if ( Size + m_UsedMemory > m_MemorySize ) 
        throw ResourceException("LocalMemory is out of memory", LOCATION);
	
	void* Pointer = static_cast<void*>(&m_BaseAddress[ m_UsedMemory ]);
	
	m_UsedMemory += Size;
	
	return Pointer;
}


void LocalMemory::free(ASAAC_Address AllocatedAddress)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
}


void LocalMemory::reset()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_UsedMemory = 0;
}


unsigned long LocalMemory::getSize() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_MemorySize;
}


unsigned long LocalMemory::getFreeMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return ( m_MemorySize - m_UsedMemory );
}


unsigned long LocalMemory::getUsedMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_UsedMemory;
}


unsigned long LocalMemory::getAllocationCounter() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return 1;
}


