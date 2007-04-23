#ifndef FAULTMANAGER_HH_
#define FAULTMANAGER_HH_

// Class for handling SMOS Fault Management functionality

#include "OpenOSIncludes.hh"

#include "IPC/MessageQueue.hh"
#include "IPC/SimpleCommandInterface.hh"

#include "ProcessManagement/Process.hh"

//! singleton class encapsulating the SMOS Fault Management functions
/*! the FaultManager contains all methods required to provide the SMOS Fault Management
 *  functions as specified in STANAG 4626, Part II, Section 11.7.2.
 * 
 * It gathers fault information of all participating applications via a POSIX message
 * queue and sends corresponding replies to them via the already existing SimpleCommandInterface
 * provided by the Process class.
 */

class FaultManager
{
public:
	//! get single, initialized instance of the FaultManager
	static FaultManager*		getInstance();
	
	void				initialize(bool IsMaster = true);
	void				deinitialize();
	
	//! wait for an error to occur and provide error information to the caller as specified in STANAG 4626, Part II, Section 11.7.2.1
	ASAAC_TimedReturnStatus		getError( ASAAC_ErrorInfo& error_info, const ASAAC_TimeInterval& time_out );
	/*!< this function blocks the caller until an OS report is provided via the message queue
	 *   or the timeout elapses.
	 * 
	 *   \param[out] error_info		Data field providing information about the error that occurred and its
	 *                              location.
	 *   \param[in]	 time_out		ASAAC_Time to wait before returning with ASAAC_TM_TIMEOUT
	 * 
	 *   \returns ASAAC_TM_SUCCESS if an error message was properly obtained within the specified
	 *            time. ASAAC_TM_TIMEOUT if the timeout elapsed. ASAAC_TM_ERROR if an error occurred
	 *            during the operation.
	 */


	//! actuvate the error handler in a process as specified in STANAG 4626, Part II, Section 11.7.2.2
	ASAAC_TimedReturnStatus		activateErrorHandler( ASAAC_PublicId process_id, 
											ASAAC_PublicId faulty_thread_id,
											ASAAC_ErrorType error_type,
											ASAAC_ErrorCode error_code,
											const ASAAC_CharacterSequence& error_message,
											ASAAC_ReturnStatus& error_handler_status,
											const ASAAC_TimeInterval& Timeout );
	/*!< this function sends a command to the client process via the SimpleCommandInterface
	 *   instantiated in the client-related instance of Process, that causes the client to
	 *   lock its thread preemption state and start its thread number zero, if present.
	 * 
	 *   The function waits for a return from the client process, up to a time specified
	 *   by the Timeout parameter.
	 *   
	 *   Important Note: The STANAG states that the Error handler has a ThreadId of one,
	 *                   which is already claimed for the main thread of the process.
	 *                   Therefore, the present implementation assumes a ThreadId of zero
	 *                   for the Error Handler.
	 * 
	 *   \param[in] process_id				ASAAC_PublicId of the process intended to start the error handler
	 *   \param[in] faulty_thread_id 		Information on the Id of the faulty thread that originally
	 *                              		sent the error message to the OS, if it could be determined.
	 * 
	 * 	 \param[in] error_type				Error type information as received in getError
	 *   \param[in] error_message			Error message as received in getError
	 *   \param[out] error_handler_status	Return status of the error handler.
	 *   \param[in] Timeout					ASAAC_Time to wait for reply from the error handler
	 * 
	 * 
	 *   \returns When the service is completed successfully, ASAAC_TM_SUCCESS is returned. When
	 *            the time specified by the Timeout has elapsed without reply from the client's
	 *            error handler thread, ASAAC_TM_TIMEOUT is returned. ASAAC_TM_ERROR is returned for invalid
	 *            parameters (process_id, faulty_thread_id, error_type out of range or not indicating
	 *            an existing object), or for errors occuring during the communication and
	 *            operation.
	 */

	virtual ~FaultManager();
	
private:
	FaultManager();
	
	bool			m_IsInitialized;
	bool			m_IsMaster;
	
	MessageQueue	m_ErrorMessageQueue;

};

#endif /*FAULTMANAGER_HH_*/
