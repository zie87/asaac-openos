#ifndef TIMEOPERATIONS_HH_
#define TIMEOPERATIONS_HH_


ASAAC_Time inline operator+(const ASAAC_Time& Time1, const ASAAC_Time& Time2)
{
	ASAAC_Time thisTime;
	
	thisTime.nsec = Time1.nsec + Time2.nsec;
	thisTime.sec  = Time1.sec  + Time2.sec;
	
	if ( thisTime.nsec > 1000000000 )
	{
		thisTime.sec ++;
		thisTime.nsec -= 1000000000;
	}
	
	return thisTime;
}


ASAAC_Time inline operator-(const ASAAC_Time& Time1, const ASAAC_Time& Time2)
{
	ASAAC_Time thisTime = Time1;
	
	if ( Time2.nsec > Time1.nsec )
	{
		thisTime.sec --;
		thisTime.nsec += 1000000000;
	}
	
	thisTime.sec  -= Time2.sec;
	thisTime.nsec -= Time2.nsec;
	
	return thisTime;
}

ASAAC_Time inline floatingTime( float FloatingTime )
{
	ASAAC_Time thisTime;
	
	thisTime.sec  = (unsigned long) FloatingTime;
	thisTime.nsec = (unsigned long) (( FloatingTime - thisTime.sec ) * 1000000000 );

	return thisTime;
}	


ASAAC_Time inline operator*(unsigned long Multiplier, const ASAAC_Time& MultiTime)
{
	ASAAC_Time thisTime;
	
	thisTime.nsec = MultiTime.nsec * Multiplier;
	thisTime.sec  = MultiTime.sec  * Multiplier;
	
	while ( thisTime.nsec > 1000000000 )
	{
		thisTime.sec ++;
		thisTime.nsec -= 1000000000;
	}
	
	return thisTime;
}



bool inline operator>(const ASAAC_Time& Time1, const ASAAC_Time& Time2)
{
	if ( Time1.sec > Time2.sec ) return true;
	
	if ( Time1.sec == Time2.sec )
	{
		if ( Time1.nsec > Time2.nsec ) return true;
	}
	
	return false;
}


bool inline operator<(const ASAAC_Time& Time1, const ASAAC_Time& Time2)
{
	if ( Time1.sec < Time2.sec ) return true;
	
	if ( Time1.sec == Time2.sec )
	{
		if ( Time1.nsec < Time2.nsec ) return true;
	}
	
	return false;
}


bool inline operator==( const ASAAC_Time& Time1, const ASAAC_Time& Time2 )
{
	return (( Time1.sec == Time2.sec ) && ( Time1.nsec == Time2.nsec ));
}



#endif /*TIMEOPERATIONS_HH_*/
