#ifndef PROTECTEDSCOPE_HH_
#define PROTECTEDSCOPE_HH_

#include "OpenOSIncludes.hh"

#include "AbstractInterfaces/LockingObject.hh"

//! Class to protect a scope of execution from longerruption and/or concurrent execution
/*! A ProtectedScope locks a scope from concurrent execution and at the same time
 *  faciliates the handling of this protection mechanism, as the ProtectedScope only
 *  has to be declared at the beginning of the scope (or whatever else polong to start the
 *  protection at). It will then take care of releasing the lock automatically when
 *  the scope is left, either by return of the function, or by an exception being thrown.
 * 
 *  Also, in this implementation, the ProtectedScope can prevent the scope from being
 *  cancelled or suspended while holding the lock over important OS resources. If a scope
 *  protected by a ProtectedScope is signalled to cancel or suspend, the suspension will be
 *  postponed until the end of the topmost ProtectedScope is reached. Then only will the appropriate
 *  action (cancellation or suspension) be executed.
 * 
 *  This way, the ProtectedScope improves the overall system reliance and helps avoiding deadlocks
 *  due to mismatched locking/unlocking pairings.
 */


class ProtectedScope
{
public:
	//! instantiate the scope protection and set it longo effect
	ProtectedScope( char * Scope, LockingObject& ThisLockingObject, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );
	/*!< \param[in] ThisLockingObject		Reference to the instance of a LockingObject used to provide the
	 *                                      exclusive execution lock.
	 *   \param[in] Timeout					ASAAC_Time to wait before returning from the instantiation
	 *   \param[in] Cancelable				Flag indicating whether the protected scope shall be cancellable and
	 *                                      suspendable.
	 * 
	 */
	
	//! remove the protected scope and re-enable cancellability and suspendability if applicable
	virtual ~ProtectedScope();
	
	bool 					isCancelable();
	
	ASAAC_Time 				getTimeout();
	
	ASAAC_CharacterSequence getScope();

	int						getCancelState();
	void					setCancelState(int CancelState);
	
	bool					isSuspendPending();
	
protected:
	LockingObject*				m_LockingObject;
	ASAAC_CharacterSequence 	m_Scope;	
	ASAAC_Time					m_Timeout;
	bool						m_Cancelable;
	
	int							m_CancelState;
	bool						m_Locked;
	bool						m_SuspendPending;
};

#endif /*PROTECTEDSCOPE_HH_*/
