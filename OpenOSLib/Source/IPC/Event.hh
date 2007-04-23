#ifndef EVENT_HH_
#define EVENT_HH_

#include "OpenOSIncludes.hh"

#include "Common/Shared.hh"


//! class for handling of Events
/* The event class provides the functionality required by ASAAC Events (as described in STANAG 4626, Part II,
 * Section 11.4.4), but extends the functionality to allow for inter-process synchronization as required for
 * the implemented communication mechanisms.
 * 
 * The mechanisms required to provide the Event services make use of mutexes and condition variables as
 * specified in the THR optional component of the POSIX standard IEEE Std 1003.1, 2004 Edition.
 * 
 */
static const unsigned int INITIALIZED_FLAG = 54749010;

class Event
{

public:
	//! definition of all shared data required to access an event from multiple processes and threads
	struct EventData {
		oal_thread_mutex_t	    Mutex;
		oal_thread_cond_t 		Condition;
		oal_thread_condattr_t	ConditionAttribute;
		bool				EventSet;
		unsigned int		WaitingThreads;
		unsigned int   	InitializedFlag;
	};

	//! initializing constructor (see initialize())
	Event( Allocator* ThisAllocator, bool IsMaster = true );
	
	//! non-initializing constructor
	Event( );

	//! explicit initialization
	void initialize( Allocator* ThisAllocator, bool IsMaster = true );
	/*!< \param[in] ThisAllocator Reference to the allocator used to allocate memory for the EventData
	 *   \param[in] IsMaster      Flag indicating whether the calling thread shall be the master instance
	 *                            of the Event and set up all control structures required for synchronization
	 *                            via the POSIX shared condition.
	 */

	virtual ~Event();
	
	//! explicit deinitialization
	void deinitialize();

	//! set the event
	void setEvent();
	
	//! clear/reset the event
	void resetEvent();
	
	//! wait for the event to be set
	ASAAC_TimedReturnStatus waitForEvent( const ASAAC_Time& Timeout = TimeInfinity );
	/*!< The function transfers the calling thread into WAITING state until either
	 *   the event is set, or the Timeout has expired.
	 * 
	 * \param[in] Timeout Timeinterval to wait until aborting the wait and returning ASAAC_TM_TIMEOUT
	 * 
	 * \returns ASAAC_TM_SUCCESS if event was set before timeout elapsed.
	 *          ASAAC_TM_TIMEOUT if the timeout elapsed.
	 *          ASAAC_TM_ERROR if an error occurred during the waiting.
	 */
	 
	//! wait for the event to be set
	ASAAC_TimedReturnStatus waitForEventReset( const ASAAC_Time& Timeout = TimeInfinity );
	/*!< The function transfers the calling thread into WAITING state until either
	 *   the event is reset, or the Timeout has expired.
	 * 
	 * \param[in] Timeout Timeinterval to wait until aborting the wait and returning ASAAC_TM_TIMEOUT
	 * 
	 * \returns ASAAC_TM_SUCCESS if event was reset before timeout elapsed.
	 *          ASAAC_TM_TIMEOUT if the timeout elapsed.
	 *          ASAAC_TM_ERROR if an error occurred during the waiting.
	 */
	
	//! query the status of the event
	bool isEventSet();
	/*< \returns true if the event is set. false if the event is not set.
	 */
	
	//! get the number of threads waiting for a status change of the event
	unsigned int getWaitingThreads();
	/*! \returns number of waiting threads */
	
	//! return amount of memory required from allocator by control and data structures of this object
	static size_t predictSize();

	//! override for the new operator, to obtain memory-static behaviour
	void* operator new( size_t size );
	/*!< rather than using the standard functions for new and delete,
	 * the current implementation of Event maps these to calls to an ObjectPool that
	 * holds a number (OS_MAX_NUMBER_OF_EVENTS) of Event objects for dynamic allocation 
	 * and de-allocation.
	 */
	
	//! override for the delete operator, to obtain memory-static behaviour
	void operator delete( void* location );
	/*!< rather than using the standard functions for new and delete,
	 * the current implementation of Event maps these to calls to an ObjectPool that
	 * holds a number (OS_MAX_NUMBER_OF_EVENTS) of Event objects for dynamic allocation 
	 * and de-allocation.
	 */

private:
	// prevent implicit copying and assignment
	Event( const Event& Source );
	Event& operator=( const Event& Source );

	bool m_IsMaster;
	bool m_IsInitialized;
	Shared<EventData> Global;
	
	//! wait for the event to be set to a certain status
	ASAAC_TimedReturnStatus waitForEventStatus( bool Status, const ASAAC_Time& Timeout );
	/*!< The function transfers the calling thread into WAITING state until either
	 *   the event is set to the indicated status, or the Timeout has expired.
	 * 
	 * \param[in] Status Event status to wait for. true means event set, false means event reset.
	 * \param[in] Timeout Timeinterval to wait until aborting the wait and returning ASAAC_TM_TIMEOUT
	 * 
	 * \returns ASAAC_TM_SUCCESS if event status matched before timeout elapsed.
	 *          ASAAC_TM_TIMEOUT if the timeout elapsed.
	 *          ASAAC_TM_ERROR if an error occurred during the waiting.
	 */


	//! set the Event status to the one indicated by the parameter
	void setEventStatus( bool Status );
	/*!< the function updates the Event status with the one indicated by the parameter,
	 * and notifies all waiting instances of the change, so these can then check for their
	 * wait condition to be fulfilled.
	 * 
	 * \param[in] Status Status of the Event to be set. true means event set, false means event reset.
	 */

};


#endif /*Event_HH_*/
