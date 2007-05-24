#include "TimeManager.hh"

using namespace std;

TimeManager::TimeManager()
{
}

TimeManager::~TimeManager()
{
}


ASAAC_ReturnStatus TimeManager::getAbsoluteLocalTime( ASAAC_Time& absolute_local_time )
{
	timespec TimeNow;
	
	if ( clock_gettime( CLOCK_REALTIME, &TimeNow ) != 0 )
	{
		return ASAAC_ERROR;
	}
	
	absolute_local_time.sec  = TimeNow.tv_sec;
	absolute_local_time.nsec = TimeNow.tv_nsec;
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus TimeManager::getAbsoluteGlobalTime( ASAAC_Time& absolute_global_time )
{
	return getAbsoluteLocalTime( absolute_global_time );
}


ASAAC_ReturnStatus TimeManager::getRelativeLocalTime( ASAAC_Time& relative_local_time )
{
	return getAbsoluteLocalTime( relative_local_time );
}


