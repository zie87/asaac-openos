#include "TimeOperators.h"



ASAAC_Time inline addIntervalToTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval)
{
	ASAAC_Time thisTime;
	
	thisTime.nsec = Time.nsec + Interval.nsec;
	thisTime.sec  = Time.sec  + Interval.sec;
	
	if ( thisTime.nsec > 1000000000 )
	{
		thisTime.sec ++;
		thisTime.nsec -= 1000000000;
	}
	
	return thisTime;
}



ASAAC_TimeInterval inline addIntervalToInterval(const ASAAC_TimeInterval Interval1, const ASAAC_TimeInterval Interval2)
{
	
}



ASAAC_Time inline subtractIntervalFromTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval)
{
	
}



ASAAC_TimeInterval inline subtractTimeFromTime(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	ASAAC_TimeInterval thisInterval;
	thisInterval.sec = Time1.sec;
	thisInterval.nsec = Time1.nsec;
	
	if ( Time2.nsec > Time1.nsec )
	{
		thisInterval.sec --;
		thisInterval.nsec += 1000000000;
	}
	
	thisInterval.sec  -= Time2.sec;
	thisInterval.nsec -= Time2.nsec;
	
	return thisInterval;
}



ASAAC_TimeInterval inline multiplyIntervalWithArg(const ASAAC_TimeInterval Interval, double Arg )
{
	ASAAC_TimeInterval thisInterval;
	
	thisInterval.nsec = Interval.nsec * Arg;
	thisInterval.sec  = Interval.sec  * Arg;
	
	while ( thisInterval.nsec > 1000000000 )
	{
		thisInterval.sec ++;
		thisInterval.nsec -= 1000000000;
	}
	
	return thisInterval;
}



ASAAC_TimeInterval inline devideIntervalByArg(const ASAAC_TimeInterval TimeInterval, double Arg )
{
	
}



char inline lower(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	if ( Time1.sec < Time2.sec ) 
		return 1;
	
	if ( Time1.sec == Time2.sec )
	{
		if ( Time1.nsec < Time2.nsec ) 
			return 1;
	}
	
	return 0;
}



char inline lowerequal(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	
}



char inline equal( const ASAAC_Time Time1, const ASAAC_Time Time2 )
{
	return (( Time1.sec == Time2.sec ) && ( Time1.nsec == Time2.nsec ));
}



char inline greaterequal(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	
}



char inline greater(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	if ( Time1.sec > Time2.sec ) 
		return 1;
	
	if ( Time1.sec == Time2.sec )
	{
		if ( Time1.nsec > Time2.nsec ) 
			return 1;
	}
	
	return 0;
}



ASAAC_Time inline FloatToTime( double Value )
{
	ASAAC_Time thisTime;
	
	thisTime.sec  = (unsigned long) Value;
	thisTime.nsec = (unsigned long) (( Value - thisTime.sec ) * 1000000000 );

	return thisTime;
}	

