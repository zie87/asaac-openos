#ifndef ALLOCATORMANAGER_HH_
#define ALLOCATORMANAGER_HH_

#include "OpenOSIncludes.hh"
#include "Allocator/Allocator.hh"
#include "Allocator/SharedMemory.hh"

class AllocatorManager
{
	public:
		~AllocatorManager();	
		static AllocatorManager *getInstance();
		
		void registerAllocator( Allocator *Object );
		void unregisterAllocator( Allocator *Object );
		
		void reallocateAllObjects( SessionId NewSessionId );
	private:
		AllocatorManager();
		
		long findAllocator( Allocator *Object );
		
		Allocator *m_Allocator[OS_MAX_NUMBER_OF_ALLOCATOR];
		unsigned short m_AllocatorCounter;
};

#endif /*ALLOCATORMANAGER_HH_*/
