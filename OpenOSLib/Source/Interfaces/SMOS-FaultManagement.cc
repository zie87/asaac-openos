#include "OpenOS.hh"

#include "FaultManagement/FaultManager.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_getError(
	ASAAC_ErrorInfo* error_info, 
	const ASAAC_TimeInterval* time_out)
{
	return FaultManager::getInstance()->getError( *error_info, *time_out);
}

ASAAC_TimedReturnStatus ASAAC_SMOS_activateErrorHandler(
	const ASAAC_PublicId process_id, 
	const ASAAC_PublicId faulty_thread_id, 
	const ASAAC_ErrorType error_type, 
	const ASAAC_ErrorCode error_code, 
	const ASAAC_CharacterSequence* error_message, 
	ASAAC_ReturnStatus* error_handler_status, 
	const ASAAC_TimeInterval* time_out)
{
	return FaultManager::getInstance()->activateErrorHandler(
		process_id, 
		faulty_thread_id, 
		error_type, 
		error_code, 
		*error_message, 
		*error_handler_status, 
		*time_out);
}

