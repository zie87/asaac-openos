#include "GlobalVcSender.hh"

GlobalVcSender::GlobalVcSender() : m_Consumer(0), m_Configuration(0)
{
}


GlobalVcSender::GlobalVcSender( VcMessageConsumer& Consumer, PCSConfiguration& Configuration ) :
		m_Consumer( &Consumer ), m_Configuration( &Configuration )
{
}



GlobalVcSender::~GlobalVcSender()
{
}


ASAAC_ReturnStatus GlobalVcSender::processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
		cout << "GlobalVcSender::processMessage(" << GlobalVc<< "," << Data<< "," << Length << ")" << endl;
#endif

	if ( m_Consumer == 0 )
	{
		cerr << "GlobalVcSender::processMessage() No OutputConsumer set" << endl;
		throw PCSException( 0, GlobalVc, "No OutputConsumer set." );

	}
	
	ASAAC_PublicId LocalVc = m_Configuration->getLocalVc( GlobalVc );
	
	if ( LocalVc == 0 )
	{
		cerr << "GlobalVcSender::processMessage() No matching LocalVc found" << endl;
		throw PCSException( 0, GlobalVc, "No matching LocalVc found" );
	}
	
	return m_Consumer->processVcMessage( LocalVc, Data, Length );
}


void GlobalVcSender::setOutputConsumer( VcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void GlobalVcSender::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}

