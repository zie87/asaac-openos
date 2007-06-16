#ifndef MUTEX_HH_
#define MUTEX_HH_

#include "Common/Templates/Shared.hh"
#include "AbstractInterfaces/LockingObject.hh"

//! !!! DRAFT OBJECT !!! exclusive execution lock providing protection from priority inversion
/*! !!! DRAFT OBJECT !!!
 * 
 * 	the mutex object can, if the required functionality is provided
 *  by the underlying POSIX layer (THR and either TPI or TPP extension,
 *  IEEE 1003.1, 2004 Edition), lock the
 *  exclusive execution of areas of code as established with ProtectedScopes
 *  using a Priority Inheritance protocol to ensure that no Priority Inversion
 *  can occur.
 * 
 *  On the Linux test system, the required functionality is not available, therefore
 *  this implementation shall only serve as a basis to be extended if the system is
 *  ported onto a THR+(TPI/TPP) providing platform.
 * 
 *  Accordingly, the functionality of this piece of code could not be tested properly.
 */


class Mutex : public LockingObject
{

public:
	//! definition of all shared data required to access a mutex
	struct MutexData {
		oal_thread_mutex_t	    Mutex;
	};

	//! initializing constructor (see initialize())
	Mutex( Allocator* ThisAllocator, bool IsMaster = true, long Procotol = 0 /* = PTHREAD_PRIO_INHERIT */ );
	
	//! non-initializing constructor
	Mutex( );

	//! explicit initialization
	void initialize( Allocator* ThisAllocator, bool IsMaster = true, long Protocol = 0 /* = PTHREAD_PRIO_INHERIT */ );
	/*!< \param[in] ThisAllocator Reference to the allocator used to allocate memory for the EventData
	 *   \param[in] IsMaster      Flag indicating whether the calling thread shall be the master instance
	 *                            of the Event and set up all control structures required for synchronization
	 *                            via the POSIX shared condition.
	 *   \param[in] Protocol	  Priority protocol of the mutex. In order to avoid priority inversions,
	 *                            the default value PTHREAD_PRIO_INHERIT should not be changed in most cases.
	 */

	virtual ~Mutex();
	
	//! explicit deinitialization
	void deinitialize();


    virtual ASAAC_TimedReturnStatus lock( const ASAAC_Time& Timeout = TimeInfinity );
    virtual ASAAC_ReturnStatus release();

	
	//! return amount of memory required from allocator by control and data structures of this object
	static size_t predictSize();


private:
	// prevent implicit copying and assignment
	Mutex( const Mutex& Source );
	Mutex& operator=( const Mutex& Source );

	bool m_IsMaster;
	bool m_IsInitialized;
	Shared<MutexData> Global;
	
	
};



#endif /*MUTEX_HH_*/
