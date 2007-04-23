
#include "IPC/EventTable.hh"

#include "Exceptions/Exceptions.hh"

#include "Common/ObjectPool.hh"

#include "ProcessManagement/ProcessManager.hh"



EventTable::EventTable( Allocator* ThisAllocator, bool IsMaster, unsigned int Size  ) : m_IsInitialized(false)
{
	initialize( ThisAllocator, IsMaster, Size  );
}


EventTable::EventTable() : m_IsInitialized(false)
{
}


void EventTable::initialize( Allocator* ThisAllocator, bool IsMaster, unsigned int Size )
{
	// Avoid double initialization of object.
	if ( m_IsInitialized ) throw DoubleInitializationException();
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
			for ( unsigned int i = 0; i < m_Global->Size; i++ )
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
	if ( m_IsInitialized ) deinitialize();
}


void EventTable::deinitialize()
{
	if ( m_IsInitialized == false ) return;
	
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
	

void EventTable::setEvent( unsigned int TableIndex )
{
	setEventStatus( TableIndex, true );
}


void EventTable::resetEvent( unsigned int TableIndex )
{
	setEventStatus( TableIndex, false );
}


ASAAC_TimedReturnStatus EventTable::waitForEvent( unsigned int TableIndex, const ASAAC_Time& Timeout )
{
	return waitForEventStatus( TableIndex, true, Timeout );
}


ASAAC_TimedReturnStatus EventTable::waitForEventReset( unsigned int TableIndex, const ASAAC_Time& Timeout )
{
	return waitForEventStatus( TableIndex, false, Timeout );
}


ASAAC_TimedReturnStatus EventTable::waitForEventStatus( unsigned int TableIndex, bool Status, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();

	timespec TimeSpecTimeout;

	TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();
		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );

	m_Global->WaitingThreads++;
	
	int iErrorCode = 0;

	if ( ThisThread != 0 ) 
		ThisThread->setState( ASAAC_WAITING );
	
	while (( m_EventTable[ TableIndex ] == Status ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(m_Global->Condition), &(m_Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(m_Global->Condition), &(m_Global->Mutex), &TimeSpecTimeout );
		
	}
	
	m_Global->WaitingThreads--;

    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) 
		ThisThread->setState( ASAAC_RUNNING );
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) 
		return ASAAC_TM_TIMEOUT;
	
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus EventTable::waitForMultipleEvents( unsigned int TableSize, unsigned int* Table, unsigned int MinEvents, const ASAAC_Time& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();

	timespec TimeSpecTimeout;

	TimeSpecTimeout.tv_sec  = Timeout.sec;
	TimeSpecTimeout.tv_nsec = Timeout.nsec;

		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) throw OSException( LOCATION );

	m_Global->WaitingThreads++;
	
	int iErrorCode = 0;
	
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_WAITING );
	
	unsigned int iEventsFound = 0;
	
	while (( iEventsFound < MinEvents ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(m_Global->Condition), &(m_Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(m_Global->Condition), &(m_Global->Mutex), &TimeSpecTimeout );
		
		for ( unsigned int i = 0; i < TableSize; i ++ )
		{
			unsigned int EventIndex = Table[ i ];
			
			if ( m_EventTable[ EventIndex ] )
			{
				iEventsFound ++;
			}
		}
	}
	
	m_Global->WaitingThreads--;

    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_RUNNING );
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) throw OSException( LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) return ASAAC_TM_TIMEOUT;
	
	return ASAAC_TM_SUCCESS;
}


unsigned int EventTable::getWaitCount()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned int iNumberOfThreads;
		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) throw OSException( LOCATION );
	
	iNumberOfThreads = m_Global->WaitingThreads;
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) throw OSException( LOCATION );
	
	return iNumberOfThreads;
}	


void EventTable::setEventStatus( unsigned int TableIndex, bool Status )
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



size_t EventTable::predictSize( unsigned int TableSize )
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

