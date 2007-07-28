#include "IPC/ProtectedScope.hh"
#include "Exceptions/Exceptions.hh"

#include "ProcessManagement/ThreadManager.hh"
#include "ProcessManagement/Thread.hh"

ProtectedScope::ProtectedScope( char * Scope, LockingObject& ThisLockingObject, const ASAAC_Time& Timeout, bool Cancelable ) : m_LockingObject(&ThisLockingObject), m_Locked(false), m_SuspendPending(false)
{
	m_Scope = CharSeq(Scope).asaac_str();
	m_Timeout = Timeout;
	m_Cancelable = Cancelable;

	try
	{		
		m_LockingObject->lock( Timeout );
		m_Locked = true;

		Thread* CurrentThread = NULL;
		NO_EXCEPTION( CurrentThread = ThreadManager::getInstance()->getCurrentThread() );
		
		if (CurrentThread != NULL)
		{
			m_SuspendPending = CurrentThread->isSuspendPending();
			CurrentThread->enterProtectedScope(*this);
		}
	}
	catch ( ASAAC_Exception &e )
	{
		CharSeq ErrorString;
		
		e.addPath( (ErrorString << "Protected Scope couldn't be entered: " << Scope).c_str(), LOCATION);
		
		throw;		
	}
}


ProtectedScope::~ProtectedScope()
{
	try
	{
		if ( m_Locked == true )
			m_LockingObject->release();
		
		Thread* CurrentThread = NULL;
		NO_EXCEPTION( CurrentThread = ThreadManager::getInstance()->getCurrentThread() );
		
		if (CurrentThread != NULL)
			CurrentThread->exitProtectedScope(*this);
	}
	catch (ASAAC_Exception &e)
	{
		CharSeq ErrorString;
		e.addPath( (ErrorString << "Error exiting ProtectedScope: '" << m_Scope << "'").c_str() );
		
		throw;
	}
}

bool ProtectedScope::isCancelable()
{
	return m_Cancelable;
}


ASAAC_Time ProtectedScope::getTimeout()
{
	return m_Timeout;
}


ASAAC_CharacterSequence ProtectedScope::getScope()
{
	return m_Scope;
}


int	ProtectedScope::getCancelState()
{
	return m_CancelState;
}


void ProtectedScope::setCancelState(int CancelState)
{
	m_CancelState = CancelState;
}


bool ProtectedScope::isSuspendPending()
{
	return m_SuspendPending;
}

