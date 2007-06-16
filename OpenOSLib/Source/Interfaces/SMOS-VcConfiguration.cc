#include "OpenOS.hh"

#include "Communication/CommunicationManager.hh"


ASAAC_ReturnStatus ASAAC_SMOS_createVirtualChannel(const ASAAC_VcDescription* vc_desc)
{
	return ( CommunicationManager::getInstance()->createGlobalVirtualChannel( *vc_desc ) == 0 ? ASAAC_ERROR : ASAAC_SUCCESS );
}


ASAAC_ReturnStatus ASAAC_SMOS_destroyVirtualChannel(const ASAAC_PublicId vc_id)
{
	return CommunicationManager::getInstance()->destroyGlobalVirtualChannel( vc_id );
}


ASAAC_ReturnStatus ASAAC_SMOS_attachChannelToProcessOrThread(const ASAAC_VcMappingDescription* vc_mapping)
{
	return CommunicationManager::getInstance()->attachChannelToProcessOrThread( *vc_mapping );
}


ASAAC_ReturnStatus ASAAC_SMOS_detachAllThreadsOfProcessFromVc(const ASAAC_PublicId vc_id, const ASAAC_PublicId process_id)
{
	return CommunicationManager::getInstance()->detachAllThreadsOfProcessFromVc( vc_id, process_id );
}


ASAAC_ReturnStatus ASAAC_SMOS_attachTransferConnectionToVirtualChannel(const ASAAC_VcToTcMappingDescription* vc_to_tc_mapping)
{
	return CommunicationManager::getInstance()->attachTransferConnectionToVirtualChannel( *vc_to_tc_mapping );
}


ASAAC_ReturnStatus ASAAC_SMOS_detachTransferConnectionFromVirtualChannel(const ASAAC_PublicId vc_id, const ASAAC_PublicId tc_id)
{
	return CommunicationManager::getInstance()->detachTransferConnectionFromVirtualChannel( vc_id, tc_id );
}

