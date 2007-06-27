#include "VcTcSwitch.hh"

#include <iostream>

#include <string.h>
#include <arpa/inet.h>

using namespace std;

VcTcSwitch::VcTcSwitch()
{
}

VcTcSwitch::VcTcSwitch( MessageConsumer& Consumer, PCSConfiguration& Configuration ) :
	m_OutputConsumer(&Consumer), m_Configuration(&Configuration)
{
}


VcTcSwitch::~VcTcSwitch()
{
}


void VcTcSwitch::setOutputConsumer( MessageConsumer& OutputConsumer )
{
	m_OutputConsumer = &OutputConsumer;
}


void VcTcSwitch::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}



ASAAC_ReturnStatus VcTcSwitch::processVcMessage( ASAAC_PublicId LocalVc, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
	cout << "VcTcSwitch::processVcMessage(" << LocalVc<< "," << Data<< "," << Length << ")" << endl;
#endif

	if ( m_OutputConsumer == 0 ) 
	{
#ifdef _DEBUG_       
		cerr << "VcTcSwitch::processVcMessage() no output consumer" << endl;
#endif
		return ASAAC_ERROR;
	}
	if ( m_Configuration  == 0 ) 
	{
#ifdef _DEBUG_       
		cerr << "VcTcSwitch::processVcMessage() no configuration" << endl;
#endif
		return ASAAC_ERROR;
	}
	

	ASAAC_PublicId GlobalVc = m_Configuration->getGlobalVc( LocalVc );

	if ( GlobalVc == 0 )
	{
#ifdef _DEBUG_       
		cerr << "VcTcSwitch::processVcMessage() no global VC found" << endl;
#endif
		
		return ASAAC_ERROR;
	}
	
	
	unsigned long Number;
	ASAAC_PublicIdSet TargetTcs;

	if ( m_Configuration->getTcMapping( GlobalVc, TargetTcs, Number ) == ASAAC_ERROR ) return ASAAC_ERROR;

	for ( unsigned long Index = 0; TargetTcs.vc_id[ Index ] != 0; Index ++ )
	{
		if ( m_OutputConsumer->processMessage( TargetTcs.vc_id[ Index ], GlobalVc, Data, Length ) != ASAAC_SUCCESS ) 
		{
			return ASAAC_ERROR;
		}
	}
	
	return ASAAC_SUCCESS;
}

