#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include "OpenOSIncludes.hh"


class ThreadManager
{
public:
	//! get single instance of ThreadManager
	static ThreadManager* getInstance();
	/*!< \returns reference to instance of the ThreadManager
	 */

	//! destructor
	virtual ~ThreadManager();	


	// APOS Calls
	ASAAC_ReturnStatus sleep(const ASAAC_TimeInterval timeout);
		
	ASAAC_ReturnStatus sleepUntil(const ASAAC_Time absolute_local_time);
	
	ASAAC_ReturnStatus getMyThreadId(ASAAC_PublicId thread_id);
	
	ASAAC_ReturnStatus startThread(const ASAAC_PublicId thread_id);

	ASAAC_ReturnStatus suspendSelf();
	
	ASAAC_ReturnStatus stopThread(const ASAAC_PublicId thread_id);
		
	void terminateSelf();
		
	ASAAC_ReturnStatus lockThreadPreemption(unsigned long lock_level);
	
	ASAAC_ReturnStatus unlockThreadPreemption(unsigned long lock_level);
		
	ASAAC_ReturnStatus getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus thread_status);
	
	
	// SMOS Calls
	ASAAC_ReturnStatus createThread(const ASAAC_ThreadDescription thread_desc);
	
	ASAAC_ReturnStatus setSchedulingParameters(const ASAAC_ThreadSchedulingInfo thread_scheduling_info);

	ASAAC_ReturnStatus getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus &thread_status);	

private:
	ThreadManager();		
};

#endif /*THREADMANAGER_HH_*/
