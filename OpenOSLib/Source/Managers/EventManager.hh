#ifndef EVENTMANAGER_HH_
#define EVENTMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "IPC/Event.hh"
#include "IPC/Semaphore.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/AllocatedArea.hh"


//! container class to encapsulate all event-based APOS functionality

/*!< the EventManager is a singleton class that stores all required data to provide the event-based
 *   services as described by STANAG 4626, Part II, Section 11.4.4.
 */

class EventManager
{
public:
	//! get instance of the EventManager
	static EventManager* getInstance();
	/*!< the EventManager shall not be able to be instantiated dynamically,
	 *   if access to its functions is required, a pointer to the one single
	 *   instance is obtained via EventManager::getInstance()
	 * 
	 * \returns Reference to the EventManager instance
	 */
	
	
	//! explicit longialization
	void initialize();
	
	//! explicit deinitialization
	void deinitialize();
	

	ASAAC_ResourceReturnStatus createEvent( const ASAAC_CharacterSequence& Name,
										  ASAAC_PrivateId& EventId );
										  
	ASAAC_ReturnStatus deleteEvent( ASAAC_PrivateId EventId );
	
   	ASAAC_ReturnStatus setEvent( ASAAC_PrivateId EventId );

   	ASAAC_ReturnStatus resetEvent( ASAAC_PrivateId EventId );


	ASAAC_TimedReturnStatus waitForEvent( ASAAC_PrivateId EventId,
									    const ASAAC_Time& Timeout );

	
	ASAAC_ReturnStatus getEventStatus( ASAAC_PrivateId EventId,
								 ASAAC_EventStatus& EventStatus,
								 unsigned long& WaitingCallers );
									 
	ASAAC_ReturnStatus getEventId( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& EventId );

	virtual ~EventManager();
	
protected:
	EventManager();
	
	long findEventByName( const ASAAC_CharacterSequence& EventName );
	long findEventByPrivateId( const ASAAC_PrivateId EventId );

	LocalMemory		m_GlobalAllocator;

	typedef struct {
		ASAAC_PrivateId			EventId;
		ASAAC_CharacterSequence	EventName;
		Event*				EventObject;
		AllocatedArea		EventAllocator;
		
	} EventData;


	bool           m_IsInitialized;
	Semaphore  	   m_AccessSemaphore;
	
	unsigned long   m_NumberOfEvents;
	EventData 	   m_Events[ OS_MAX_NUMBER_OF_EVENTS ];
	
};


#endif /*EVENTMANAGER_HH_*/
