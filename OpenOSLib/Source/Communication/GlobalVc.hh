#ifndef GLOBALVC_HH_
#define GLOBALVC_HH_

#include "OpenOSIncludes.hh"

#include "Allocator/SharedMemory.hh"

#include "Common/Templates/Shared.hh"
#include "Common/Templates/SharedCyclicQueue.hh"

#include "IPC/SpinLock.hh"


class GlobalVc;

#include "Communication/LocalVc.hh"
#include "Communication/GlobalVcQueueCallback.hh"

//! Data structure storing information about a global vc buffer
struct BufferInfo {
	
		unsigned long	BufferLength;
		unsigned long	ContentLength;
				 long	UsageCount;
		
};

const long MAX_USAGE_COUNT = 32767;
const long UNDEF_USAGE_COUNT = -1;

//! object handling a number of local vc ports and the exchange of data between them

/*! Every GlobalVc comprises a number of local vc slots that will be assigned to
 *  applications for communication via the APOS-interfaces sendBuffer(), etc.
 *  The GlobalVc holds the required buffer resources for this communication, and
 *  distributes them to the instances of LocalVc. When initialized, the GlobalVc
 *  allocates the memory required for a maximum number of local vcs and forces the
 *  LocalVc instances to prepare and reserve an appropriate amount of memory in the
 *  total memory area of the GlobalVc. This way, the LocalVc's can initialize the
 *  data structures they require at a later point, without disrupting the overall
 *  memory structure of the other LocalVc's or the GlobalVc.
 */

typedef struct {
	unsigned long	NumberOfConnectedVCs;
	unsigned long	NumberOfConnectedTCs;
	unsigned long	NumberOfBuffers;
	
	long			SenderVcIndex;
} GlobalVcStatus;

 
class GlobalVc
{
	
public:
	//! non-initializing constructor
	GlobalVc();
	
	//! initialization method
	void initialize(ASAAC_PublicId GlobalVcId, bool IsMaster, const ASAAC_VcDescription& Data, SessionId p_SessionId );
	/* this method determines the shared memory object provided by the underlying
	 * operating system for the indicated GlobalVc, and connects to it. The
	 * master instance shall initialize all synchronization objects required for
	 * a working communication. It also initializes all Buffer Information blocks,
	 * and the free-buffers-queue.
	 * 
	 * 
	 * \param[in] GlobalVcId ASAAC_PublicId of the global vc resource to be handled
	 *						 by this object instance.
	 * \param[in] IsMaster	 Flag determining whether the calling instance
	 *                       shall be the master of the longernal data structures of
	 * 						 the global vc, i.e. whether it shall be responsible for
	 *                       determining shared memory sizes, and initializing the
	 *                       synchronization resources commonly used by all instances of
	 *                       GlobalVc participating in the communication over this virtual channel.
	 * \param[in] Data		 ASAAC_VcDescription structure as handed over to the createVirtualChannel()
	 *                       APOS call. It is only of significance for the master instance,
	 *                       for all other instances, the data provided here will be disregarded.
	 * 
	 * On error, initialize can throw an ASAAC_Exception or a child class of the latter.
	 */
	                       

	virtual ~GlobalVc();
	
	//! explicit deinitialization method
	void deinitialize();
	
	bool isInitialized();
	
	//! get the ASAAC_PublicId of the global virtual channel associated with this object instance
	ASAAC_PublicId getId();
	/*!< \return ASAAC_PublicId of the global virtual channel associated with this object instance,
	 *           or zero, if the instance is not yet associated with a global vc.
	 */
	
	GlobalVcStatus getStatus();
	
	//! initialize a local vc slot
	void createLocalVc( const ASAAC_VcMappingDescription& Description );
	/*!< this method initializes the next free local vc slot for usage as indicated
	 *   by the ASAAC_VcMappingDescription provided as parameter. Note that the instances
	 *   of LocalVc contained in the GlobalVc instance are already initialized in a way that
	 *   only ONE instance needs to configure a free local vc slot in order for ALL OTHER
	 *   (remote) instances to be able to use it.
	 * 
	 * \param[in] Description ASAAC_VcMappingDescription providing the information necessary
	 *                        to establish the local vc port.
	 * 
	 * \return    ASAAC_SUCCESS, if the local vc could successfully be established.
	 * 			  ASAAC_ERROR if the requested mapping conflicts with an already existing mapping,
	 * 			  the target process is in running state,
	 *            or if an error occurred during the configuration of the mapping.
	 */
	 

	//! uninitialize a local vc slot	 
	void removeLocalVc( ASAAC_PublicId ProcessId, ASAAC_PublicId LocalVcId );
	/*!< this method uninitializes the indicated local vc slot, preventing any further
	 *   sending or receiving via it. The longernal data structures are reset longo
	 *   a corresponding state.
	 * 
	 * \param[in] ProcessId		ASAAC_PublicId of the process owning the local vc to be removed
	 * \param[in] LocalVcId		ASAAC_PublicId of the local vc slot to be removed on the indicated process
	 * 
	 * \return	  ASAAC_SUCCESS if the local vc could successfully be removed.
	 *            ASAAC_ERROR, if the indicated local vc slot could not be found, 
	 *            the respective process is in running state, or an error
	 *            occurred during the operation.
	 */
	
	//! remove all local vc slots from one process
	void removeLocalVcsFromProcess( ASAAC_PublicId ProcessId );
	/*< this method causes the the list of local vc slots on this global vc to be
	 *  traversed, and all local vc slots owned by the indicated processor to be
	 *  removed via removeLocalVc().
	 * 
	 * \param[in] ProcessId		ASAAC_PublicId of the process to be cleared of all local vc connections
	 *                          to this global vc.
	 * 
	 * \return    ASAAC_SUCCESS on successful operation. ASAAC_ERROR if the target process is in running state,
	 *            or if at least one removeLocalVc() operation returned with an error.
	 */

	//! remove all local vc slots from one process
	void removeAllLocalVcs( );
	/*< this method causes the the list of local vc slots on this global vc to be
	 *  traversed, and all local vc slots to be removed via removeLocalVc().
	 * 
	 * \return    ASAAC_SUCCESS on successful operation. ASAAC_ERROR if one of the target processes is in running state,
	 *            or if at least one removeLocalVc() operation returned with an error.
	 */


	//! get reference to specified LocalVc instance
	LocalVc* getLocalVc( ASAAC_PublicId ProcessId, ASAAC_PublicId LocalVcId );
	/*!< returns the reference to the LocalVc instance that handles the input or output via
	 *   the indicated local vc slot on the indicated process.
	 * 
	 * \param[in] ProcessId		ASAAC_PublicId of the process
	 * \param[in] LocalVcId		ASAAC_PublicId of the local vc slot on the indicated process
	 * 
	 * \return    reference to the LocalVc object instance handling the indicated local vc I/O.
	 *            0, if the indicated local vc has not been created in this global vc.
	 */
	

	//! get list index of a specified LocalVc instance
	long     getLocalVcIndex( ASAAC_PublicId ProcessId, ASAAC_PublicId LocalVcId );
	/*!< returns the list index of the LocalVc instance that handles the input or output via
	 *   the indicated local vc slot on the indicated process in the global vc's list of local vcs.
	 * 
	 * \param[in] ProcessId		ASAAC_PublicId of the process
	 * \param[in] LocalVcId		ASAAC_PublicId of the local vc slot on the indicated process
	 * 
	 * \return    Index of the LocalVc object instance handling the indicated local vc I/O.
	 *            -1, if the indicated local vc has not been created in this global vc.
	 */

	//! get list index of the sending LocalVc instance, if available
	long	getSendingLocalVcIndex();
	/*!< returns the list index of the sending LocalVc instance.
	 * 
	 * \return    Index of the LocalVc object instance.
	 *            -1, if the indicated local vc has not been created in this global vc.
	 */


	//! get the reference of a LocalVc instance in the global vc's list
	LocalVc* getLocalVcByIndex( long Index );
	/*!< \param[in] Index Index of the LocalVc instance in the GlobalVc's list of LocalVcs.
	 * 
	 * \return reference to LocalVc object, if Index denotes a valid index in the list.
	 *         0 in all other cases.
	 */
	

	
	// Buffer Allocation/Information Methods
	
	//! get the next index from the free buffer list
	unsigned long getFreeBuffer();
	/*!< this method fetches one element from the free buffers list and returns it to the caller.
	 *   If no free buffer is instantly available, it blocks execution until one buffer becomes 
	 *   available.
	 * 
	 * \return Buffer Index of next free buffer.
	 */
	
	
	//! perform an atomic increase of the Buffer's usage counter
	void claimBuffer( unsigned long Buffer );
	/*!< \param[in] Buffer Buffer index of the buffer to be claimed as in use.
	 */
	
	//! perform an atomic decrease of the Buffer's usage counter
	void releaseBuffer(unsigned long Buffer);
	/*!< If the Buffer's usage counter drops to zero, the buffer will be returned to the
	 *   free buffers queue, and will be available via getFreeBuffer().
	 * 
	 * \param[in] Buffer Buffer index of the buffer to be noted as no longer in use by calling instance.
	 */
	
	//! get reference to a buffer information block
	BufferInfo* getBufferInfo(unsigned long Buffer) const;
	/*!< get the reference to the buffer information block corresponding to the Buffer with the
	 *   indicated index. The buffer information block contains data such as content length and
	 *   usage count of the buffer.
	 * 
	 * \param[in] Buffer Buffer index of the buffer for which the buffer information block shall
	 *                   be returned.
	 * 
	 * \return    reference to buffer information block. 0 for invalid buffer indices.
	 */
	
	//! get data area for a buffer
	ASAAC_Address getBufferArea(unsigned long Buffer) const;
	/*!< get the data area associated with the indicated buffer index number.
	 * 
	 * \param[in] Buffer Buffer index of the buffer for which the data area shall be returned.
	 * 
	 * \return    location of data area in virtual memory. 0 for invalid buffer indices.
	 */

	//! get the buffer index for a given data area
	long getBufferNumber( ASAAC_Address BufferLocation ) const;
	/*!< this function determines the buffer index number for a data area starting at the indicated
	 *   address. For this, it makes use of the buffer number being stored in the unsigned long value
	 *   just before the actual data buffer.
	 * 
	 *   To avoid wrong determinations, the result is double-checked. Invalid pointers return a buffer index
	 *   of -1.
	 * 
	 * \param[in] BufferLocation memory location of the buffer start
	 * 
	 * \returns buffer index of the corresponding buffer, or -1 if no buffer of the current GlobalVc starts at the
	 *          indicated address.
	 */
	
	
	//! send a buffer to all receivers
	void sendBuffer( unsigned long SourceBuffer, const ASAAC_Time& Timeout ); 
	/*!< this is the function to do the actual communication. By means of this method, all local vc slots of
	 *   the current global vc are traversed, and all initialized local vc slots configured as receivers (which
	 *   should be all except for one) will receive notification of the buffer indicated by its index number
	 *   to contain data ready to be read. For each receiver, the usage counter in the buffer info block will
	 *   be increased by one, so the buffer will only be returned to the free-buffers queue once all
	 *   receiving local vcs have actually received and read the message.
	 * 
	 *   The function works all-or-nothing style. It delays transmission of the buffer until ALL receiving
	 *   instances are clear to receive, and eventually cancels transmission with a ASAAC_TM_TIMEOUT if the timeout
	 *   has elapsed. Only when all receiving slots are available, it sends data to them.
	 * 
	 *   \param[in] SourceBuffer buffer index of the buffer containing the data to be sent. actual data
	 *                           has been written longo the memory area indicated by getBufferArea(), and the
	 *                           content length has been set in the buffer info block, found by getBufferInfo()
	 * 
	 *   \param[in] Timeout      maximum time to wait until before aborting delivery with a ASAAC_TM_TIMEOUT
	 * 
	 *   \returns				 ASAAC_TM_SUCCESS if all receivers were successfully written to.
	 *                           ASAAC_TM_TIMEOUT if the timeout elapsed before all receivers were available.
	 *                           ASAAC_TM_ERROR if a different error was encountered during the delivery.
	 */


	//! return vc description
	ASAAC_VcDescription *getDescription();

    //! return local session id
    SessionId getSessionId();

	//! predict size of control and data structures required from an allocator by an instance of global vc
	static size_t predictSize( const ASAAC_VcDescription& Description );
	

protected:	

	// Internal variables of importance for initialization issues
	bool							 m_IsInitialized;
	bool							 m_IsMaster;
	
	SharedMemory					 m_Allocator;

    SessionId                        m_SessionId;

    ASAAC_VcDescription              m_DescriptionBackup;

	// Shared memory variables.
	// Order also reflects order in shared memory block.

	Shared<ASAAC_VcDescription>		 m_Description;
	Shared<GlobalVcStatus>			 m_Status;
	
	Semaphore						 m_ActionSemaphore;
	Semaphore						 m_AllocationSemaphore;

	Shared<BufferInfo>				 m_BufferInfo;
	Shared<char>					 m_BufferData;
	
	SharedCyclicQueue<unsigned long>	 m_FreeBuffersQueue;

	GlobalVcQueueCallback			 m_GlobalVcQueueCallback;
	
	LocalVc							 m_LocalVc[ OS_MAX_NUMBER_OF_LOCALVCS ];
	
private:
	// no implicit copying and assignment
	GlobalVc( const GlobalVc& Source );
	GlobalVc& operator= ( const GlobalVc& Source );

#ifdef DEBUG_BUFFER
	//This method is for debug purpose only
	void printMessageQueue();
#endif	
};


#endif /*GLOBALVC_HH_*/
