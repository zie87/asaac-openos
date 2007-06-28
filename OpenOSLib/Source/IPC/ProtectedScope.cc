#include "IPC/ProtectedScope.hh"
#include "Exceptions/Exceptions.hh"

#include "ProcessManagement/ProcessManager.hh"
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
		e.addPath("Protected Scope couldn't be entered", LOCATION);
		
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
			
				Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
			
				if ( ThisThread != 0)
				{
					if (  ThisThread->isSuspendPending() )
					{
						oal_thread_kill( oal_thread_self(), OS_SIGNAL_SUSPEND );
					}
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
	Thread * ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if (ThisThread == 0)
		return;
		
	ThisThread->enterProtectedScope(this);
}


void ProtectedScope::exit()
{
	Thread * ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if (ThisThread == 0)
		return;
		
	ThisThread->exitProtectedScope(this);
}

