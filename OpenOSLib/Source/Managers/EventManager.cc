#include "Managers/EventManager.hh"
#include "Exceptions/Exceptions.hh"

#include "Managers/PrivateIdManager.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/AllocatedArea.hh"

#include "IPC/ProtectedScope.hh"

EventManager::EventManager() : m_IsInitialized(false)
{
	printf("Initializing Event Manager..."); fflush(stdout);
	initialize();
	printf("done\n"); fflush(stdout);
}


EventManager::~EventManager()
{
	deinitialize();
}


EventManager* EventManager::getInstance()
{
	static EventManager ThisManager;
	
	return &ThisManager;
}


void EventManager::initialize()
{
	if ( m_IsInitialized ) throw DoubleInitializationException();
	
	m_NumberOfEvents = OS_MAX_NUMBER_OF_EVENTS;
	
	m_GlobalAllocator.initialize( m_NumberOfEvents * Event::predictSize() + Semaphore::predictSize() );
	
	m_AccessSemaphore.initialize( &m_GlobalAllocator, true, 1 );
																	  
	for ( unsigned long Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		m_Events[ Index ].EventAllocator.initialize( &m_GlobalAllocator, Event::predictSize() );
																		
		m_Events[ Index ].EventId = 0;
	}

	m_IsInitialized = true;
}


void EventManager::deinitialize()
{
	for ( unsigned long Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		if ( m_Events[ Index ].EventObject != 0 ) delete m_Events[ Index ].EventObject;
		
		m_Events[Index].EventAllocator.deinitialize();
	}
	
	m_GlobalAllocator.deinitialize();
	
	m_IsInitialized = false;
}


long EventManager::findEventByName( const ASAAC_CharacterSequence& EventName )
{
	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		if ( EventName.size != m_Events[ Index ].EventName.size ) continue;
		
		if ( strncmp( EventName.data, m_Events[ Index ].EventName.data, EventName.size ) ) continue;
		
		break;
	}
	
	if ( Index == m_NumberOfEvents ) return -1;
	
	return Index;
}


long EventManager::findEventByPrivateId( const ASAAC_PrivateId EventId )
{
	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		if ( m_Events[ Index ].EventId == EventId ) break;
	}
	
	if ( Index == m_NumberOfEvents ) return -1;
	
	return Index;
}


ASAAC_ResourceReturnStatus EventManager::createEvent( const ASAAC_CharacterSequence& Name,
										  			    ASAAC_PrivateId& EventId )
{
	if ( Name.size > OS_MAX_STRING_SIZE ) return ASAAC_RS_ERROR;
	
	assert( Name.size < OS_MAX_STRING_SIZE );

	ProtectedScope Access( "Creating an event", m_AccessSemaphore );

	long Index = findEventByName( Name );
	
	if ( Index < 0 )
	{
		// New Event has to be created
		
		// Find first free spot
		Index = findEventByPrivateId( 0 );
		
		if ( Index < 0 ) // no more free Event Slots?
		{
			m_AccessSemaphore.post();
			return ASAAC_RS_ERROR;
		}
		
		m_Events[ Index ].EventId = PrivateIdManager::getInstance()->getNextId();

		m_Events[ Index ].EventName.size = Name.size;
		strncpy( m_Events[ Index ].EventName.data, Name.data, Name.size );
		
		m_Events[ Index ].EventObject = new Event( & (m_Events[Index ].EventAllocator) );
		
		EventId = m_Events[ Index ].EventId;
		
		m_AccessSemaphore.post();

		return ASAAC_RS_SUCCESS;
	}
	

	EventId = m_Events[ Index ].EventId;
	m_AccessSemaphore.post();
	
	return ASAAC_RS_RESOURCE;
}


ASAAC_ReturnStatus EventManager::deleteEvent( ASAAC_PrivateId EventId )
{
	m_AccessSemaphore.wait();
	
	long Index = findEventByPrivateId( EventId );
	
	if ( Index < 0 )
	{
		m_AccessSemaphore.post();
		return ASAAC_ERROR;
	}
	
	if ( m_Events[ Index ].EventObject->getWaitingThreads() > 0 ) 
	{
		m_AccessSemaphore.post();
		return ASAAC_ERROR;
	}
	
	delete m_Events[ Index ].EventObject;
	m_Events[ Index ].EventObject = 0;
	
	m_Events[ Index ].EventId = 0;
	m_Events[ Index ].EventName.size = 0;
	
	m_AccessSemaphore.post();
	
	return ASAAC_ERROR;
}

	

ASAAC_TimedReturnStatus EventManager::waitForEvent( ASAAC_PrivateId EventId,
												      const ASAAC_Time& Timeout )
{
	m_AccessSemaphore.wait();

	long Index = findEventByPrivateId( EventId );
	
	m_AccessSemaphore.post();

	if ( Index < 0 )
	{
		return ASAAC_TM_ERROR;
	}
	
	return m_Events[ Index ].EventObject->waitForEvent( Timeout );
}



ASAAC_ReturnStatus EventManager::setEvent( ASAAC_PrivateId EventId )
{
	m_AccessSemaphore.wait();

	long Index = findEventByPrivateId( EventId );
	
	m_AccessSemaphore.post();

	if ( Index < 0 )
	{
		return ASAAC_ERROR;
	}
	
	m_Events[ Index ].EventObject->setEvent();
	
	return ASAAC_SUCCESS;
}
	
	

ASAAC_ReturnStatus EventManager::resetEvent( ASAAC_PrivateId EventId )
{
	m_AccessSemaphore.wait();

	long Index = findEventByPrivateId( EventId );
	
	m_AccessSemaphore.post();

	if ( Index < 0 )
	{
		return ASAAC_ERROR;
	}
	
	m_Events[ Index ].EventObject->resetEvent();
	
	return ASAAC_SUCCESS;
}
	

	

ASAAC_ReturnStatus EventManager::getEventStatus( ASAAC_PrivateId EventId,
									 ASAAC_EventStatus& CurrentValue,
									 unsigned long& WaitingCallers )
{
	m_AccessSemaphore.wait();

	long Index = findEventByPrivateId( EventId );
	

	if ( Index < 0 )
	{
		m_AccessSemaphore.post();
		return ASAAC_ERROR;
	}
	
	CurrentValue   = m_Events[ Index ].EventObject->isEventSet() ? ASAAC_EVENT_STATUS_SET : ASAAC_EVENT_STATUS_RESET;
	WaitingCallers = m_Events[ Index ].EventObject->getWaitingThreads();
	
	m_AccessSemaphore.post();
	
	return ASAAC_SUCCESS;
} 		
	
									 
ASAAC_ReturnStatus EventManager::getEventId( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& EventId )
{
	m_AccessSemaphore.wait();

	long Index = findEventByName( Name );
	
	if ( Index < 0 )
	{
		m_AccessSemaphore.post();
		return ASAAC_ERROR;
	}
	
	EventId = m_Events[ Index ].EventId;
	
	m_AccessSemaphore.post();
	
	return ASAAC_SUCCESS;
}
	

