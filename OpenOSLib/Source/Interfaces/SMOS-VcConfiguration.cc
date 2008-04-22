#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_ReturnStatus ASAAC_SMOS_createVirtualChannel(const ASAAC_VcDescription* vc_desc)
{
    ASAAC_ReturnStatus result;
    
    try 
    {
    	CommunicationManager::getInstance()->createVirtualChannel( *vc_desc );

        result = ASAAC_SUCCESS;

        LOG_SERVICE1(*vc_desc, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::createVirtualChannel", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
	return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_destroyVirtualChannel(const ASAAC_PublicId vc_id)
{
    ASAAC_ReturnStatus result;
    
    try 
    {
    	CommunicationManager::getInstance()->destroyVirtualChannel( vc_id );

        result = ASAAC_SUCCESS;

        LOG_SERVICE1(vc_id, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::destroyVirtualChannel", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_attachChannelToProcessOrThread(const ASAAC_VcMappingDescription* vc_mapping)
{
    ASAAC_ReturnStatus result;
    
    try 
    {
    	CommunicationManager::getInstance()->attachChannelToProcessOrThread( *vc_mapping );

        result = ASAAC_SUCCESS;

        LOG_SERVICE1(*vc_mapping, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::attachChannelToProcessOrThread", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_detachAllThreadsOfProcessFromVc(const ASAAC_PublicId vc_id, const ASAAC_PublicId process_id)
{
    ASAAC_ReturnStatus result;
    
    try 
    {
    	CommunicationManager::getInstance()->detachAllThreadsOfProcessFromVc( vc_id, process_id );

        result = ASAAC_SUCCESS;

        LOG_SERVICE2(vc_id, process_id, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::detachAllThreadsOfProcessFromVc", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_attachTransferConnectionToVirtualChannel(const ASAAC_VcToTcMappingDescription* vc_to_tc_mapping)
{
    ASAAC_ReturnStatus result;
    
    try 
    {
    	CommunicationManager::getInstance()->attachTransferConnectionToVirtualChannel( *vc_to_tc_mapping );

        result = ASAAC_SUCCESS;

        LOG_SERVICE1(*vc_to_tc_mapping, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::attachTransferConnectionToVirtualChannel", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}


ASAAC_ReturnStatus ASAAC_SMOS_detachTransferConnectionFromVirtualChannel(const ASAAC_PublicId vc_id, const ASAAC_PublicId tc_id)
{
    ASAAC_ReturnStatus result;
    
    try 
    {
    	CommunicationManager::getInstance()->detachTransferConnectionFromVirtualChannel( vc_id, tc_id );

        result = ASAAC_SUCCESS;

        LOG_SERVICE2(vc_id, tc_id, result);
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::detachTransferConnectionFromVirtualChannel", LOCATION);
        e.raiseError();
        
        result = ASAAC_ERROR;
    }
    
    return result;
}

