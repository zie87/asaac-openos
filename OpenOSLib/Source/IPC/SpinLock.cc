#include "SpinLock.hh"


#include "Exceptions/Exceptions.hh"
#include "IPC/BlockingScope.hh"
#include "Common/Templates/ObjectPool.hh"

SpinLock::SpinLock() : m_IsInitialized(false)
{
}


void SpinLock::initialize(Allocator* ThisAllocator,
    					  const bool IsMaster )
{
	if ( m_IsInitialized ) 
		throw DoubleInitializationException();
	
	m_IsInitialized = true;

	try
	{
		SpinLockData.initialize( ThisAllocator );
		
		m_IsMaster = IsMaster;
		
		if ( IsMaster )
		{
			if ( oal_thread_spin_init( SpinLockData.getLocation(), PTHREAD_PROCESS_SHARED ) != 0 ) 
				throw OSException( LOCATION );
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing SpinLock", LOCATION);
		
		deinitialize();
		
		e.raiseError();
	}	
}
		
        					  
SpinLock::~SpinLock()
{
}
    
 
void SpinLock::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;

	m_IsInitialized = false;

	try
	{	
		if ( m_IsMaster )
		{
			if ( oal_thread_spin_destroy( SpinLockData.getLocation() ) != 0 ) 
				throw OSException( LOCATION );
		}
		
		SpinLockData.deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing SpinLock", LOCATION);
		e.raiseError();
	}
}


void SpinLock::lock( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
   	BlockingScope TimeoutScope();
		
	long iError = oal_thread_spin_lock( SpinLockData.getLocation() );

	if ( iError != 0 ) 
        throw OSException( strerror(errno), LOCATION );
}


void SpinLock::release()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	long iError = oal_thread_spin_unlock( SpinLockData.getLocation() );
	
	if ( iError != 0 ) 
        throw OSException( strerror(errno), LOCATION );
}
	

size_t SpinLock::predictSize()
{
	return Shared<oal_thread_spinlock_t>::predictSize();
}


static ObjectPool<SpinLock,OS_MAX_NUMBER_OF_SPINLOCKS> SpinLockPool;

void* SpinLock::operator new( size_t size )
{
	return (void*)SpinLockPool.acquireObject( TimeIntervalInstant );
}	


void SpinLock::operator delete( void* location )
{
	SpinLockPool.releaseObject((SpinLock*)location);
}


