#include "OpenOS.hh"

#include "FaultManagement/ErrorHandler.hh"

/* ASAAC_ERROR HANDLING */

ASAAC_ReturnStatus	ASAAC_APOS_logMessage( 
	const ASAAC_CharacterSequence* log_message, 
	const ASAAC_LogMessageType message_type)
{
	return ErrorHandler::getInstance()->logMessage( *log_message, message_type );
}


ASAAC_ReturnStatus	ASAAC_APOS_raiseApplicationError( 
	const ASAAC_ErrorCode error_code, 
	const ASAAC_CharacterSequence* error_message)
{
	return ErrorHandler::getInstance()->raiseError( error_code, *error_message );
}

	

ASAAC_ReturnStatus ASAAC_APOS_getErrorInformation( 
	ASAAC_PublicId* faulty_thread_id, 
	ASAAC_ErrorType* error_type, 
	ASAAC_ErrorCode* error_code, 
	ASAAC_CharacterSequence* error_message)
{
	return ErrorHandler::getInstance()->getErrorInformation(*faulty_thread_id, *error_type, *error_code, *error_message); 
}

