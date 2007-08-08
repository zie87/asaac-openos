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
	ASAAC_TimerReturnStatus Result = ASAAC_MOS_getAbsoluteLocalTime( &absolute_local_time );
	
	if ( Result == ASAAC_MOS_TIMER_CALL_FAILED )
		throw OSException("Error retrieving time", LOCATION);
}


void TimeManager::getAbsoluteGlobalTime( ASAAC_Time& absolute_global_time )
{
	ASAAC_TimerReturnStatus Result = ASAAC_MOS_getAbsoluteGlobalTime( &absolute_global_time );
	
	if ( Result == ASAAC_MOS_TIMER_CALL_FAILED )
		throw OSException("Error retrieving time", LOCATION);
}


void TimeManager::getRelativeLocalTime( ASAAC_Time& relative_local_time )
{
	ASAAC_TimerReturnStatus Result = ASAAC_MOS_getRelativeLocalTime( &relative_local_time );
	
	if ( Result == ASAAC_MOS_TIMER_CALL_FAILED )
		throw OSException("Error retrieving time", LOCATION);
}


