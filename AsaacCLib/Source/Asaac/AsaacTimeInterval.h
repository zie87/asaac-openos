#ifndef ASAACTIMEINTERVAL_H_
#define ASAACTIMEINTERVAL_H_

#include "Source/AsaacCIncludes.h"

#ifdef __cplusplus
extern "C" {
#endif

ASAAC_ReturnStatus TimeInterval_assign( ASAAC_TimeInterval *handle, ASAAC_TimeInterval *value );  
ASAAC_ReturnStatus TimeInterval_assignInstant( ASAAC_TimeInterval *handle );  
ASAAC_ReturnStatus TimeInterval_assignInfinity( ASAAC_TimeInterval *handle );  
ASAAC_ReturnStatus TimeInterval_assignFloat128( ASAAC_TimeInterval *handle, float128_t *value );
ASAAC_ReturnStatus TimeInterval_assignUInt64( ASAAC_TimeInterval *handle, uint64_t *value );
ASAAC_ReturnStatus TimeInterval_assignTimeval( ASAAC_TimeInterval *handle, struct timeval *value );

ASAAC_ReturnStatus TimeInterval_add( ASAAC_TimeInterval *handle, const ASAAC_TimeInterval *interval);
ASAAC_ReturnStatus TimeInterval_subtract( ASAAC_TimeInterval *handle, const ASAAC_TimeInterval *interval);
ASAAC_ReturnStatus TimeInterval_multiply( ASAAC_TimeInterval *handle, float128_t arg );
ASAAC_ReturnStatus TimeInterval_devide( ASAAC_TimeInterval *handle, float128_t arg );

CompareReturnStatus TimeInterval_compare( ASAAC_TimeInterval *handle , ASAAC_TimeInterval *comparing_interval);

ASAAC_ReturnStatus TimeInterval_convertToFloat128( ASAAC_TimeInterval *handle, float128_t *target );
ASAAC_ReturnStatus TimeInterval_convertToUInt64( ASAAC_TimeInterval *handle, uint64_t *target );
ASAAC_ReturnStatus TimeInterval_convertToTimeval( ASAAC_TimeInterval *handle, struct timeval *target );

#ifdef __cplusplus
}
#endif

#endif /*ASAACTIMEINTERVAL_H_*/
