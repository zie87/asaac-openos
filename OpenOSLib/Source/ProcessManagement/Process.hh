#ifndef PROCESS_HH_
#define PROCESS_HH_

#include "OpenOSIncludes.hh"

#include "Common/Templates/Shared.hh"

#include "Allocator/SharedMemory.hh"
#include "Allocator/LocalMemory.hh"

#include "IPC/Semaphore.hh"
#include "IPC/SimpleCommandInterface.hh"

#include "Communication/LocalVc.hh"

#include "ProcessManagement/EntryPoint.hh"
#include "ProcessManagement/Thread.hh"
#include "ProcessManagement/ProcessStatus.hh"


enum MemoryLocation {
		
	LOCAL,
	SHARED
		
};


enum SMOS_Actions {
	
	CMD_NULL_COMMAND,
	
	CMD_GET_PID,
	
	CMD_CREATE_PROCESS,
	CMD_RUN_PROCESS,
	CMD_STOP_PROCESS,
	CMD_DESTROY_PROCESS,
	
	CMD_ATTACH_VC,
	CMD_DETACH_VC,

	CMD_TERM_PROCESS,
	CMD_TERM_ENTITY,

	CMD_FLUSH_SESSION,
	
	CMD_ACTIVATE_ERROR_HANDLER,
	CMD_INVOKE_OS_SCOPE
	
};

typedef char OSScopeCommandBuffer[ OS_SIZE_OF_OSSCOPE_COMMANDBUFFER ];

typedef ASAAC_ReturnStatus(*OSScopeFunction)(OSScopeCommandBuffer);

typedef struct 
{
    long Policy;
    sched_param Parameter;
} SchedulingData;

//! object class responsible for handling all operations related to a process

/*! the process class comprises all process control operations, and holds
 *  information and structures required for thread manipulations.
 * 
 *  Information and communication between GSM and application is mostly
 *  done via direct access to shared memory without required longeraction
 *  on the other side, yet some functions require the application to take
 *  direct action. Those functions use a SimpleCommandInterface that is
 *  instantiated during the process-initialization.
 *  This SimpleCommandInterface can then be used to establish RPC's in a
 *  convenient and performant manner.
 * 
 *  For every process, an instance of the Process class is set up both on
 *  the GSM side as in the process itself.
 */

class Process
{
public:

	bool 					isInitialized();
	
	//! get Process' Global ASAAC_PublicId
	ASAAC_PublicId			getId();
	
	static ASAAC_PublicId	getId( ProcessAlias Alias );
	ProcessAlias			getAlias();
	
	//! get Process' POSIX PID (process id) and store it inside the object
	void					refreshPosixPid();
		
	//! set up a thread inside the process
	void		            createThread( const ASAAC_ThreadDescription& Description );
	/*!< \param[in] Description Thread description as supplied by the SMOS call
	 *                          createThread()
	 * 
	 *   \return ASAAC_SUCCESS if the thread could be properly set up.
	 *           ASAAC_ERROR otherwise.
	 */

	signed long				getThreadIndex( ASAAC_PublicId ThreadId );

	//! get reference to a thread identified by thread ASAAC_PublicId
	Thread*					getThread( ASAAC_PublicId ThreadId, const bool do_throw = true );
	/*!< \param[in] ThreadId ASAAC_PublicId of thread object to obtain reference to
	 *   \returns   Reference to indicated thread. 0, if no thread with the
	 *              indicated ASAAC_PublicId is found within this process.
	 */
	
	//! get reference to the currently executed thread
	Thread*					getCurrentThread( const bool do_throw = true );
	 /*!<   \returns   Reference to current thread. 0, if current thread is
	  *                not found in this process.
	  */


	//! add an entry point	
	void            		addEntryPoint( ASAAC_CharacterSequence Name, EntryPointAddr Address );
	/*!< this function adds an entry polong to the table of entry points stored
	 *   for this process. These entry points are required in order to
	 *   indicate jump-in addresses for threads to be started inside the process.
	 * 
	 *   \param[in] Name ASAAC_CharacterSequence identifying the entry polong descriptively
	 *   \param[in] ASAAC_Address Jump address to the function constituting the thread.
	 * 
	 *   \returns ASAAC_SUCCESS if the thread entry polong could successfully be added to
	 *            the table. ASAAC_ERROR in case of an error.
	 */
	     
	signed long				getEntryPointIndex( ASAAC_CharacterSequence Name );
	
	//! remove an entry point
	EntryPoint*				getEntryPoint( ASAAC_CharacterSequence Name );
	/*!< retrieve the entry address for an entry polong stored for this process.
	 * 
	 *   \param[in] Name Name of the entry point
	 *   \returns   Entry polong address. 0, if entry polong was not found.
	 */
	 

	//! attach the process to a Virtual Channel
	void		            attachLocalVc( ASAAC_PublicId GlobalVcId, ASAAC_PublicId LocalVcId );
	/*!< this function establishes the connection of a process with a virtual channel.
	 *   if required, the shared memory object constituting the virtual channel is opened,
	 *   and the reference to the proper LocalVc slot is stored in the process.
	 * 
	 *   \param[in] GlobalVcId		Global ASAAC_PublicId of the virtual channel to connect to
	 *   \param[in] LocalVcId		Process-Local ASAAC_PublicId to assign to the virtual channel
	 *   \param[in] LocalVcIndex	Index of local Vc slot in the Global Vc's list of LocalVc's
	 *                              (to avoid ambiguities)
	 * 
	 *   \returns ASAAC_SUCCESS if the LocalVc was properly attached. ASAAC_ERROR otherwise.
	 */
	

	//! detach the process from a Virtual Channel	
	void		            detachLocalVc( ASAAC_PublicId LocalVcId );
	/*!<
	 *   \param[in] LocalVcId	Process-Local ASAAC_PublicId of the virtual channel
	 * 
	 *   \returns ASAAC_SUCCESS if the LocalVc could be properly detached. ASAAC_ERROR otherwise.
	 */
	 
	 //! get reference to a LovalVc attached to this process
	LocalVc*				getAttachedVirtualChannel( ASAAC_PublicId LocalVcId );
	/*!< \param[in] LocalVcId		Process-local ASAAC_PublicId of the virtual channel
	 * 
	 *   \returns Reference to the LocalVc object handling input and/or output for
	 *            the indicated virtual channel. 0, if no LocalVc object with the
	 *            supplied LocalVcId could be found.
	 */
    long                    getAttachedVirtualChannelIndex( ASAAC_PublicId LocalVcId );
    
    bool                    isAttachedTo( ASAAC_PublicId LocalVcId );


	//! suspend all threads of the process, except for the calling thread
	void		            lockThreadPreemption( unsigned long& LockLevel );
	
	
	//! resume all threads of the process
	void		            unlockThreadPreemption( unsigned long& LockLevel );
	
	//! get lock level for the process resulting from lockThreadPreemption() and unlockThreadPreemption()
	unsigned long			getLockLevel();
	
	//! get the status of the process	
	ProcessStatus			getState();
	
	//! get the AuthenticationCode of the process
	unsigned long			getAuthenticationCode();
	/*!< the authentication code is a dynamically created (randomed) 32-bit
	 *   number that is only known to the GSM and the application, so the
	 *   application can prove the authenticity of a message's origin to
	 *   a multi-sender queue like the logging and error message queues.
	 * 
	 *   \returns Authentication code of the process
	 */
	
	ASAAC_ProcessDescription getProcessDescription();
	
	//! return reference to the Process' access semaphore.
	Semaphore*				getSemaphore();
	/*!< \returns reference to process' semaphore
	 * 
	 *   This call is used by child threads when needing to
	 *   access the process' lock level information, so
	 *   no duplicate lockings/unlockings take place.
	 */
	
	//! forward for SimpleCommandInterface::addCommandHandler()
	void 	 	            addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler );
	
	//! forward for SimpleCommandInterface::removeCommandHandler()
	void 	 	            removeCommandHandler( unsigned long CommandIdentifier );

	//! forward for SimpleCommandInterface::removeAllCommandHandler()
	void 	 	            removeAllCommandHandler();

	//! forward for SimpleCommandInterface::sendCommand()
	void	                sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );
	
	//! forward for SimpleCommandInterface::sendCommandNonblocking()
	void 		            sendCommandNonblocking( unsigned long CommandIdentifier, CommandBuffer Buffer );
	
	//! forward for SimpleCommandInterface::handleOneCommand()
	void 	 	            handleOneCommand( unsigned long& CommandIdentifier );
		
	//! start the process, or resume its runinng state
	void					run();

	//! suspend execution of the process
	void					stop();
		
	//! predict the amount of memory for control and data structures to be allocated via an allocator
	static size_t			predictSize();
	
	bool					isOSScope();
    
	void                    invokeOSScope(OSScopeFunction foo, OSScopeCommandBuffer param);
	
    SchedulingData          getOSScopeSchedulingData();
    
private:
	friend class ProcessManager;

	Process();
	virtual ~Process();
		
	//! explicit initialization
	void 					initialize( bool IsServer, bool IsMaster, const ASAAC_ProcessDescription& Description, MemoryLocation Location, SimpleCommandInterface *CommandInterface );
	/*!< \param[in] IsMaster		Flag indicating whether the calling instance shall act as master and
	 *                              initialize all communication structures of the process (usually the
	 *                              GSM should do this, and call with IsMaster = true )
	 *   \param[in] Description		Process description as indicated with the APOS call createProcess().
	 * 								Can be fed with a dummy variable on the client side.
	 * 
	 *   \param[in] Location		Flag indicating where to store the control structures.
	 *                              The GSM itself does not need to share its control structures with
	 *                              other processes, hence it can choose to store its data in LOCAL memory rather
	 *                              than SHARED.
	 * 
	 */

	//! explicit deinitialization
	void 					deinitialize();	

	void 					setServer( bool IsServer );
	bool 					isServer();

	//! launch the ProcessStarter to initialize the process
	void					launch();
	/*!< The actual starting of the new process in its current implementation, without
	 *   the longended mechanism via a MasterProcess/ProcessManager makes use of functionality
	 *   defined in the POSIX specification IEEE 1003.1, 2004 Edition 
	 *   (CX extension to the ISO C standard and the optional SPN module).
	 */ 
		
	//! stop and delete the process
	void		            destroy();
	/*!< The termination of another process in its current implementation depends on
	 *   waitpid() and kill() functions as defined in the POSIX specification IEEE 1003.1, 2004 Edition
	 */

	void		            detachAndDestroyAllLocalVcs();

	void		            resumeAllThreads();

	void		            suspendAllThreads();

	void		            terminateAllThreads();

	typedef struct  
	{
		ASAAC_ProcessDescription	Description;
		ProcessStatus				Status;
		unsigned long				LockLevel;
		unsigned long				AuthenticationCode;	
        SchedulingData              OSScopeSchedulingData;
	} ProcessData;
	
	typedef struct 
	{	
		ASAAC_PublicId		GlobalVcId;
		ASAAC_PublicId		LocalVcId;	
	} VCData;
	
    typedef union 
    {    
        CommandBuffer       ReturnBuffer;
        ASAAC_ReturnStatus  Return;
    } CommandData;
    
    typedef union {
        CommandBuffer       Buffer;
        ASAAC_ReturnStatus  Return;
        pid_t               PosixPid;
    } PIDCommandData;
    
	typedef union  
	{	
		CommandBuffer       ReturnBuffer;
		ASAAC_ReturnStatus  Return;
		VCData              VC;
	} VCCommandData;

	typedef union  
	{
		CommandBuffer       ReturnBuffer;
		ASAAC_ReturnStatus  Return;
		struct 
		{
			OSScopeFunction      foo;
			OSScopeCommandBuffer param;
		} Scope;
	} OSScopeCommandData;
	
	
	SharedMemory				m_SharedAllocator;
	LocalMemory					m_LocalAllocator;
	Allocator* 					m_Allocator;

	bool						m_IsInitialized;
	bool						m_IsMaster;
	bool						m_IsServer;
	
	Semaphore					m_Semaphore; 

	Shared<ProcessData>			m_ProcessData;
	Shared<VCData>				m_LocalVCs;
	Shared<EntryPoint>			m_EntryPoints;
	
	SimpleCommandInterface		m_InternalCommandInterface;
	SimpleCommandInterface*		m_CommandInterface;

	bool						m_ActiveMainLoop;
	
	Thread						m_Threads[ OS_MAX_NUMBER_OF_THREADS ];
	
	pid_t						m_PosixPid;
    
public:

	static void         RequestPIDHandler( CommandBuffer Buffer );
	
	static void			RunHandler( CommandBuffer Buffer );
	static void			StopHandler( CommandBuffer Buffer );
	
	static void 		DestroyHandler( CommandBuffer Buffer );
	
	static void			AttachLocalVcHandler( CommandBuffer Buffer );
	static void			DetachLocalVcHandler( CommandBuffer Buffer );
	static void 		InvokeOSScopeHandler( CommandBuffer Buffer );
};

#endif /*PROCESS_HH_*/
