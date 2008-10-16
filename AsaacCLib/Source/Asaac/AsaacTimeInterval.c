/** 
 * @ingroup AsaacCLib 
 *  
 * $Revision: 2484 $
 *
 * \author (C) Copyright 2008 ESG GmbH, Fuerstenfeldbruck, Germany.
 *
 * \file AsaacTimeInterval.c
 *
 * \brief Implements Character Sequence functions.
 * 
 * \details Implements Character Sequence functions, converting routines from and to a CharSeq, append, find, etc.
 *  
 */

#include "AsaacTimeInterval.h"

ASAAC_ReturnStatus TimeInterval_assign( ASAAC_TimeInterval *handle, ASAAC_TimeInterval *value )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL && value != NULL)
    {
    	*handle = *value;
    	status = ASAAC_SUCCESS;
    }
    
    return status;
}
  
ASAAC_ReturnStatus TimeInterval_assignInstant( ASAAC_TimeInterval *handle )
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
  
ASAAC_ReturnStatus TimeInterval_assignInfinity( ASAAC_TimeInterval *handle )
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
  
ASAAC_ReturnStatus TimeInterval_assignFloat128( ASAAC_TimeInterval *handle, float128_t *value )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL && value != NULL)
    {  
	    handle->sec  = (uint32_t) *value;
	    handle->nsec = (uint32_t) (( *value - handle->sec ) * ASAAC_NANOSECONDS_OF_ONE_SECOND );
	
	    status = ASAAC_SUCCESS;
    }
    
    return status;
}

ASAAC_ReturnStatus TimeInterval_assignUInt64( ASAAC_TimeInterval *handle, uint64_t *value )
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


ASAAC_ReturnStatus TimeInterval_assignTimeval( ASAAC_TimeInterval *handle, struct timeval *value )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    if (handle != NULL && value != NULL)
    {
	    handle->sec  = value->tv_sec;
	    handle->nsec = value->tv_usec * 1000;
	
	    status = ASAAC_SUCCESS;
    }
    
    return status;
}


ASAAC_ReturnStatus TimeInterval_add( ASAAC_TimeInterval *handle, const ASAAC_TimeInterval *interval)
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

ASAAC_ReturnStatus TimeInterval_subtract( ASAAC_TimeInterval *handle, const ASAAC_TimeInterval *interval)
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

ASAAC_ReturnStatus TimeInterval_multiply( ASAAC_TimeInterval *handle, float128_t arg )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    float128_t value;

    status = TimeInterval_convertToFloat128( handle, &value );
    
    if (status == ASAAC_SUCCESS)
    {
	    value *= arg;
	
	    status = TimeInterval_assignFloat128( handle, &value );
    }
    return status;
}

ASAAC_ReturnStatus TimeInterval_devide( ASAAC_TimeInterval *handle, float128_t arg )
{
    ASAAC_ReturnStatus status = ASAAC_ERROR;
    
    float128_t value;

    status = TimeInterval_convertToFloat128( handle, &value );
    
    if (status == ASAAC_SUCCESS)
    {
	    value /= arg;
	
	    status = TimeInterval_assignFloat128( handle, &value );
    }
    
    return status;
}


CompareReturnStatus TimeInterval_compare( ASAAC_TimeInterval *handle , ASAAC_TimeInterval *comparing_interval)
{
    CompareReturnStatus status = CRS_Undefined;
    ASAAC_ReturnStatus returnStatus;
    uint64_t value, comparing_value;

    returnStatus = TimeInterval_convertToUInt64( handle, &value );
    
    if (returnStatus != ASAAC_ERROR)
    {
    	returnStatus = TimeInterval_convertToUInt64( comparing_interval, &comparing_value );
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


ASAAC_ReturnStatus TimeInterval_convertToFloat128( ASAAC_TimeInterval *handle, float128_t *target )
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
	    *target = (float128_t)handle->sec + ((float128_t)handle->nsec / ASAAC_NANOSECONDS_OF_ONE_SECOND);
	    status = ASAAC_SUCCESS;
    }
    return status;
}

ASAAC_ReturnStatus TimeInterval_convertToUInt64( ASAAC_TimeInterval *handle, uint64_t *target )
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

ASAAC_ReturnStatus TimeInterval_convertToTimeval( ASAAC_TimeInterval *handle, struct timeval *target )
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
	    target->tv_usec = handle->nsec / 1000;
	
	    status = ASAAC_SUCCESS;
    }
    
    return status;
}




/**********************************************************************/
/* OLD CODE BEGINS HERE                                               */
/**********************************************************************/



ASAAC_TimeInterval multiplyIntervalWithArg(const ASAAC_TimeInterval Interval, double Arg )
{
    ASAAC_TimeInterval thisInterval;
    
    thisInterval.nsec = Interval.nsec * Arg;
    thisInterval.sec  = Interval.sec  * Arg;
    
    while ( thisInterval.nsec > 1000000000 )
    {
        thisInterval.sec ++;
        thisInterval.nsec -= 1000000000;
    }
    
    return thisInterval;
}



ASAAC_TimeInterval devideIntervalByArg(const ASAAC_TimeInterval Interval, double Arg )
{
    ASAAC_TimeInterval thisInterval;
    
    thisInterval.sec = Interval.sec / Arg;
    thisInterval.nsec = Interval.nsec / Arg;
    
    return thisInterval;
}

