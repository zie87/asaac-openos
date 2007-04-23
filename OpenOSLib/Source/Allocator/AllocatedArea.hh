#ifndef ALLOCATEDAREA_HH_
#define ALLOCATEDAREA_HH_

#include "OpenOSIncludes.hh"

#include "Allocator/Allocator.hh"


//! Allocator for a memory area located inside another Allocator

/*! An AllocatedArea can be used to reserve certain areas inside
 *  another Allocator for later allocation/use. It allocates a memory
 *  chunk of the given size inside its parent allocator and takes
 *  control of the allocation/deallocation of the former itself.
 */

class AllocatedArea : public Allocator
{
	
public:
	AllocatedArea( Allocator* ParentAllocator, unsigned int Size );
	//!< instantiate AllocatedArea and initialize instantly (see initialize()).
	
	AllocatedArea();

	virtual ~AllocatedArea();

	void initialize( Allocator* ParentAllocator, unsigned int Size );
	//!< initialize AllocatedArea
	/*!< \param[in] ParentAllocator Allocator to use for the allocation of the memory area governed by this
	 *                              AllocatedArea.
	 *   \param[in] Size            Size of the memory area to allocate and reserve for later use.
	 * 
	 *   if parent allocator did not contain enough space to allocate the memory required for this AllocatedArea,
	 *   an AllocationException is thrown.
	 */
	 
	void deinitialize();

	// Allocator Interface	
	void reset();
	
    virtual ASAAC_Address allocate(int Size);
    virtual void free(ASAAC_Address AllocatedAddress);
    
    virtual unsigned int getSize() const;
    virtual unsigned int getFreeMemory() const;
    virtual unsigned int getUsedMemory() const;
	
	virtual unsigned long getAllocationCounter() const;

protected:
	bool		   m_IsInitialized;
	
	char* 		   m_BaseAddress; // Type char is used to faciliate bytewise increases easily

	int 		   m_MemorySize;
	int			   m_UsedMemory;
	Allocator* 	   m_ParentAllocator;
	
private:
	// no implicit assignment and copying
	AllocatedArea( const AllocatedArea& Source );
	AllocatedArea& operator= ( const AllocatedArea& Source );
	
};

#endif /*ALLOCATEDAREA_HH_*/
