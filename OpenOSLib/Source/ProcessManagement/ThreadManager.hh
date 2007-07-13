#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "ProcessManager.hh"

class ThreadManager
{
public:
	//! get single instance of ThreadManager
	static ThreadManager* getInstance();
	/*!< \returns reference to instance of the ThreadManager
	 */

	//! destructor
	virtual ~ThreadManager();	

	void initialize();
	void deinitialize();
	bool isInitialized();
	
	// APOS Calls
	void sleep(const ASAAC_TimeInterval timeout);
		
	void sleepUntil(const ASAAC_Time absolute_local_time);
	
	void getMyThreadId(ASAAC_PublicId thread_id);
	
	void startThread(const ASAAC_PublicId thread_id);

	void suspendSelf();
	
	void stopThread(const ASAAC_PublicId thread_id);
		
	void terminateSelf();
		
	void lockThreadPreemption(unsigned long lock_level);
	
	void unlockThreadPreemption(unsigned long lock_level);
		
	void getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus thread_status);
	
	
	// SMOS Calls
	void createThread(const ASAAC_ThreadDescription thread_desc);
	
	void setSchedulingParameters(const ASAAC_ThreadSchedulingInfo thread_scheduling_info);

	void getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus &thread_status);
	
	
	// Access
	
	//! get reference to current thread's controlling Thread instance
	Thread* getCurrentThread( const bool do_throw = true );

	//! get reference to thread defined by current process and ThreadId
	Thread* getThread( ASAAC_PublicId ThreadId, const bool do_throw = true );

	//! get reference to thread defined by ProcessId and ThreadId
	Thread* getThread( ASAAC_PublicId ProcessId, ASAAC_PublicId ThreadId, const bool do_throw = true );

private:
	ThreadManager();
	
	bool m_IsInitialized;
};

#endif /*THREADMANAGER_HH_*/


