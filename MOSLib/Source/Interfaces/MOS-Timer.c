#include "MOSIncludes.h"


ASAAC_TimerReturnStatus ASAAC_MOS_getAbsoluteLocalTime(ASAAC_Time* ac_system_time)
{
	struct timespec TimeNow;
	
	if ( clock_gettime( CLOCK_REALTIME, &TimeNow ) != 0 )
		return ASAAC_MOS_TIMER_CALL_FAILED;
	
	ac_system_time->sec  = TimeNow.tv_sec;
	ac_system_time->nsec = TimeNow.tv_nsec;
	
	return ASAAC_MOS_TIMER_CALL_OK;
}


ASAAC_TimerReturnStatus ASAAC_MOS_getRelativeLocalTime(ASAAC_Time* cfm_time)
{
	struct timespec TimeNow;
	
	if ( clock_gettime( CLOCK_REALTIME, &TimeNow ) != 0 )
		return ASAAC_MOS_TIMER_CALL_FAILED;
	
	cfm_time->sec  = TimeNow.tv_sec;
	cfm_time->nsec = TimeNow.tv_nsec;
	
	return ASAAC_MOS_TIMER_CALL_OK;
}


ASAAC_TimerReturnStatus ASAAC_MOS_getAbsoluteGlobalTime(ASAAC_Time* absolute_global_time)
{
	struct timespec TimeNow;
	
	if ( clock_gettime( CLOCK_REALTIME, &TimeNow ) != 0 )
		return ASAAC_MOS_TIMER_CALL_FAILED;
	
	absolute_global_time->sec  = TimeNow.tv_sec;
	absolute_global_time->nsec = TimeNow.tv_nsec;
	
	return ASAAC_MOS_TIMER_CALL_OK;
}


ASAAC_TimerReturnStatus ASAAC_MOS_configureClock(const ASAAC_ClockInfo* clock_info)
{
	return ASAAC_MOS_TIMER_CALL_FAILED;
}


ASAAC_TimerReturnStatus ASAAC_MOS_attachFederatedClock(const ASAAC_FederatedClockInfo* federated_clock_info)
{
	return ASAAC_MOS_TIMER_CALL_FAILED;
}


ASAAC_TimerReturnStatus ASAAC_MOS_setupTimer(const ASAAC_PublicId timer_id, const ASAAC_Time* time_to_expire, const ASAAC_PublicId callback_id, const ASAAC_AlarmType alarm_type)
{
	return ASAAC_MOS_TIMER_CALL_FAILED;
}


ASAAC_TimerReturnStatus ASAAC_MOS_startTimer(const ASAAC_PublicId timer_id)
{
	return ASAAC_MOS_TIMER_CALL_FAILED;
}


ASAAC_TimerReturnStatus ASAAC_MOS_stopTimer(const ASAAC_PublicId timer_id)
{
	return ASAAC_MOS_TIMER_CALL_FAILED;
}


ASAAC_TimerReturnStatus ASAAC_MOS_readTimer(const ASAAC_PublicId timer_id, ASAAC_Time* time_to_expire)
{
	return ASAAC_MOS_TIMER_CALL_FAILED;
}


