class TimeStamp;

#ifndef TIMESTAMP_HH_
#define TIMESTAMP_HH_

#include "OpenOSIncludes.hh"

typedef enum 
{
	AbsoluteLocal,
	AbsoluteGlobal,
	RelativeLocal
} TimeType;

class TimeStamp
{
public:
	TimeStamp(unsigned long seconds = 0, unsigned long nano_seconds = 0);
	TimeStamp(ASAAC_TimeInterval Interval);
	TimeStamp(ASAAC_Time Time);
	TimeStamp(timespec Time);
	
	virtual ~TimeStamp();

	TimeStamp & addHours(long hours);
	TimeStamp & addMinutes(long minutes);
	TimeStamp & addSeconds(long seconds);
	TimeStamp & addMilliSeconds(long milli_seconds);
	TimeStamp & addMicroSeconds(long micro_seconds);
	TimeStamp & addNanoSeconds(long nano_seconds);

	TimeStamp & addInterval(ASAAC_TimeInterval Interval);
	TimeStamp & addInterval(timespec Interval);

	TimeStamp & subInterval(ASAAC_TimeInterval Interval);
	TimeStamp & subInterval(timespec Interval);
	
	const timespec timespec_Time();
	const tm tm_Time();
	const ASAAC_Time asaac_Time();
	const ASAAC_TimeInterval asaac_Interval();
	const unsigned long long sec();
	const unsigned long long nsec();
	
	bool isInfinity();
	bool isZero();
	
	TimeStamp & operator=(ASAAC_Time Time);
	TimeStamp & operator=(timespec Time);
	TimeStamp & operator=(TimeStamp Time);
	
	TimeStamp & operator+=(ASAAC_TimeInterval Interval);
	TimeStamp & operator+=(timespec Interval);
	
	TimeStamp & operator-=(ASAAC_TimeInterval Interval);
	TimeStamp & operator-=(timespec Interval);
	
	bool operator<(ASAAC_Time Time);
	bool operator<(timespec Time);
	bool operator<(TimeStamp Time);
	
	bool operator<=(ASAAC_Time Time);
	bool operator<=(timespec Time);
	bool operator<=(TimeStamp Time);
	
	bool operator==(ASAAC_Time Time);
	bool operator==(timespec Time);
	bool operator==(TimeStamp Time);
	
	bool operator>=(ASAAC_Time Time);
	bool operator>=(timespec Time);
	bool operator>=(TimeStamp Time);
	
	bool operator>(ASAAC_Time Time);
	bool operator>(timespec Time);
	bool operator>(TimeStamp Time);
	
	static TimeStamp  Zero();
	static TimeStamp  Now(TimeType type = AbsoluteLocal);
	static TimeStamp  Instant();
	static TimeStamp  Infinity();
private:
	void reset();
	
	ASAAC_Time m_Time;
};

const ASAAC_Time TimeZero = TimeStamp::Zero().asaac_Time();
const ASAAC_Time TimeInfinity = TimeStamp::Infinity().asaac_Time();

#endif /*TIMESTAMP_H_*/
