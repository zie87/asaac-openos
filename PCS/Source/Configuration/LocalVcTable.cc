#include "LocalVcTable.hh"

LocalVcTable::LocalVcTable()
{
	m_NextFreeMapping = 0;
}


void LocalVcTable::initialize()
{
	m_NextFreeMapping = 0;
}


void LocalVcTable::deinitialize()
{
	m_NextFreeMapping = 0;
}


ASAAC_ReturnStatus	LocalVcTable::addLocalVc( ASAAC_PublicId LocalVc, ASAAC_PublicId GlobalVc )
{
	if ( m_NextFreeMapping == ( SIZE_OF_VC_TABLE - 1 )) return ASAAC_ERROR;
	
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; Index ++ )
	{
		if (( m_Mapping[ Index ].LocalVc == LocalVc ) || ( m_Mapping[ Index ].GlobalVc == GlobalVc )) return ASAAC_ERROR;
	}
	
	m_Mapping[ m_NextFreeMapping ].LocalVc  = LocalVc;
	m_Mapping[ m_NextFreeMapping ].GlobalVc = GlobalVc;
	
	m_NextFreeMapping ++;

	return ASAAC_SUCCESS;
}

unsigned long LocalVcTable::getNumberOfVc()
{
	return m_NextFreeMapping;	
}

ASAAC_ReturnStatus LocalVcTable::getVc(unsigned long n, ASAAC_PublicId & id)
{
	if(n >= m_NextFreeMapping)
		return ASAAC_ERROR;
	
	id =  m_Mapping[n].LocalVc;
	
	return ASAAC_SUCCESS;	
}
	
ASAAC_ReturnStatus	LocalVcTable::removeLocalVc( ASAAC_PublicId LocalVc )
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; Index ++ )
	{
		if ( m_Mapping[ Index ].LocalVc == LocalVc )
		{
			m_Mapping[ Index ] = m_Mapping[ m_NextFreeMapping - 1 ];
			m_NextFreeMapping --;
			
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}
	
	
ASAAC_PublicId		LocalVcTable::getLocalVc( ASAAC_PublicId GlobalVc )
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; Index ++ )
	{
		if ( m_Mapping[ Index ].GlobalVc == GlobalVc ) return m_Mapping[ Index ].LocalVc;
	}
	
	return 0;
}


ASAAC_PublicId		LocalVcTable::getGlobalVc( ASAAC_PublicId LocalVc )
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; Index ++ )
	{
		if ( m_Mapping[ Index ].LocalVc == LocalVc ) return m_Mapping[ Index ].GlobalVc;
	}
	
	return 0;
}


ASAAC_PublicId		LocalVcTable::getFreeLocalVc( ASAAC_PublicId MinValue )
{
	for ( unsigned long Value = MinValue ;; Value ++ )
	{
		if ( getGlobalVc( Value ) == 0 ) return Value;
	}
}

