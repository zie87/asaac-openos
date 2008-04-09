class TimeInterval;

#ifndef TIMEINTERVAL_HH_
#define TIMEINTERVAL_HH_

#include "OpenOSIncludes.hh"
#include "TimeStamp.hh"

typedef enum {
	NanoSeconds,
	MicroSeconds,
	MilliSeconds,
	Seconds,
	Minutes,
	Hours
} TimeUnit;

class TimeInterval
{
public:
	TimeInterval();
	TimeInterval(const unsigned long value, const TimeUnit unit);
	TimeInterval(const ASAAC_TimeInterval interval);

	virtual ~TimeInterval();
	
	TimeInterval & assign(const unsigned long value, const TimeUnit unit);
	TimeInterval & assign(const ASAAC_TimeInterval interval);

	TimeInterval & add(const long value, const TimeUnit unit);
	TimeInterval & addHours(const long hours);
	TimeInterval & addMinutes(const long minutes);
	TimeInterval & addSeconds(const long seconds);
	TimeInterval & addMilliSeconds(const long milli_seconds);
	TimeInterval & addMicroSeconds(const long micro_seconds);
	TimeInterval & addNanoSeconds(const long nano_seconds);

	TimeInterval & addInterval(const ASAAC_TimeInterval Interval);
	TimeInterval & addInterval(const TimeInterval &Interval);

	TimeInterval & subInterval(const ASAAC_TimeInterval Interval);
	TimeInterval & subInterval(const TimeInterval &Interval);

	const timespec timespec_Interval() const;
	const ASAAC_TimeInterval asaac_Interval() const;
	const unsigned long long sec() const;
	const unsigned long long nsec() const;
	
	bool isInstant() const;
	bool isInfinity() const;

	TimeInterval & operator=(const ASAAC_TimeInterval Interval);
	TimeInterval & operator=(const TimeInterval &Interval);
	
	TimeInterval & operator+=(const ASAAC_TimeInterval Interval);
	TimeInterval & operator+=(const TimeInterval &Interval);
	
	TimeInterval & operator-=(const ASAAC_TimeInterval Interval);
	TimeInterval & operator-=(const TimeInterval &Interval);
	
	bool operator<(const ASAAC_TimeInterval Interval) const;
	bool operator<(const TimeInterval Interval) const;
	
	bool operator<=(const ASAAC_TimeInterval Interval) const;
	bool operator<=(const TimeInterval Interval) const;
	
	bool operator==(const ASAAC_TimeInterval Interval) const;
	bool operator==(const TimeInterval Interval) const;
	
	bool operator>=(const ASAAC_TimeInterval Interval) const;
	bool operator>=(const TimeInterval Interval) const;
	
	bool operator>(const ASAAC_TimeInterval Interval) const;
	bool operator>(const TimeInterval Interval) const;

	TimeStamp operator+(const TimeStamp &data) const;
	TimeStamp operator+(const ASAAC_Time data) const;
	TimeInterval operator+(const TimeInterval &data) const;
	TimeInterval operator+(const ASAAC_TimeInterval data) const;
	
	TimeInterval operator-(const TimeInterval &data) const;
	TimeInterval operator-(const ASAAC_TimeInterval data) const;
	
	static TimeInterval  Instant();
	static TimeInterval  Infinity();

private:
	void reset();
	
	ASAAC_TimeInterval m_Interval;
};

const ASAAC_TimeInterval TimeIntervalInstant = {0,0};
const ASAAC_TimeInterval TimeIntervalInfinity = {OS_TIME_INFINITY_SECONDS, OS_TIME_INFINITY_NANOSECONDS};

#endif /*TIMEINTERVAL_HH_*/
