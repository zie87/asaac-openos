#include "MOSIncludes.hh"

ASAAC_NiiReturnStatus ASAAC_MOS_configureInterface(
		const ASAAC_PublicId interface_id,
		const ASAAC_NetworkDescriptor* network_id,
		const ASAAC_InterfaceConfigurationData* configuration_data)
{
	switch (interface_id)
	{
		case MOS_INTERFACE_ID_NII: 	
			return cMosNii::getInstance()->configureInterface(
				*network_id,
				*configuration_data);
			break;
			
		default: return ASAAC_MOS_NII_INVALID_INTERFACE; 
	}
}


ASAAC_NiiReturnStatus ASAAC_MOS_configureTransfer(const ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor* network_id,
		const ASAAC_TransferDirection send_receive,
		const ASAAC_TransferType message_streaming,
		const ASAAC_TC_ConfigurationData configuration_data,
		const ASAAC_Bool trigger_callback, const ASAAC_PublicId callback_id)
{
	return cMosNii::getInstance()->configureTransfer(
			tc_id,
			*network_id, 
			send_receive, 
			message_streaming, 
			configuration_data, 
			trigger_callback, 
			callback_id );
}


ASAAC_NiiReturnStatus ASAAC_MOS_sendTransfer(const ASAAC_PublicId tc_id,
		const ASAAC_CharAddress transmit_data, const ASAAC_Length data_length,
		const ASAAC_Time* time_out)
{
	return cMosNii::getInstance()->sendTransfer(
			tc_id, 
			transmit_data, 
			data_length, 
			*time_out);
}


ASAAC_NiiReturnStatus ASAAC_MOS_receiveTransfer(
		const ASAAC_PublicId tc_id,
		ASAAC_CharAddress* receive_data,
		const ASAAC_Length data_length_available, 
		ASAAC_Length* data_length,
		const ASAAC_Time* time_out)
{
	return cMosNii::getInstance()->receiveTransfer(
			tc_id, 
			*receive_data,
			data_length_available, 
			*data_length, 
			*time_out);

}


ASAAC_NiiReturnStatus ASAAC_MOS_destroyTransfer(const ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor* network_id)
{
	return cMosNii::getInstance()->destroyTransfer(tc_id, *network_id);
}


ASAAC_NiiReturnStatus ASAAC_MOS_getNetworkPortStatus(
		const ASAAC_NetworkDescriptor* network_id,
		ASAAC_NetworkPortStatus* info_data)
{
	return ASAAC_MOS_NII_CALL_FAILED;

}


ASAAC_NiiReturnStatus ASAAC_MOS_receiveNetwork(
		const ASAAC_NetworkDescriptor* network,
		ASAAC_CharAddress* receive_data,
		const ASAAC_Length data_length_available, ASAAC_Length* data_length,
		ASAAC_PublicId* tc_id, const ASAAC_Time* time_out)
{
	return cMosNii::getInstance()->receiveNetwork(
			*network, 
			*receive_data,
			data_length_available, 
			*data_length, 
			*tc_id, 
			*time_out);
}


