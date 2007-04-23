#include "TimeInterval.hh"

#include "OpenOS.hh"

const long TimeInfinitySec = 2147483647;
const long TimeInfinityNSec = 999999999;


TimeInterval::TimeInterval()
{
	reset();
}

TimeInterval::TimeInterval(unsigned long value, TimeUnit unit)
{
	assign(value, unit);
}

TimeInterval::TimeInterval(ASAAC_TimeInterval interval)
{
	assign(interval);
}

TimeInterval::~TimeInterval()
{
}

TimeInterval & TimeInterval::assign(unsigned long value, TimeUnit unit)
{
	reset();
	return add(value, unit);
}

TimeInterval & TimeInterval::assign(ASAAC_TimeInterval interval)
{
	reset();
	addSeconds(interval.sec);
	addNanoSeconds(interval.nsec);
	return *this;
}

TimeInterval & TimeInterval::add(long value, TimeUnit unit)
{
	switch (unit) {
		case NanoSeconds: addNanoSeconds(value); break;
		case MicroSeconds: addMicroSeconds(value); break;
		case MilliSeconds: addMilliSeconds(value); break;
		case Seconds: addSeconds(value); break;
		case Minutes: addMinutes(value); break;
		case Hours: addHours(value); break;
		default: break;
	}
	return *this;
}

TimeInterval & TimeInterval::addHours(long hours)
{
	return addMinutes(hours * 60);
}

TimeInterval & TimeInterval::addMinutes(long minutes)
{
	return addSeconds(minutes * 60);
}

TimeInterval & TimeInterval::addSeconds(long seconds)
{
	if ((TimeInfinitySec - m_Interval.sec) < seconds)
		m_Interval.sec = TimeInfinitySec;
	else m_Interval.sec += seconds;	
	
	return *this;
}

TimeInterval & TimeInterval::addMilliSeconds(long milli_seconds)
{
	return addMicroSeconds(milli_seconds * 1000);
}

TimeInterval & TimeInterval::addMicroSeconds(long micro_seconds)
{
	return addNanoSeconds(micro_seconds * 1000);
}

TimeInterval & TimeInterval::addNanoSeconds(long nano_seconds)
{
	//div_t d = div(nano_seconds, (long)1000000000);
	ldiv_t d = div(nano_seconds, (long)1000000000);
	
	addSeconds(d.quot);	
	m_Interval.nsec += d.rem;
	
	if (m_Interval.nsec < 0)
	{
		m_Interval.nsec = 1000000000 + m_Interval.nsec;
		addSeconds(-1);
	}
	
	if (m_Interval.nsec > 999999999)
	{
		m_Interval.nsec = -1000000000 + m_Interval.nsec;
		addSeconds(+1);
	}

	if (m_Interval.sec == TimeInfinitySec)
		m_Interval.nsec = TimeInfinityNSec;

	return *this;
}

const ASAAC_TimeInterval TimeInterval::asaac_Interval()
{
	return m_Interval;
}

bool TimeInterval::isInstant()
{
	return ((m_Interval.sec == 0) && (m_Interval.nsec == 0));	
}

bool TimeInterval::isInfinity()
{
	return ((m_Interval.sec == TimeInfinitySec) && (m_Interval.nsec == TimeInfinityNSec));	
}

TimeInterval  TimeInterval::Instant()
{
	return TimeInterval(0, Seconds);
}

TimeInterval  TimeInterval::Infinity()
{
	TimeInterval Interval;
	Interval.m_Interval.sec = TimeInfinitySec;
	Interval.m_Interval.nsec = TimeInfinityNSec;
	return Interval;
}

void TimeInterval::reset()
{
	m_Interval.sec = 0;
	m_Interval.nsec = 0;
}

