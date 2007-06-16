#ifndef NOLOCK_HH_
#define NOLOCK_HH_

#include "LockingObject.hh"

//! non-locking object for ProtectedScopes
/*! the NoLock object can be used for a ProtectedScope where only the
 *  non-interruption character of the ProtectedScope is required.
 *  Both the lock() and the release() functions are stubs without function.
 */

class NoLock : public LockingObject
{
public:
	virtual ASAAC_ReturnStatus lock() { return ASAAC_SUCCESS; }
	virtual ASAAC_ReturnStatus release() { return ASAAC_SUCCESS; }
	
	
	virtual ~NoLock() {}
};

#endif /*NOLOCK_HH_*/
