#ifndef TIMEOPERATIONS_H_
#define TIMEOPERATIONS_H_

#include "Source/AsaacCIncludes.h"

ASAAC_Time inline addIntervalToTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval);
ASAAC_TimeInterval inline addIntervalToInterval(const ASAAC_TimeInterval Interval1, const ASAAC_TimeInterval Interval2);
ASAAC_Time inline subtractIntervalFromTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval);

ASAAC_TimeInterval inline subtractTimeFromTime(const ASAAC_Time Time1, const ASAAC_Time Time2);
ASAAC_TimeInterval inline multiplyIntervalWithArg(const ASAAC_TimeInterval TimeInterval, double Arg );
ASAAC_TimeInterval inline devideIntervalByArg(const ASAAC_TimeInterval TimeInterval, double Arg );

char inline lower(const ASAAC_Time Time1, const ASAAC_Time Time2);
char inline lowerequal(const ASAAC_Time Time1, const ASAAC_Time Time2);
char inline equal( const ASAAC_Time Time1, const ASAAC_Time Time2 );
char inline greaterequal(const ASAAC_Time Time1, const ASAAC_Time Time2);
char inline greater(const ASAAC_Time Time1, const ASAAC_Time Time2);

ASAAC_Time inline FloatToTime( double Value );
ASAAC_Time inline TimespecToTime( struct timespec Value );



#endif /*TIMEOPERATIONS_H_*/
