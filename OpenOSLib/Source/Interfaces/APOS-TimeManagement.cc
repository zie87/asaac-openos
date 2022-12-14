#include "OpenOSObject.hh"

#include "Managers/TimeManager.hh"
#include "ProcessManagement/ProcessManager.hh"

/* *************************************************************************************************** */
/*                                 T I M E   M A N A G E M E N T                                       */
/* *************************************************************************************************** */

ASAAC_ReturnStatus ASAAC_APOS_getAbsoluteLocalTime( ASAAC_Time* absolute_local_time )
{
	try
	{
		TimeManager::getAbsoluteLocalTime( *absolute_local_time );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getAbsoluteLocalTime", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_getAbsoluteGlobalTime( ASAAC_Time* absolute_global_time )
{
	try
	{
		TimeManager::getAbsoluteGlobalTime( *absolute_global_time );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getAbsoluteGlobalTime", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_getRelativeLocalTime( ASAAC_Time* relative_local_time )
{
	try
	{
		TimeManager::getRelativeLocalTime( *relative_local_time );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getRelativeLocalTime", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

