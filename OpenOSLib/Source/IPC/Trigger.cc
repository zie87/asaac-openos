#include "IPC/Trigger.hh"

#include "Exceptions/Exceptions.hh"
#include "Common/Templates/ObjectPool.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "IPC/BlockingScope.hh"


Trigger::Trigger() : m_IsInitialized(false)
{
}


void Trigger::initialize( Allocator* ParentAllocator, bool IsMaster )
{
	// Avoid double initialization of object.
	if ( m_IsInitialized ) 
		throw DoubleInitializationException();

	try
	{
		m_IsInitialized = true;

		m_IsMaster = IsMaster;
		
		m_Allocator.initialize( ParentAllocator, predictSize() );	
		
		// Initialize shared data structure
		m_Global.initialize( &m_Allocator );
		
		// If this is the master object, perform Mutex/Shared Condition initialization, too
		if ( m_IsMaster )
		{
			oal_thread_condattr_t& ConditionAttribute = m_Global->ConditionAttribute;
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
			if ( oal_thread_mutex_init(&(m_Global->Mutex), &MutexAttribute ) ) 
				throw OSException( strerror(errno), LOCATION );
			
			if ( oal_thread_cond_init(&(m_Global->Condition), &ConditionAttribute ) )
				throw OSException( strerror(errno), LOCATION );
			
			// Events shall be reset originally
			m_Global->TriggerValue = 0;
		}
	
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error initializing Trigger", LOCATION);

		deinitialize();
		
		throw;
	}
}
	

Trigger::~Trigger()
{
}


void Trigger::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;

	try
	{
		m_IsInitialized = false;
		
		// If this is the only initialized object, destroy mutex and condition
		if ( m_Allocator.getAllocationCounter() == 1 )
		{
			oal_thread_cond_destroy( &(m_Global->Condition) );
			oal_thread_mutex_destroy( &(m_Global->Mutex) );
		}
		
		m_Global.deinitialize();
		
		m_Allocator.deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing Trigger", LOCATION);
		e.raiseError();
	}
}

bool Trigger::IsInitialized()
{
	return m_IsInitialized;
}
	

void Trigger::trigger()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	// Get exclusive access
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	// Increase Trigger Count
	m_Global->TriggerValue ++;
	
	// Release exclusive access
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );

	// Broadcast change of Event flag to all waiting threads and processes
	if ( oal_thread_cond_broadcast(&(m_Global->Condition)) ) 
		throw OSException( strerror(errno), LOCATION );
}


unsigned long Trigger::getTriggerState()
{
	return m_Global->TriggerValue;
}


void Trigger::waitForTrigger( unsigned long& TriggerState, const ASAAC_TimeInterval& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
   	BlockingScope TimeoutScope();

	timespec TimeSpecTimeout;
	TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();

	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	m_Global->WaitingThreads++;

	long iErrorCode = 0;
	
	while (( m_Global->TriggerValue <= TriggerState ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(m_Global->Condition), &(m_Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(m_Global->Condition), &(m_Global->Mutex), &TimeSpecTimeout );
		
	}
	
	m_Global->WaitingThreads--;

	TriggerState = m_Global->TriggerValue;
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) 
		throw TimeoutException(LOCATION);
}


void Trigger::waitForTrigger( const ASAAC_TimeInterval& Timeout )
{
	unsigned long TriggerState = getTriggerState();
	
	waitForTrigger( TriggerState, Timeout );
}




unsigned long Trigger::getWaitCount()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned long iNumberOfThreads;
		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	iNumberOfThreads = m_Global->WaitingThreads;
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( strerror(errno), LOCATION );
	
	return iNumberOfThreads;
}	




size_t Trigger::predictSize()
{
	return Shared<TriggerData>::predictSize();
}


static ObjectPool<Trigger,OS_MAX_NUMBER_OF_TRIGGERS> TriggerPool;

void* Trigger::operator new( size_t size )
{
	return (void*)TriggerPool.acquireObject( TimeIntervalInstant );
}	


void Trigger::operator delete( void* location )
{
	TriggerPool.releaseObject((Trigger*)location);
}


