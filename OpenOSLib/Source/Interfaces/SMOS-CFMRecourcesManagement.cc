#include "OpenOS.hh"


ASAAC_TimedReturnStatus ASAAC_SMOS_requestDownloadToCfm(const ASAAC_CfmDescription* remoteCfm)
{
	throw OSException("SMOS call not yet implemented.", LOCATION);
	return ASAAC_TM_ERROR;
}

ASAAC_TimedReturnStatus ASAAC_SMOS_getRemoteInfo(const ASAAC_RemoteServiceId serviceid, const ASAAC_CfmMliChannel* mliChannel, const ASAAC_InputLocalParameters* input_buffer, const unsigned long input_length, ASAAC_OutputRemoteParameters* output_buffer, const unsigned long output_max_length, unsigned long* output_actual_length)
{
	throw OSException("SMOS call not yet implemented.", LOCATION);
	return ASAAC_TM_ERROR;
}

