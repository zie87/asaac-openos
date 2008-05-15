#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_ReturnStatus ASAAC_SMOS_configureInterface(const ASAAC_InterfaceData* if_config)
{
    ASAAC_ReturnStatus result;

    try 
    {
    	CommunicationManager::getInstance()->configureInterface( *if_config );

        result = ASAAC_SUCCESS;

        LOG_SERVICE1(*if_config, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::configureInterface", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}

ASAAC_ReturnStatus ASAAC_SMOS_createTransferConnection(const ASAAC_TcDescription* tc_desc)
{
    ASAAC_ReturnStatus result;

    try 
    {
    	CommunicationManager::getInstance()->createTransferConnection( *tc_desc );

        result = ASAAC_SUCCESS;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::createTransferConnection", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }

    LOG_SERVICE1(*tc_desc, result);
    
    return result;
}

ASAAC_ReturnStatus ASAAC_SMOS_destroyTransferConnection(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_descr)
{
    ASAAC_ReturnStatus result;

    try 
    {
    	CommunicationManager::getInstance()->destroyTransferConnection( tc_id, *network_descr );

        result = ASAAC_SUCCESS;

        LOG_SERVICE2(tc_id, *network_descr, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::destroyTransferConnection", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}

ASAAC_ReturnStatus ASAAC_SMOS_getNetworkPortStatus(const ASAAC_NetworkDescriptor* network_desc, ASAAC_NetworkPortStatus* network_status)
{
    ASAAC_ReturnStatus result;

    try 
    {
    	CommunicationManager::getInstance()->getNetworkPortStatus( *network_desc, *network_status);

        result = ASAAC_SUCCESS;

        LOG_SERVICE2(*network_desc, *network_status, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::getNetworkPortStatus", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}

