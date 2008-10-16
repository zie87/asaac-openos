#ifndef ERRORHANDLER_HH_
#define ERRORHANDLER_HH_

#include "OpenOSIncludes.hh"

#include "IPC/SimpleCommandInterface.hh"
#include "IPC/MessageQueue.hh"

#include "Allocator/LocalMemory.hh"
#include "IPC/Trigger.hh"

#include "ProcessManagement/ProcessManager.hh"

//! singleton class encapsulating the error handling APOS functions
/*! the ErrorHandler contains all methods required to provide the APOS Error Handling
 *  longerfaces as specified in STANAG 4626, Part II, Section 11.4.5.
 * 
 *  It sends to the SMOS instance handling the error requests via a
 *  POSIX message queue, and receives error information and instructions for error
 *  handling via the SimpleCommandInterface provided by the Process.
 */
const ASAAC_NetworkDescriptor UndefNetworkDescriptor = {0,0};

class ErrorHandler
{
public:
	//! get single instance of ErrorHandler
	static ErrorHandler*		getInstance();
	/*!< \return Reference to initialized instance of ErrorHandler
	 */
	
	//! explicitely initialize or re-initialize ErrorHandler
	void				initialize();
	/*! Note: the instance obtained with the first getInstance() call will already be initialized.
	 *        Re-initialization will only be required if the instance has been deinitialized first.
	 */
	
	//! explicitely deinitialize the ErrorHandler
	void				deinitialize();
	
	//! raise an application error as specified in STANAG 4626, Part II, Section 11.4.5.2
	ASAAC_ReturnStatus			raiseError(  
											ASAAC_ErrorCode error_code, 
											const ASAAC_CharacterSequence& error_message, 
											ASAAC_ErrorType error_type = ASAAC_APPLICATION_ERROR, 
											ASAAC_Address location = 0,
											ASAAC_PublicId vc_id = 0, 
											ASAAC_PublicId tc_id = 0,
											ASAAC_NetworkDescriptor network = UndefNetworkDescriptor);
	/*!< the function places the error indicated by the parameters, along with information
	 *   about originating thread and a process verification code that is known only to the
	 *   GSM and the application, longo the error queue that conveys messages to the GSM.
	 * 
	 *   \param[in] error_code Error code to be provided to GSM
	 *   \param[in] error_message Descriptive text of the error, used for displaying/logging.
	 * 
	 *   \returns ASAAC_SUCCESS if the error could successfully be placed longo the error queue,
	 *            ASAAC_ERROR otherwise.
	 */

	
	//! get error information as specified in STANAG 4626, Part II, Section 11.4.5.3
	ASAAC_ReturnStatus			getErrorInformation( ASAAC_PublicId& faulty_thread_id,
											ASAAC_ErrorType& error_type,
											ASAAC_ErrorCode& error_code,
											ASAAC_CharacterSequence& error_message );
	/*!< when the error handler is started, data about the error it shall handle is
	 *   stored in the client thread. This function accesses this stored data,
	 *   giving the calling thread information about the error to handle.
	 * 
	 *   This command is only relevant within the error handler. Calling
	 *   from a thread with a nonzero ThreadId will instantly return with ASAAC_ERROR.
	 * 
	 *   \param[out] faulty_thread_id		ThreadId of the thread that raised the
	 * 										application error that led to the activation
	 * 										of the error handler.
	 * 
	 * 	 \param[out] error_type				Error type as provided with raiseApplicationError
	 *   \param[out] error_code				Error code as provided with raiseApplicationError
	 *   \param[out] error_message			Error message as provided
	 * 
	 *   \returns ASAAC_SUCCESS if the information could successfully be retrieved. ASAAC_ERROR otherwise.
	 */
	 
	//! get debug error information as specified in STANAG 4626, Part II, Section 11.4.6.1
	ASAAC_ReturnStatus			getDebugErrorInformation( 
											ASAAC_ErrorType& error_type,
											ASAAC_ErrorCode& error_code,
											ASAAC_CharacterSequence& error_message );
										
	//! terminate the error handler as specified in STANAG 4626, Part II, Section 11.4.5.4
	void				terminateErrorHandler( ASAAC_ReturnStatus return_status );
	/*!< this function shall be used to terminate the error handler. It returns an error code
	 *   to the GSM and terminates the calling thread.
	 * 
	 *   \param[in] return_status ASAAC_ReturnStatus to send to the GSM
	 */	
	
	
	//! write a log message to the module log as specified in STANAG 4626, Part II, Section 11.4.5.1
	ASAAC_ReturnStatus			logMessage( const ASAAC_CharacterSequence& log_message, ASAAC_LogMessageType message_type );
	/*!< write the indicated message to the log message queue, to be picked up by the GSM and
	 *   processed on from there.
	 * 
	 *   \param[in] log_message		Message to be placed to log
	 *   \param[in] message_type	Type of message to be logged
	 * 
	 *   \returns ASAAC_SUCCESS if the message could be properly placed longo the log message queue.
	 * 			  ASAAC_ERROR otherwise.
	 */
											
	virtual ~ErrorHandler();

private:
	ErrorHandler();
	friend class ProcessManager;
	
	typedef struct 
	{
		ASAAC_ErrorCode error_code;
		ASAAC_ErrorType error_type;
		ASAAC_CharacterSequence error_message;
		bool hasInformation;
	} DebugErrorInfo;
	
	//! static function to be called by Process's SimpleCommandInterface to activate the process' error handler
	static void			ActivateErrorHandler( CommandBuffer Buffer );

	bool				m_IsInitialized;
	
	ASAAC_ErrorInfo		m_ErrorInformation;
	bool 				m_HaveNewErrorInformation;
	bool 				m_HaveErrorInformation;
	
	MessageQueue		m_ErrorMessageQueue;
	MessageQueue 		m_LoggingMessageQueue;
	
	LocalMemory			m_LocalAllocator;
	
	Trigger				m_ErrorHandlerTrigger;
	ASAAC_ReturnStatus 	m_ErrorHandlerReturnStatus;

};

#endif /*ERRORHANDLER_HH_*/
