#ifndef TIMEOPERATIONS_H_
#define TIMEOPERATIONS_H_

#include "Source/AsaacCIncludes.h"

#ifdef __cplusplus
extern "C" {
#endif
	
ASAAC_Time TimeZero();	
ASAAC_Time TimeInstant();	
ASAAC_Time TimeInfinity();	

ASAAC_TimeInterval TimeIntervalInstant();	
ASAAC_TimeInterval TimeIntervalInfinity();	
	
ASAAC_Time addIntervalToTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval);
ASAAC_TimeInterval addIntervalToInterval(const ASAAC_TimeInterval Interval1, const ASAAC_TimeInterval Interval2);

ASAAC_Time subtractIntervalFromTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval);
ASAAC_TimeInterval subtractTimeFromTime(const ASAAC_Time Time1, const ASAAC_Time Time2);

ASAAC_TimeInterval multiplyIntervalWithArg(const ASAAC_TimeInterval TimeInterval, double Arg );
ASAAC_TimeInterval devideIntervalByArg(const ASAAC_TimeInterval TimeInterval, double Arg );

char lower(const ASAAC_Time Time1, const ASAAC_Time Time2);
char lowerequal(const ASAAC_Time Time1, const ASAAC_Time Time2);
char equal( const ASAAC_Time Time1, const ASAAC_Time Time2 );
char greaterequal(const ASAAC_Time Time1, const ASAAC_Time Time2);
char greater(const ASAAC_Time Time1, const ASAAC_Time Time2);

ASAAC_Time FloatToTime( double Value );
ASAAC_Time TimespecToTime( struct timespec Value );
ASAAC_Time TimevalToTime( struct timeval Value );
ASAAC_Time IntervalToTime( ASAAC_TimeInterval Value );

ASAAC_TimeInterval FloatToInterval( double Value );
ASAAC_TimeInterval TimespecToInterval( struct timespec Value );
ASAAC_TimeInterval TimevalToInterval( struct timeval Value );
ASAAC_TimeInterval TimeToInterval( ASAAC_Time Value );

double TimeToFloat( ASAAC_Time Value );
struct timespec TimeToTimespec( ASAAC_Time Value );
struct timeval TimeToTimeval( ASAAC_Time Value );

double IntervalToFloat( ASAAC_TimeInterval Value );
struct timespec IntervalToTimespec( ASAAC_TimeInterval Value );
struct timeval IntervalToTimeval( ASAAC_TimeInterval Value );

#ifdef __cplusplus
}
#endif

#endif /*TIMEOPERATIONS_H_*/
