#ifndef NOLOCK_HH_
#define NOLOCK_HH_

#include "AbstractInterfaces/LockingObject.hh"

//! non-locking object for ProtectedScopes
/*! the NoLock object can be used for a ProtectedScope where only the
 *  non-interruption character of the ProtectedScope is required.
 *  Both the lock() and the release() functions are stubs without function.
 */

class NoLock : public LockingObject
{
public:
	virtual void lock( const ASAAC_Time& Timeout = TimeInfinity ) {  }
	virtual void release() {  }
	
	
	virtual ~NoLock() {}
};

#endif /*NOLOCK_HH_*/
