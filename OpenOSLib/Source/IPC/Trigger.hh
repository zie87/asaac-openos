#ifndef TRIGGER_HH_
#define TRIGGER_HH_

#include "OpenOSIncludes.hh"

#include "Common/Templates/Shared.hh"

#include "Allocator/AllocatedArea.hh"

//! IPC signal using a counter increment

/*! A Trigger is a &quot;Signal&quot; that doesn't correspond to
 *  a posix signal, but rather is a construct made up
 *  of posix condition variables.
 * 
 * The mechanisms required to provide the Trigger functionality make use of 
 * mutexes and condition variables as
 * specified in the THR optional component of the POSIX standard IEEE Std 1003.1, 2004 Edition.
 * 
 *  The benefits of this kind of signal are the following:
 * 
 *  - The Triggers are not subject to the user-id check of
 *    posix signals, which limits the communication paths and
 *    is quite performance costly.
 * 
 *  - There can be an almost infinite number of such triggers,
 *    limited only by the number of signal objects and shared
 *    memory areas for the triggers that can be allocated.
 * 
 *  - Applications that are not waiting for the trigger
 *    or are oblivious of it ignore it, rather than defaulting their
 *    action to termination of the application.
 * 
 *  The trigger is implemented using a "Counter"
 *  condition variable. Waiting threads wait for an
 *  increment of that counter. This way it is assured that
 *  even two simultateous "triggers" reliably
 *  release waiting threads.
 */


class Trigger
{

public:
	struct TriggerData {
		oal_thread_mutex_t	    Mutex;
		oal_thread_cond_t 		Condition;
		oal_thread_condattr_t	ConditionAttribute;
		unsigned long		TriggerValue;
		unsigned long		WaitingThreads;
	};

	//! non-initializing constructor
	Trigger( );

	//! explicit initialization
	void initialize( Allocator* ParentAllocator, bool IsMaster = true );
    /*!< \param[in] ThisAllocator Reference to the Allocator used to
     *                            allocate memory for shared structures of the trigger
 	 *   \param[in] IsMaster		Flag indicating whether the calling instance will act as master of the allocated
 	 *                              data and initialize all OS structures required for the trigger to work.
 	 */
       

	virtual ~Trigger();
	
	void deinitialize();

	void trigger();
	
	//! get the current trigger counter value
	unsigned long getTriggerState();
	/*!< it can be important to obtain the current trigger state before checking for a condition that
	 *   updates with each trigger impulse.
	 * 
	 * \returns Current value of the trigger counter
	 */
	
	//! wait for trigger counter to be incremented over the supplied trigger state
	void waitForTrigger( unsigned long& TriggerState, const ASAAC_TimeInterval& Timeout = TimeIntervalInfinity );
    /*!< transfer the calling thread longo WAITING state until the counter of the trigger exceeds the value
     *   indicated as a parameter, or
     *   until the Timeout has elapsed. 
     *  
     * \param[in,out] TriggerState       Value of the trigger counter state to start the waiting from.
     *                                  Returns the trigger counter at exit of the function.
     * \param[in]	 Timeout		    Maximum time to wait
     * 
     * \returns ASAAC_TM_SUCCESS if a trigger event occurred during the waiting period
     *          ASAAC_TM_TIMEOUT if the timeout elapsed.
     *          ASAAC_TM_ERROR if an error occured during the operation
     */
	
	//! wait for trigger counter to be incremented
	void waitForTrigger( const ASAAC_TimeInterval& Timeout = TimeIntervalInfinity );
    /*!< transfer the calling thread longo WAITING state until the counter of the trigger is increased over
     *   its value at function call time,
     *   or until the Timeout has elapsed. 
     *  
     * \param[in]	 Timeout		    Maximum time to wait
     * 
     * \returns ASAAC_TM_SUCCESS if a trigger event occurred during the waiting period
     *          ASAAC_TM_TIMEOUT if the timeout elapsed.
     *          ASAAC_TM_ERROR if an error occured during the operation
     */
	
	
	//! return number of threads waiting for a trigger counter increment
	unsigned long getWaitCount();

	bool IsInitialized();

	
    //! get memory amount required from the Allocator by the trigger's data and control structures
	static size_t predictSize();

	void* operator new( size_t size );
	void operator delete( void* location );

private:
	// prevent implicit copying and assignment
	Trigger( const Trigger& Source );
	Trigger& operator=( const Trigger& Source );

	bool 				m_IsMaster;
	bool 				m_IsInitialized;
	
	Shared<TriggerData> m_Global;
	
	AllocatedArea		m_Allocator;
	
};


#endif /*TRIGGER_HH_*/
