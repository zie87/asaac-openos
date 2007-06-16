//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Semaphore.hh
//  @ Date : 12.04.2006
//  @ Author : 
//
//


#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

#include "Common/Templates/Shared.hh"

#include "OpenOSIncludes.hh"


#include "AbstractInterfaces/LockingObject.hh"


//! locking Object using a POSIX semaphore
/*!< a Semaphore can be used to either provide the APOS Semaphore functionality specified
 *   by STANAG 4626, Part II, Section 11.4.3, but it can also be used in more general applications
 *   within the implementation of the OS core, providing longra- and longerprocess locking mechanisms
 *   used for communications and thread-safe subsystems.
 * 
 *   The implementation uses the mechanisms of the optional POSIX module SEM (IEEE 1003.1, 2004 Edition).
 */

class Semaphore : public LockingObject
{
    
public:
	struct SemaphoreData {
		oal_sem_t	 Semaphore;
		int			 MaximumCount;
		unsigned long WaitingThreads;
	};


	//! initializing constructor (see initialize())
    Semaphore(Allocator* ThisAllocator,
    		  const bool IsMaster = true,
    		  const long InitialCount = 1,
    		  const long MaximumCount = -1 );
    					  
    //! non-initializing constructor
    Semaphore();


	//! explicit initialization
    void initialize(Allocator* ThisAllocator,
    					    const bool IsMaster = true,
    					    const long InitialCount = 1,
    					    const long MaximumCount = -1 );
 	/*!< \param[in] ThisAllocator	Reference to Allocator used to allocate the shared structures of the Semaphore
 	 *   \param[in] IsMaster		Flag indicating whether the calling instance will act as master of the allocated
 	 *                              data and initialize all OS structures required for the semaphore to work.
 	 *   \param[in] InitialCount    Initial Value of the Semaphore (only applicable for IsMaster = true)
 	 *   \param[in] MaximumCount    Maximum Value of the Semaphore (only applicable for IsMaster = true)
 	 * 
 	 */
        					  
    virtual ~Semaphore();
    
    void deinitialize();
    
    //! wait at semaphore until semaphore is available
    ASAAC_TimedReturnStatus wait( const ASAAC_Time& Timeout = TimeInfinity );
    /*!< transfer the calling thread longo WAITING state until the counter of the semaphore is positive, or
     *   until the Timeout has elapsed. If the counter is found to be positive, decrease it by one and resume
     *   RUNNING state with the calling thread.
     * 
     * \param[in]	Timeout		Maximum time to wait for the semaphore counter to be positive
     * 
     * \returns ASAAC_TM_SUCCESS if the semaphore was available and could be locked.
     *          ASAAC_TM_TIMEOUT if the timeout elapsed.
     *          ASAAC_TM_ERROR if an error occured during the operation
     */
    
    //! release the semaphore
    ASAAC_ReturnStatus      post();
    /*!< increase the semaphore counter by one and return.
     * 
     * \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR if an error occurred.
     */

	//! get current value of the semaphore counter
    unsigned long getCount() const;
    
    //! get number of threads currently waiting for the semaphore to become available
    unsigned long getWaitingThreads() const;
    
    //! get memory amount required from the Allocator by the semaphore's data and control structures
    static size_t predictSize();

    //! wrapper function for lock() for the LockingObject longerface
    virtual ASAAC_TimedReturnStatus lock( const ASAAC_Time& Timeout = TimeInfinity );
    
    //! wrapper function for release() for the LockingObject longerface
    virtual ASAAC_ReturnStatus release();

	//! override for the new operator, to obtain memory-static behaviour
	void* operator new( size_t size );
	/*!< rather than using the standard functions for new and delete,
	 * the current implementation of Semaphore maps these to calls to an ObjectPool that
	 * holds a number (OS_MAX_NUMBER_OF_SEMAPHORES) of Semaphore objects for dynamic allocation 
	 * and de-allocation.
	 */

	//! override for the delete operator, to obtain memory-static behaviour
	void  operator delete( void* location );
	/*!< rather than using the standard functions for new and delete,
	 * the current implementation of Semaphore maps these to calls to an ObjectPool that
	 * holds a number (OS_MAX_NUMBER_OF_SEMAPHORES) of Semaphore objects for dynamic allocation 
	 * and de-allocation.
	 */

private:
	// prevent implicit assignment and copying
	Semaphore( const Semaphore& Value );
	Semaphore& operator= ( const Semaphore& Value );

	bool 					m_IsMaster;
	bool 					m_IsInitialized;
	
	Shared<SemaphoreData>	m_Global;

};

#endif  //_SEMAPHORE_HH
