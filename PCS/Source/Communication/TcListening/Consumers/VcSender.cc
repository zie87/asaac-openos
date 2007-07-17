#include "VcSender.hh"

#include "PcsCIncludes.hh"

#include <assert.h>

#include <iostream>

using namespace std;

VcSender::VcSender() : m_Timeout(TimeIntervalInfinity)
{
}

VcSender::VcSender( const ASAAC_TimeInterval& Timeout ) : m_Timeout( Timeout )
{
}


VcSender::~VcSender()
{
}


ASAAC_ReturnStatus VcSender::processVcMessage( ASAAC_PublicId LocalVc, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_
    cout << "VcSender::processVcMessage("<<LocalVc<<", " << Data << ", " << Length << ")"<< endl;
#endif
    
	ASAAC_TimedReturnStatus Result = ASAAC_APOS_sendMessage( LocalVc, &m_Timeout, Data, Length );

	if ( Result == ASAAC_TM_ERROR )
	{
		cerr << "VcSender::processVcMessage() Could not send on named LocalVc"<< endl;
		throw PcsException( 0, LocalVc, "Could not send on named LocalVc." );

	}
	
	if ( Result == ASAAC_TM_TIMEOUT )
	{
		cerr << "VcSender::processVcMessage() Timeout occurred sending on LocalVc"<< endl;
		throw PcsException( 0, LocalVc, "Timeout occurred sending on LocalVc." );
	}
	
	return ASAAC_SUCCESS;
}


void VcSender::setTimeout( const ASAAC_TimeInterval& Timeout )
{
	m_Timeout = Timeout;
}

