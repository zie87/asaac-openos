#include "IPC/Trigger.hh"

#include "Exceptions/Exceptions.hh"

#include "Common/Templates/ObjectPool.hh"

#include "ProcessManagement/ProcessManager.hh"


Trigger::Trigger( Allocator* ThisAllocator, bool IsMaster ) : m_IsInitialized(false)
{
	initialize( ThisAllocator, IsMaster );
}


Trigger::Trigger() : m_IsInitialized(false)
{
}


void Trigger::initialize( Allocator* ParentAllocator, bool IsMaster )
{
	// Avoid double initialization of object.
	if ( m_IsInitialized ) 
		throw DoubleInitializationException();

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
			throw OSException( "Unable to initialize condition variable", LOCATION );
			
		if ( oal_thread_mutexattr_init( &MutexAttribute ) )    
			throw OSException( "Unable to initialize mutex", LOCATION );

		// Condition shall be shared between processes
		if ( oal_thread_condattr_setpshared( &ConditionAttribute, PTHREAD_PROCESS_SHARED ) )
			throw OSException( "Unable to initialize condition variable", LOCATION );

		if ( oal_thread_mutexattr_setpshared( &MutexAttribute, PTHREAD_PROCESS_SHARED ) )
			throw OSException( "Unable to initialize mutex", LOCATION );


		// Initialize Mutex and Condition
		if ( oal_thread_mutex_init(&(m_Global->Mutex), &MutexAttribute ) ) 
			throw OSException( "Unable to initialize mutex", LOCATION );
		
		if ( oal_thread_cond_init(&(m_Global->Condition), &ConditionAttribute ) )
		{
			oal_thread_mutex_destroy(&(m_Global->Mutex));
			throw OSException( "Unable to initialize condition variable", LOCATION );
		}
		
		// Events shall be reset originally
		m_Global->TriggerValue = 0;
	}

	m_IsInitialized = true;
}
	

Trigger::~Trigger()
{
	if ( m_IsInitialized ) deinitialize();
}


void Trigger::deinitialize()
{
	if ( m_IsInitialized == false ) return;
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
		throw OSException( LOCATION );
	
	// Increase Trigger Count
	m_Global->TriggerValue ++;
	
	// Release exclusive access
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );

	// Broadcast change of Event flag to all waiting threads and processes
	if ( oal_thread_cond_broadcast(&(m_Global->Condition)) ) 
		throw OSException( LOCATION );
}


unsigned long Trigger::getTriggerState()
{
	return m_Global->TriggerValue;
}


ASAAC_TimedReturnStatus Trigger::waitForTrigger( unsigned long& TriggerState, const ASAAC_TimeInterval& Timeout )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );
	
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();

	timespec TimeSpecTimeout;

	TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();

	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );
	
	m_Global->WaitingThreads++;

	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_WAITING );
	
	long iErrorCode = 0;
	
	while (( m_Global->TriggerValue <= TriggerState ) && ( iErrorCode != ETIMEDOUT ))
	{
		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iErrorCode = oal_thread_cond_wait(&(m_Global->Condition), &(m_Global->Mutex));
		else iErrorCode = oal_thread_cond_timedwait(&(m_Global->Condition), &(m_Global->Mutex), &TimeSpecTimeout );
		
	}
	
	m_Global->WaitingThreads--;

    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_RUNNING );
	
	TriggerState = m_Global->TriggerValue;
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );
	
	if ( iErrorCode == ETIMEDOUT ) 
		return ASAAC_TM_TIMEOUT;
	
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus Trigger::waitForTrigger( const ASAAC_TimeInterval& Timeout )
{
	unsigned long TriggerState = getTriggerState();
	
	return waitForTrigger( TriggerState, Timeout );
}




unsigned long Trigger::getWaitCount()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	unsigned long iNumberOfThreads;
		
	if ( oal_thread_mutex_lock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );
	
	iNumberOfThreads = m_Global->WaitingThreads;
	
	if ( oal_thread_mutex_unlock(&(m_Global->Mutex)) ) 
		throw OSException( LOCATION );
	
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

