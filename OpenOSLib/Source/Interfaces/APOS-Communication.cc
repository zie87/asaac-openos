#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


/* *************************************************************************************************** */
/*                                 C O M M U N I C A T I O N                                           */
/* *************************************************************************************************** */

ASAAC_ResourceReturnStatus ASAAC_APOS_sendMessageNonblocking(const ASAAC_PublicId local_vc_id, 
                                                            const ASAAC_Address message_buffer_reference, 
                                                            const unsigned long actual_size)
{
    ASAAC_ResourceReturnStatus Result;
    
    try 
    {
    	CommunicationManager::getInstance()->sendMessageNonblocking( local_vc_id, message_buffer_reference, actual_size );
    	
    	Result = ASAAC_RS_SUCCESS;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::sendMessageNonblocking", LOCATION);
        e.raiseError();
        
        Result = e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;
    }
    
    return Result;
}


ASAAC_ResourceReturnStatus ASAAC_APOS_receiveMessageNonblocking(const ASAAC_PublicId local_vc_id, 
                                                                const unsigned long maximum_size, 
                                                                const ASAAC_Address message_buffer_reference, 
                                                                unsigned long* actual_size)

{
    try 
    {
    	CommunicationManager::getInstance()->receiveMessageNonblocking( local_vc_id, maximum_size, message_buffer_reference, *actual_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::receiveMessageNonblocking", LOCATION);
        e.raiseError();
        
        return e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;
    }
    
	return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_sendMessage(const ASAAC_PublicId local_vc_id, 
                                                const ASAAC_TimeInterval* timeout, 
                                                const ASAAC_Address message_buffer_reference, 
                                                const unsigned long actual_size)

{
    try 
    {
    	CommunicationManager::getInstance()->sendMessage( local_vc_id, *timeout, message_buffer_reference, actual_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::sendMessage", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_receiveMessage(const ASAAC_PublicId local_vc_id, 
                                                  const ASAAC_TimeInterval* timeout, 
                                                  const unsigned long maximum_size, 
                                                  const ASAAC_Address message_buffer_reference, 
                                                  unsigned long* actual_size)
{
    try 
    {
    	CommunicationManager::getInstance()->receiveMessage( local_vc_id, *timeout, maximum_size, message_buffer_reference, *actual_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::receiveMessage", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_lockBuffer(const ASAAC_PublicId local_vc_id, 
                                              const ASAAC_TimeInterval* timeout, 
                                              ASAAC_Address* message_buffer_reference, 
                                              const unsigned long maximum_size)
{
    try 
    {
    	CommunicationManager::getInstance()->lockBuffer( local_vc_id, *timeout, *message_buffer_reference, maximum_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::lockBuffer", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}



ASAAC_ReturnStatus ASAAC_APOS_sendBuffer( const ASAAC_PublicId local_vc_id,
										  const ASAAC_Address message_buffer_reference,
										  const unsigned long maximum_size )
{
    try 
    {
    	CommunicationManager::getInstance()->sendBuffer( local_vc_id, message_buffer_reference, maximum_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::sendBuffer", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}									


ASAAC_TimedReturnStatus ASAAC_APOS_receiveBuffer(const ASAAC_PublicId local_vc_id, 
                                                const ASAAC_TimeInterval* timeout, 
                                                ASAAC_Address* message_buffer_reference, 
                                                unsigned long* actual_size)
{								
    try 
    {
    	CommunicationManager::getInstance()->receiveBuffer( local_vc_id, *timeout, *message_buffer_reference, *actual_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::receiveBuffer", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}



ASAAC_ReturnStatus ASAAC_APOS_unlockBuffer ( const ASAAC_PublicId local_vc_id,
									   		 const ASAAC_Address message_buffer_reference )
{
    try 
    {
    	CommunicationManager::getInstance()->unlockBuffer( local_vc_id, message_buffer_reference );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::unlockBuffer", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_TimedReturnStatus ASAAC_APOS_waitOnMultiChannel(const ASAAC_PublicIdSet* vc_set_in, 
                                                      const unsigned long min_no_vc, 
                                                      ASAAC_PublicIdSet* vc_set_out, 
                                                      const ASAAC_TimeInterval* timeout)
{
    try 
    {
    	CommunicationManager::getInstance()->waitOnMultiChannel( *vc_set_in, min_no_vc, *vc_set_out, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::waitOnMultiChannel", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}

