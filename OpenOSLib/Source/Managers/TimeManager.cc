#include "TimeManager.hh"

#include "OpenOSObject.hh"

TimeManager::TimeManager()
{
}

TimeManager::~TimeManager()
{
}


void TimeManager::getAbsoluteLocalTime( ASAAC_Time& absolute_local_time )
{
	try
	{
		timespec TimeNow;
		
		if ( clock_gettime( CLOCK_REALTIME, &TimeNow ) != 0 )
			throw OSException( strerror(errno), LOCATION );
		
		absolute_local_time.sec  = TimeNow.tv_sec;
		absolute_local_time.nsec = TimeNow.tv_nsec;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving absolute local time", LOCATION);
		
		throw;
	}
}


void TimeManager::getAbsoluteGlobalTime( ASAAC_Time& absolute_global_time )
{
	try
	{
		getAbsoluteLocalTime( absolute_global_time );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving absolute global time", LOCATION);
		
		throw;
	}
}


void TimeManager::getRelativeLocalTime( ASAAC_Time& relative_local_time )
{
	try
	{
		getAbsoluteLocalTime( relative_local_time );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error retrieving relative local time", LOCATION);
		
		throw;
	}
}


