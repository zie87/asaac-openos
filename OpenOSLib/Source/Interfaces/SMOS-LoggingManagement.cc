#include "OpenOS.hh"

#include "FaultManagement/LoggingManager.hh"


ASAAC_ResourceReturnStatus ASAAC_SMOS_readLog(const ASAAC_LogMessageType message_type, const unsigned long log_id, ASAAC_CharacterSequence* log_message)
{
	return LoggingManager::getInstance()->readLog( message_type, log_id, *log_message );
}


ASAAC_ResourceReturnStatus ASAAC_SMOS_writeLog(const ASAAC_LogMessageType message_type, const unsigned long log_id, const ASAAC_CharacterSequence* log_message)
{
	return LoggingManager::getInstance()->writeLog( message_type, log_id, *log_message );
}


ASAAC_TimedReturnStatus ASAAC_SMOS_getLogReport(ASAAC_CharacterSequence* log_message, ASAAC_LogMessageType* message_type, ASAAC_PublicId* process_id, const ASAAC_TimeInterval* timeout)
{
	return LoggingManager::getInstance()->getLogReport( *log_message, *message_type, *process_id, *timeout );
}
