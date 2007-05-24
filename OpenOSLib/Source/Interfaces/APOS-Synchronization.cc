#include "OpenOS.hh"

#include "Managers/SemaphoreManager.hh"
#include "Managers/EventManager.hh"


/* *************************************************************************************************** */
/*                                 S Y N C H R O N I Z A T I O N                                       */
/* *************************************************************************************************** */

/* SEMAPHORE FUNCTIONS */
ASAAC_ResourceReturnStatus ASAAC_APOS_createSemaphore( const ASAAC_CharacterSequence* name, 
                            ASAAC_PrivateId* semaphore_id, 
                            const unsigned long init_value, 
                            const unsigned long max_value, 
                            const ASAAC_QueuingDiscipline queuing_discipline)
{
	return SemaphoreManager::getInstance()->createSemaphore( *name, *semaphore_id, init_value, max_value, queuing_discipline );
}
									  
ASAAC_ReturnStatus ASAAC_APOS_deleteSemaphore(const ASAAC_PrivateId semaphore_id)							  
{
	return SemaphoreManager::getInstance()->deleteSemaphore( semaphore_id );
}

ASAAC_TimedReturnStatus ASAAC_APOS_waitForSemaphore(const ASAAC_PrivateId semaphore_id, const ASAAC_TimeInterval* timeout)
{
	return SemaphoreManager::getInstance()->waitForSemaphore( semaphore_id,
															  TimeStamp(*timeout).asaac_Time() );
}
                         
ASAAC_ReturnStatus ASAAC_APOS_postSemaphore(const ASAAC_PrivateId semaphore_id)                                 
{
	return SemaphoreManager::getInstance()->postSemaphore( semaphore_id );
}

ASAAC_ReturnStatus ASAAC_APOS_getSemaphoreStatus(const ASAAC_PrivateId semaphore_id, 
                                                unsigned long* current_value, 
                                                unsigned long* waiting_callers)
{
	return SemaphoreManager::getInstance()->getSemaphoreStatus( semaphore_id, *current_value, *waiting_callers );
}
                                 
ASAAC_ReturnStatus ASAAC_APOS_getSemaphoreId(const ASAAC_CharacterSequence* name, 
                                            ASAAC_PrivateId* semaphore_id)
{
	return SemaphoreManager::getInstance()->getSemaphoreId( *name, *semaphore_id );
}
                             

/* EVENT FUNCTIONS */

ASAAC_ResourceReturnStatus ASAAC_APOS_createEvent(const ASAAC_CharacterSequence* name, 
                                                  ASAAC_PrivateId* event_id)
{
	return EventManager::getInstance()->createEvent( *name, *event_id );
}

						         
ASAAC_ReturnStatus ASAAC_APOS_deleteEvent( const ASAAC_PrivateId event_id )
{
	return EventManager::getInstance()->deleteEvent( event_id );
}

ASAAC_ReturnStatus ASAAC_APOS_setEvent( const ASAAC_PrivateId event_id )
{
	return EventManager::getInstance()->setEvent( event_id );
}


ASAAC_ReturnStatus ASAAC_APOS_resetEvent( const ASAAC_PrivateId event_id )
{
	return EventManager::getInstance()->resetEvent( event_id );
}


ASAAC_TimedReturnStatus ASAAC_APOS_waitForEvent( const ASAAC_PrivateId event_id,
                                const ASAAC_TimeInterval* timeout )
{
	return EventManager::getInstance()->waitForEvent( event_id, TimeStamp(*timeout).asaac_Time() );
}

ASAAC_ReturnStatus ASAAC_APOS_getEventStatus(const ASAAC_PrivateId event_id, 
                                             ASAAC_EventStatus* event_status, 
                                             unsigned long* waiting_callers)                          
{
	return EventManager::getInstance()->getEventStatus( event_id, *event_status, *waiting_callers );
}
                             
ASAAC_ReturnStatus ASAAC_APOS_getEventId(const ASAAC_CharacterSequence* name, 
                                         ASAAC_PrivateId* event_id)
{
	return EventManager::getInstance()->getEventId( *name, *event_id );
}


