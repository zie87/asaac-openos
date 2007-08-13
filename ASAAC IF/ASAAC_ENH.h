#ifndef ASAAC_ENH_H_
#define ASAAC_ENH_H_

#include "ASAAC.h"

typedef union {
	struct {
		ASAAC_NiiReturnStatus status;
		ASAAC_PublicId tc_id;
	} comms_ev_buffer_sent;

	struct {
		ASAAC_NiiReturnStatus status;
		ASAAC_PublicId tc_id;
	} comms_ev_buffer_received;
} EventInfoData;

#endif /*ASAAC_ENH_H_*/
