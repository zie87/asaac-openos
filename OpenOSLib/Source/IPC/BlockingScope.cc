#include "BlockingScope.hh"

#include "ProcessManagement/ThreadManager.hh"
#include "OpenOSObject.hh"

BlockingScope::BlockingScope()
{
	
	Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread( false );
	
	if ( ThisThread != NULL ) 
		ThisThread->setState( ASAAC_WAITING );
}

BlockingScope::~BlockingScope()
{
	Thread* ThisThread = ThreadManager::getInstance()->getCurrentThread( false );
	
	if ( ThisThread != NULL ) 
		ThisThread->setState( ASAAC_RUNNING );
}
