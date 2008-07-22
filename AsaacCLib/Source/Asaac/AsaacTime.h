#ifndef ASAACTIME_H_
#define ASAACTIME_H_

#include "Source/AsaacCIncludes.h"

#ifdef __cplusplus
extern "C" {
#endif

ASAAC_ReturnStatus Time_assign( ASAAC_Time *handle, ASAAC_Time *value );  
ASAAC_ReturnStatus Time_assignZero( ASAAC_Time *handle );  
ASAAC_ReturnStatus Time_assignInfinity( ASAAC_Time *handle );  
ASAAC_ReturnStatus Time_assignUInt64( ASAAC_Time *handle, uint64_t *value );
ASAAC_ReturnStatus Time_assignTimespec( ASAAC_Time *handle, struct timespec *value );

ASAAC_ReturnStatus Time_add( ASAAC_Time *handle, ASAAC_TimeInterval *interval);
ASAAC_ReturnStatus Time_subtract( ASAAC_Time *handle, ASAAC_TimeInterval *interval);
ASAAC_ReturnStatus Time_subtractTime( ASAAC_Time *handle, ASAAC_Time *time, ASAAC_TimeInterval *result);

CompareReturnStatus Time_compare( ASAAC_Time *handle , ASAAC_Time *comparing_time);

ASAAC_ReturnStatus Time_convertToUInt64( ASAAC_Time *handle, uint64_t *target );
ASAAC_ReturnStatus Time_convertToTimespec( ASAAC_Time *handle, struct timespec *target );

#ifdef __cplusplus
}
#endif

#endif /*ASAACTIME_H_*/
