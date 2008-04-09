#ifndef PROCESSSTATUS_HH_
#define PROCESSSTATUS_HH_


//! Enumeration for the process' state
/*! The ProcessStatus is a flag indicating the process' current state.
 */
 
enum ProcessStatus {
	
	PROCESS_DORMANT,
	PROCESS_INITIALIZED,
	PROCESS_RUNNING,
	PROCESS_STOPPED
	
};

#endif /*PROCESSSTATUS_HH_*/
