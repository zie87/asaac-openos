#include "BlockingScope.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "OpenOSObject.hh"

BlockingScope::BlockingScope()
{
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread != 0 ) 
		ThisThread->setState( ASAAC_WAITING );
}

BlockingScope::~BlockingScope()
{
	Thread* ThisThread = ProcessManager::getInstance()->getCurrentThread();
	
	if ( ThisThread != 0 ) 
		ThisThread->setState( ASAAC_RUNNING );
}
