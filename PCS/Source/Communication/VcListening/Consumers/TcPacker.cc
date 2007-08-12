#include "TcPacker.hh"

#include "PcsCIncludes.hh"
#include <arpa/inet.h>

#include <iostream>

using namespace std;

TcPacker::TcPacker()
{
}


void TcPacker::initialize()
{

}


void TcPacker::deinitialize()
{
	
}


ASAAC_ReturnStatus TcPacker::processMessage( ASAAC_PublicId TcId, ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
	cout << "TcPacker::processMessage(" << TcId<< "," << GlobalVc << "," << Data << "," << Length << ")" << endl;
#endif
	
	if ( m_Consumer == NULL )
	{
		throw PcsException( TcId, GlobalVc, "No OutputConsumer set." );
		return ASAAC_ERROR;
	}
	
	ASAAC_VcDescription Description;
	
	if ( m_Configuration->getVcDescription( GlobalVc, Description ) == ASAAC_ERROR ) 
	{
		throw PcsException( TcId, GlobalVc, "Could not get VcDescription" );
		return ASAAC_ERROR;
	}

	if ( Description.vc_type == ASAAC_Application_Raw_VC )
	{
		return m_Consumer->processTcMessage( TcId, Data, Length );
	}

	struct PackedDataType 
	{
		ASAAC_PublicId	vc_header; 
		char		    Message[PCS_MAX_SIZE_OF_TCMESSAGE];
	};

	PackedDataType PackedData;
	
	PackedData.vc_header = htonl(GlobalVc);
	memcpy( &PackedData.Message, Data, Length );
	
	return m_Consumer->processTcMessage( TcId, (ASAAC_Address)&PackedData, sizeof(ASAAC_PublicId) + Length );	
}


void TcPacker::setOutputConsumer( TcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void TcPacker::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}
