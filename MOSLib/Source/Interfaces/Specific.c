#include "MOSIncludes.h"


ASAAC_ReturnStatus ASAAC_MOS_setPowerSwitch(const ASAAC_PublicId switch_id, const ASAAC_SwitchOp switch_op)
{
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus ASAAC_MOS_resetPowerSwitches()
{
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus ASAAC_MOS_getPowerSwitchStatus(ASAAC_PowerSwitch* power_switch)
{
	return ASAAC_ERROR;
}


ASAAC_MSLStatus ASAAC_MOS_SetTxData(unsigned long* key, const ASAAC_Address buffer, const unsigned long size)
{
	return ASAAC_MSL_FAILED;
}


ASAAC_MSLStatus ASAAC_MOS_SetRxData(unsigned long* key, const ASAAC_Address buffer, const unsigned long size)
{
	return ASAAC_MSL_FAILED;
}


ASAAC_MSLStatus ASAAC_MOS_StartTransfer(const unsigned long key, const unsigned long lba, const ASAAC_IOoperation op)
{
	return ASAAC_MSL_FAILED;
}


ASAAC_NiiReturnStatus ASAAC_MOS_configureFragmentedTransfer(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_id, const ASAAC_TransferDirection send_receive, const ASAAC_TransferType message_streaming, const ASAAC_TC_ConfigurationData configuration_data, const ASAAC_Bool trigger_callback, const ASAAC_PublicId callback_id)
{
	return ASAAC_MOS_NII_CALL_FAILED;
}


ASAAC_NiiReturnStatus ASAAC_MOS_sendFragmentedTransfer(const ASAAC_PublicId tc_id, const ASAAC_CharAddress transmit_data, const ASAAC_Length data_length)
{
	return ASAAC_MOS_NII_CALL_FAILED;
}


ASAAC_NiiReturnStatus ASAAC_MOS_receiveFragmentedTransfer(const ASAAC_PublicId tc_id, ASAAC_CharAddress* receive_data, ASAAC_Length* data_length_available, const ASAAC_Length data_length)
{
	return ASAAC_MOS_NII_CALL_FAILED;
}


