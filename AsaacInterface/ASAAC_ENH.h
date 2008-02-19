#ifndef ASAAC_ENH_H_
#define ASAAC_ENH_H_

#include "ASAAC.h"

typedef union 
{
	struct {
		ASAAC_ErrorInfo error_info;
	} comms_ev_error;

	struct {
		ASAAC_NetworkDescriptor network_id;
		ASAAC_NetworkPortStatus info_data;
	} comms_ev_info;

	struct {
		ASAAC_NiiReturnStatus status;
		ASAAC_NetworkDescriptor network_id;
		ASAAC_PublicId tc_id;
	} comms_ev_configure;

	struct {
		ASAAC_NiiReturnStatus status;
		ASAAC_PublicId tc_id;
	} comms_ev_buffer_sent;

	struct {
		ASAAC_NiiReturnStatus status;
		ASAAC_PublicId tc_id;
	} comms_ev_buffer_received;

	struct {
		ASAAC_PublicId timer_id;
	} timer_alarm;

	struct {
		// NULL shall be returned
	} cbit_error_detect;

	struct {
		// NULL shall be returned
	} mmm_sd_event;

} EventInfoData;



#endif /*ASAAC_ENH_H_*/
