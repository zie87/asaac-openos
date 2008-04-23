#include "TcDescriptionTable.hh"

TcDescriptionTable::TcDescriptionTable()
{
	m_NextFreeSlot = 0;
}


void TcDescriptionTable::initialize()
{
	m_NextFreeSlot = 0;
}


void TcDescriptionTable::deinitialize()
{
	m_NextFreeSlot = 0;	
}


ASAAC_ReturnStatus TcDescriptionTable::addTcDescription(const  ASAAC_TcDescription& Description )
{
	if ( m_NextFreeSlot == PCS_MAX_NUMBER_OF_TCS ) 
		return ASAAC_ERROR;

	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].tc_id == Description.tc_id ) 
			return ASAAC_ERROR;
	}
	
	m_DescriptionTable[ m_NextFreeSlot ] = Description;
	m_RateLimits[ m_NextFreeSlot ].sec = 0;
	m_RateLimits[ m_NextFreeSlot ].nsec = 0;
	m_NextFreeSlot ++;

	return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus TcDescriptionTable::removeTcDescription( ASAAC_PublicId TcId )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].tc_id == TcId )
		{
			m_DescriptionTable[ Index ] = m_DescriptionTable[ m_NextFreeSlot - 1 ];
			m_NextFreeSlot --;
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}
	

ASAAC_ReturnStatus TcDescriptionTable::getTcDescription( ASAAC_PublicId TcId, ASAAC_TcDescription& Description )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].tc_id == TcId )
		{
			Description = m_DescriptionTable[ Index ];
			return ASAAC_SUCCESS;
		}
	}

#ifdef _DEBUG_       
    cerr << "PCS::TcDescriptionTable::getTcDescription() TcId not found: " << TcId << endl;
#endif
	
	return ASAAC_ERROR;
}

ASAAC_ReturnStatus TcDescriptionTable::getTcIndex( ASAAC_PublicId TcId, unsigned long & TcIndex )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].tc_id == TcId )
		{
			TcIndex = Index ;
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}

unsigned long TcDescriptionTable::getNumberOfTc()
{
	return m_NextFreeSlot;	
}

ASAAC_ReturnStatus TcDescriptionTable::getTcDescription( ASAAC_TcDescription& Description, unsigned long index)
{
	if(index >= m_NextFreeSlot)
		return ASAAC_ERROR;
	
	Description = m_DescriptionTable[ index ];
	
	return ASAAC_SUCCESS;	
}

ASAAC_ReturnStatus TcDescriptionTable::getTcRateLimit( ASAAC_TimeInterval& RateLimit, ASAAC_PublicId& TcId, unsigned long Index)
{
	if(Index >= m_NextFreeSlot)
		return ASAAC_ERROR;
	
	TcId = m_DescriptionTable[ Index ].tc_id;
	
	if(m_DescriptionTable[ Index ].is_receiver == ASAAC_BOOL_TRUE)
		return ASAAC_ERROR;
		
	RateLimit = m_RateLimits[ Index ];
	
	return ASAAC_SUCCESS;
};
	
ASAAC_ReturnStatus TcDescriptionTable::addTcRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& RateLimit)
{
	unsigned long TcIndex;
	if(getTcIndex(TcId, TcIndex) == ASAAC_SUCCESS)
	{
		m_RateLimits[ TcIndex ] = RateLimit;
		return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;
};
