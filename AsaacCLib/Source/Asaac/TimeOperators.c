#include "TimeOperators.h"


ASAAC_Time TimeInstant()
{
	ASAAC_Time ac_system_time = TimeZero;
	
	ASAAC_MOS_getAbsoluteLocalTime( &ac_system_time );
	
	return ac_system_time;
}



ASAAC_Time addIntervalToTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval)
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



ASAAC_TimeInterval addIntervalToInterval(const ASAAC_TimeInterval Interval1, const ASAAC_TimeInterval Interval2)
{
	
}



ASAAC_Time subtractIntervalFromTime(const ASAAC_Time Time, const ASAAC_TimeInterval Interval)
{
	
}



ASAAC_TimeInterval subtractTimeFromTime(const ASAAC_Time Time1, const ASAAC_Time Time2)
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



ASAAC_TimeInterval multiplyIntervalWithArg(const ASAAC_TimeInterval Interval, double Arg )
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



ASAAC_TimeInterval devideIntervalByArg(const ASAAC_TimeInterval TimeInterval, double Arg )
{
	
}



char lower(const ASAAC_Time Time1, const ASAAC_Time Time2)
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



char lowerequal(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	return ( lower( Time1, Time2 ) || equal( Time1, Time2 ) );	
}



char equal( const ASAAC_Time Time1, const ASAAC_Time Time2 )
{
	return (( Time1.sec == Time2.sec ) && ( Time1.nsec == Time2.nsec ));
}



char greaterequal(const ASAAC_Time Time1, const ASAAC_Time Time2)
{
	return ( greater( Time1, Time2 ) || equal( Time1, Time2 ) );
}



char greater(const ASAAC_Time Time1, const ASAAC_Time Time2)
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



ASAAC_Time FloatToTime( double Value )
{
	ASAAC_Time thisTime;
	
	thisTime.sec  = (unsigned long) Value;
	thisTime.nsec = (unsigned long) (( Value - thisTime.sec ) * 1000000000 );

	return thisTime;
}	



ASAAC_Time TimespecToTime( struct timespec Value )
{
	ASAAC_Time thisTime;
	
	thisTime.sec  = Value.tv_sec;
	thisTime.nsec = Value.tv_nsec;

	return thisTime;
}



ASAAC_Time TimevalToTime( struct timeval Value )
{
	ASAAC_Time thisTime = TimeInstant();
	
	ASAAC_TimeInterval Interval = TimevalToInterval( Value );
	
	return addIntervalToTime( thisTime, Interval );
}



ASAAC_TimeInterval FloatToInterval( double Value )
{
	ASAAC_TimeInterval thisInterval;
	
	thisInterval.sec  = (unsigned long) Value;
	thisInterval.nsec = (unsigned long) (( Value - thisInterval.sec ) * 1000000000 );

	return thisInterval;
}



ASAAC_TimeInterval TimespecToInterval( struct timespec Value )
{
	ASAAC_Time Time = TimespecToTime( Value );
	ASAAC_Time Now = TimeInstant();
	
	return subtractTimeFromTime(Time, Now);
}



ASAAC_TimeInterval TimevalToInterval( struct timeval Value )
{
	ASAAC_TimeInterval thisInterval;
	
	thisInterval.sec  = Value.tv_sec;
	thisInterval.nsec = Value.tv_usec * 1000;

	return thisInterval;
}



double TimeToFloat( ASAAC_Time Value )
{
	double thisTime = Value.sec + (Value.nsec / 1000000000);
	
	return thisTime;
}



struct timespec TimeToTimespec( ASAAC_Time Value )
{
	struct timespec thisTime;

	thisTime.tv_sec = Value.sec;
	thisTime.tv_nsec = Value.nsec;
	
	return thisTime;
}



struct timeval TimeToTimeval( ASAAC_Time Value )
{
	ASAAC_TimeInterval Interval = subtractTimeFromTime( Value, TimeInstant() );
	
	struct timeval thisTime;
	
	thisTime.tv_sec = Interval.sec;
	thisTime.tv_usec = Interval.nsec / 1000;
	
	return thisTime;
}



double IntervalToFloat( ASAAC_TimeInterval Value )
{
	double thisTime = Value.sec + (Value.nsec / 1000000000);
	
	return thisTime;
}



struct timespec IntervalToTimespec( ASAAC_TimeInterval Value )
{
	struct timespec thisTime;

	thisTime.tv_sec = Value.sec;
	thisTime.tv_nsec = Value.nsec;
	
	return thisTime;
}



struct timeval IntervalToTimeval( ASAAC_TimeInterval Value )
{
	struct timeval thisTime;
	
	thisTime.tv_sec = Value.sec;
	thisTime.tv_usec = Value.nsec / 1000;
	
	return thisTime;
}

