//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Semaphore.cc
//  @ Date : 12.04.2006
//  @ Author : 
//
//
#include "Semaphore.hh"

#include "ProcessManagement/ProcessManager.hh"

#include "Common/Templates/ObjectPool.hh"

#include "Allocator/Allocator.hh"

#include "Exceptions/Exceptions.hh"

Semaphore::Semaphore(Allocator* ThisAllocator, const bool IsMaster, const long InitialCount, const long MaximumCount ) : m_IsInitialized(false)
{
	initialize( ThisAllocator, IsMaster, InitialCount );
}


Semaphore::Semaphore() : m_IsInitialized(false)
{
}


void Semaphore::initialize(Allocator* ThisAllocator, const bool IsMaster, const long InitialCount, const long MaximumCount )
{
	// avoid duplicate initialization
//	cout << "Semaphore::initialize( " << this << ", " << Allocator << ", " << IsMaster << ", " << InitialCount << ")"<< endl;
	if ( m_IsInitialized )
	{
		throw DoubleInitializationException( LOCATION );
	}
	m_IsInitialized = true;

	try
	{
		// Explicit parent initialization call
		m_Global.initialize( ThisAllocator );
	
		if ( IsMaster )
		{
			if ( oal_sem_init( &(m_Global->Semaphore), 1, InitialCount ) != 0 ) 
	            throw OSException( strerror(errno), LOCATION );
	
    		m_Global->WaitingThreads = 0;
			m_Global->MaximumCount   = MaximumCount;
		}

	}
	catch ( ASAAC_Exception& E )
	{
		deinitialize();
		
		throw;
	}
}



Semaphore::~Semaphore()
{
	if ( m_IsInitialized ) 
		deinitialize();
}


void Semaphore::deinitialize()
{
	if ( m_IsInitialized == false ) 
		return;
	
	try
	{
		if ( m_IsMaster )
		{
			if ( oal_sem_destroy( &(m_Global->Semaphore) ) != 0 ) 
				throw OSException( "Error destroying semaphore", LOCATION );
		}

		m_Global.deinitialize();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error deinitializing semaphore", LOCATION);
		e.raiseError();
	}
	
	m_IsInitialized = false;
}


ASAAC_TimedReturnStatus Semaphore::wait( const ASAAC_Time& Timeout )
{	
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	long iError;

	timespec TimeSpecTimeout = TimeStamp(Timeout).timespec_Time();
	
	m_Global->WaitingThreads++;

	// The while loop is necessary because, unlike "man oal_sem_init" informs us,
	// oal_sem_wait can be longerrupted by signals and in this case returns with
	// a nonzero result.

	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_WAITING );

	do 
	{ 

		if (( Timeout.sec == TimeInfinity.sec ) && ( Timeout.nsec == TimeInfinity.nsec ))
			iError = oal_sem_wait( &(m_Global->Semaphore) );
		else iError = oal_sem_timedwait( &(m_Global->Semaphore), &TimeSpecTimeout );
		
		if ( ( iError == -1 ) && ( errno == ETIMEDOUT )) break;
		 
	} while (( iError !=  0 ) && ( errno = EINTR ));

    ThisThread = ProcessManager::getInstance()->getCurrentThread();
	if ( ThisThread != 0 ) ThisThread->setState( ASAAC_RUNNING );

	m_Global->WaitingThreads--;
	
	if (( iError == -1 ) && ( errno == ETIMEDOUT ))
	{
		return ASAAC_TM_TIMEOUT;
	}
	
	return ASAAC_TM_SUCCESS;
}



ASAAC_ReturnStatus Semaphore::post()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	if ( long(getCount()) == m_Global->MaximumCount )
	{
		throw OSException( "Semaphore exceeding maximum", LOCATION );
	}
		
	if ( oal_sem_post( &(m_Global->Semaphore) ) == -1)
	{
		perror("Semaphore::post() ");	
		throw OSException( "Error posting semaphore", LOCATION );
	}
	
	return ASAAC_SUCCESS;
}


unsigned long Semaphore::getCount() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	int iReturn;
	
	if ( oal_sem_getvalue( &(m_Global->Semaphore), &iReturn ) != 0 ) 
        throw OSException( LOCATION );
	
	return iReturn;
}


unsigned long Semaphore::getWaitingThreads() const
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	return m_Global->WaitingThreads;
}


size_t Semaphore::predictSize()
{
	return Shared<SemaphoreData>::predictSize();
}


/* Generic LockingObject Interfaces */

ASAAC_TimedReturnStatus Semaphore::lock( const ASAAC_Time& Timeout )
{
	try 
	{
		return wait( Timeout );
	}
	catch ( ASAAC_Exception& E )
	{
		return ASAAC_TM_ERROR;
	}
}


ASAAC_ReturnStatus Semaphore::release()
{
	return ( post() == ASAAC_SUCCESS ) ? ASAAC_SUCCESS : ASAAC_ERROR;
}


static ObjectPool<Semaphore,OS_MAX_NUMBER_OF_SEMAPHORES> SemaphorePool;

void* Semaphore::operator new( size_t size )
{
	return (void*)SemaphorePool.acquireObject( TimeIntervalInstant );
}	


void Semaphore::operator delete( void* location )
{
	SemaphorePool.releaseObject((Semaphore*)location);
}
