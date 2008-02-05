#include "OpenOSObject.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_requestDownloadToCfm(const ASAAC_CfmDescription* remoteCfm)
{
	OSException("SMOS::requestDownloadToCfm - not yet implemented.", LOCATION).raiseError();
	return ASAAC_TM_ERROR;
}

ASAAC_TimedReturnStatus ASAAC_SMOS_getRemoteInfo(const ASAAC_RemoteServiceId serviceid, const ASAAC_CfmMliChannel* mliChannel, const ASAAC_InputLocalParameters* input_buffer, const unsigned long input_length, ASAAC_OutputRemoteParameters* output_buffer, const unsigned long output_max_length, unsigned long* output_actual_length)
{
	OSException("SMOS::getRemoteInfo - not yet implemented.", LOCATION).raiseError();
	return ASAAC_TM_ERROR;
}

