#ifndef SHAREDMEMORYALLOCATOR_HH_
#define SHAREDMEMORYALLOCATOR_HH_

#include "OpenOSIncludes.hh"

#include "Allocator/Allocator.hh"


//! Allocator for memory located in a POSIX shared memory area

/*! The SharedMemory Allocator provides access to data structures contained
 *  within POSIX shared memory objects. Such a shared memory object must be
 * under the control of one master instance, which initializes the POSIX resources
 * and assigns the size to the allocated area. Subsequent connects from "slave" instances
 * do not require an explicit declaration of the file size, they can determine the size of
 * the memory area to be allocated via POSIX facilities.
 * 
 * A shared memory object provided by this allocator contains a connection counter
 * to be able to determine the number of connected instances, for debugging purposes.
 * 
 * If the master instance of a POSIX shared memory object disconnects from the object,
 * the object itself remains active until the last slave instance has closed its connection
 * to the provided memory area.
 * 
 * File handles of shared memory objects shall be obtained from the FileManager, in order
 * to be able to provide security mechanisms and forbid non-privileged applications from
 * opening shared memory objects that they shall not have access to.
 * 
 * The POSIX functionality required for this allocator is specified in the optional module SHM
 * of the POSIX standard IEEE 1003.1, 2004 Edition.
 */

class AllocatorManager;

class SharedMemory : public Allocator
{
	
public:
	SharedMemory( const ASAAC_CharacterSequence& Name, bool IsMaster, unsigned long Size );
	//!< instantiate allocator and initialize it instantly (see initialize()).

	SharedMemory();
	
	void initialize( const ASAAC_CharacterSequence& Name, bool IsMaster, unsigned long Size, bool EraseMemory = true, bool EvaluateSession = true );
	//!< initialize shared memory allocator
	/*!< \param[in] Name           File name of the POSIX shared memory object to connect to
	 *   \param[in] IsMaster       Flag to indicate whether this instance shall be the master instance of the
	 *                             shared memory object.
	 *   \param[in] Size           Size of the memory area to be allocated (mandatory only for the master instance)
	 *   \param[in] EraseMemory    Memory shall be erased/initialized to zero
	 *   \param[in] EvaluatSession check, if session of object is valid. If not an exception is thrown
	 * 
	 *   If this instance of the SharedMemory Allocator is already initialized, the function will
	 *   throw a DoubleInitializationException.
	 *   If the designated shared memory object cannot be opened, due to lack of privileges or
	 *   other POSIX-related errors, a ResourceException will be thrown. 
	 *   In case of an error during the memory mapping of the shared data, an OSException will be thrown.
	 */
	
	void deinitialize( );
	//!< deinitialize shared memory allocator
	/*!< unmap the shared memory object from memory and reset longernal data structures of the
	 *   object. Note that the actual POSIX shared memory object will not be closed, so it can
	 *   still be re-opened at later times, even if the application does no longer hold
	 *   privileges to open other shared memory objects.
	 * 
	 *   In case of an error during the unmapping, an OSException is thrown.
	 */

	virtual ~SharedMemory();
	
	bool isInitialized();
	
    virtual ASAAC_Address allocate(long Size);
    virtual void free(ASAAC_Address AllocatedAddress);
    
    virtual void reset();
    
    virtual unsigned long getSize() const;
    virtual unsigned long getFreeMemory() const;
    virtual unsigned long getUsedMemory() const;

	virtual unsigned long getAllocationCounter() const;
    
    SessionId getSessionId();
    
    ASAAC_CharacterSequence getName();
    
protected:
	friend class OpenOS; 
	friend class AllocatorManager;
	
    typedef struct {
    	unsigned long MagicNumber;
        unsigned long Size;
        unsigned long AllocationCounter;
        SessionId      	MemorySessionId;
    } MemoryHeader;

    union SharedAddress {
        void *        ptr;
        char *        ch;
        unsigned long number;
    };

    MemoryHeader *getMemoryHeader() const;    
    void setSessionId(SessionId id);
	void setAllocationCounter(unsigned long id);
    
	bool 					m_IsInitialized;

	SharedAddress			m_BaseAddress;   
	SharedAddress			m_HeaderAddress; 
	SharedAddress			m_MemoryAddress; 
	
	ASAAC_CharacterSequence m_Name;
	
	bool 					m_IsMaster;
	long 					m_FileHandle;
	unsigned long 			m_BaseMemorySize;
	unsigned long 			m_UsedMemory;
	
private:
	// no implicit assignment and copying
	SharedMemory( const SharedMemory& Source );
	SharedMemory& operator= ( const SharedMemory& Source );
	
};



#endif /*SHAREDMEMORYALLOCATOR_HH_*/
