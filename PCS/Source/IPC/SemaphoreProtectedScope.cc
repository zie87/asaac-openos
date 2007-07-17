#include "SemaphoreProtectedScope.hh"

#include "PcsCIncludes.hh"

SemaphoreProtectedScope::SemaphoreProtectedScope( ASAAC_PublicId SemaphoreId, const ASAAC_TimeInterval& Timeout )
{
	m_Semaphore = SemaphoreId;
	ASAAC_TimedReturnStatus Result = ASAAC_APOS_waitForSemaphore( SemaphoreId, &Timeout );
	
	if ( Result == ASAAC_TM_TIMEOUT ) 
		throw PcsException( 0, 0, "Semaphore wait exceeded timeout" );
	
	if ( Result == ASAAC_TM_ERROR   ) 
		throw PcsException( 0, 0, "Semaphore wait failed" );
}

SemaphoreProtectedScope::~SemaphoreProtectedScope()
{
	ASAAC_APOS_postSemaphore( m_Semaphore );
}
