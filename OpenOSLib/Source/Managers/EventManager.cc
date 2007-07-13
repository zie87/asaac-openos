#include "Managers/EventManager.hh"
#include "Exceptions/Exceptions.hh"

#include "Managers/PrivateIdManager.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/AllocatedArea.hh"

#include "IPC/ProtectedScope.hh"

EventManager::EventManager() : m_IsInitialized(false)
{
}


EventManager::~EventManager()
{
}


EventManager* EventManager::getInstance()
{
	static EventManager ThisManager;
	
	return &ThisManager;
}


void EventManager::initialize()
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException();
	
	m_NumberOfEvents = OS_MAX_NUMBER_OF_EVENTS;
	
	m_GlobalAllocator.initialize( m_NumberOfEvents * Event::predictSize() + Semaphore::predictSize() );
	
	m_AccessSemaphore.initialize( &m_GlobalAllocator, true, 1 );
																	  
	for ( unsigned long Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		m_Events[ Index ].EventAllocator.initialize( &m_GlobalAllocator, Event::predictSize() );
																		
		m_Events[ Index ].EventId = OS_UNUSED_ID;
	}

	m_IsInitialized = true;
}


void EventManager::deinitialize()
{
	for ( unsigned long Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		if ( m_Events[ Index ].EventObject != 0 ) 
			delete m_Events[ Index ].EventObject;
		
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
		if ( EventName.size != m_Events[ Index ].EventName.size ) 
			continue;
		
		if ( strncmp( EventName.data, m_Events[ Index ].EventName.data, EventName.size ) ) 
			continue;
		
		break;
	}
	
	if ( Index == m_NumberOfEvents ) 
		return -1;
	
	return Index;
}


long EventManager::findEventByPrivateId( const ASAAC_PrivateId EventId )
{
	unsigned long Index;
	
	for ( Index = 0; Index < m_NumberOfEvents; Index ++ )
	{
		if ( m_Events[ Index ].EventId == EventId ) 
			break;
	}
	
	if ( Index == m_NumberOfEvents ) 
		return -1;
	
	return Index;
}


void EventManager::createEvent( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& EventId )
{
	try
	{
		ProtectedScope Access( "Creating an event", m_AccessSemaphore );
	
		long Index = findEventByName( Name );
		
		if ( Index >= 0 )
		{
			EventId = m_Events[ Index ].EventId;
			throw ResourceException("Event already created", LOCATION);
		}
		
		Index = findEventByPrivateId( OS_UNUSED_ID );
		
		if ( Index < 0 ) // no more free Event Slots?
			throw OSException("No more free Event Slots", LOCATION);
		
		m_Events[ Index ].EventId = PrivateIdManager::getInstance()->getNextId();
		m_Events[ Index ].EventName = Name;
		m_Events[ Index ].EventObject = new Event( & (m_Events[Index ].EventAllocator) );
		
		EventId = m_Events[ Index ].EventId;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error creating an event", LOCATION);
		
		throw;
	}
}


void EventManager::deleteEvent( ASAAC_PrivateId EventId )
{
	try
	{
		ProtectedScope Access( "Deleting an event", m_AccessSemaphore );
	
		long Index = findEventByPrivateId( EventId );
		
		if ( Index < 0 )
			throw OSException("Event with decicated EventId not found", LOCATION);
		
		if ( m_Events[ Index ].EventObject->getWaitingThreads() > 0 ) 
			throw OSException("Some threads are still waiting for this event", LOCATION);
		
		delete m_Events[ Index ].EventObject;
		
		m_Events[ Index ].EventObject = NULL;
		m_Events[ Index ].EventId = OS_UNUSED_ID;
		m_Events[ Index ].EventName = CharSeq("").asaac_str();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deleting an event", LOCATION);
		
		throw;
	}
}

	

void EventManager::waitForEvent( ASAAC_PrivateId EventId, const ASAAC_Time& Timeout )
{
	try
	{
		long Index = findEventByPrivateId( EventId );

		if ( Index < 0 )
			throw OSException("Event with decicated EventId not found", LOCATION);
	
		m_Events[ Index ].EventObject->waitForEvent( Timeout );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error waiting for an event", LOCATION);
		
		throw;
	}
}



void EventManager::setEvent( ASAAC_PrivateId EventId )
{
	try
	{
		long Index = findEventByPrivateId( EventId );
	
		if ( Index < 0 )
			throw OSException("Event with decicated EventId not found", LOCATION);
		
		m_Events[ Index ].EventObject->setEvent();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deleting an event", LOCATION);
		
		throw;
	}
}
	
	

void EventManager::resetEvent( ASAAC_PrivateId EventId )
{
	try
	{
		long Index = findEventByPrivateId( EventId );

		if ( Index < 0 )
			throw OSException("Event with decicated EventId not found", LOCATION);
	
		m_Events[ Index ].EventObject->resetEvent();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deleting an event", LOCATION);
		
		throw;
	}
}
	

	

void EventManager::getEventStatus( ASAAC_PrivateId EventId, ASAAC_EventStatus& CurrentValue, unsigned long& WaitingCallers )
{
	try
	{
		long Index = findEventByPrivateId( EventId );
		
		if ( Index < 0 )
			throw OSException("Event with decicated EventId not found", LOCATION);
		
		CurrentValue   = m_Events[ Index ].EventObject->isEventSet() ? ASAAC_EVENT_STATUS_SET : ASAAC_EVENT_STATUS_RESET;
		WaitingCallers = m_Events[ Index ].EventObject->getWaitingThreads();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving an event status", LOCATION);
		
		throw;
	}
} 		
	
									 
void EventManager::getEventId( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& EventId )
{
	try
	{
		long Index = findEventByName( Name );
		
		if ( Index < 0 )
			throw OSException("Event with decicated name not found", LOCATION);
		
		EventId = m_Events[ Index ].EventId;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error identifying event by name", LOCATION);
		
		throw;
	}
}
	

