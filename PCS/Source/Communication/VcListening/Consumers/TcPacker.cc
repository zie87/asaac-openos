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
	if ( m_Consumer == 0 )
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

	char PackedBuffer[ PCS_MAX_SIZE_OF_NWMESSAGE ]; //two headers may be added (VC,TC)
	
	struct PackedDataType {
		ASAAC_PublicId	header[2]; //space for IDs ahead of the message
		char		Message[PCS_MAX_SIZE_OF_TCMESSAGE];
	};

	PackedDataType* PackedData = reinterpret_cast<PackedDataType*>(PackedBuffer);
	
	memcpy( PackedData->Message, Data, Length );

	if ( Description.vc_type == ASAAC_Application_Raw_VC )
	{
		PackedData->header[1] = htonl(TcId);
		return m_Consumer->processTcMessage( TcId, static_cast<ASAAC_Address>(PackedBuffer+4), Length+4 );
	}
	else
	{
		PackedData->header[0] = htonl(TcId );
		PackedData->header[1] = htonl(GlobalVc);
		return m_Consumer->processTcMessage( TcId, static_cast<ASAAC_Address>(PackedBuffer), Length+8 );
	}
	
	cerr << "TcPacker::processMessage() DONT LET THIS CASE HAPPEN" << endl;
	
	return m_Consumer->processTcMessage( TcId, Data, Length );
	
	
	
	
}


void TcPacker::setOutputConsumer( TcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void TcPacker::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}
