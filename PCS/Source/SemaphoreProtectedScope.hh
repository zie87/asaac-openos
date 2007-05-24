#ifndef SEMAPHOREPROTECTEDSCOPE_HH_
#define SEMAPHOREPROTECTEDSCOPE_HH_

#include "OpenOS.hh"

//! Scope-protecting Semaphore Object

/*! A SemaphoreProtectedScope object, one instantiated, causes execution to depend
 *  on the availability of the Semaphore denoted in its constructor.
 * 
 * This mechanism can be employed to protect a function scope from concurrent execution,
 * automatically taking care of freeing said semaphore even if the function is exited by
 * means of a thrown Exception.
 * 
 */
 
class SemaphoreProtectedScope
{
public:
	SemaphoreProtectedScope( ASAAC_PublicId SemaphoreId, const ASAAC_TimeInterval& Timeout);
	/*!< begin a Semaphore protected code block.
	 * 
	 * @param[in] SemaphoreId denotes a Semaphore successfully created by means of createSemaphore()
	 * @param[in] Timeout     indicates a maximum waiting time to obtain lock on named semaphore
	 * 
	 * @pre SemaphoreId must denote a valid ASAAC Semaphore object
	 * 
	 * In case of a timeout or error during the waiting for and locking of the semaphore,
	 * an Exception is thrown.
	 */
	 
	virtual ~SemaphoreProtectedScope();
	
private:
	ASAAC_PublicId	m_Semaphore; //!< Public Id of Semaphore
};

#endif /*SEMAPHOREPROTECTEDSCOPE_HH_*/
