#include "OpenOS.hh"

#include "FaultManagement/ErrorHandler.hh"


ASAAC_ReturnStatus ASAAC_APOS_getDebugErrorInformation(ASAAC_ErrorType* error_type, ASAAC_ErrorCode* error_code, ASAAC_CharacterSequence* error_message)
{
	return ErrorHandler::getInstance()->getDebugErrorInformation(*error_type, *error_code, *error_message);
}
