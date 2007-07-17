#include "VcDescriptionTable.hh"

VcDescriptionTable::VcDescriptionTable()
{
	m_NextFreeSlot = 0;
}


void VcDescriptionTable::initialize()
{
	m_NextFreeSlot = 0;
}


void VcDescriptionTable::deinitialize()
{
	m_NextFreeSlot = 0;
}


ASAAC_ReturnStatus VcDescriptionTable::addVcDescription( const ASAAC_VcDescription& Description )
{
	if ( m_NextFreeSlot == PCS_MAX_NUMBER_OF_VCS ) 
		return ASAAC_ERROR;

	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].global_vc_id == Description.global_vc_id ) return ASAAC_ERROR;
	}
	
	m_DescriptionTable[ m_NextFreeSlot ] = Description;
	m_NextFreeSlot ++;

	return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus VcDescriptionTable::removeVcDescription( ASAAC_PublicId GlobalVcId )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].global_vc_id == GlobalVcId )
		{
			m_DescriptionTable[ Index ] = m_DescriptionTable[ m_NextFreeSlot - 1 ];
			m_NextFreeSlot --;
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}
	

ASAAC_ReturnStatus VcDescriptionTable::getVcDescription( ASAAC_PublicId GlobalVcId, ASAAC_VcDescription& Description )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_DescriptionTable[ Index ].global_vc_id == GlobalVcId )
		{
			Description = m_DescriptionTable[ Index ];
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}

