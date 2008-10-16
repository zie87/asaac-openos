/** 
 * @ingroup AsaacCLib 
 *  
 * $Revision: 1.1 $
 *
 * \author (C) Copyright 2008 ESG GmbH, Fuerstenfeldbruck, Germany.
 *
 * \file AsaacTime.c
 *
 * \brief Implements Character Sequence functions.
 * 
 * \details Implements Character Sequence functions, converting routines from and to a CharSeq, append, find, etc.
 *  
 */

#include "AsaacTime.h"


ASAAC_ReturnStatus Time_assign( ASAAC_Time *handle, ASAAC_Time *value )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL && value != NULL)
    {
    	*handle = *value;
    	status = ASAAC_SUCCESS;
    }
    
    return status;
}

ASAAC_ReturnStatus Time_assignZero( ASAAC_Time *handle )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL)
    {
	    handle->sec = ASAAC_ZERO_SECONDS;
	    handle->nsec = ASAAC_ZERO_NANOSECONDS;
	    
	    status = ASAAC_SUCCESS;
    }
    
    return status;
}
  
ASAAC_ReturnStatus Time_assignInfinity( ASAAC_Time *handle )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL)
    {
	    handle->sec = ASAAC_INFINITY_SECONDS;
	    handle->nsec = ASAAC_INFINITY_NANOSECONDS;
	    
	    status = ASAAC_SUCCESS;
    }
    return status;
}

ASAAC_ReturnStatus Time_assignUInt64( ASAAC_Time *handle, uint64_t *value )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL && value != NULL)
    {
    	if ( (*value / ASAAC_NANOSECONDS_OF_ONE_SECOND) < ASAAC_INFINITY_SECONDS )
    	{
		    handle->sec = (uint32_t)(*value / ASAAC_NANOSECONDS_OF_ONE_SECOND);
		    handle->nsec = (uint32_t)(*value % ASAAC_NANOSECONDS_OF_ONE_SECOND);
		
		    status = ASAAC_SUCCESS;
    	}
    }

    return status;
}

ASAAC_ReturnStatus Time_assignTimespec( ASAAC_Time *handle, struct timespec *value )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL && value != NULL)
    {
	    handle->sec  = value->tv_sec;
	    handle->nsec = value->tv_nsec;
	
	    status = ASAAC_SUCCESS;
    }

    return status;
}


ASAAC_ReturnStatus Time_add( ASAAC_Time *handle, ASAAC_TimeInterval *interval)
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle == NULL || interval == NULL)
    {
    	status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS - handle->sec) < interval->sec)
    {
        status = ASAAC_ERROR;        
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < handle->nsec) || (0 > handle->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < handle->sec) || (0 > handle->sec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < interval->nsec) || (0 > interval->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < interval->sec) || (0 > interval->sec))
    {
        status = ASAAC_ERROR;
    }
    else
    {
        handle->sec  += interval->sec;
        handle->nsec += interval->nsec;
        
        if ( handle->nsec > ASAAC_NANOSECONDS_OF_ONE_SECOND )
        {
            handle->sec ++;
            handle->nsec -= ASAAC_NANOSECONDS_OF_ONE_SECOND;
        }
    
        status = ASAAC_SUCCESS;
    }

    return status;
}

ASAAC_ReturnStatus Time_subtract( ASAAC_Time *handle, ASAAC_TimeInterval *interval)
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle == NULL || interval == NULL)
    {
    	status = ASAAC_ERROR;
    }
    else if (handle->sec < interval->sec)
    {
        status = ASAAC_ERROR;        
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < handle->nsec) || (0 > handle->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < handle->sec) || (0 > handle->sec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < interval->nsec) || (0 > interval->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < interval->sec) || (0 > interval->sec))
    {
        status = ASAAC_ERROR;
    }
    else
    {
        if ( interval->nsec > handle->nsec )
        {
            handle->sec --;
            handle->nsec += ASAAC_NANOSECONDS_OF_ONE_SECOND;
        }
    
        handle->sec  -= interval->sec;
        handle->nsec -= interval->nsec;

        status = ASAAC_SUCCESS;
    }

    return status;
}

ASAAC_ReturnStatus Time_subtractTime( ASAAC_Time *handle, ASAAC_Time *time, ASAAC_TimeInterval *result)
{
    ASAAC_ReturnStatus status;
    
    if (handle == NULL || time == NULL || result == NULL)
    {
    	status = ASAAC_ERROR;
    }
    else if (handle->sec < time->sec)
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < handle->nsec) || (0 > handle->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < handle->sec) || (0 > handle->sec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < time->nsec) || (0 > time->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < time->sec) || (0 > time->sec))
    {
        status = ASAAC_ERROR;
    }
    else
    {
        result->sec = handle->sec;
        result->nsec = handle->nsec;
        
        if ( time->nsec > result->nsec )
        {
            result->sec --;
            result->nsec += ASAAC_NANOSECONDS_OF_ONE_SECOND;
        }
        
        result->sec  -= time->sec;
        result->nsec -= time->nsec;

        status = ASAAC_SUCCESS;
    }

    return status;
}


CompareReturnStatus Time_compare( ASAAC_Time *handle , ASAAC_Time *comparing_time)
{
    CompareReturnStatus status = CRS_Undefined;
    ASAAC_ReturnStatus returnStatus;
    uint64_t value, comparing_value;

    returnStatus = Time_convertToUInt64( handle, &value );
    
    if (returnStatus != ASAAC_ERROR)
    {
    	returnStatus = Time_convertToUInt64( comparing_time, &comparing_value );
    }
    
    if (returnStatus != ASAAC_ERROR)
    {
	    if (value == comparing_value)
	    {
	        status = CRS_Equal;
	    }
	    else if (value < comparing_value)
	    {
	        status = CRS_Lower;
	    }
	    else
	    {
	        status = CRS_Greater;
	    }
    }
    
    return status;
}

ASAAC_ReturnStatus Time_convertToUInt64( ASAAC_Time *handle, uint64_t *target )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;

    if (handle == NULL || target == NULL )
    {
    	status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < handle->nsec) || (0 > handle->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < handle->sec) || (0 > handle->sec))
    {
        status = ASAAC_ERROR;
    }
    else
    {
	    *target = ((uint64_t)handle->sec * ASAAC_NANOSECONDS_OF_ONE_SECOND) + (uint64_t)handle->nsec;
	    status = ASAAC_SUCCESS;
    }
    return status;
}

ASAAC_ReturnStatus Time_convertToTimespec( ASAAC_Time *handle, struct timespec *target )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle == NULL || target == NULL )
    {
    	status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_NANOSECONDS < handle->nsec) || (0 > handle->nsec))
    {
        status = ASAAC_ERROR;
    }
    else if ((ASAAC_INFINITY_SECONDS < handle->sec) || (0 > handle->sec))
    {
        status = ASAAC_ERROR;
    }
    else
    {
	    target->tv_sec = handle->sec;
	    target->tv_nsec = handle->nsec;
	
	    status = ASAAC_SUCCESS;
    }
    
    return status;
}

