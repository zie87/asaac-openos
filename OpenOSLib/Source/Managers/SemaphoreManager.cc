#include "Managers/SemaphoreManager.hh"
#include "Managers/PrivateIdManager.hh"

#include "Exceptions/Exceptions.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/AllocatedArea.hh"

#include "IPC/ProtectedScope.hh"

using namespace std;

SemaphoreManager::SemaphoreManager() : m_IsInitialized(false)
{
	initialize();
}


SemaphoreManager::~SemaphoreManager()
{
	deinitialize();
}


SemaphoreManager* SemaphoreManager::getInstance()
{
	static SemaphoreManager ThisManager;
	
	return &ThisManager;
}


void SemaphoreManager::initialize()
{
	if ( m_IsInitialized == true ) 
		throw DoubleInitializationException();
	
	m_IsInitialized = true;

	try
	{
		m_NumberOfSemaphores = OS_MAX_NUMBER_OF_SEMAPHORES;
		
		m_GlobalAllocator.initialize( ( m_NumberOfSemaphores + 1 ) * Semaphore::predictSize() );
	
		m_AccessSemaphore.initialize( &m_GlobalAllocator, true, 1 );	
																		  
		for ( unsigned long Index = 0; Index < m_NumberOfSemaphores; Index ++ )
		{
			m_Semaphores[ Index ].SemaphoreAllocator.initialize( & m_GlobalAllocator, Semaphore::predictSize() );
																			
			m_Semaphores[ Index ].SemaphoreId = OS_UNUSED_ID;
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing SemaphoreManager", LOCATION);
		
		deinitialize();
		
		throw;
	}
}



void SemaphoreManager::deinitialize()
{
	if ( m_IsInitialized == false )
		return;

	for ( unsigned long Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		if ( m_Semaphores[ Index ].SemaphoreObject != 0 ) 
			delete ( m_Semaphores[ Index ].SemaphoreObject );
		
		m_Semaphores[ Index ].SemaphoreAllocator.deinitialize();
	}

	m_GlobalAllocator.deinitialize();

	m_IsInitialized = false;
}



long SemaphoreManager::findSemaphoreByName( const ASAAC_CharacterSequence& SemaphoreName )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		if ( SemaphoreName.size != m_Semaphores[ Index ].SemaphoreName.size ) 
			continue;
		
		if ( strncmp( SemaphoreName.data, m_Semaphores[ Index ].SemaphoreName.data, SemaphoreName.size ) ) 
			continue;
		
		break;
	}
	
	if ( Index == m_NumberOfSemaphores ) return -1;
	
	return Index;
}


long SemaphoreManager::findSemaphoreByPrivateId( const ASAAC_PrivateId SemaphoreId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		if ( m_Semaphores[ Index ].SemaphoreId == SemaphoreId ) 
			break;
	}
	
	if ( Index == m_NumberOfSemaphores ) return -1;
	
	return Index;
}


void SemaphoreManager::createSemaphore( const ASAAC_CharacterSequence& Name,
										  			    ASAAC_PrivateId& SemaphoreId,
										  				unsigned long InitialValue,
										  				unsigned long MaximumValue,
										  				ASAAC_QueuingDiscipline Discipline )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		if ( MaximumValue < 1 ) 
			throw OSException("MaximumValue is lower then 1", LOCATION);
			
		if ( InitialValue > MaximumValue )
			throw OSException("InitialValue is bigger then MaximumValue", LOCATION);
		
		ProtectedScope Access( "Creating a semaphore", m_AccessSemaphore );
	
		long Index = findSemaphoreByName( Name );
		
		if ( Index < 0 )
		{
			// New semaphore has to be created
			
			// Find first free spot
			Index = findSemaphoreByPrivateId( OS_UNUSED_ID );
			
			if ( Index < 0 ) // no more free Semaphore Slots?
				throw OSException("no more free semaphore slots", LOCATION);
	
			m_Semaphores[ Index ].SemaphoreObject = new Semaphore;
			
			m_Semaphores[ Index ].SemaphoreId = PrivateIdManager::getInstance()->getNextId();
	
			m_Semaphores[ Index ].SemaphoreName = Name;
			
			m_Semaphores[ Index ].SemaphoreDiscipline = Discipline;
			
			m_Semaphores[ Index ].SemaphoreObject->initialize( & (m_Semaphores[ Index ].SemaphoreAllocator), true, InitialValue, MaximumValue );
			
			SemaphoreId = m_Semaphores[ Index ].SemaphoreId;
		}
		else
		{
			if ( m_Semaphores[ Index ].SemaphoreDiscipline != Discipline )
				throw OSException("Semaphore with same name, but different discipline found", LOCATION);
			
			SemaphoreId = m_Semaphores[ Index ].SemaphoreId;
			
			throw OSException("Semaphore with same name and adequate properties already created", LOCATION);
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error creating semaphore", LOCATION);
		
		throw;
	}
}



void SemaphoreManager::deleteSemaphore( ASAAC_PrivateId SemaphoreId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		ProtectedScope	Access( "Deleting a semaphore", m_AccessSemaphore );
		
		long Index = findSemaphoreByPrivateId( SemaphoreId );
		
		if ( Index < 0 )
			throw OSException("Semaphore with dedicated id not found", LOCATION);
		
		if ( m_Semaphores[ Index ].SemaphoreObject->getWaitingThreads() > 0 ) 
			throw OSException("One or more threads are still waiting for semaphore", LOCATION);
		
		delete m_Semaphores[ Index ].SemaphoreObject;
		
		m_Semaphores[ Index ].SemaphoreObject = 0;
		
		m_Semaphores[ Index ].SemaphoreId = OS_UNUSED_ID;
		m_Semaphores[ Index ].SemaphoreName.size = 0;
	
		m_Semaphores[ Index ].SemaphoreAllocator.reset();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deleting semaphore", LOCATION);
		
		throw;
	}
}

	

void SemaphoreManager::waitForSemaphore( ASAAC_PrivateId SemaphoreId,
												      const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		long Index = findSemaphoreByPrivateId( SemaphoreId );
		
		if ( Index < 0 )
			throw OSException("Semaphore with dedicated id not found", LOCATION);
		
		m_Semaphores[ Index ].SemaphoreObject->wait( Timeout );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Problem or error waiting for semaphore", LOCATION);
		
		throw;
	}
}



void SemaphoreManager::postSemaphore( ASAAC_PrivateId SemaphoreId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		long Index = findSemaphoreByPrivateId( SemaphoreId );
		
		if ( Index < 0 )
			throw OSException("Semaphore with dedicated id not found", LOCATION);
		
		m_Semaphores[ Index ].SemaphoreObject->post();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error posting semaphore", LOCATION);
		
		throw;
	}
}
	
	
	

void SemaphoreManager::getSemaphoreStatus( ASAAC_PrivateId SemaphoreId,
									 unsigned long& CurrentValue,
									 unsigned long& WaitingCallers )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		long Index = findSemaphoreByPrivateId( SemaphoreId );
	
		if ( Index < 0 )
			throw OSException("Semaphore with dedicated id not found", LOCATION);
		
		CurrentValue   = m_Semaphores[ Index ].SemaphoreObject->getCount();
		WaitingCallers = m_Semaphores[ Index ].SemaphoreObject->getWaitingThreads();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving semaphore status", LOCATION);
		
		throw;
	}
} 		
	
									 
void SemaphoreManager::getSemaphoreId( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& SemaphoreId )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		long Index = findSemaphoreByName( Name );
		
		if ( Index < 0 )
			throw OSException("Semaphore with dedicated id not found", LOCATION);
		
		SemaphoreId = m_Semaphores[ Index ].SemaphoreId;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving semaphore id", LOCATION);
		
		throw;
	}
}
	

