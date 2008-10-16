#ifndef LOGREPORTDATA_HH_
#define LOGREPORTDATA_HH_

#include "OpenOSIncludes.hh"


//! Format of messages to be sent from application to GSM for the transmission of a log message

typedef struct LogReportData {
	
	ASAAC_CharacterSequence		log_message;
	ASAAC_LogMessageType		message_type;
	ASAAC_PublicId				process_id;
	ASAAC_PublicId				thread_id;
	ASAAC_Time					time;
	
	unsigned long		process_authentication_code;
	
};

#endif /*LOGREPORTDATA_HH_*/
