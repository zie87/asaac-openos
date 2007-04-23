#include "OpenOS-Application.hh"

#include "ProcessManagement/ProcessManager.hh"


ASAAC_ReturnStatus OpenOS_registerThread(char * name, EntryPointAddr address)
{
	ASAAC_CharacterSequence threadName = CharacterSequence(name).asaac_str();
	return ProcessManager::getInstance()->addEntryPoint( threadName, address );
}
