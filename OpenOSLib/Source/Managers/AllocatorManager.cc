#include "AllocatorManager.hh"

#include "Allocator/SharedMemory.hh"

#include "OpenOSObject.hh"


AllocatorManager::AllocatorManager()
{
	m_AllocatorCounter = 0;
}


AllocatorManager::~AllocatorManager()	
{
}


AllocatorManager *AllocatorManager::getInstance()
{
	static AllocatorManager Instance;
	
	return &Instance;
}


long AllocatorManager::findAllocator( Allocator *Object )
{
	for (long Index = 0; Index < m_AllocatorCounter; Index++)
	{
		if (m_Allocator[Index] == Object)
			return Index;
	}
	
	return -1;
}


void AllocatorManager::registerAllocator( Allocator *Object )
{
	if (m_AllocatorCounter == OS_MAX_NUMBER_OF_ALLOCATOR)
		throw FatalException("Maximum number of allocators reached", LOCATION);
	
	if ( findAllocator( Object ) != -1 )
		throw OSException("Allocator object already registered", LOCATION);
		 
	m_Allocator[m_AllocatorCounter] = Object;
	m_AllocatorCounter++;
}


void AllocatorManager::unregisterAllocator( Allocator *Object )
{
	long Index = findAllocator( Object );
	
	if ( Index == -1)
		throw OSException("Allocator object not found", LOCATION);
	
	for (; Index < (m_AllocatorCounter-1); Index++)
	{
		m_Allocator[Index] = m_Allocator[Index+1]; 
	}
	
	m_AllocatorCounter--;
}


void AllocatorManager::deallocateAllObjects()
{
	unsigned short Index;

	for (Index = 0; Index < m_AllocatorCounter; Index++)
	{
		SharedMemory * ShM = dynamic_cast<SharedMemory*>(m_Allocator[Index]);
		
		if (ShM != NULL) 
		{
			if ( ShM->isInitialized() )	
			{
				//dekrement AllocationCounter to global indicate allocation of object
				ShM->setAllocationCounter( ShM->getAllocationCounter() - 1 );
			}
		}
	}
}


void AllocatorManager::reallocateAllObjects()
{
	reallocateAllObjects( OpenOS::getInstance()->getSessionId() );
}


void AllocatorManager::reallocateAllObjects( SessionId NewSessionId )
{
	unsigned short Index;

	for (Index = 0; Index < m_AllocatorCounter; Index++)
	{
		SharedMemory * ShM = dynamic_cast<SharedMemory*>(m_Allocator[Index]);
		
		if (ShM != NULL) 
		{
			if ( ShM->isInitialized() )	
			{
#ifdef DEBUG_SHM
				CharSeq LogString;
				LogString << "ShM reallocateAllObjects: NewSessionId: " << NewSessionId << ", OldSessionId: " << ShM->getSessionId();
				OSException(LogString.c_str()).printErrorMessage();
#endif	
				if (ShM->getSessionId() == NewSessionId)
				{
					//Inkrement AllocationCounter to global indicate allocation of object
					ShM->setAllocationCounter( ShM->getAllocationCounter() + 1 );
				}
				else
				{
					//For this session this is the first allocation
					ShM->setAllocationCounter( 1 );
					ShM->setSessionId(NewSessionId);
				}
			}
		}
	}
}

