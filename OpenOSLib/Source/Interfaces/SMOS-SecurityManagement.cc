#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_getPMData(ASAAC_PublicId* vc_id, ASAAC_Address* message_buffer_reference, const unsigned long max_msg_length, unsigned long* msg_length, const ASAAC_TimeInterval* timeout)
{
    try 
    {
    	CommunicationManager::getInstance()->getPMData( *vc_id, *message_buffer_reference, max_msg_length, *msg_length, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::getPMData", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}

ASAAC_ReturnStatus ASAAC_SMOS_returnPMData(const ASAAC_PublicId vc_id, const ASAAC_Address message_buffer_reference, const unsigned long msg_length, const ASAAC_ReturnStatus sm_return_status)
{
    try 
    {
    	CommunicationManager::getInstance()->returnPMData( vc_id, message_buffer_reference, msg_length, sm_return_status );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::returnPMData", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_TimedReturnStatus ASAAC_SMOS_getAuditData(ASAAC_BreachType* breach_type, ASAAC_CharacterSequence* audit_message, ASAAC_Time* rel_time, ASAAC_Time* abs_time, const ASAAC_TimeInterval* time_out)
{
    try 
    {
    	throw OSException("SMOS call not yet implemented.", LOCATION);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::getAuditData", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}

ASAAC_ReturnStatus ASAAC_SMOS_erasePhysicalMemory()
{
    try 
    {
    	throw OSException("SMOS call not yet implemented.", LOCATION);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::erasePhysicalMemory", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

