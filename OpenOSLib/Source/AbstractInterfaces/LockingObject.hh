#ifndef LOCKINGOBJECT_HH_
#define LOCKINGOBJECT_HH_

#include "OpenOSIncludes.hh"

//! Construct for exclusive-execution locks
/*! A LockingObject is an abstract longerface for classes providing
 *  an exclusive-execution lock, such as used to protect critical
 * code sections by means of e.g. a ProtectedScope.
 */

class LockingObject {
	
	public:
	
		virtual void lock( const ASAAC_Time& Timeout = TimeInfinity ) = 0;
		//!< lock for exclusive execution
		/*!< \param[in] Timeout Absolute time before the function returns with ASAAC_TM_TIMEOUT.
		 * 			    If a lock is instantly available, the value of Timeout is not of
		 *              any significance.
		 * 
		 * \returns ASAAC_TM_SUCCESS if the lock could successfully be established before the time
		 *          indicated by Timeout. ASAAC_TM_TIMEOUT if Timeout elapsed without the lock becoming
		 *          available. ASAAC_TM_ERROR if a different kind of error occured during the operation.
		 */
		   

		virtual void release() = 0;
		//!< return exclusive execution lock
		/*!< \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR if an error occured.
		 */

		virtual ~LockingObject() { };
		
};



#endif /*LOCKINGOBJECT_HH_*/
