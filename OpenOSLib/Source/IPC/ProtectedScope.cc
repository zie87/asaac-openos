#include "IPC/ProtectedScope.hh"
#include "Exceptions/Exceptions.hh"

#include "ProcessManagement/ThreadManager.hh"
#include "ProcessManagement/Thread.hh"

ProtectedScope::ProtectedScope( char * Scope, LockingObject& ThisLockingObject, const ASAAC_Time& Timeout, bool Cancelable ) : m_LockingObject(&ThisLockingObject), m_Cancelable(Cancelable)
{
	m_Scope = Scope;
	m_Timeout = Timeout;

	try
	{		
		m_LockingObject->lock( Timeout );
	
		enter();
	
		if ( m_Cancelable == false )
		{
			oal_thread_setcancelstate( PTHREAD_CANCEL_DISABLE, &m_CancelState );
		}
	
		m_Status = LOCKED;
	}
	catch ( ASAAC_Exception &e )
	{
		CharSeq ErrorString;
		
		e.addPath( (ErrorString << "Protected Scope couldn't be entered: " << Scope).c_str(), LOCATION);
		
		throw;		
	}
}


bool ProtectedScope::isCancelable()
{
	return m_Cancelable;
}


ASAAC_Time ProtectedScope::timeout()
{
	return m_Timeout;
}


ASAAC_CharacterSequence ProtectedScope::scope()
{
	return m_Scope.asaac_str();
}


ProtectedScope::~ProtectedScope()
{
	try
	{
		if ( m_Status == LOCKED )
		{
			m_LockingObject->release();
		}
		
		if ( m_Cancelable == false )
		{
			oal_thread_setcancelstate( m_CancelState, NULL );
			
			if ( m_CancelState == PTHREAD_CANCEL_ENABLE )
			{
				oal_thread_testcancel();
			
				try
				{
					if (  ThreadManager::getInstance()->getCurrentThread()->isSuspendPending() )
					{
						oal_thread_kill( oal_thread_self(), OS_SIGNAL_SUSPEND );
					}
				}
				catch ( ASAAC_Exception &e )
				{
					// do nothing
				}
			}
		}
	}
	catch (ASAAC_Exception &e)
	{
		CharSeq ErrorString;
		e.addPath( (ErrorString << "Error exiting ProtectedScope: '" << m_Scope << "'").c_str() );
		
		exit();
		
		throw;
	}
	
	exit();
}


void ProtectedScope::enter()
{
	try
	{
		ThreadManager::getInstance()->getCurrentThread()->enterProtectedScope(this);
	}
	catch ( ASAAC_Exception &e )
	{
		// do nothing
	}
}


void ProtectedScope::exit()
{
	try
	{
		ThreadManager::getInstance()->getCurrentThread()->exitProtectedScope(this);
	}
	catch ( ASAAC_Exception &e )
	{
		// do nothing
	}
}

