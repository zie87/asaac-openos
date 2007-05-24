#include "TcDumper.hh"

#include <arpa/inet.h>

#include <iostream>

using namespace std;

TcDumper::TcDumper() : m_Consumer(0)
{
}


TcDumper::TcDumper( TcMessageConsumer& Consumer ) : m_Consumer(&Consumer)
{
}


TcDumper::~TcDumper()
{
}


ASAAC_ReturnStatus TcDumper::processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length )
{
	struct TcDataType {
		ASAAC_PublicId	VcId;
		char		Message[];
	};
	
	TcDataType* TcData = static_cast<TcDataType*>(Data);
	
	cout << "sending message on TC " << TcId << " for VC " << TcData->VcId << ": '" << TcData->Message << "'" << endl;
	
	if ( m_Consumer != 0 )
	{
		return m_Consumer->processTcMessage( TcId, Data, Length );
	}
	
	return ASAAC_SUCCESS;
}


void TcDumper::setOutputConsumer( TcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}
