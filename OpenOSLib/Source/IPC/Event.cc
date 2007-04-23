
#include "ProcessManagement/ProcessManager.hh"

#include "Common/ObjectPool.hh"

#include "IPC/Event.hh"

#include "Exceptions/Exceptions.hh"

#include "Managers/TimeManager.hh"



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
		
			if ( oal_thread_condattr_init( &ConditionAttribute ) ) throw OSException( LOCATION );
			if ( oal_thread_mutexattr_init( &MutexAttribute ) )    throw OSException( LOCATION );

			// Condition shall be shared between processes
			if ( oal_thread_condattr_setpshared( &ConditionAttribute, PTHREAD_PROCESS_SHARED ) )
				throw OSException( LOCATION );

			if ( oal_thread_mutexattr_setpshared( &MutexAttribute, PTHREAD_PROCESS_SHARED ) )
				throw OSException( LOCATION );


			// Initialize Mutex and Condition
			if ( oal_thread_mutex_init(&(Global->Mutex), &MutexAttribute ) ) throw OSException( LOCATION );
		
			if ( oal_thread_cond_init(&(Global->Condition), &ConditionAttribute ) )
			{
				oal_thread_mutex_destroy(&(Global->Mutex));
				throw OSException( LOCATION );
			}
		
			// Events shall be reset originally
			Global->EventSet = false;
			
			Global->InitializedFlag = INITIALIZED_FLAG;
			
			Global->WaitingThreads = 0;

		}
	}
	catch ( ASAAC_Exception& E )
	{
		deinitialize();

		throw;
	}

}
	

Event::~Event()
{
	if ( m_IsInitialized ) deinitialize();
}


void Event::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
		// If this is the master object, destroy mutex and condition
	// deallocation of shared objects will be performed by Shareable<> destructor.
	if ( m_IsMaster )
	{
//		oal_thread_cond_destroy( &(Global->Condition) );
//		oal_thread_mutex_destroy( &(Global->Mutex) );
		Global->InitializedFlag = 0;
	}
	
	Global.deinitialize();
	
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


ASAAC_TimedReturnStatus Event::waitForEvent( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return waitForEventStatus( true, Timeout );
}



ASAAC_TimedReturnStatus Event::waitForEventReset( const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return waitForEventStatus( false, Timeout );
}



bool Event::isEventSet()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	bool bValue;
	
	if ( oal_thread_mutex_lock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	bValue = Global->EventSet;
	
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	return bValue;
}



unsigned int Event::getWaitingThreads()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned int iNumberOfThreads;
		
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
	if ( oal_thread_mutex_lock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	// Set Event flag
	Global->EventSet = Status;
	
	// Release exclusive access
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) ) throw OSException( LOCATION );

	// Broadcast change of Event flag to all waiting threads and processes
	if ( oal_thread_cond_broadcast(&(Global->Condition)) ) throw OSException( LOCATION );
}


ASAAC_TimedReturnStatus Event::waitForEventStatus( bool Status, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	timespec TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();

	if ( oal_thread_mutex_lock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	Global->WaitingThreads++;
	
	int iErrorCode = 0;
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_WAITING );
		
	while (( Global->EventSet != Status ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(Global->Condition), &(Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(Global->Condition), &(Global->Mutex), &TimeSpecTimeout );
	}
	
    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_RUNNING );	
	
	Global->WaitingThreads--;
	if ( oal_thread_mutex_unlock(&(Global->Mutex)) ) throw OSException( LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) 
	{
		return ASAAC_TM_TIMEOUT;
	}
	
	return ASAAC_TM_SUCCESS;
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



