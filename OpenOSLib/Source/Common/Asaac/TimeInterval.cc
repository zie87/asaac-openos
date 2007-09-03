#include "TimeInterval.hh"

#include "OpenOSObject.hh"

TimeInterval::TimeInterval()
{
	reset();
}

TimeInterval::TimeInterval(const unsigned long value, const TimeUnit unit)
{
	assign(value, unit);
}

TimeInterval::TimeInterval(const ASAAC_TimeInterval interval)
{
	assign(interval);
}

TimeInterval::~TimeInterval()
{
}

TimeInterval & TimeInterval::assign(const unsigned long value, const TimeUnit unit)
{
	reset();
	return add(value, unit);
}

TimeInterval & TimeInterval::assign(const ASAAC_TimeInterval interval)
{
	reset();
	addSeconds(interval.sec);
	addNanoSeconds(interval.nsec);
	return *this;
}

TimeInterval & TimeInterval::add(const long value, const TimeUnit unit)
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

TimeInterval & TimeInterval::addHours(const long hours)
{
	return addMinutes(hours * 60);
}

TimeInterval & TimeInterval::addMinutes(const long minutes)
{
	return addSeconds(minutes * 60);
}

TimeInterval & TimeInterval::addSeconds(const long seconds)
{
	if ((OS_TIME_INFINITY_SECONDS - m_Interval.sec) < seconds)
		m_Interval.sec = OS_TIME_INFINITY_SECONDS;
	else m_Interval.sec += seconds;
	
	if ( m_Interval.sec < 0 )
		m_Interval.sec = 0;
	
	return *this;
}

TimeInterval & TimeInterval::addMilliSeconds(const long milli_seconds)
{
	return addMicroSeconds(milli_seconds * 1000);
}

TimeInterval & TimeInterval::addMicroSeconds(const long micro_seconds)
{
	return addNanoSeconds(micro_seconds * 1000);
}

TimeInterval & TimeInterval::addNanoSeconds(const long nano_seconds)
{
	//div_t d = div(nano_seconds, (long)1000000000);
	ldiv_t d = div(nano_seconds, (long)1000000000);
	
	addSeconds(d.quot);	
	m_Interval.nsec += d.rem;
	
	if (m_Interval.nsec < 0)
	{
		if (m_Interval.sec > 0)
		{
			m_Interval.nsec = 1000000000 + m_Interval.nsec;
			addSeconds(-1);
		}
		else
		{
			m_Interval.nsec = 0;
		}
	}
	
	if (m_Interval.nsec > 999999999)
	{
		m_Interval.nsec = -1000000000 + m_Interval.nsec;
		addSeconds(+1);
	}

	if (m_Interval.sec == OS_TIME_INFINITY_SECONDS)
		m_Interval.nsec = OS_TIME_INFINITY_NANOSECONDS;

	return *this;
}

TimeInterval & TimeInterval::addInterval(const ASAAC_TimeInterval Interval)
{
	addSeconds( Interval.sec );
	addNanoSeconds( Interval.nsec );
	return *this;
}

TimeInterval & TimeInterval::addInterval(const TimeInterval &Interval)
{
	addSeconds( Interval.sec() );
	addNanoSeconds( Interval.nsec() );
	return *this;
}

TimeInterval & TimeInterval::subInterval(const ASAAC_TimeInterval Interval)
{
	addSeconds( -Interval.sec );
	addNanoSeconds( -Interval.nsec );
	return *this;
}

TimeInterval & TimeInterval::subInterval(const TimeInterval &Interval)
{
	addSeconds( -Interval.sec() );
	addNanoSeconds( -Interval.nsec() );
	return *this;
}

const timespec TimeInterval::timespec_Interval() const
{
	timespec result;
	result.tv_sec = m_Interval.sec;
	result.tv_nsec = m_Interval.nsec;
	return result;
}

const ASAAC_TimeInterval TimeInterval::asaac_Interval() const
{
	return m_Interval;
}

const unsigned long long TimeInterval::sec() const
{
	return m_Interval.sec;
}

const unsigned long long TimeInterval::nsec() const
{
	return m_Interval.nsec;
} 

bool TimeInterval::isInstant() const
{
	return ((m_Interval.sec == 0) && (m_Interval.nsec == 0));	
}

bool TimeInterval::isInfinity() const
{
	return ((m_Interval.sec == OS_TIME_INFINITY_SECONDS) && (m_Interval.nsec == OS_TIME_INFINITY_NANOSECONDS));	
}

TimeInterval & TimeInterval::operator=(const ASAAC_TimeInterval Interval)
{
	m_Interval = Interval;
	return *this;
}

TimeInterval & TimeInterval::operator=(const TimeInterval &Interval)
{
	m_Interval = Interval.m_Interval;
	return *this;
}

TimeInterval & TimeInterval::operator+=(const ASAAC_TimeInterval Interval)
{
	addInterval( Interval );
	return *this;
}

TimeInterval & TimeInterval::operator+=(const TimeInterval &Interval)
{
	addInterval( Interval );
	return *this;
}

TimeInterval & TimeInterval::operator-=(const ASAAC_TimeInterval Interval)
{
	subInterval( Interval );
	return *this;
}

TimeInterval & TimeInterval::operator-=(const TimeInterval &Interval)
{
	subInterval( Interval );
	return *this;
}

bool TimeInterval::operator<(const ASAAC_TimeInterval Interval) const
{
	return (*this < TimeInterval(Interval));
}

bool TimeInterval::operator<(const TimeInterval Interval) const
{
	return ((sec() < Interval.sec()) ||
			((sec() == Interval.sec()) && (nsec() < Interval.nsec())) );
}

bool TimeInterval::operator<=(const ASAAC_TimeInterval Interval) const
{
	return (*this <= TimeInterval(Interval));
}

bool TimeInterval::operator<=(const TimeInterval Interval) const
{
	return ((sec() < Interval.sec()) ||
			((sec() == Interval.sec()) && (nsec() <= Interval.nsec())) );
}

bool TimeInterval::operator==(const ASAAC_TimeInterval Interval) const
{
	return (*this == TimeInterval(Interval));
}

bool TimeInterval::operator==(const TimeInterval Interval) const
{
	return ((sec() == Interval.sec()) && (nsec() == Interval.nsec()) );	
}

bool TimeInterval::operator>=(const ASAAC_TimeInterval Interval) const
{
	return (*this >= TimeInterval(Interval));
}

bool TimeInterval::operator>=(const TimeInterval Interval) const
{
	return ((sec() > Interval.sec()) ||
			((sec() == Interval.sec()) && (nsec() >= Interval.nsec())) );
}

bool TimeInterval::operator>(const ASAAC_TimeInterval Interval) const
{
	return (*this > TimeInterval(Interval));
}

bool TimeInterval::operator>(const TimeInterval Interval) const
{
	return ((sec() > Interval.sec()) ||
			((sec() == Interval.sec()) && (nsec() > Interval.nsec())) );
}

TimeStamp TimeInterval::operator+(const TimeStamp &data) const
{
	return (data + *this);
}

TimeStamp TimeInterval::operator+(const ASAAC_Time data) const
{
	return (*this + TimeStamp(data));
}

TimeInterval TimeInterval::operator+(const TimeInterval &data) const
{
	TimeInterval ti(*this);
	return ti.addInterval(data);
}

TimeInterval TimeInterval::operator+(const ASAAC_TimeInterval data) const
{
	TimeInterval ti(*this);
	return ti.addInterval(data);
}

TimeInterval TimeInterval::operator-(const TimeInterval &data) const
{
	TimeInterval ti(*this);
	return ti.subInterval(data);
}

TimeInterval TimeInterval::operator-(const ASAAC_TimeInterval data) const
{
	TimeInterval ti(*this);
	return ti.subInterval(data);
}

TimeInterval  TimeInterval::Instant()
{
	return TimeInterval(0, Seconds);
}

TimeInterval  TimeInterval::Infinity()
{
	return TimeInterval(TimeIntervalInfinity);
}

void TimeInterval::reset()
{
	m_Interval.sec = 0;
	m_Interval.nsec = 0;
}

