#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_ReturnStatus ASAAC_SMOS_configureInterface(const ASAAC_InterfaceData* if_config)
{
    try 
    {
    	CommunicationManager::getInstance()->configureInterface( *if_config );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::configureInterface", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus ASAAC_SMOS_createTransferConnection(const ASAAC_TcDescription* tc_desc)
{
    try 
    {
    	CommunicationManager::getInstance()->createTransferConnection( *tc_desc );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::createTransferConnection", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus ASAAC_SMOS_destroyTransferConnection(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_descr)
{
    try 
    {
    	CommunicationManager::getInstance()->destroyTransferConnection( tc_id, *network_descr );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::destroyTransferConnection", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus ASAAC_SMOS_getNetworkPortStatus(const ASAAC_NetworkDescriptor* network_desc, ASAAC_NetworkPortStatus* network_status)
{
    try 
    {
    	CommunicationManager::getInstance()->getNetworkPortStatus( *network_desc, *network_status);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::getNetworkPortStatus", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

