#ifndef ASAAC_ENH_H_
#define ASAAC_ENH_H_

#include "ASAAC.h"

typedef struct {
		ASAAC_NiiReturnStatus status;
		ASAAC_PublicId tc_id;
} EventInfoData_BufferSent;

typedef struct {
	ASAAC_NiiReturnStatus status;
	ASAAC_PublicId tc_id;
} EventInfoData_BufferReceived;

#endif /*ASAAC_ENH_H_*/
