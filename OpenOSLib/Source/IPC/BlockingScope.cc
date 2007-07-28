#include "BlockingScope.hh"

#include "ProcessManagement/ThreadManager.hh"
#include "OpenOSObject.hh"


BlockingScope::BlockingScope(): m_ThreadId(OS_UNUSED_ID)
{
	try
	{		 
		Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();
	
		m_ThreadId = ThisThread->getId();
		ThisThread->setState( ASAAC_WAITING );
	}
	catch ( ASAAC_Exception &e )
	{
		// do nothing
	}
}


BlockingScope::~BlockingScope()
{
	try
	{		 
		Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();

		if ( ThisThread->getId() == m_ThreadId )
			ThisThread->setState( ASAAC_RUNNING );
	}
	catch ( ASAAC_Exception &e )
	{
		// do nothing
	}
}
