#include "IPC/EventTable.hh"
#include "IPC/BlockingScope.hh"

#include "Exceptions/Exceptions.hh"

#include "Common/Templates/ObjectPool.hh"



EventTable::EventTable( Allocator* ThisAllocator, bool IsMaster, unsigned long Size  ) : m_IsInitialized(false)
{
	initialize( ThisAllocator, IsMaster, Size  );
}


EventTable::EventTable() : m_IsInitialized(false)
{
}


void EventTable::initialize( Allocator* ThisAllocator, bool IsMaster, unsigned long Size )
{
	// Avoid double initialization of object.
	if ( m_IsInitialized ) 
		throw DoubleInitializationException();

	m_IsInitialized = true;

	try
	{
		m_IsMaster = IsMaster;	
	
		// Initialize shared data structure
		m_Global.initialize( ThisAllocator );
	
		// If this is the master object, perform Mutex/Shared Condition initialization, too
		if ( m_IsMaster )
		{
			if ( oal_thread_condattr_init( &(m_Global->ConditionAttribute) ) ) throw OSException( LOCATION );

			// Condition shall be shared between processes
			if ( oal_thread_condattr_setpshared( &(m_Global->ConditionAttribute), PTHREAD_PROCESS_SHARED ) )
				throw OSException( LOCATION );

			// Initialize Mutex and Condition
			if ( oal_thread_mutex_init(&(m_Global->Mutex), NULL ) ) throw OSException( LOCATION );
		
			if ( oal_thread_cond_init(&(m_Global->Condition), &(m_Global->ConditionAttribute) ) ) 
			{
				oal_thread_mutex_destroy(&(m_Global->Mutex));
				throw OSException( LOCATION );
			}

			m_Global->Size           = Size;
			m_Global->WaitingThreads = 0;		
		}

		m_EventTable.initialize( ThisAllocator, m_Global->Size );
	
		if ( m_IsMaster )
		{
			for ( unsigned long i = 0; i < m_Global->Size; i++ )
			{
				m_EventTable[ i ] = 0;
			}
		}
	}
	catch ( ASAAC_Exception& E )
	{
		deinitialize();
		
		throw;
	}

}
	

EventTable::~EventTable()
{
}


void EventTable::deinitialize()
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
		}
		
		m_Global.deinitialize();
		m_EventTable.deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing EventTable", LOCATION);
		e.raiseError();
	}

	m_IsInitialized = false;
}
	

void EventTable::setEvent( unsigned long TableIndex )
{
	setEventStatus( TableIndex, true );
}


void EventTable::resetEvent( unsigned long TableIndex )
{
	setEventStatus( TableIndex, false );
}


void EventTable::waitForEvent( unsigned long TableIndex, const ASAAC_Time& Timeout )
{
	waitForEventStatus( TableIndex, true, Timeout );
}


void EventTable::waitForEventReset( unsigned long TableIndex, const ASAAC_Time& Timeout )
{
	waitForEventStatus( TableIndex, false, Timeout );
}


void EventTable::waitForEventStatus( unsigned long TableIndex, bool Status, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	BlockingScope TimeoutScope();
	
	timespec TimeSpecTimeout;

	TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();
		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );

	m_Global->WaitingThreads++;
	
	long iErrorCode = 0;

	while (( m_EventTable[ TableIndex ] == Status ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(m_Global->Condition), &(m_Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(m_Global->Condition), &(m_Global->Mutex), &TimeSpecTimeout );
	}
	
	m_Global->WaitingThreads--;

	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) != 0 ) 
		throw OSException( strerror(errno), LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) 
		throw TimeoutException(LOCATION);
}


void EventTable::waitForMultipleEvents( unsigned long TableSize, unsigned long* Table, unsigned long MinEvents, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	BlockingScope TimeoutScope();

	timespec TimeSpecTimeout;

	TimeSpecTimeout.tv_sec  = Timeout.sec;
	TimeSpecTimeout.tv_nsec = Timeout.nsec;

	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );

	m_Global->WaitingThreads++;
	
	long iErrorCode = 0;
	
	unsigned long iEventsFound = 0;
	
	while (( iEventsFound < MinEvents ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(m_Global->Condition), &(m_Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(m_Global->Condition), &(m_Global->Mutex), &TimeSpecTimeout );
		
		for ( unsigned long i = 0; i < TableSize; i ++ )
		{
			unsigned long EventIndex = Table[ i ];
			
			if ( m_EventTable[ EventIndex ] )
			{
				iEventsFound ++;
			}
		}
	}
	
	m_Global->WaitingThreads--;

	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	if ( iErrorCode == ETIMEDOUT )
		throw TimeoutException(LOCATION);
}


unsigned long EventTable::getWaitCount()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned long iNumberOfThreads;
		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) throw OSException( LOCATION );
	
	iNumberOfThreads = m_Global->WaitingThreads;
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) throw OSException( LOCATION );
	
	return iNumberOfThreads;
}	


void EventTable::setEventStatus( unsigned long TableIndex, bool Status )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	// Get exclusive access
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) throw OSException( LOCATION );
	
	m_EventTable[ TableIndex ] = Status;
	
	// Release exclusive access
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) throw OSException( LOCATION );

	// Broadcast change of Event flag to all waiting threads and processes
	if ( oal_thread_cond_broadcast(&(m_Global->Condition)) ) throw OSException( LOCATION );
}



size_t EventTable::predictSize( unsigned long TableSize )
{
	return ( Shared<EventData>::predictSize() +
			 Shared<bool>::predictSize( TableSize ) );
}


static ObjectPool<EventTable,OS_MAX_NUMBER_OF_EVENTTABLES> EventTablePool;

void* EventTable::operator new( size_t size )
{
	return (void*)EventTablePool.acquireObject( TimeIntervalInstant );
}

void EventTable::operator delete( void* location )
{
	EventTablePool.releaseObject( (EventTable*) location );
}

