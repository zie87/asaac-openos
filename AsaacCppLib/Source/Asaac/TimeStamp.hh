class TimeStamp;

#ifndef TIMESTAMP_HH_
#define TIMESTAMP_HH_

#include "AsaacCppIncludes.hh"
#include "TimeInterval.hh"

typedef enum 
{
	AbsoluteLocal,
	AbsoluteGlobal,
	RelativeLocal
} TimeType;

class TimeStamp
{
public:
	TimeStamp(const unsigned long seconds = 0, const unsigned long nano_seconds = 0);
	TimeStamp(const ASAAC_TimeInterval Interval);
	TimeStamp(const ASAAC_Time Time);
	TimeStamp(const timespec Time);
	
	virtual ~TimeStamp();

	TimeStamp & addHours(const long hours);
	TimeStamp & addMinutes(const long minutes);
	TimeStamp & addSeconds(const long seconds);
	TimeStamp & addMilliSeconds(const long milli_seconds);
	TimeStamp & addMicroSeconds(const long micro_seconds);
	TimeStamp & addNanoSeconds(const long nano_seconds);

	TimeStamp & addInterval(const ASAAC_TimeInterval Interval);
	TimeStamp & addInterval(const TimeInterval &Interval);
	TimeStamp & addInterval(const timespec Interval);

	TimeStamp & subInterval(const ASAAC_TimeInterval Interval);
	TimeStamp & subInterval(const TimeInterval &Interval);
	TimeStamp & subInterval(const timespec Interval);
	
	const timespec timespec_Time() const;
	const tm tm_Time() const;
	const ASAAC_Time asaac_Time() const;
	const ASAAC_TimeInterval asaac_Interval() const;
	const unsigned long long sec() const;
	const unsigned long long nsec() const;
	
	bool isInfinity() const;
	bool isZero() const;
	
	TimeStamp & operator=(const ASAAC_Time Time);
	TimeStamp & operator=(const timespec Time);
	TimeStamp & operator=(const TimeStamp Time);
	
	TimeStamp & operator+=(const ASAAC_TimeInterval Interval);
	TimeStamp & operator+=(const timespec Interval);
	
	TimeStamp & operator-=(const ASAAC_TimeInterval Interval);
	TimeStamp & operator-=(const timespec Interval);
	
	bool operator<(const ASAAC_Time Time) const;
	bool operator<(const timespec Time) const;
	bool operator<(const TimeStamp Time) const;
	
	bool operator<=(const ASAAC_Time Time) const;
	bool operator<=(const timespec Time) const;
	bool operator<=(const TimeStamp Time) const;
	
	bool operator==(const ASAAC_Time Time) const;
	bool operator==(const timespec Time) const;
	bool operator==(const TimeStamp Time) const;
	
	bool operator>=(const ASAAC_Time Time) const;
	bool operator>=(const timespec Time) const;
	bool operator>=(const TimeStamp Time) const;
	
	bool operator>(const ASAAC_Time Time) const;
	bool operator>(const timespec Time) const;
	bool operator>(const TimeStamp Time) const;

	TimeStamp operator+(const TimeInterval &data) const;
	TimeStamp operator+(const ASAAC_TimeInterval data) const;
	
	TimeInterval operator-(const TimeStamp &data) const;
	TimeInterval operator-(const ASAAC_Time data) const;
	TimeStamp operator-(const TimeInterval &data) const;
	TimeStamp operator-(const ASAAC_TimeInterval data) const;
	
	static TimeStamp  Zero();
	static TimeStamp  Now(TimeType type = AbsoluteLocal);
	static TimeStamp  Instant();
	static TimeStamp  Infinity();
private:
	void reset();
	
	ASAAC_Time m_Time;
};

const ASAAC_Time TimeZero = {0,0};
const ASAAC_Time TimeInfinity = {OS_TIME_INFINITY_SECONDS, OS_TIME_INFINITY_NANOSECONDS};

#endif /*TIMESTAMP_H_*/
