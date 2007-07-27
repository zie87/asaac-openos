#ifndef THREAD_HH_
#define THREAD_HH_

#include "OpenOSIncludes.hh"

#include "Common/Templates/Shared.hh"
#include "AbstractInterfaces/Callback.hh"

#include "IPC/ProtectedScope.hh"

class Process;

//! object class responsible for handling all operations related to a thread

class Thread
{
public:
	Thread();
	virtual ~Thread();

	//! explicit initialization
	void 		initialize( bool IsMaster, 
			                const ASAAC_ThreadDescription& Description,
			                Process* ParentProcess,
			                Allocator* ThisAllocator );
	/*!< This function allocates the memory required for the storing of
	 *   thread configuration and control of one thread in a process.
	 * 
	 *   The initialization of a Thread structure is independent of an actual
	 *   assignment of the structure to a specific thread. All thread instances
	 *   of a process need to be initialized when the Process itself is initialized,
	 *   and will be assigned later via assign() once a respective thread creation
	 *   is requested.
	 *
	 *   \param[in] ThisAllocator Reference to Allocator used to allocate memory
	 *                            for this Thread's structures.
	 *   \param[in] IsMaster      Flag indicating whether this instance shall be
	 *                            initialized as a Master instance that configures
	 *                            all required synchronization mechanisms and
	 *                            initializes all required values.
	 *   \param[in] ParentProcess Reference to the parent process of this thread.
	 * 
	 *  
	 */
	 
	 
	void 		deinitialize();
	
	bool 		isInitialized();
	
	//! check if this thread is the one currently running
	bool		isCurrentThread();
	/*!< \returns true, if this thread is the one currently running. false otherwise.
	 */
	
	bool 		isCancelable( ASAAC_CharacterSequence &scope );
	

	//! get the ASAAC_PublicId of this thread
	ASAAC_PublicId	getId();
	/*!< \returns ASAAC_PublicId of the thread handled by this Thread instance.
	 *           'UnusedId', if the thread is not yet assigned.
	 */
	
	//! set the ASAAC_ThreadStatus of this thread
	void		setState( ASAAC_ThreadStatus State );
	
	//! get the ASAAC_ThreadStatus of this thread
	void		getState( ASAAC_ThreadStatus& State );
	
	//! set the scheduling parameters of this thread	
	void		setSchedulingParameters( const ASAAC_ThreadSchedulingInfo& SchedulingInfo );
	/*!< actually, this function is a stub still, doing nothing but returning ASAAC_SUCCESS
	 *   if the preconditions of the function are met.
	 */
	
	void		update();
	
	//! start the thread
	void		start();
	/*!< This function starts a previously configured and assigned thread, i.e.
	 *   it creates the corresponding POSIX thread with the parameters and entry
	 *   polong as assigned to this Thread instance and it sets the thread state
	 *   to RUNNING.
	 * 
	 *   \returns ASAAC_SUCCESS on successful operation.
	 * 				ASAAC_ERROR if the thread is not configured, or not in DORMANT state.
	 */
	
	
	//! stop the thread
	void		stop();
	/*!< This function causes the thread to be terminated. If the thread is currently
	 *   suspended, it is resumed for as long as it requires to reach the next cancellation
	 *   polong where it is safe to cancel execution without blocking OS resources shared
	 *   with other threads. After cancellation, the thread's state is DORMANT.
	 * 
	 *   \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR if the thread is in DORMANT
	 *            state.
	 */
	
	
	//! wait for the termination of this thread
	void		waitForTermination();
	/*!< This function causes the calling instance to block until this thread is
	 *   terminated. If called on the current thread, the function returns ASAAC_ERROR.
	 * 
	 *  \returns ASAAC_SUCCESS on successful completion of the waiting. ASAAC_ERROR if the
	 *          current thread is waited for.
	 */


	//! suspend execution of this thread
	void		suspend();
	/*!< This function causes the controlled thread to be excluded from scheduling.
	 *   If the controlled thread currently is executing in a ProtectedScope and holding
	 *   shared OS resources, the suspension will be enacted as soon as the thread leaves
	 *   its protected scope, so no OS resources will be blocked during a suspension.
	 * 
	 *   \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR in case of an error, e.g.
	 *            if the calling thread is trying to suspend itself, or the controlled
	 *            thread is in DORMANT state.
	 */
	
	
	//! suspend execution of this thread if it is the current one, for this scheduling cycle
	void		suspendSelf();
	/*!< This function causes the controlled thread, if it is the current one,
	 *   to give up the remaints of its scheduling slice, in favour of other threads and/or
	 *   processes. The exact behaviour of this function is hardly described in the
	 *   specification. The current implementation translates suspendSelf() longo
	 *   a call of sched_yield().
	 * 
	 *   \returns ASAAC_ERROR if the process' current lock level is > 0. ASAAC_SUCCESS otherwise.
	 */

	
	//! resume execution of the thread
	void		resume();
	/*!< This function awakens the controlled thread from a suspended state.
	 *   If the affected thread is the current thread, ASAAC_ERROR is returned. Likewise
	 *   if the affected thread is in DORMANT state.
	 * 
	 *   \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR if controlled thread is not
	 *            resumed, is the current thread, or is in DORMANT state.
	 */
	
	
	//! query if the current thread has a suspension pending
	bool		isSuspendPending();
	/*!< By means of this function, the status of a thread is queried whenever
	 *   it leaves a ProtectedScope.
	 * 
	 *   \returns true, if the current thread shall be suspended at the next possible
	 *            occasion. false otherwise.
	 */
	
	
	//! set the suspend-pending flag of the controlled thread
	void		setSuspendPending( bool Value );
	/*!< This function sets the suspend-pending flag of the controlled thread.
	 */
	
	
	//! stop execution of current thread
	void		terminateSelf();
	/*!< If the called thread is the currently executed thread,
	 *   the thread execution will be stopped and the current thread will
	 *   return longo DORMANT state.
	 */



	//! stop execution
	void		terminate();
	/*!< The thread execution will be stopped and the current thread will
	 *   return longo DORMANT state.
	 */

	void 		sleep(const ASAAC_TimeInterval timeout);
		
	void 		sleepUntil(const ASAAC_Time absolute_local_time);

	
	//! predict the amount of memory for control and data structures to be allocated via an allocator	
	static size_t	predictSize();
	
			
private:
	friend class ProtectedScope;

	void enterProtectedScope(ProtectedScope *ps);
	void exitProtectedScope(ProtectedScope *ps);

	struct ThreadData 
    {	
		ASAAC_ThreadDescription 		Description;
		ASAAC_ThreadSchedulingInfo		SchedulingInfo;
		ASAAC_ThreadStatus				Status;
		oal_thread_t					PosixThread;
		oal_thread_attr_t				PosixThreadAttributes;
		unsigned long					SuspendLevel;
	};

	Process*			m_ParentProcess;
	
	Shared<ThreadData>	m_ThreadData;
	
	bool				m_IsInitialized;
	
	bool				m_SuspendPending;
	
	static				void* ThreadStartWrapper( void* );
	
	ProtectedScope*		m_ProtectedScopeStack[OS_MAX_NUMBER_OF_PROTECTEDSCOPES];
	unsigned long		m_ProtectedScopeStackSize;

};




#endif /*THREAD_HH_*/
