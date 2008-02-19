#include "MappingTable.hh"

MappingTable::MappingTable()
{
	m_NextFreeMapping = 0;
}


void MappingTable::initialize()
{
	m_NextFreeMapping = 0;
}


void MappingTable::deinitialize()
{
	m_NextFreeMapping = 0;
}



ASAAC_ReturnStatus	MappingTable::addTcMapping( const ASAAC_VcToTcMappingDescription & Description )
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; Index ++ )
	{
		if (( m_MappingTable[ Index ].global_vc_id == Description.global_vc_id ) && 
		    ( m_MappingTable[ Index ].tc_id        == Description.tc_id ))
		{
			return ASAAC_ERROR;
		}
	}
	
	m_MappingTable[ m_NextFreeMapping ] = Description;
	
	m_NextFreeMapping ++;
	
	return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus	MappingTable::removeTcMapping( ASAAC_PublicId VcId, ASAAC_PublicId TcId )
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; Index ++ )
	{
		if (( m_MappingTable[ Index ].global_vc_id == VcId ) && ( m_MappingTable[ Index ].tc_id == TcId ))
		{
			m_MappingTable[ Index ] = m_MappingTable[ m_NextFreeMapping - 1 ];
			m_NextFreeMapping --;
			
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}
	

ASAAC_ReturnStatus	MappingTable::getTcMapping( ASAAC_PublicId VcId, ASAAC_PublicIdSet& TcMapping, unsigned long& Number )
{
	unsigned long MappingIndex = 0;
	
	for ( unsigned long TableIndex = 0; TableIndex < m_NextFreeMapping; TableIndex ++ )
	{
		if ( m_MappingTable[ TableIndex ].global_vc_id == VcId )
		{
			TcMapping[ MappingIndex ] = m_MappingTable[ TableIndex ].tc_id;
			MappingIndex++;
		}
	}

	Number = MappingIndex;

	if ( MappingIndex == 0 ) return ASAAC_ERROR;
	
	
	for ( ; MappingIndex < ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE ; MappingIndex ++ )
	{
		TcMapping[ MappingIndex ] = 0;
	}


	return ASAAC_SUCCESS;
}
	

ASAAC_ReturnStatus	MappingTable::getVcMapping( ASAAC_PublicId TcId, ASAAC_PublicIdSet& VcMapping, unsigned long& Number )
{
	unsigned long MappingIndex = 0;
	
	for ( unsigned long TableIndex = 0; TableIndex < m_NextFreeMapping; TableIndex ++ )
	{
		if ( m_MappingTable[ TableIndex ].tc_id == TcId )
		{
			VcMapping[ MappingIndex ] = m_MappingTable[ TableIndex ].global_vc_id;
			MappingIndex++;
		}
	}

	Number = MappingIndex;

	if ( MappingIndex == 0 ) 
		return ASAAC_ERROR;
	
	for ( ; MappingIndex < ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE ; MappingIndex ++ )
	{
		VcMapping[ MappingIndex ] = 0;
	}

	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus	MappingTable::getCDRFlag( ASAAC_PublicId VcId, ASAAC_PublicId TcId, ASAAC_Bool& Flag )
{
	for ( unsigned long TableIndex = 0; TableIndex < m_NextFreeMapping; TableIndex ++ )
	{
		if (( m_MappingTable[ TableIndex ].global_vc_id == VcId ) &&
			( m_MappingTable[ TableIndex ].tc_id        == TcId ))
		{
			Flag = m_MappingTable[ TableIndex ].is_data_representation;
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}

