#ifndef PROCESSMANAGER_HH_
#define PROCESSMANAGER_HH_

#include "Thread.hh"
#include "Process.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/SharedMemory.hh"

//! central instance for the management of processes

/*! The ProcessManager holds the data of all processes and
 *  that related to the latter, as well as providing the
 *  required functionality to create new processes.
 * 
 */

class ProcessManager
{
public:
	//! get single instance of ProcessManager
	static ProcessManager* getInstance();
	/*!< \returns reference to instance of the ProcessManager
	 */

	//! destructor
	virtual ~ProcessManager();

	ASAAC_ReturnStatus		addEntryPoint( ASAAC_CharacterSequence Name, EntryPointAddr Address );

	//! set up structures for current process as a master process
	ASAAC_ReturnStatus initializeEntityProcess(  bool IsMaster, Allocator *ParentAllocator, ASAAC_PublicId CpuId );
	/*! This function sets up all structures of the process manager
	 *  for the current thread in a local, self-contained way. The
	 *  master process does not need any controlling instance, it
	 *  does not need to be configured by an external source, so
	 *  it stores all its required data in local memory rather than
	 *  shared memory.
	 * 
	 *  \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR in case of an
	 *           error encountered, such as the current process
	 *           already being initialized.
	 */
	
	
	//! start a new process under control of the current one
	ASAAC_TimedReturnStatus createClientProcess( const ASAAC_ProcessDescription& Description );
	/*!< This function contains the functionality required by the SMOS
	 *   call createProcess(). It sets up the data structures required by the
	 *   process and for the communication between this process as a master and
	 *   the newly created process as a slave.
	 * 
	 *   Actually, instead of the process itself, in the current implementation scheme,
	 *   createClientProcess() launches the ProcessStarter which holds control and
	 *   prepares OS resources for the actual application until the latter is finally
	 *   started when it is set longo RUNNING state, via Process::run().
	 * 
	 *   \param[in] Description Process description as supplied to the SMOS call
	 *                          createProcess()
	 * 
	 *   \returns ASAAC_TM_SUCCESS if the process starter could properly be launched.
	 *            ASAAC_TM_TIMEOUT if the process starter did not respond within the
	 *                       time frame set up as timeout for the process creation.
	 *            ASAAC_TM_ERROR   if an error occurred during the launching of the
	 *                       ProcessStarter.
	 */
	 
	ASAAC_ReturnStatus destroyClientProcess( const ASAAC_PublicId& ProcessId );

	ASAAC_ReturnStatus runProcess(const ASAAC_PublicId process_id);
	
	ASAAC_ReturnStatus stopProcess(const ASAAC_PublicId process_id);

	// Basic initialize/deinitialize tasks
	void initialize(  bool IsServer, bool IsMaster, Allocator *ParentAllocator, ASAAC_PublicId CpuId, MemoryLocation Location = SHARED );
	void deinitialize();
	
	bool isInitialized();

	long getProcessIndex( ASAAC_PublicId ProcessId );
	Process* getProcess( ASAAC_PublicId ProcessId, long &Index );
	//! get process instance
	Process* getProcess( ASAAC_PublicId ProcessId );
	/*!< This function returns a reference to the process instance handling
	 *   the process with the indicated ProcessId, if such a process is
	 *   a slave of the current process, or the current process itself.
	 * 
	 *   \param[in] ProcessId ASAAC_PublicId of the requested Process.
	 *   \returns   Reference to Process instance. 0, if no process with indicated
	 *              ProcessId could be found.
	 */
	
	Process* createProcess( bool IsMasterProcess, const ASAAC_ProcessDescription& Description, long &Index );
	
	//! set up structures for current process as a client/slave process
	ASAAC_ReturnStatus initializeClientProcess(  Allocator *ParentAllocator, ASAAC_PublicId CpuId, ASAAC_PublicId ProcessId, MemoryLocation Location );
	/*!< This function sets up all structures of the process manager
	 *  for the current thread in a way that allows the process to
	 *  be controlled and configured by the respective master by
	 *  access to common data structures and the SimpleCommandInterface
	 *  supplied for master/client communication.
	 * 
	 *  \param[in] ProcessId ASAAC_PublicId of the current process.
	 *  \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR in case of an
	 *           error encountered, such as the current process
	 *           already being initialized.
	 */
		
    void releaseProcess( ASAAC_PublicId ProcessId );    
    void releaseAllProcesses();    
    void releaseAllClientProcesses();
        
    //! set the id of the 'current process' controlling Process instance
    void setCurrentProcess( ASAAC_PublicId ProcessId );
	
	//! get reference to current process' controlling Process instance
	Process* getCurrentProcess();
	
	//! get reference to current thread's controlling Thread instance
	Thread*  getCurrentThread();
	
	//! forward for SimpleCommandInterface::addCommandHandler()
	ASAAC_ReturnStatus 	 addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler );
	
	//! forward for SimpleCommandInterface::removeCommandHandler()
	ASAAC_ReturnStatus 	 removeCommandHandler( unsigned long CommandIdentifier );

	//! forward for SimpleCommandInterface::removeCommandHandler()
	ASAAC_ReturnStatus 	 removeAllCommandHandler();

	//! forward for SimpleCommandInterface::sendCommand()
	ASAAC_TimedReturnStatus sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );
	
	//! forward for SimpleCommandInterface::handleOneCommand()
	ASAAC_ReturnStatus 	 handleOneCommand( unsigned long& CommandIdentifier );
	
	ASAAC_ReturnStatus destroyAllClientProcesses();
	ASAAC_ReturnStatus destroyEntity();
	
	ASAAC_PublicId getCpuId();
	
	ASAAC_PublicId getProcessId( ProcessAlias Alias );
	
	//! predict the amount of memory for control and data structures to be allocated via an external allocator
	static size_t	predictSize();
	
	//! predict the amount of memory for control and data structures to be allocated via an longernal allocator
	static size_t	predictInternalSize();
	

private:
	ProcessManager();
	void handleBufferMemory();

	typedef struct {
		ASAAC_ProcessDescription Description;
		ASAAC_Time Timeout;
	} ProcessCommandData;
	

	union CommandData {		
		CommandBuffer ReturnBuffer;
		ProcessCommandData Data;
		ASAAC_TimedReturnStatus  Return;
	};

	EntryPoint				m_BufferedEntryPoints[OS_MAX_NUMBER_OF_ENTRYPOINTS];
	unsigned long			m_BufferedEntryPointCounter;

	LocalMemory	 			m_LocalMemoryAllocator;
	SharedMemory 			m_SharedMemoryAllocator;

	long					m_CurrentProcessIndex;
	
	Process					m_ProcessObject[ OS_MAX_NUMBER_OF_PROCESSES ];
	Shared<ASAAC_PublicId>	m_ProcessId;
	
	Semaphore   			m_Semaphore;
	
	SimpleCommandInterface	m_CommandInterface;
	
	ASAAC_PublicId 			m_CpuId;
	
	bool					m_IsMaster;
	bool					m_IsServer;
	bool					m_IsInitialized;
	
	
public:
	static void	CreateProcessHandler( CommandBuffer Buffer );
	static void DestroyProcessHandler( CommandBuffer Buffer );	
	static void DestroyEntityHandler( CommandBuffer Buffer );	
		
};

#endif /*PROCESSMANAGER_HH_*/
