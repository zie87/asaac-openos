#ifndef LOCALVC_HH_
#define LOCALVC_HH_

#include "OpenOSIncludes.hh"

#include "Common/Templates/Shared.hh"
#include "Common/Templates/SharedCyclicQueue.hh"

#include "Allocator/AllocatedArea.hh"

#include "AbstractInterfaces/MessageInterface.hh"

class GlobalVc;


//! class for objects handling the I/O ports of applications via virtual channels

class LocalVc : public MessageInterface
{
public:
	//! non-initializing constructor
    LocalVc();
    
    //! initializing constructor (see initialize())
    LocalVc( GlobalVc* ParentGlobalVc,
			 Allocator* ThisAllocator,
			 bool IsMaster,
	 		 unsigned long MaximumSize,
	 		 Callback* OverwriteCallback );
    
    
    virtual ~LocalVc();


	//! initializaiton method
    void initialize( GlobalVc* ParentGlobalVc,
    				 Allocator* ThisAllocator,
    				 bool IsMaster,
			 		 unsigned long MaximumSize,
			 		 Callback* OverwriteCallback );
	/*!< \param[in] ParentGlobalVc Reference to the GlobalVc instance that contains this LocalVc instance
	 *   \param[in] ThisAllocator  Reference to the Allocator that shall be used for the longernal data structures
	 *                             of the LocalVc
	 *   \param[in] IsMaster       Flag indicating whether the calling instance shall be the master instance of this
	 *                             LocalVc, i.e. whether it shall initialize the synchronization objects, etc. Only
	 *                             callers via SMOS longerfaces should act as master.
	 *   \param[in] MaximumSize	   Maximum Size of the queue. This value determines the size of the memory area to
	 *                             be allocated for the LocalVc. The LocalVc cannot be configured to hold a mapping
	 *                             that requires more slots than configured here.
	 *   \param[in] OverwriteCallback Reference to Callback instance handling messages that are overwritten in
	 *                             localvc queues with the corresponding queueing discipline.
	 * 
	 *  Note: The initialization only sets up the longernal structures and allocates memory for the operation
	 *        of the local vc. Before any I/O can take place, the LocalVc instance must be assigned to a certain mapping,
	 *        tho.
	 */
			 		
	//! explicit deinitialization 
	void deinitialize();
    
    bool isInitialized();
    
	//! assign LocalVc instance to a distinct mapping
    void assign( const ASAAC_VcMappingDescription& Description );
    /*!< With the assign call, the LocalVc instance is set up to control a specified
     *   local vc port. Due to the way the data of the LocalVc is stored in memory,
     *   the assignment only needs to be carried out on one copy of this LocalVc,
     *   and will, by shared-memory mechanisms, be valid for all existing copies
     *   of the same LocalVc on this and other processes.
     * 
     * \param[in] Description ASAAC_VcMappingDescription containing the mapping information
     *                        provided by the corresponding
     *                        attachChannelToProcessOrThread() call.
     * 
     * \returns   ASAAC_SUCCESS on successful operation. ASAAC_ERROR if an error occurred during the assignment,
     *            or if the LocalVc was already assigned to another mapping.
     */
     
    //! unassign mapping of the LocalVc instance
    void unassign();
    /*!< this function is called by the GlobalVc to remove 
     *   the assignment of a local vc mapping from this instance of
     *   LocalVc and all its (remote) copies. The queue will be emptied and messages queued
     *   therein will be released. Until the LocalVc is re-assign()'ed, it cannot be used
     *   for communications any longer
     * 
     * 	\returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR if an error occurred during the unassignment,
     *                   or if the LocalVc instance was not yet assigned to a mapping
     */
    
    //! hand over request to remove this LocalVc to its parent GlobalVc. Used by Process::destroy()
    void remove();
    /*!< this function causes the parent GlobalVc of this LocalVc instance to unassign() this LocalVc instance
     *   and take all precautions to clean up the data structures associated with it.
     * 
     * \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR if an error occurred.
     */
    
    //! get the local vc id of this LocalVc instance
    ASAAC_PublicId getId();
    /*!< \returns ASAAC_PublicId of this LocalVc instance, to be used with the appropriate APOS and SMOS calls,
     *            0, if LocalVc instance is not assigned to a mapping yet.
     */
        
    
    //! Copy a message longo a buffer and send on a VC.
    virtual void sendMessageNonblocking( ASAAC_Address BufferReference, unsigned long Size );
    /* The OS shall copy the message data from the  buffer in the caller
     * memory to a buffer in the OS memory that is associated with a VC
     * for sending direction. The OS controls fuerther thransfer of the
     * buffer data through the associated VC.
     * 
     * The service returns immediately, whether there were free slots in
     * the receivers' queues or not.
     * 
     * \param[in] BufferReference Location of the Buffer's start in memory
     * \param[in] Size            ASAAC_Length of the message to be sent.
     * 
     * \return    ASAAC_RS_SUCCESS on successful completion.
     *            ASAAC_RS_RESOURCE if one or more receivers had no free slots in their
     *                        receiving queues.
     *            ASAAC_RS_ERROR on other errors.
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.1)
     */

    
    //! read the next available buffer associated with the LocalVC. 
    virtual void receiveMessageNonblocking( ASAAC_Address BufferReference, 
    												unsigned long MaxSize, 
    												unsigned long& ActualSize );
    /*!< If a message is available, the operating system copies the message
     * data from operating system memory longo the buffer in the caller
     * memory that is specified in terms of location and maximum
     * size by the parameters.
     * The OS further provides the actual size of the message buffer
     * as a return parameter.
     * If no message buffer is available, the service returns immediately.
     * 
     * \param[in] BufferReference Location of a buffer longo which the received message is
     *                            to be copied.
     * \param[in] MaxSize         ASAAC_Length of the receiving buffer.
     * \param[out] ActualSize     ASAAC_Length of the received message.
     * 
     * \return ASAAC_RS_SUCCESS on successful completion.
     *         ASAAC_RS_RESOURCE if no message was instantly available.
     *         ASAAC_RS_ERROR on other errors
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.2)
     */
     
     
    //! copy a message longo a buffer and send on a VC.
    virtual void sendMessage( ASAAC_Address BufferReference, 
    							   unsigned long Size, 
    							   const ASAAC_Time& Timeout );
    /*!< The function copies the message data from the  buffer in the caller
     * memory to a buffer in the OS memory that is associated with a VC
     * for sending direction. It controls further thransfer of the
     * buffer data through the associated VC.
     * 
     * If sufficient OS buffer space is not immediately available, the
     * calling thread is transferred longo a waiting state. The thread
     * will be transferred longo a ready state when the buffer has
     * become available for writing or if waiting time exceeds the
     * period specified.
     * 
     * \param[in] BufferReference Location of the Buffer's start in memory
     * \param[in] Size            ASAAC_Length of the message to be sent.
     * \param[in] Timeout         Maximum Interval to wait for slots in
     *                            receiver queues to become available before
     *                            returning with ASAAC_TM_TIMEOUT
     * 
     * \return    ASAAC_TM_SUCCESS on successful completion.
     *            ASAAC_TM_TIMEOUT if one or more receivers had no free slots in their
     *                        receiving queues after Timeout has elapsed
     *            ASAAC_TM_ERROR on other errors.
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.3)
     */


	//! receive a message on a virtual channel
    virtual void receiveMessage( ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout );
    /*!< Read the next available buffer associated with the LocalVC. If a
     * message is available, the function copies the message
     * data from operating system memory longo the buffer in the caller
     * memory that is specified in terms of location and maximum
     * size by the parameters.
     * It further provides the actual size of the message buffer
     * as a return parameter.
     * If no message is immediately available, the
     * calling thread is transferred longo a waiting state. The thread
     * will be transferred longo a ready state when the buffer has
     * become available for reading or if waiting time exceeds the
     * period specified.
     * 
     * \param[in] BufferReference Location of a buffer longo which the received message is
     *                            to be copied.
     * \param[in] MaxSize         ASAAC_Length of the receiving buffer.
     * \param[out] ActualSize     ASAAC_Length of the received message.
     * \param[in] Timeout         Maximum Interval to wait for data
     *                            to become available before
     *                            returning with ASAAC_TM_TIMEOUT
     * 
     * \return    ASAAC_TM_SUCCESS on successful completion.
     *            ASAAC_TM_TIMEOUT if no data was available after Timeout has elapsed
     *            ASAAC_TM_ERROR on other errors.
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.4)
     */


	//! lock a buffer to populate it with data and send at a later point
    void lockBuffer( ASAAC_Address& BufferReference, 
    							  unsigned long Size, 
    							  const ASAAC_Time& Timeout );
    /*!< Lock message buffer associated with VC in order to provide
     * message data longo this buffer for sending.  The OS shall
     * lock a buffer of sufficient size.
     * 
     * If a buffer is not immediately available, the calling thread is
     * transferred longo WAITING state. It will be passed on to
     * READY state either when a message buffer has become
     * available or when waiting time exceeds the timeout period.
     * 
     * \param[out] BufferReference Location of the obtained buffer in OS memory space.
     * \param[in]  Size            Requested size of the buffer
     * \param[in]  Timeout         Maximum longerval to wait for a buffer to become
     *                                 available.
     * 
     * \return	ASAAC_TM_SUCCESS on successful completion.
     * 			ASAAC_TM_TIMEOUT if no buffer was available within the time specified by the Timeout.
     * 			ASAAC_TM_ERROR if buffer size requested exceeds buffer size configured for the virtual channel,
     *          if the vc is configured for receiving direction, or if an other kind of error occurred.
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.5)
     */


	//! send a formerly locked buffer
    void sendBuffer( ASAAC_Address BufferReference, 
    					     	  unsigned long Size,
    					     	  const ASAAC_Time& Timeout = TimeInfinity );
    /*!< Send the passed message buffer through the VC.
     * The buffer shall have been locked before via lockBuffer().
	 *
     * After transmitting the buffer, it is unlocked.
     * 
     * \param[in] BufferReference Location of the Buffer's start in memory
     * \param[in] Size            ASAAC_Length of the message to be sent.
     * \param[in] Timeout         Maximum Interval to wait for slots in
     *                            receiver queues to become available before
     *                            returning with ASAAC_TM_TIMEOUT
     * 
     * \return    ASAAC_TM_SUCCESS on successful completion.
     *            ASAAC_TM_TIMEOUT if one or more receivers had no free slots in their
     *                        receiving queues after Timeout has elapsed
     *            ASAAC_TM_ERROR on other errors.
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.6)
     */
 




	//! receive a message on a virtual channel, leaving the data in OS memory
    void receiveBuffer( ASAAC_Address& BufferReference, 
    							     unsigned long& Size, 
    							     const ASAAC_Time& Timeout );
    /*!< Provide the next available message buffer that is associated
     * with the VC. After completing of message processing, it is up to the
     * caller to unlock the buffer.
     * 
     * If a buffer, filled with message data, is not immediately available,
     * the calling thread is transferred longo WAITING state. The calling
     * thread will be passed on to READY state when a message buffer
     * has become available or when timeout has elapsed.
     * 
     * \param[out] BufferReference	Location of the obtained message buffer in OS memory space.
     * \param[out] Size				ASAAC_Length of the received message.
     * \param[in]  Timeout          Maximum longerval to wait for a message to become
     *                                 available.
     * 
     * \return	ASAAC_TM_SUCCESS on successful completion.
     * 			ASAAC_TM_TIMEOUT if no buffer was available within the time specified by the Timeout.
     * 			ASAAC_TM_ERROR if the vc is configured for sending direction, or if an other kind of error occurred.
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.7)
     */


    //! unlock a message buffer associated with the VC.
    void unlockBuffer( ASAAC_Address BufferReference );
    /*!< unlock a buffer resource that has formerly been acquired via receiveBuffer().
     * 
     * \param[in] BufferReference Location of the Buffer's start in memory
     * 
     * \returns ASAAC_SUCCESS on successful completion.
     * 			ASAAC_ERROR if the provided buffer is not a valid buffer obtained via receiveBuffer().
     * 
     * (see STANAG 4626, Part II, Section 11.4.7.8)
     */
 


	//! wait for data in this buffer to become available
	void waitForAvailableData( const ASAAC_Time& Timeout );
	/*!<  transfer the calling thread longo WAITING state until either data
	 *    is available to be read for the LocalVc, or the Timeout has elapsed.
	 *    After that, the thread is returned longo RUNNING state.
	 * 
	 * \param[in] Timeout Maximum Timeinterval to wait before returning with a ASAAC_TM_TIMEOUT
	 * 
	 * \returns   ASAAC_TM_SUCCESS if data has become available.
	 * 			  ASAAC_TM_TIMEOUT if the timeout has elapsed
	 *            ASAAC_TM_ERROR if an error has occurred during the waiting.
	 */
	
	
	//! wait for free receiving slots in this local vc
	void waitForFreeCells( const ASAAC_Time& Timeout );
	/*!< transfers the calling thread longo WAITING state until either a
	 *   free slot is available for writing data longo this LocalVc, or the Timeout
	 *   has elapsed. After that, the thread is returned longo RUNNING state.
	 * 
	 * \param[in] Timeout Maximum Timeinterval to wait before returning with a ASAAC_TM_TIMEOUT
	 * 
	 * \returns   ASAAC_TM_SUCCESS if free slot has become available.
	 * 			  ASAAC_TM_TIMEOUT if the timeout has elapsed
	 *            ASAAC_TM_ERROR if an error has occurred during the waiting.
	 */


	//! push one buffer index number longo the receiver queue of this local vc
	void queueBuffer( unsigned long BufferNumber, const ASAAC_Time& Timeout = TimeInfinity );
	/*!< Interface used by GlobalVc to provide the LocalVc with data. The function waits for
	 *  a free slot to be available in the LocalVc's queue, or for the timeout to elapse, whatever
	 *  happens first.
	 *  Should the slot have become available within the indicated timeout, the buffer index number
	 *  is pushed longo the LocalVc's queue.
	 * 
	 * \param[in] BufferNumber buffer index number of the global vc's buffer to be assigned to
	 *                         the local vc.
	 * \param[in] Timeout      maximum time to wait for free slot to be available in the LocalVc's
	 *                         queue before returning with ASAAC_TM_TIMEOUT
	 * 
	 * \returns   ASAAC_TM_SUCCESS if free slot has become available.
	 * 			  ASAAC_TM_TIMEOUT if the timeout has elapsed
	 *            ASAAC_TM_ERROR if an error has occurred during the waiting.
	 */

	//! get reference to ASAAC_VcMappingDescription of this LocalVc
	ASAAC_VcMappingDescription* getDescription();
	/*!< \returns	reference to the mapping description of this LocalVc instance. 
	 * 				0, if is LocalVc instance has not been assigned to a mapping yet.
	 */

	//! predict memory requirements of this LocalVc's data and control structures inside its allocator
	static size_t predictSize( unsigned long MaximumBuffers );
	
	
public:
	bool								m_IsInitialized; 
	bool								m_IsMaster;

    Shared<ASAAC_VcMappingDescription>	m_Description;
    
	AllocatedArea						m_QueueAllocator;
    SharedCyclicQueue<unsigned long>  	m_Queue;
    Callback*							m_OverwriteCallback;

    GlobalVc*					     	m_ParentGlobalVc;

    
};


#endif /*LOCALVC_HH_*/
