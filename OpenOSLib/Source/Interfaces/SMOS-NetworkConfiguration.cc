#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_ReturnStatus ASAAC_SMOS_configureInterface(const ASAAC_InterfaceData* if_config)
{
	return CommunicationManager::getInstance()->configureInterface( *if_config );
}

ASAAC_ReturnStatus ASAAC_SMOS_createTransferConnection(const ASAAC_TcDescription* tc_desc)
{
	return CommunicationManager::getInstance()->createTransferConnection( *tc_desc );
}

ASAAC_ReturnStatus ASAAC_SMOS_destroyTransferConnection(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_descr)
{
	return CommunicationManager::getInstance()->destroyTransferConnection( tc_id, *network_descr );
}

ASAAC_ReturnStatus ASAAC_SMOS_getNetworkPortStatus(const ASAAC_NetworkDescriptor* network_desc, ASAAC_NetworkPortStatus* network_status)
{
	return CommunicationManager::getInstance()->getNetworkPortStatus( *network_desc, *network_status);
}

