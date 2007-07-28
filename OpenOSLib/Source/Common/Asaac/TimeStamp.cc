#include "TimeStamp.hh"

#include "OpenOSObject.hh"

#include "Managers/TimeManager.hh"

TimeStamp::TimeStamp(unsigned long seconds, unsigned long nano_seconds)
{
	reset();
	addSeconds(seconds);
	addNanoSeconds(nano_seconds);
}

TimeStamp::TimeStamp(ASAAC_TimeInterval Interval)
{
	*this = Now().addInterval(Interval);
}

TimeStamp::TimeStamp(ASAAC_Time Time)
{
	reset();
	addSeconds(Time.sec);
	addNanoSeconds(Time.nsec);
}

TimeStamp::TimeStamp(timespec Time)
{
	reset();
	addSeconds(Time.tv_sec);
	addNanoSeconds(Time.tv_nsec);
}

TimeStamp::~TimeStamp()
{
}

TimeStamp & TimeStamp::addHours(long hours)
{
	return addMinutes(hours * 60);
}

TimeStamp & TimeStamp::addMinutes(long minutes)
{
	return addSeconds(minutes * 60);
}

TimeStamp & TimeStamp::addSeconds(long seconds)
{
	if ((abs(m_Time.sec) < abs(seconds)) && 
	    (seconds < 0))
		throw FatalException("Time value beyond zero"); //beyond zero
	
	if ((OS_TIME_INFINITY_SECONDS - m_Time.sec) < seconds)
		m_Time.sec = OS_TIME_INFINITY_SECONDS;
	else m_Time.sec += seconds;	
	
	return *this;
}

TimeStamp & TimeStamp::addMilliSeconds(long milli_seconds)
{
	return addMicroSeconds(milli_seconds * 1000);
}

TimeStamp & TimeStamp::addMicroSeconds(long micro_seconds)
{
	return addNanoSeconds(micro_seconds * 1000);
}

TimeStamp & TimeStamp::addNanoSeconds(long nano_seconds)
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

TimeStamp & TimeStamp::addInterval(ASAAC_TimeInterval Interval)
{
	addSeconds(Interval.sec);
	addNanoSeconds(Interval.nsec);
	return *this;
}

TimeStamp & TimeStamp::addInterval(timespec Interval)
{
	addSeconds(Interval.tv_sec);
	addNanoSeconds(Interval.tv_nsec);
	return *this;
}

TimeStamp & TimeStamp::subInterval(ASAAC_TimeInterval Interval)
{
	addSeconds(-Interval.sec);
	addNanoSeconds(-Interval.nsec);
	return *this;
}

TimeStamp & TimeStamp::subInterval(timespec Interval)
{
	addSeconds(-Interval.tv_sec);
	addNanoSeconds(-Interval.tv_nsec);
	return *this;
}

const timespec TimeStamp::timespec_Time()
{
	timespec result;
	result.tv_sec = m_Time.sec;
	result.tv_nsec = m_Time.nsec;
	return result;
}

const tm TimeStamp::tm_Time()
{
	time_t time = timespec_Time().tv_sec;
	tm* tm_ptr = gmtime(&time);
	tm result = *tm_ptr;
	return result;
}

const ASAAC_Time TimeStamp::asaac_Time()
{
	return m_Time;
}
 
const ASAAC_TimeInterval TimeStamp::asaac_Interval()
{
	ASAAC_Time time;
	TimeManager::getAbsoluteLocalTime(time);
	
	ASAAC_TimeInterval interval;
	interval.sec = m_Time.sec-time.sec;
	interval.nsec = m_Time.sec-time.nsec;
	
	if (interval.nsec < 0)
	{
		interval.nsec+= 1000000000;
		interval.sec--;
	} 
	
	if (interval.sec < 0)
	{
		interval.sec = 0;
		interval.nsec = 0;
	}
	
	return interval;
}

const unsigned long long TimeStamp::sec()
{
	return m_Time.sec;
}

const unsigned long long TimeStamp::nsec()
{
	return m_Time.nsec;
}
 
bool TimeStamp::isInfinity()
{
	return ((m_Time.sec == OS_TIME_INFINITY_SECONDS) && (m_Time.nsec == OS_TIME_INFINITY_NANOSECONDS));
}

bool TimeStamp::isZero()
{
	return ((m_Time.sec == 0) && (m_Time.nsec == 0));
} 
 
TimeStamp & TimeStamp::operator=(ASAAC_Time Time)
{
	m_Time = Time;
	return *this;
}

TimeStamp & TimeStamp::operator=(timespec Time)
{
	m_Time.sec = Time.tv_sec;
	m_Time.nsec = Time.tv_nsec;
	return *this;
}

TimeStamp & TimeStamp::operator=(TimeStamp Time)
{
	m_Time = Time.asaac_Time();
	return *this;
}

TimeStamp & TimeStamp::operator+=(ASAAC_TimeInterval Interval)
{
	return addInterval(Interval);
}

TimeStamp & TimeStamp::operator+=(timespec Interval)
{
	return addInterval(Interval);
}

TimeStamp & TimeStamp::operator-=(ASAAC_TimeInterval Interval)
{
	return subInterval(Interval);
}

TimeStamp & TimeStamp::operator-=(timespec Interval)
{
	return subInterval(Interval);
}

bool TimeStamp::operator<(ASAAC_Time Time)
{
	return (*this < TimeStamp(Time));
}

bool TimeStamp::operator<(timespec Time)
{
	return (*this < TimeStamp(Time));
}

bool TimeStamp::operator<(TimeStamp Time)
{
	return ((sec() < Time.sec()) ||
			((sec() == Time.sec()) && (nsec() < Time.nsec())) );
}

bool TimeStamp::operator<=(ASAAC_Time Time)
{
	return (*this <= TimeStamp(Time));
}

bool TimeStamp::operator<=(timespec Time)
{
	return (*this <= TimeStamp(Time));
}

bool TimeStamp::operator<=(TimeStamp Time)
{
	return ((sec() < Time.sec()) ||
			((sec() == Time.sec()) && (nsec() <= Time.nsec())) );
}

bool TimeStamp::operator==(ASAAC_Time Time)
{
	return (*this == TimeStamp(Time));
}

bool TimeStamp::operator==(timespec Time)
{
	return (*this == TimeStamp(Time));
}

bool TimeStamp::operator==(TimeStamp Time)
{
	return ((sec() == Time.sec()) && (nsec() == Time.nsec()) );
}

bool TimeStamp::operator>=(ASAAC_Time Time)
{
	return (*this >= TimeStamp(Time));
}

bool TimeStamp::operator>=(timespec Time)
{
	return (*this >= TimeStamp(Time));
}

bool TimeStamp::operator>=(TimeStamp Time)
{
	return ((sec() > Time.sec()) ||
			((sec() == Time.sec()) && (nsec() >= Time.nsec())) );
}

bool TimeStamp::operator>(ASAAC_Time Time)
{
	return (*this > TimeStamp(Time));
}

bool TimeStamp::operator>(timespec Time)
{
	return (*this > TimeStamp(Time));
}

bool TimeStamp::operator>(TimeStamp Time)
{
	return ((sec() > Time.sec()) ||
			((sec() == Time.sec()) && (nsec() > Time.nsec())) );
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

