#ifndef LOGGINGMANAGER_HH_
#define LOGGINGMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "IPC/MessageQueue.hh"

//! singleton class encapsulating all SMOS Logging Management functions
/*! The LoggingManager comprises all methods required to provide the SMOS
 *  Logging Management functionality as specified by STANAG 4626, Part II, Section 11.7.10
 * 
 * It gathers log messages from participating clients via a POSIX message queue
 */

class LoggingManager
{
public:
	static LoggingManager* getInstance();

	void initialize(bool IsMaster = true);
	void deinitialize();

	//! request the OS to read a message in the log device (STANAG 4626, Part II, Section 11.7.10.1)
	ASAAC_ResourceReturnStatus	readLog( ASAAC_LogMessageType message_type,
								unsigned long log_id,
								ASAAC_CharacterSequence& log_message );

	//! write a message to the OS to be written in the log device (STANAG 4626, Part II, Section 11.7.10.2)
	ASAAC_ResourceReturnStatus	writeLog( ASAAC_LogMessageType message_type,
								 unsigned long log_id,
								 const ASAAC_CharacterSequence& log_message );
								 
	//! retrieves an application log message from an application thread (STANAG 4626, Part II, Section 11.7.10.3)
	ASAAC_TimedReturnStatus		getLogReport( ASAAC_CharacterSequence& log_message,
									 ASAAC_LogMessageType& message_type,
									 ASAAC_PublicId& ProcessId,
									 const ASAAC_TimeInterval& Timeout );

	virtual ~LoggingManager();


private:
	LoggingManager();
	
	bool 			m_IsInitialized;
	bool			m_IsMaster;
	
	ASAAC_PrivateId	m_LogFileDescriptorArray[ 4 ];
	
	MessageQueue	m_LoggingQueue;

};

#endif /*LOGGINGMANAGER_HH_*/
