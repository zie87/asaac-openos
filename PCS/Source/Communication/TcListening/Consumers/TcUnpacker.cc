#include "TcUnpacker.hh"

#include "PcsCIncludes.hh"

#include <arpa/inet.h>

#include <iostream>
using namespace std;

TcUnpacker::TcUnpacker()
{
}

void TcUnpacker::initialize()
{

}


void TcUnpacker::deinitialize()
{
	
}


ASAAC_ReturnStatus TcUnpacker::processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
		cout << "TcUnpacker::processTcMessage(" << TcId<< "," << Data<< "," << Length << ")" << endl;
#endif

	ASAAC_PublicIdSet ReceivingVcs;
	unsigned long Number;

	if ( m_Configuration->getVcMapping( TcId, ReceivingVcs, Number ) == ASAAC_ERROR )
	{
		cerr << "TcUnpacker::processTcMessage() no mapping for TC " << TcId << endl;
		return ASAAC_ERROR;
	}
	
	if ( Number == 1 )
	{
#ifdef _DEBUG_       
		cout << "TcUnpacker::processTcMessage() only one VC for TC " << TcId<< endl;
#endif
		ASAAC_PublicId	  GlobalVc;
		// IF there is only one receiver for the given TC, it could be an Application_Raw_VC
		// if so, handle it properly

		
		GlobalVc = ReceivingVcs.vc_id[ 0 ];
		
		ASAAC_VcDescription Description;
		
		if ( m_Configuration->getVcDescription( GlobalVc, Description ) == ASAAC_ERROR )
		{
		    cerr << "TcUnpacker::processTcMessage() no description for VC " << GlobalVc << endl;
			return ASAAC_ERROR;
		}

		if ( Description.vc_type == ASAAC_Application_Raw_VC )
		{
#ifdef _DEBUG_       
			cout << "TcUnpacker::processTcMessage() process RAW VC " << GlobalVc << endl;
#endif	
			return m_Consumer->processVcMessage(GlobalVc, Data, Length );
		}
		else
		{
#ifdef _DEBUG_       
		cout << "TcUnpacker::processTcMessage() not RAW VC" << GlobalVc << endl;
#endif	
		}
	}

#ifdef _DEBUG_       
	cout << "TcUnpacker::processTcMessage() multiple VC for TC " << TcId<< endl;
#endif

	struct TcDataType {
		ASAAC_PublicId	VcId;
		char		Message[];
	};

	TcDataType* TcData = static_cast<TcDataType*>(Data);
	
	ASAAC_PublicId GlobalVc =  ntohl(TcData->VcId);


	// Check whether indicated GlobalVc
	for ( unsigned long Index = 0;; Index ++ )
	{
		if ( ReceivingVcs.vc_id[ Index ] == GlobalVc ) break;
		
		if ( Index == Number ) 
		{
			PcsException( TcId, GlobalVc, "Received Tc packet contains invalid Vc" );
			return ASAAC_ERROR;
		}
	}

	return m_Consumer->processVcMessage(GlobalVc, TcData->Message,  Length-4 );
}
	

void TcUnpacker::setOutputConsumer( VcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void TcUnpacker::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}


