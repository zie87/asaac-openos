#include "Allocator.hh"

#include "Managers/AllocatorManager.hh"

unsigned long getDWordSize( unsigned long Size )
{
	return ( Size + (( 4 - ( Size % 4 ) ) % 4 ) );
}

Allocator::Allocator()
{
	AllocatorManager::getInstance()->registerAllocator(this);
}

Allocator::~Allocator()
{
	AllocatorManager::getInstance()->unregisterAllocator(this);	
}
