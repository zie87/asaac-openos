#include "MOSIncludes.h"

/***********************************************************************************/
/*                          DATA DEFINITIONS                                       */      
/***********************************************************************************/

typedef struct {
	ASAAC_EventType event_type;
	ASAAC_PublicId  callback_id;
	ASAAC_Address   callback;
	ASAAC_Bool      enabled;
} CallbackData;

CallbackData CallbackArray[MOS_MAX_NUMBER_OF_CALLBACKS];
long CallbackArraySize = 0;


/***********************************************************************************/
/*                        DATA ACCESS FUNCTIONS                                    */      
/***********************************************************************************/

long addCallback()
{
	if ( CallbackArraySize == MOS_MAX_NUMBER_OF_CALLBACKS)
		return -1;
	
	long Index = CallbackArraySize;
	
	CallbackArraySize++;
	
	return Index;
}

long indexOfCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id)
{
	long Index = 0;
	
	for (; Index < CallbackArraySize; Index++)
	{
		if ((CallbackArray[Index].event_type == event_type) &&
		    (CallbackArray[Index].callback_id == callback_id))
			return Index;
	}
	
	return -1;
}

void removeCallback(const long Index)
{
	if ( Index < 0)
		return;
	
	if ( Index >= CallbackArraySize)
		return;
	
	memmove(&(CallbackArray[Index]), &(CallbackArray[Index+1]), ( (CallbackArraySize-1) - Index) * sizeof(CallbackData));
	
	CallbackArraySize--;
}



/***********************************************************************************/
/*                        MOS IMPLEMENTATION                                       */      
/***********************************************************************************/

ASAAC_MSLStatus ASAAC_MOS_registerCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id, const ASAAC_Address callback)
{
	if ( callback == NULL)
		return ASAAC_MSL_CALLBACK_INVALID_PARAMETER;

	long Index = addCallback();
	
	if ( Index == -1 )
		return ASAAC_MSL_CALLBACK_FAILED;
	
	CallbackArray[Index].event_type = event_type;
	CallbackArray[Index].callback_id = callback_id;
	CallbackArray[Index].callback = callback;
	CallbackArray[Index].enabled = ASAAC_BOOL_FALSE;

	return ASAAC_MSL_OK;
}


ASAAC_MSLStatus ASAAC_MOS_enableCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id)
{
	long Index = indexOfCallback( event_type, callback_id );
	
	if (Index == -1)
		return ASAAC_MSL_CALLBACK_INVALID_PARAMETER;
	
	CallbackArray[Index].enabled = ASAAC_BOOL_TRUE;
	
	return ASAAC_MSL_OK;
}


ASAAC_MSLStatus ASAAC_MOS_disableCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id)
{
	long Index = indexOfCallback( event_type, callback_id );
	
	if (Index == -1)
		return ASAAC_MSL_CALLBACK_INVALID_PARAMETER;
	
	CallbackArray[Index].enabled = ASAAC_BOOL_FALSE;
	
	return ASAAC_MSL_OK;
}


ASAAC_MSLStatus ASAAC_MOS_deleteCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id)
{
	long Index = indexOfCallback( event_type, callback_id );
	
	if (Index == -1)
		return ASAAC_MSL_CALLBACK_INVALID_PARAMETER;

	removeCallback( Index );
	
	return ASAAC_MSL_OK;
}


void ASAAC_MOS_callbackHandler(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id, const ASAAC_Address event_info_data)
{
	long Index = indexOfCallback( event_type, callback_id );
	
	if (Index == -1)
		return;
	
	if (CallbackArray[Index].enabled == ASAAC_BOOL_FALSE)
		return;
	 
	typedef void(*CallbackFunction)(const ASAAC_Address);
	CallbackFunction Callback = (CallbackFunction)CallbackArray[Index].callback;
	
	Callback( event_info_data );
}





