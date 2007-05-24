#include "Mutex.hh"
#include "ProcessManagement/ProcessManager.hh"

Mutex::Mutex( Allocator* ThisAllocator, bool IsMaster, long Protocol ) : m_IsInitialized(false)
{
	initialize( ThisAllocator, IsMaster, Protocol );
}


Mutex::Mutex() : m_IsInitialized(false)
{
}


void Mutex::initialize( Allocator* ThisAllocator, bool IsMaster, long Protocol )
{
	// Avoid double initialization of object.
	if ( m_IsInitialized ) throw DoubleInitializationException(LOCATION);
	m_IsInitialized = true;

	m_IsMaster = IsMaster;	

	try
	{	
		// Initialize shared data structure
		Global.initialize( ThisAllocator );
	
		// If this is the master object, perform Mutex initialization, too
		if ( m_IsMaster )
		{
			oal_thread_mutexattr_t MutexAttribute;
		
			if ( oal_thread_mutexattr_init( &MutexAttribute ) )    throw OSException( LOCATION );

			// Mutex shall be shareable between processes
			if ( oal_thread_mutexattr_setpshared( &MutexAttribute, PTHREAD_PROCESS_SHARED ) )
				throw OSException( LOCATION );
				
//          NOTE: uncomment this line once on a TPI/TPP compatible system				
//			if ( oal_thread_mutexattr_setprotocol( &MutexAttribute, Protocol ) )
//				throw OSException( LOCATION );


			// Initialize Mutex 
			if ( oal_thread_mutex_init(&(Global->Mutex), &MutexAttribute ) ) throw OSException( LOCATION );
		}
	}
	catch ( ASAAC_Exception& E )
	{
		deinitialize();

		throw;
	}

}
	

Mutex::~Mutex()
{
	if ( m_IsInitialized ) deinitialize();
}


void Mutex::deinitialize()
{
	if ( m_IsInitialized == false ) return;
	
	// If this is the master object, destroy mutex and condition
	// deallocation of shared objects will be performed by Shareable<> destructor.
	if ( m_IsMaster )
	{
		oal_thread_mutex_destroy( &(Global->Mutex) );
	}
	
	Global.deinitialize();
	
	m_IsInitialized = false;
}
	

ASAAC_TimedReturnStatus Mutex::lock( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) throw OSException( LOCATION );
	
	
	timespec TimeSpecTimeout; // timespec in guaranteed alignment

	TimeSpecTimeout.tv_sec  = Timeout.sec;
	TimeSpecTimeout.tv_nsec = Timeout.nsec;
	
	
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_WAITING );
		
	long iError = oal_thread_mutex_timedlock( &( Global->Mutex ), &TimeSpecTimeout );

    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_RUNNING );
	
	if ( iError == 0 ) return ASAAC_TM_SUCCESS;
	if ( iError == ETIMEDOUT ) return ASAAC_TM_TIMEOUT;
	return ASAAC_TM_ERROR;
}


ASAAC_ReturnStatus Mutex::release()
{
	if ( m_IsInitialized == false ) throw OSException( LOCATION );
	
	long iError = oal_thread_mutex_unlock( &(Global->Mutex) );
	
	if ( iError == 0 ) return ASAAC_SUCCESS;
	
	return ASAAC_ERROR;
}
