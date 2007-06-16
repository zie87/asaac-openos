#include "OpenOS.hh"

#include "Managers/TimeManager.hh"
#include "ProcessManagement/ProcessManager.hh"

/* *************************************************************************************************** */
/*                                 T I M E   M A N A G E M E N T                                       */
/* *************************************************************************************************** */

ASAAC_ReturnStatus ASAAC_APOS_getAbsoluteLocalTime( ASAAC_Time* absolute_local_time )
{
	return TimeManager::getAbsoluteLocalTime( *absolute_local_time );
}


ASAAC_ReturnStatus ASAAC_APOS_getAbsoluteGlobalTime( ASAAC_Time* absolute_global_time )
{
	return TimeManager::getAbsoluteGlobalTime( *absolute_global_time );
}


ASAAC_ReturnStatus ASAAC_APOS_getRelativeLocalTime( ASAAC_Time* relative_local_time )
{
	return TimeManager::getRelativeLocalTime( *relative_local_time );
}

