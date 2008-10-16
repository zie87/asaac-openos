#ifndef SPINLOCK_HH_
#define SPINLOCK_HH_

#include "OpenOSIncludes.hh"
#include "Common/Templates/Shared.hh"

#include "AbstractInterfaces/LockingObject.hh"

//! locking object using a POSIX spinlock
/*!< spinlocks feature some advantages over semaphores, most importantly
 *   that a spinlock is a lot easier in nature, and does not require a call
 *   longo the OS layer, thereby providing an advantage in execution speed of
 *   functions favouring spinlocks over semaphores.
 *   The waiting for a spinlock, however, is done by continuously polling
 *   of a memory location, at the cost of cpu time that is not spent when
 *   using a semaphore, which puts the thread to sleep until the semaphore
 *   is available.
 * 
 *   Hence, spinlocks offer great improvements to performance in situations
 *   where the lock has to be held only for short periods of time, so
 *   the worst-case-scenario of multiple threads waiting for a spinlock
 *   is rarely found.
 */


class SpinLock : public LockingObject
{
public:
    //! non-initializing constructor
    SpinLock();

	//! explicit initialization
    void initialize(Allocator* ThisAllocator,
    				const bool IsMaster = true );
    /*!< \param[in] ThisAllocator Reference to the Allocator used to
     *                            allocate memory for shared structures of the spinlock
 	 *   \param[in] IsMaster		Flag indicating whether the calling instance will act as master of the allocated
 	 *                              data and initialize all OS structures required for the spinlock to work.
 	 */
       
        					  
    virtual ~SpinLock();
    
    //! explicit deinitialization
    void deinitialize();
    
    virtual void lock( const ASAAC_Time& Timeout = TimeInfinity );
    virtual void release();


    //! get memory amount required from the Allocator by the spinlock's data and control structures
    static size_t predictSize();
    
    void* operator new( size_t size );
    void operator delete( void* location );


private:
	// prevent implicit assignment and copying
	SpinLock( const SpinLock& Value );
	SpinLock& operator= ( const SpinLock& Value );

	bool m_IsMaster;
	bool m_IsInitialized;
	
	Shared<oal_thread_spinlock_t>	SpinLockData;

	

};

#endif /*SPINLOCK_HH_*/
