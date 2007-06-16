#include "OpenOS.hh"

#include "Communication/CommunicationManager.hh"


/* *************************************************************************************************** */
/*                                 C O M M U N I C A T I O N                                           */
/* *************************************************************************************************** */

ASAAC_ResourceReturnStatus ASAAC_APOS_sendMessageNonblocking(const ASAAC_PublicId local_vc_id, 
                                                            const ASAAC_Address message_buffer_reference, 
                                                            const unsigned long actual_size)
{
	return CommunicationManager::getInstance()->sendMessageNonblocking( local_vc_id, message_buffer_reference, actual_size );
}


ASAAC_ResourceReturnStatus ASAAC_APOS_receiveMessageNonblocking(const ASAAC_PublicId local_vc_id, 
                                                                const unsigned long maximum_size, 
                                                                const ASAAC_Address message_buffer_reference, 
                                                                unsigned long* actual_size)

{
	return CommunicationManager::getInstance()->receiveMessageNonblocking( local_vc_id, maximum_size, message_buffer_reference, *actual_size );
}


ASAAC_TimedReturnStatus ASAAC_APOS_sendMessage(const ASAAC_PublicId local_vc_id, 
                                                const ASAAC_TimeInterval* timeout, 
                                                const ASAAC_Address message_buffer_reference, 
                                                const unsigned long actual_size)

{
	return CommunicationManager::getInstance()->sendMessage( local_vc_id, *timeout, message_buffer_reference, actual_size );
}


ASAAC_TimedReturnStatus ASAAC_APOS_receiveMessage(const ASAAC_PublicId local_vc_id, 
                                                  const ASAAC_TimeInterval* timeout, 
                                                  const unsigned long maximum_size, 
                                                  const ASAAC_Address message_buffer_reference, 
                                                  unsigned long* actual_size)
{
	return CommunicationManager::getInstance()->receiveMessage( local_vc_id, *timeout, maximum_size, message_buffer_reference, *actual_size );
}


ASAAC_TimedReturnStatus ASAAC_APOS_lockBuffer(const ASAAC_PublicId local_vc_id, 
                                              const ASAAC_TimeInterval* timeout, 
                                              ASAAC_Address* message_buffer_reference, 
                                              const unsigned long maximum_size)
{
	return CommunicationManager::getInstance()->lockBuffer( local_vc_id, *timeout, *message_buffer_reference, maximum_size );
}



ASAAC_ReturnStatus ASAAC_APOS_sendBuffer( const ASAAC_PublicId local_vc_id,
										  const ASAAC_Address message_buffer_reference,
										  const unsigned long maximum_size )
{
	return CommunicationManager::getInstance()->sendBuffer( local_vc_id, message_buffer_reference, maximum_size );
}									


ASAAC_TimedReturnStatus ASAAC_APOS_receiveBuffer(const ASAAC_PublicId local_vc_id, 
                                                const ASAAC_TimeInterval* timeout, 
                                                ASAAC_Address* message_buffer_reference, 
                                                unsigned long* actual_size)
{								
	return CommunicationManager::getInstance()->receiveBuffer( local_vc_id, *timeout, *message_buffer_reference, *actual_size );
}



ASAAC_ReturnStatus ASAAC_APOS_unlockBuffer ( const ASAAC_PublicId local_vc_id,
									   		 const ASAAC_Address message_buffer_reference )
{
	return CommunicationManager::getInstance()->unlockBuffer( local_vc_id, message_buffer_reference );
}

ASAAC_TimedReturnStatus ASAAC_APOS_waitOnMultiChannel(const ASAAC_PublicIdSet* vc_set_in, 
                                                      const unsigned long min_no_vc, 
                                                      ASAAC_PublicIdSet* vc_set_out, 
                                                      const ASAAC_TimeInterval* timeout)
{
	return CommunicationManager::getInstance()->waitOnMultiChannel( *vc_set_in, min_no_vc, *vc_set_out, *timeout );
}

