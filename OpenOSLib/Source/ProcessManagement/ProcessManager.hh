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
	virtual  ~ProcessManager();

	// TODO: move this function out of this class
	void	 addEntryPoint( ASAAC_CharacterSequence Name, EntryPointAddr Address );

	// Basic initialize/deinitialize tasks
	void     initialize( bool IsServer, bool IsMaster, Allocator *ParentAllocator, ASAAC_PublicId CurrentCpuId, ASAAC_PublicId CurrentProcessId, MemoryLocation Location = SHARED );
	void     deinitialize();
	
	bool     isInitialized();

	//! start a new process under control of the current one
	void     createProcess( const ASAAC_ProcessDescription& Description );
	/*!< This function contains the functionality required by the SMOS
	 *   call createProcess(). It sets up the data structures required by the
	 *   process and for the communication between this process as a master and
	 *   the newly created process as a slave.
	 * 
	 *   \param[in] Description Process description as supplied to the SMOS call
	 *                          createProcess()
	 */
	 
	void     destroyProcess( const ASAAC_PublicId& ProcessId );

	void     runProcess(const ASAAC_PublicId process_id);
	 
	void     stopProcess(const ASAAC_PublicId process_id);

	long     getProcessIndex( ASAAC_PublicId ProcessId );

	//! get process instance
	Process* getProcess( ASAAC_PublicId ProcessId, const bool do_throw = true );
	/*!< This function returns a reference to the process instance handling
	 *   the process with the indicated ProcessId, if such a process is
	 *   a slave of the current process, or the current process itself.
	 * 
	 *   \param[in] ProcessId ASAAC_PublicId of the requested Process.
	 *   \returns   Reference to Process instance. 0, if no process with indicated
	 *              ProcessId could be found.
	 */
	
	Process* allocateProcess( const ASAAC_ProcessDescription& Description, long &Index );
			
    void     releaseProcess( ASAAC_PublicId ProcessId );    
    void     releaseAllProcesses();    
    void     releaseAllClientProcesses();
        
    //! set the id of the 'current process' controlling Process instance
    void     setCurrentProcess( ASAAC_PublicId ProcessId );
	
	//! get reference to current process' controlling Process instance
	Process* getCurrentProcess(  const bool do_throw = true );
	
	//! forward for SimpleCommandInterface::addCommandHandler()
	void 	 addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler );
	
	//! forward for SimpleCommandInterface::removeCommandHandler()
	void 	 removeCommandHandler( unsigned long CommandIdentifier );

	//! forward for SimpleCommandInterface::removeCommandHandler()
	void 	 removeAllCommandHandler();

	//! forward for SimpleCommandInterface::sendCommand()
	void	 sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );

	//! forward for SimpleCommandInterface::sendCommand()
	void 	 sendCommandNonblocking( unsigned long CommandIdentifier, CommandBuffer Buffer );
	
	//! forward for SimpleCommandInterface::handleOneCommand()
	void 	 handleOneCommand( unsigned long& CommandIdentifier );
	
	void     destroyAllClientProcesses();
	void     destroyEntity();
	
	ASAAC_PublicId getCurrentCpuId();
	ASAAC_PublicId getCurrentProcessId();
	ASAAC_PublicId getCurrentThreadId();
	
	//! predict the amount of memory for control and data structures to be allocated via an external allocator
	static size_t	predictSize();
	
	//! predict the amount of memory for control and data structures to be allocated via an longernal allocator
	static size_t	predictInternalSize();
	

private:
	ProcessManager();
	
	// TODO: move this function out of this class
	void handleBufferMemory();

	typedef struct 
	{
		ASAAC_ProcessDescription Description;
		ASAAC_Time 				 Timeout;
	} ProcessCommandData;
	

	union CommandData 
	{		
		CommandBuffer 			 ReturnBuffer;
		ProcessCommandData 		 Data;
		ASAAC_TimedReturnStatus  Return;
	};

	LocalMemory	 			m_LocalMemoryAllocator;
	SharedMemory 			m_SharedMemoryAllocator;

	long					m_CurrentProcessIndex;
	
	Process					m_ProcessObject[ OS_MAX_NUMBER_OF_PROCESSES ];
	Shared<ASAAC_PublicId>	m_ProcessId;
	
	Semaphore   			m_Semaphore;
	
	SimpleCommandInterface	m_CommandInterface;
	
	ASAAC_PublicId 			m_CurrentCpuId;
	ASAAC_PublicId 			m_CurrentProcessId;
	ASAAC_PublicId 			m_CurrentThreadId;
	
	bool					m_IsMaster;
	bool					m_IsServer;
	bool					m_IsInitialized;
	
	
public:
	static void	CreateProcessHandler( CommandBuffer Buffer );
	static void DestroyProcessHandler( CommandBuffer Buffer );	
	static void DestroyEntityHandler( CommandBuffer Buffer );	
		
};

#endif /*PROCESSMANAGER_HH_*/
