#include "TimeStamp.hh"

#include "OpenOSObject.hh"

#include "Managers/TimeManager.hh"

TimeStamp::TimeStamp(const unsigned long seconds, const unsigned long nano_seconds)
{
	reset();
	addSeconds(seconds);
	addNanoSeconds(nano_seconds);
}

TimeStamp::TimeStamp(const ASAAC_TimeInterval Interval)
{
	*this = Now().addInterval(Interval);
}

TimeStamp::TimeStamp(const ASAAC_Time Time)
{
	reset();
	addSeconds(Time.sec);
	addNanoSeconds(Time.nsec);
}

TimeStamp::TimeStamp(const timespec Time)
{
	reset();
	addSeconds(Time.tv_sec);
	addNanoSeconds(Time.tv_nsec);
}

TimeStamp::~TimeStamp()
{
}

TimeStamp & TimeStamp::addHours(const long hours)
{
	return addMinutes(hours * 60);
}

TimeStamp & TimeStamp::addMinutes(const long minutes)
{
	return addSeconds(minutes * 60);
}

TimeStamp & TimeStamp::addSeconds(const long seconds)
{
	if ((abs(m_Time.sec) < abs(seconds)) && 
	    (seconds < 0))
		throw FatalException("Time value beyond zero"); //beyond zero
	
	if ((OS_TIME_INFINITY_SECONDS - m_Time.sec) < seconds)
		m_Time.sec = OS_TIME_INFINITY_SECONDS;
	else m_Time.sec += seconds;	
	
	return *this;
}

TimeStamp & TimeStamp::addMilliSeconds(const long milli_seconds)
{
	return addMicroSeconds(milli_seconds * 1000);
}

TimeStamp & TimeStamp::addMicroSeconds(const long micro_seconds)
{
	return addNanoSeconds(micro_seconds * 1000);
}

TimeStamp & TimeStamp::addNanoSeconds(const long nano_seconds)
{
	ldiv_t d = div(nano_seconds, (long)1000000000);
	
	addSeconds(d.quot);	
	m_Time.nsec += d.rem;
	
	if (m_Time.nsec < 0)
	{
		m_Time.nsec = 1000000000 + m_Time.nsec;
		addSeconds(-1);
	}
	
	if (m_Time.nsec > 999999999)
	{
		m_Time.nsec = -1000000000 + m_Time.nsec;
		addSeconds(+1);
	}

	if (m_Time.sec == OS_TIME_INFINITY_SECONDS)
		m_Time.nsec = OS_TIME_INFINITY_NANOSECONDS;
	
	return *this;
}

TimeStamp & TimeStamp::addInterval(const ASAAC_TimeInterval Interval)
{
	addSeconds( Interval.sec );
	addNanoSeconds( Interval.nsec );
	return *this;
}

TimeStamp & TimeStamp::addInterval(const TimeInterval &Interval)
{
	addSeconds( Interval.sec() );
	addNanoSeconds( Interval.nsec() );
	return *this;
}

TimeStamp & TimeStamp::addInterval(const timespec Interval)
{
	addSeconds( Interval.tv_sec );
	addNanoSeconds( Interval.tv_nsec );
	return *this;
}

TimeStamp & TimeStamp::subInterval(const ASAAC_TimeInterval Interval)
{
	addSeconds( -Interval.sec );
	addNanoSeconds( -Interval.nsec );
	return *this;
}

TimeStamp & TimeStamp::subInterval(const TimeInterval &Interval)
{
	addSeconds( -Interval.sec() );
	addNanoSeconds( -Interval.nsec() );
	return *this;
}

TimeStamp & TimeStamp::subInterval(const timespec Interval)
{
	addSeconds( -Interval.tv_sec );
	addNanoSeconds( -Interval.tv_nsec );
	return *this;
}

const timespec TimeStamp::timespec_Time() const
{
	timespec result;
	result.tv_sec = m_Time.sec;
	result.tv_nsec = m_Time.nsec;
	return result;
}

const tm TimeStamp::tm_Time() const
{
	time_t time = timespec_Time().tv_sec;
	tm* tm_ptr = gmtime(&time);
	tm result = *tm_ptr;
	return result;
}

const ASAAC_Time TimeStamp::asaac_Time() const
{
	return m_Time;
}
 
const ASAAC_TimeInterval TimeStamp::asaac_Interval() const
{
	ASAAC_TimeInterval Interval = (Now() - *this).asaac_Interval();
	
	return Interval;
}

const unsigned long long TimeStamp::sec() const
{
	return m_Time.sec;
}

const unsigned long long TimeStamp::nsec() const
{
	return m_Time.nsec;
}
 
bool TimeStamp::isInfinity() const
{
	return ((m_Time.sec == OS_TIME_INFINITY_SECONDS) && (m_Time.nsec == OS_TIME_INFINITY_NANOSECONDS));
}

bool TimeStamp::isZero() const
{
	return ((m_Time.sec == 0) && (m_Time.nsec == 0));
} 
 
TimeStamp & TimeStamp::operator=(const ASAAC_Time Time)
{
	m_Time = Time;
	return *this;
}

TimeStamp & TimeStamp::operator=(const timespec Time)
{
	m_Time.sec = Time.tv_sec;
	m_Time.nsec = Time.tv_nsec;
	return *this;
}

TimeStamp & TimeStamp::operator=(const TimeStamp Time)
{
	m_Time = Time.asaac_Time();
	return *this;
}

TimeStamp & TimeStamp::operator+=(const ASAAC_TimeInterval Interval)
{
	return addInterval(Interval);
}

TimeStamp & TimeStamp::operator+=(const timespec Interval)
{
	return addInterval(Interval);
}

TimeStamp & TimeStamp::operator-=(const ASAAC_TimeInterval Interval)
{
	return subInterval(Interval);
}

TimeStamp & TimeStamp::operator-=(const timespec Interval)
{
	return subInterval(Interval);
}

bool TimeStamp::operator<(const ASAAC_Time Time) const
{
	return (*this < TimeStamp(Time));
}

bool TimeStamp::operator<(const timespec Time) const
{
	return (*this < TimeStamp(Time));
}

bool TimeStamp::operator<(const TimeStamp Time) const
{
	return ((sec() < Time.sec()) ||
			((sec() == Time.sec()) && (nsec() < Time.nsec())) );
}

bool TimeStamp::operator<=(const ASAAC_Time Time) const
{
	return (*this <= TimeStamp(Time));
}

bool TimeStamp::operator<=(const timespec Time) const
{
	return (*this <= TimeStamp(Time));
}

bool TimeStamp::operator<=(const TimeStamp Time) const
{
	return ((sec() < Time.sec()) ||
			((sec() == Time.sec()) && (nsec() <= Time.nsec())) );
}

bool TimeStamp::operator==(const ASAAC_Time Time) const
{
	return (*this == TimeStamp(Time));
}

bool TimeStamp::operator==(const timespec Time) const
{
	return (*this == TimeStamp(Time));
}

bool TimeStamp::operator==(const TimeStamp Time) const
{
	return ((sec() == Time.sec()) && (nsec() == Time.nsec()) );
}

bool TimeStamp::operator>=(const ASAAC_Time Time) const
{
	return (*this >= TimeStamp(Time));
}

bool TimeStamp::operator>=(const timespec Time) const
{
	return (*this >= TimeStamp(Time));
}

bool TimeStamp::operator>=(const TimeStamp Time) const
{
	return ((sec() > Time.sec()) ||
			((sec() == Time.sec()) && (nsec() >= Time.nsec())) );
}

bool TimeStamp::operator>(const ASAAC_Time Time) const
{
	return (*this > TimeStamp(Time));
}

bool TimeStamp::operator>(const timespec Time) const
{
	return (*this > TimeStamp(Time));
}

bool TimeStamp::operator>(const TimeStamp Time) const
{
	return ((sec() > Time.sec()) ||
			((sec() == Time.sec()) && (nsec() > Time.nsec())) );
}

TimeStamp TimeStamp::operator+(const TimeInterval &data) const
{
	TimeStamp ts(*this);
	return ts.addInterval(data);
}

TimeStamp TimeStamp::operator+(const ASAAC_TimeInterval data) const
{
	TimeStamp ts(*this);
	return ts.addInterval(data);
}

TimeInterval TimeStamp::operator-(const TimeStamp &data) const
{
	return *this - data.asaac_Time();
}

TimeInterval TimeStamp::operator-(const ASAAC_Time data) const
{
	ASAAC_TimeInterval ti_this;
	ti_this.sec  = sec();
	ti_this.nsec = nsec();
	
	ASAAC_TimeInterval ti_data;
	ti_data.sec  = data.sec;
	ti_data.nsec = data.nsec;
	
	TimeInterval This( ti_data );
	
	This -= TimeInterval(ti_this);
	
	return This;
}

TimeStamp TimeStamp::operator-(const TimeInterval &data) const
{
	TimeStamp ts(*this);
	return ts.subInterval(data);
}

TimeStamp TimeStamp::operator-(const ASAAC_TimeInterval data) const
{
	TimeStamp ts(*this);
	return ts.subInterval(data);
}
	
TimeStamp TimeStamp::Zero()
{
	return TimeStamp(0);
}

TimeStamp TimeStamp::Now(TimeType type)
{
	ASAAC_Time t;
	
	switch (type)
	{
		case AbsoluteLocal:  TimeManager::getAbsoluteLocalTime(t); break;
		case AbsoluteGlobal: TimeManager::getAbsoluteGlobalTime(t); break;
		case RelativeLocal:  TimeManager::getRelativeLocalTime(t); break;
		default: break;
	}
	
	return TimeStamp(t);
}

TimeStamp TimeStamp::Instant()
{
	return Now();
}

TimeStamp TimeStamp::Infinity()
{
	return TimeStamp(TimeInfinity);
}

void TimeStamp::reset()
{
	m_Time.sec = 0;
	m_Time.nsec = 0;
}

