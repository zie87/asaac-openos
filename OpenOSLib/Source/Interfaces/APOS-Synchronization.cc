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
	try
	{
		SemaphoreManager::getInstance()->createSemaphore( *name, *semaphore_id, init_value, max_value, queuing_discipline );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createSemaphore", LOCATION);
        e.raiseError();
        
        return e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;
    }
    
	return ASAAC_RS_SUCCESS;
}
									  
ASAAC_ReturnStatus ASAAC_APOS_deleteSemaphore(const ASAAC_PrivateId semaphore_id)							  
{
	try
	{
		SemaphoreManager::getInstance()->deleteSemaphore( semaphore_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::deleteSemaphore", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_TimedReturnStatus ASAAC_APOS_waitForSemaphore(const ASAAC_PrivateId semaphore_id, const ASAAC_TimeInterval* timeout)
{
	try
	{
		SemaphoreManager::getInstance()->waitForSemaphore( semaphore_id,
															  TimeStamp(*timeout).asaac_Time() );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::waitForSemaphore", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}
                         
ASAAC_ReturnStatus ASAAC_APOS_postSemaphore(const ASAAC_PrivateId semaphore_id)                                 
{
	try
	{
		SemaphoreManager::getInstance()->postSemaphore( semaphore_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::postSemaphore", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus ASAAC_APOS_getSemaphoreStatus(const ASAAC_PrivateId semaphore_id, 
                                                unsigned long* current_value, 
                                                unsigned long* waiting_callers)
{
	try
	{
		SemaphoreManager::getInstance()->getSemaphoreStatus( semaphore_id, *current_value, *waiting_callers );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getSemaphoreStatus", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}
                                 
ASAAC_ReturnStatus ASAAC_APOS_getSemaphoreId(const ASAAC_CharacterSequence* name, 
                                            ASAAC_PrivateId* semaphore_id)
{
	try
	{
		SemaphoreManager::getInstance()->getSemaphoreId( *name, *semaphore_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getSemaphoreId", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
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


