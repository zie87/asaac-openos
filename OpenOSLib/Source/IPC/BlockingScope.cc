#include "BlockingScope.hh"

#include "ProcessManagement/ThreadManager.hh"
#include "OpenOSObject.hh"


BlockingScope::BlockingScope(): m_ThreadId(OS_UNUSED_ID)
{
	try
	{		 
		Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread();
		
		m_ThreadId = ThisThread->getId();

		ThisThread->enterBlockingScope(*this);
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
		ThreadManager::getInstance()->getCurrentThread()->exitBlockingScope(*this);
	}
	catch ( ASAAC_Exception &e )
	{
		// do nothing
	}
}


ASAAC_PublicId BlockingScope::getThreadId()
{
	return m_ThreadId;
}
