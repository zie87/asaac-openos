#include "SharedMemory.hh"

#include "OpenOSObject.hh"

using namespace std;

#include "Exceptions/Exceptions.hh"
#include "Managers/FileManager.hh"
#include "FaultManagement/ErrorHandler.hh"


SharedMemory::SharedMemory() : 
		m_IsInitialized(false),
		m_Name(),
		m_IsMaster(false),
		m_FileHandle(0),
		m_BaseMemorySize(0),
		m_UsedMemory(0)
{
	m_BaseAddress.ptr = 0;
	m_HeaderAddress.ptr = 0;
	m_MemoryAddress.ptr = 0;
}

	
void SharedMemory::initialize( const ASAAC_CharacterSequence& Name, bool IsMaster, unsigned long Size, bool EraseMemory, bool EvaluateSession )
{
	if ( m_IsInitialized )
		throw OSException("Double Initialisation",LOCATION);
	
    m_IsInitialized = true;
	
	try
	{
        CharacterSequence ErrorString;

        m_IsMaster = IsMaster;
    	m_Name = Name;
    	
    	m_BaseAddress.ptr = 0;
    	m_HeaderAddress.ptr = 0;
    	m_MemoryAddress.ptr = 0;        

    	if ( Size == 0 )
    		throw OSException( "Trying to open zero size ShMO.", LOCATION );
 		
 		// If Master, create Shared Memory File
		if ( m_IsMaster )
			FileManager::getInstance()->createSharedMemory(m_Name, ASAAC_RW, Size + sizeof(MemoryHeader));
		
		// Open Shared Memory File
		const ASAAC_UseOption UseOption = {ASAAC_READWRITE, ASAAC_SHARE};
		FileManager::getInstance()->openSharedMemory( m_Name, UseOption, m_FileHandle );
		
        if (m_IsMaster == false)
        {
			//	Map header of Shared Memory Area to virtual memory
			FileManager::getInstance()->mapFile( m_FileHandle, sizeof(MemoryHeader), 0, m_BaseAddress.ptr ); 
			
			m_BaseMemorySize = sizeof(MemoryHeader);
			
			m_HeaderAddress = m_BaseAddress;

			if (getMemoryHeader()->MagicNumber != OS_MAGIC_NUMBER)
				throw FatalException("Magic number is not correct", LOCATION);
            
            if (EvaluateSession)
            {
                 if ( getMemoryHeader()->MemorySessionId != OpenOS::getInstance()->getSessionId() )
                    throw OSException("SessionId is not the current one", LOCATION);
            }
	
			if ( getMemoryHeader()->Size < Size )
 				throw FatalException( (ErrorString << "Size of of shared memory (" << getMemoryHeader()->Size << ") is smaller than requested size (" << Size << ")").c_str(), LOCATION);

			getMemoryHeader()->AllocationCounter ++;
				
			Size = getMemoryHeader()->Size;
			
			FileManager::getInstance()->unmapFile( m_HeaderAddress.ptr, sizeof(MemoryHeader) );
			
			
		}		
		
		// Map whole Shared Memory Area to virtual memory
		FileManager::getInstance()->mapFile( m_FileHandle, Size + sizeof(MemoryHeader), 0, m_BaseAddress.ptr ); 
	
		m_BaseMemorySize = Size + sizeof(MemoryHeader);
	
		m_HeaderAddress.number = m_BaseAddress.number;
		m_MemoryAddress.number = m_BaseAddress.number + sizeof(MemoryHeader);
		
		
		// Master Object sets counter to 1, slave objects increase by one.
		if ( m_IsMaster == true )
		{
			//set custom memory to zero
			if (  EraseMemory )
				for(unsigned long i = 0; i < Size; i++)
					m_MemoryAddress.ch[i] = 0;
					
			getMemoryHeader()->MagicNumber = OS_MAGIC_NUMBER;

			getMemoryHeader()->Size = Size;
			
			if (EvaluateSession)
            {
                if ( getMemoryHeader()->MemorySessionId != OpenOS::getInstance()->getSessionId() )
                {
				    getMemoryHeader()->MemorySessionId = OpenOS::getInstance()->getSessionId();
                    getMemoryHeader()->AllocationCounter = 1;
                }
                else 
                {
                    getMemoryHeader()->AllocationCounter ++;
                }
            }
            else
            {
                getMemoryHeader()->AllocationCounter = 1;
            }
		}
		
		m_UsedMemory = 0;

#ifdef DEBUG_SHM
		CharSeq LogString;
		LogString << "ShM inititialized: " << CharSeq(m_Name) << ", Size: " << getMemoryHeader()->Size 
			<< ", Counter: " << getMemoryHeader()->AllocationCounter; 
		OSException(LogString.c_str()).printErrorMessage();
#endif	
	}
	catch ( ASAAC_Exception &e )
	// If an exception is caught, make sure everything is cleaned
	// up before exiting.
	{
		CharSeq ErrorString;
		e.addPath( (ErrorString << "Error initializing SharedMemory object: " << m_Name << " [" << m_BaseMemorySize << "]").c_str(), LOCATION);

        deinitialize();
		
		throw;
	}
}
			

void SharedMemory::deinitialize()
{
	if (m_IsInitialized == false) 
		return;

	try
	{	
		if ( m_BaseAddress.ptr != 0 )
		{ 
			// Get allocation counter and decrease by one.
			if (getMemoryHeader()->AllocationCounter == 0 )
				throw FatalException("SharedMemory structures are currupted", LOCATION);

			if ( getMemoryHeader()->MemorySessionId != OpenOS::getInstance()->getSessionId() )
				throw OSException("SessionId is not the current one", LOCATION);

			getMemoryHeader()->AllocationCounter--;
			
			unsigned long AllocationCounter = getMemoryHeader()->AllocationCounter; 

#ifdef DEBUG_SHM
			CharSeq LogString;
			LogString << "ShM deinititialized: " << CharSeq(m_Name) << ", Size: " << getMemoryHeader()->Size 
				<< ", Counter: " << getMemoryHeader()->AllocationCounter;			
			OSException(LogString.c_str()).printErrorMessage();
#endif	
			
			// Unmap memory mapping of shared memory file.	
			FileManager::getInstance()->unmapFile( m_BaseAddress.ptr, m_BaseMemorySize );

            // Check if no more objects using the memory    
            if (AllocationCounter == 0 )
            {
                FileManager::getInstance()->deleteSharedMemory(m_Name, ASAAC_NORMAL, TimeIntervalInstant);
            }

            if (AllocationCounter > 0 )
            {
                FileManager::getInstance()->closeFile(m_FileHandle);
            }
        }
	}
	catch (ASAAC_Exception &e)
	{
		CharSeq ErrorString;
		e.addPath( (ErrorString << "Error deinitializing SharedMemory object: " << m_Name << " [" << m_BaseMemorySize << "]").c_str(), LOCATION);
		e.raiseError();
	}
	
	m_BaseAddress.ptr = 0;
	m_HeaderAddress.ptr = 0;
	m_MemoryAddress.ptr = 0;
	
	m_IsInitialized = false;
}



SharedMemory::~SharedMemory()
{
	deinitialize();
}


bool SharedMemory::isInitialized()
{
	return m_IsInitialized;
}


ASAAC_Address SharedMemory::allocate( long Size )
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	// Align in double-words. Otherwise, it seems that semaphores like to crash.
	long ActualSize = getDWordSize( Size );

	// If new allocation exceeds remaining allocatable memory, throw AllocationException
	if ( (unsigned long)(ActualSize + m_UsedMemory) > getMemoryHeader()->Size ) 
		throw OSException("SharedMemory is out of memory", LOCATION);
	
	// return address of next available memory slot
	// (the first unsigned long is used for allocation count)
	void* Pointer = static_cast<void*>(&m_MemoryAddress.ch[ m_UsedMemory ]); 
	
	// Update UsedMemory statistics
	m_UsedMemory += ActualSize;
		
	return Pointer;
}


void SharedMemory::free(ASAAC_Address AllocatedAddress)
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);
}


void SharedMemory::reset()
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	m_UsedMemory = 0;
}
	

unsigned long SharedMemory::getSize() const
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return getMemoryHeader()->Size;
}


unsigned long SharedMemory::getFreeMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return ( getMemoryHeader()->Size - m_UsedMemory );
}


unsigned long SharedMemory::getUsedMemory() const
{
	if (m_IsInitialized == false) 
		throw UninitializedObjectException(LOCATION);

	return m_UsedMemory;
}


SessionId SharedMemory::getSessionId()
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return getMemoryHeader()->MemorySessionId;
}


void SharedMemory::setSessionId(SessionId id)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	getMemoryHeader()->MemorySessionId = id;		
}


unsigned long SharedMemory::getAllocationCounter() const
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);

	return getMemoryHeader()->AllocationCounter;
}


void SharedMemory::setAllocationCounter(unsigned long value)
{
    if (m_IsInitialized == false) 
        throw UninitializedObjectException(LOCATION);
    
	getMemoryHeader()->AllocationCounter = value;
	
#ifdef DEBUG_SHM
	CharSeq LogString;
	LogString << "ShM setAllocationCounter: " << CharSeq(m_Name) << ", Size: " << getMemoryHeader()->Size 
		<< ", Counter: " << getMemoryHeader()->AllocationCounter;
	OSException(LogString.c_str()).printErrorMessage();
#endif	
}


SharedMemory::MemoryHeader *SharedMemory::getMemoryHeader() const
{
    return (MemoryHeader*)m_HeaderAddress.ptr;
}


ASAAC_CharacterSequence SharedMemory::getName()
{
	return m_Name;
}


