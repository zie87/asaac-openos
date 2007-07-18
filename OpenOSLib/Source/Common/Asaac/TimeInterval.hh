class TimeInterval;

#ifndef TIMEINTERVAL_HH_
#define TIMEINTERVAL_HH_

#include "OpenOSIncludes.hh"

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
	TimeInterval(unsigned long value, TimeUnit unit);
	TimeInterval(ASAAC_TimeInterval interval);

	virtual ~TimeInterval();
	
	TimeInterval & assign(unsigned long value, TimeUnit unit);
	TimeInterval & assign(ASAAC_TimeInterval interval);

	TimeInterval & add(long value, TimeUnit unit);
	TimeInterval & addHours(long hours);
	TimeInterval & addMinutes(long minutes);
	TimeInterval & addSeconds(long seconds);
	TimeInterval & addMilliSeconds(long milli_seconds);
	TimeInterval & addMicroSeconds(long micro_seconds);
	TimeInterval & addNanoSeconds(long nano_seconds);

	const timespec timespec_Interval();
	const ASAAC_TimeInterval asaac_Interval();
	
	bool isInstant();
	bool isInfinity();
	
	static TimeInterval  Instant();
	static TimeInterval  Infinity();

private:
	void reset();
	
	ASAAC_TimeInterval m_Interval;
};

const ASAAC_TimeInterval TimeIntervalInstant = {0,0};
const ASAAC_TimeInterval TimeIntervalInfinity = {OS_TIME_INFINITY_SECONDS, OS_TIME_INFINITY_NANOSECONDS};

#endif /*TIMEINTERVAL_HH_*/
