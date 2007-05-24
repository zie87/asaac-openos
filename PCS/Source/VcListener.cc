#include "VcListener.hh"

// #include "Interface/APOS-sleep.hh"

#include <iostream>

using namespace std;



VcListener::VcListener()
{
	m_NextFreeSlot = 0;
}


VcListener::~VcListener()
{
}


ASAAC_ReturnStatus VcListener::addListeningConsumer( ASAAC_PublicId LocalVc, VcMessageConsumer& Consumer )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_ListeningVcInfo[ Index ].LocalVc == LocalVc ) return ASAAC_ERROR;
	}
	
	m_ListeningVcInfo[ m_NextFreeSlot ].LocalVc  = LocalVc;
	m_ListeningVcInfo[ m_NextFreeSlot ].Consumer = &Consumer;
	
	m_NextFreeSlot ++;
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus VcListener::removeListeningConsumer( ASAAC_PublicId LocalVc )
{
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		if ( m_ListeningVcInfo[ Index ].LocalVc == LocalVc )
		{
			m_ListeningVcInfo[ Index ] = m_ListeningVcInfo[ m_NextFreeSlot - 1 ];
			m_NextFreeSlot--;
			
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}


ASAAC_TimedReturnStatus VcListener::listen( const ASAAC_TimeInterval& Timeout )
{
	if(m_NextFreeSlot == 0)
	{		
		return ASAAC_TM_SUCCESS;
	}
	
	ASAAC_PublicIdSet ListeningVcs;
	ASAAC_PublicIdSet ReceivingVcs;
	
	for ( unsigned long i = 0; i < ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE; i++ )
	{
		ListeningVcs.vc_id[ i ] = 0;
		ReceivingVcs.vc_id[ i ] = 0;
	}
	
	
	for ( unsigned long Index = 0; Index < m_NextFreeSlot; Index ++ )
	{
		ListeningVcs.vc_id[ Index ] = m_ListeningVcInfo[ Index ].LocalVc;
		#ifdef _DEBUG_
		cout << "VcListener::listen() adding local VC " << m_ListeningVcInfo[ Index ].LocalVc << endl;
		#endif
	}
	#ifdef _DEBUG_
	cout << "VcListener::listen()" << endl;
	#endif 
	ASAAC_TimedReturnStatus Result = ASAAC_APOS_waitOnMultiChannel( &ListeningVcs, 1, &ReceivingVcs, &Timeout );
		
	if ( Result == ASAAC_TM_ERROR ) 
	{
		#ifdef _DEBUG_
		cout << "VcListener::listen() ASAAC_APOS_waitOnMultiChannel failed" << endl;
		#endif
		return Result;
	}
	else if( Result == ASAAC_TM_TIMEOUT ) 
	{
		#ifdef _DEBUG_
		cout << "VcListener::listen() ASAAC_APOS_waitOnMultiChannel timed out" << endl;
		#endif
		return Result;
	}
	
	#ifdef _DEBUG_
	cout << "VcListener::listen() ASAAC_APOS_waitOnMultiChannel returned with SUCCESS" << endl;
	#endif 
	
	//ASAAC_TimeInterval TimeInstant = {0,0};
	
	for ( unsigned long Index = 0; ReceivingVcs.vc_id[ Index ] != 0; Index ++ )
	{
		for ( unsigned long VcIndex = 0; VcIndex < m_NextFreeSlot; VcIndex ++ )
		{
			if ( m_ListeningVcInfo[ VcIndex ].LocalVc == ReceivingVcs.vc_id[ Index ] )
			{
				ASAAC_Address BufferAddress;
				unsigned long DataLength;
				
				ASAAC_APOS_receiveBuffer( ReceivingVcs.vc_id[ Index ], &TimeIntervalInstant, &BufferAddress, &DataLength );
				
				#ifdef _DEBUG_
				cout << "VcListener::listen() processVcMessage (" << ReceivingVcs.vc_id[ Index ]<< "," << BufferAddress<< "," << DataLength << ")" << endl;
				#endif 
			
				m_ListeningVcInfo[ VcIndex ].Consumer->processVcMessage( ReceivingVcs.vc_id[ Index ], BufferAddress, DataLength );
				
				
				ASAAC_APOS_unlockBuffer( ReceivingVcs.vc_id[ Index ], BufferAddress );
				
				break;
			}
		}
	}
	
	return ASAAC_TM_SUCCESS;
}


