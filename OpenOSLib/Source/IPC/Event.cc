#include "Common/Templates/ObjectPool.hh"

#include "IPC/Event.hh"
#include "IPC/BlockingScope.hh"

#include "Exceptions/Exceptions.hh"



Event::Event( Allocator* ThisAllocator, bool IsMaster ) : m_IsInitialized(false)
{
	initialize( ThisAllocator, IsMaster );
}


Event::Event() : m_IsInitialized(false)
{
}


void Event::initialize( Allocator* ThisAllocator, bool IsMaster )
{
	// Avoid double initialization of object.
	if ( m_IsInitialized ) 
		throw DoubleInitializationException( LOCATION );
		
	m_IsInitialized = true;

	m_IsMaster = IsMaster;	

	try
	{	
		// Initialize shared data structure
		Global.initialize( ThisAllocator );
	
		// If this is the master object, perform Mutex/Shared Condition initialization, too
		if ( m_IsMaster )
		{
			oal_thread_condattr_t& ConditionAttribute = Global->ConditionAttribute;
			oal_thread_mutexattr_t MutexAttribute;
		
			if ( oal_thread_condattr_init( &ConditionAttribute ) ) 
				throw OSException( strerror(errno), LOCATION );

			if ( oal_thread_mutexattr_init( &MutexAttribute ) )    
				throw OSException( strerror(errno), LOCATION );

			// Condition shall be shared between processes
			if ( oal_thread_condattr_setpshared( &ConditionAttribute, PTHREAD_PROCESS_SHARED ) )
				throw OSException( strerror(errno), LOCATION );

			if ( oal_thread_mutexattr_setpshared( &MutexAttribute, PTHREAD_PROCESS_SHARED ) )
				throw OSException( strerror(errno), LOCATION );


			// Initialize Mutex and Condition
			if ( oal_thread_mutex_init(&(Global->Mutex), &MutexAttribute ) ) 
				throw OSException( strerror(errno), LOCATION );
		
			if ( oal_thread_cond_init(&(Global->Condition), &ConditionAttribute ) )
			{
				oal_thread_mutex_destroy(&(Global->Mutex));
				throw OSException( strerror(errno), LOCATION );
			}
		
			// Events shall be reset originally
			Global->EventSet = false;
			
			Global->InitializedFlag = INITIALIZED_FLAG;
			
			Global->WaitingThreads = 0;

		}
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error deinitializing Event", LOCATION);
		
		deinitialize();

		throw;
	}

}
	

Event::~Event()
{
}


void Event::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	try
	{
		// If this is the master object, destroy mutex and condition
		// deallocation of shared objects will be performed by Shareable<> destructor.
		if ( m_IsMaster )
		{
	//		oal_thread_cond_destroy( &(Global->Condition) );
	//		oal_thread_mutex_destroy( &(Global->Mutex) );
			Global->InitializedFlag = 0;
		}
		
		Global.deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing Event", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
}
	

void Event::setEvent()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	setEventStatus( true );
}


void Event::resetEvent()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	setEventStatus( false );
}


void Event::waitForEvent( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	waitForEventStatus( true, Timeout );
}



void Event::waitForEventReset( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	waitForEventStatus( false, Timeout );
}



bool Event::isEventSet()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	bool bValue;
	
	if ( oal_thread_mutex_lock(&(Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	bValue = Global->EventSet;
	
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	return bValue;
}



unsigned long Event::getWaitingThreads()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned long iNumberOfThreads;
		
	if ( oal_thread_mutex_lock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	iNumberOfThreads = Global->WaitingThreads;
	
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	return iNumberOfThreads;
}	


void Event::setEventStatus( bool Status )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	// Get exclusive access
	if ( oal_thread_mutex_lock(&(Global->Mutex)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );
	
	// Set Event flag
	Global->EventSet = Status;
	
	// Release exclusive access
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );

	// Broadcast change of Event flag to all waiting threads and processes
	if ( oal_thread_cond_broadcast(&(Global->Condition)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );
}


void Event::waitForEventStatus( bool Status, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	BlockingScope TimeoutScope();
	
	timespec TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();

	if ( oal_thread_mutex_lock(&(Global->Mutex)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );
	
	Global->WaitingThreads++;
	
	long iErrorCode = 0;
	
	while (( Global->EventSet != Status ) && ( iErrorCode != ETIMEDOUT ))
	{
		if ( TimeStamp(Timeout).isInfinity() )
			iErrorCode = oal_thread_cond_wait(&(Global->Condition), &(Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(Global->Condition), &(Global->Mutex), &TimeSpecTimeout );
	}
	
	Global->WaitingThreads--;
	
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) 
		throw TimeoutException(LOCATION);
}


size_t Event::predictSize()
{
	return Shared<EventData>::predictSize();
}


static ObjectPool<Event,OS_MAX_NUMBER_OF_EVENTS> EventPool;

void* Event::operator new( size_t size )
{
	return (void*)EventPool.acquireObject( TimeIntervalInstant );
}

void Event::operator delete( void* location )
{
	EventPool.releaseObject( (Event*) location );
}



