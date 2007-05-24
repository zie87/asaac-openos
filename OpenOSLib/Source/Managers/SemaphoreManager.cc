#include "Managers/SemaphoreManager.hh"
#include "Managers/PrivateIdManager.hh"

#include "Exceptions/Exceptions.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/AllocatedArea.hh"

#include "IPC/ProtectedScope.hh"

using namespace std;

SemaphoreManager::SemaphoreManager() : m_IsInitialized(false)
{
	printf("Initializing SemaphoreManager..."); fflush(stdout);
	initialize();
	printf("done\n"); fflush(stdout);
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
	if ( m_IsInitialized ) throw DoubleInitializationException();
	
	m_NumberOfSemaphores = OS_MAX_NUMBER_OF_SEMAPHORES;
	
	m_GlobalAllocator.initialize( ( m_NumberOfSemaphores + 1 ) * Semaphore::predictSize() );

	m_AccessSemaphore.initialize( &m_GlobalAllocator, true, 1 );	
																	  
	for ( unsigned long Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		m_Semaphores[ Index ].SemaphoreAllocator.initialize( & m_GlobalAllocator, Semaphore::predictSize() );
																		
		m_Semaphores[ Index ].SemaphoreId = 0;
	}

	m_IsInitialized = true;
}



void SemaphoreManager::deinitialize()
{
	for ( unsigned long Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		if ( m_Semaphores[ Index ].SemaphoreObject != 0 ) delete ( m_Semaphores[ Index ].SemaphoreObject );
		
		m_Semaphores[ Index ].SemaphoreAllocator.deinitialize();
	}

	m_GlobalAllocator.deinitialize();

	m_IsInitialized = false;
}



long SemaphoreManager::findSemaphoreByName( const ASAAC_CharacterSequence& SemaphoreName )
{
	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		if ( SemaphoreName.size != m_Semaphores[ Index ].SemaphoreName.size ) continue;
		
		if ( strncmp( SemaphoreName.data, m_Semaphores[ Index ].SemaphoreName.data, SemaphoreName.size ) ) continue;
		
		break;
	}
	
	if ( Index == m_NumberOfSemaphores ) return -1;
	
	return Index;
}


long SemaphoreManager::findSemaphoreByPrivateId( const ASAAC_PrivateId SemaphoreId )
{
	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfSemaphores; Index ++ )
	{
		if ( m_Semaphores[ Index ].SemaphoreId == SemaphoreId ) break;
	}
	
	if ( Index == m_NumberOfSemaphores ) return -1;
	
	return Index;
}


ASAAC_ResourceReturnStatus SemaphoreManager::createSemaphore( const ASAAC_CharacterSequence& Name,
										  			    ASAAC_PrivateId& SemaphoreId,
										  				unsigned long InitialValue,
										  				unsigned long MaximumValue,
										  				ASAAC_QueuingDiscipline Discipline )
{
	if ( Name.size > OS_MAX_STRING_SIZE ) return ASAAC_RS_ERROR;
	
	if ( MaximumValue < 1 ) return ASAAC_RS_ERROR;
	if ( InitialValue > MaximumValue ) return ASAAC_RS_ERROR;
	
	
	assert( Name.size < OS_MAX_STRING_SIZE );

	ProtectedScope Access( "Creating a semaphore", m_AccessSemaphore );

	long Index = findSemaphoreByName( Name );
	
	if ( Index < 0 )
	{
		// New semaphore has to be created
		
		// Find first free spot
		Index = findSemaphoreByPrivateId( 0 );
		
		if ( Index < 0 ) // no more free Semaphore Slots?
		{
			return ASAAC_RS_ERROR;
		}

		m_Semaphores[ Index ].SemaphoreObject = new Semaphore;
		
		if ( m_Semaphores[ Index ].SemaphoreObject == 0 ) return ASAAC_RS_ERROR;
		
		m_Semaphores[ Index ].SemaphoreId = PrivateIdManager::getInstance()->getNextId();

		m_Semaphores[ Index ].SemaphoreName.size = Name.size;
		strncpy( m_Semaphores[ Index ].SemaphoreName.data, Name.data, Name.size );
		
		m_Semaphores[ Index ].SemaphoreDiscipline = Discipline;
		
		m_Semaphores[ Index ].SemaphoreObject->initialize( & (m_Semaphores[ Index ].SemaphoreAllocator), true, InitialValue, MaximumValue );
		
		SemaphoreId = m_Semaphores[ Index ].SemaphoreId;
		
		return ASAAC_RS_SUCCESS;
	}
	
	if ( m_Semaphores[ Index ].SemaphoreDiscipline != Discipline )
	{
		return ASAAC_RS_ERROR;
	}
	
	SemaphoreId = m_Semaphores[ Index ].SemaphoreId;
	
	return ASAAC_RS_RESOURCE;
}



ASAAC_ReturnStatus SemaphoreManager::deleteSemaphore( ASAAC_PrivateId SemaphoreId )
{
	ProtectedScope	Access( "Deleting a semaphore", m_AccessSemaphore );
	
	long Index = findSemaphoreByPrivateId( SemaphoreId );
	
	if ( Index < 0 )
	{
		return ASAAC_ERROR;
	}
	
	if ( m_Semaphores[ Index ].SemaphoreObject->getWaitingThreads() > 0 ) 
	{
		return ASAAC_ERROR;
	}
	
	delete m_Semaphores[ Index ].SemaphoreObject;
	
	m_Semaphores[ Index ].SemaphoreObject = 0;
	
	m_Semaphores[ Index ].SemaphoreId = 0;
	m_Semaphores[ Index ].SemaphoreName.size = 0;

	m_Semaphores[ Index ].SemaphoreAllocator.reset();
	
	return ASAAC_ERROR;
}

	

ASAAC_TimedReturnStatus SemaphoreManager::waitForSemaphore( ASAAC_PrivateId SemaphoreId,
												      const ASAAC_Time& Timeout )
{
	long Index = findSemaphoreByPrivateId( SemaphoreId );
	
	if ( Index < 0 )
	{
		return ASAAC_TM_ERROR;
	}
	
	return m_Semaphores[ Index ].SemaphoreObject->wait( Timeout );
}



ASAAC_ReturnStatus SemaphoreManager::postSemaphore( ASAAC_PrivateId SemaphoreId )
{
	long Index = findSemaphoreByPrivateId( SemaphoreId );
	
	if ( Index < 0 )
	{
		return ASAAC_ERROR;
	}
	
	return m_Semaphores[ Index ].SemaphoreObject->post();
}
	
	
	

ASAAC_ReturnStatus SemaphoreManager::getSemaphoreStatus( ASAAC_PrivateId SemaphoreId,
									 unsigned long& CurrentValue,
									 unsigned long& WaitingCallers )
{
	long Index = findSemaphoreByPrivateId( SemaphoreId );

	if ( Index < 0 )
	{
		return ASAAC_ERROR;
	}
	
	CurrentValue   = m_Semaphores[ Index ].SemaphoreObject->getCount();
	WaitingCallers = m_Semaphores[ Index ].SemaphoreObject->getWaitingThreads();
	
	return ASAAC_SUCCESS;
} 		
	
									 
ASAAC_ReturnStatus SemaphoreManager::getSemaphoreId( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& SemaphoreId )
{
	long Index = findSemaphoreByName( Name );
	
	if ( Index < 0 )
	{
		return ASAAC_ERROR;
	}
	
	SemaphoreId = m_Semaphores[ Index ].SemaphoreId;
	
	return ASAAC_SUCCESS;
}
	

