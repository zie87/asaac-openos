#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_ReturnStatus ASAAC_SMOS_createVirtualChannel(const ASAAC_VcDescription* vc_desc)
{
    try 
    {
    	CommunicationManager::getInstance()->createVirtualChannel( *vc_desc );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::createVirtualChannel", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_destroyVirtualChannel(const ASAAC_PublicId vc_id)
{
    try 
    {
    	CommunicationManager::getInstance()->destroyVirtualChannel( vc_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::destroyVirtualChannel", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_attachChannelToProcessOrThread(const ASAAC_VcMappingDescription* vc_mapping)
{
    try 
    {
    	CommunicationManager::getInstance()->attachChannelToProcessOrThread( *vc_mapping );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::attachChannelToProcessOrThread", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_detachAllThreadsOfProcessFromVc(const ASAAC_PublicId vc_id, const ASAAC_PublicId process_id)
{
    try 
    {
    	CommunicationManager::getInstance()->detachAllThreadsOfProcessFromVc( vc_id, process_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::detachAllThreadsOfProcessFromVc", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_attachTransferConnectionToVirtualChannel(const ASAAC_VcToTcMappingDescription* vc_to_tc_mapping)
{
    try 
    {
    	CommunicationManager::getInstance()->attachTransferConnectionToVirtualChannel( *vc_to_tc_mapping );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::attachTransferConnectionToVirtualChannel", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_SMOS_detachTransferConnectionFromVirtualChannel(const ASAAC_PublicId vc_id, const ASAAC_PublicId tc_id)
{
    try 
    {
    	CommunicationManager::getInstance()->detachTransferConnectionFromVirtualChannel( vc_id, tc_id );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("SMOS::detachTransferConnectionFromVirtualChannel", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

