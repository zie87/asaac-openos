#ifndef TCPACKER_HH_
#define TCPACKER_HH_

#include "Interfaces/MessageConsumer.hh"
#include "Interfaces/TcMessageConsumer.hh"

#include "Configuration/PCSConfiguration.hh"

#include "PcsHIncludes.hh"

class TcPacker : public MessageConsumer
{
public:
	TcPacker();

	void initialize();
	void deinitialize();	

	virtual ASAAC_ReturnStatus processMessage( ASAAC_PublicId TcId, ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( TcMessageConsumer& Consumer );
	void setConfiguration( PCSConfiguration& Configuration );
	
private:

	TcMessageConsumer*	m_Consumer;
	PCSConfiguration*	m_Configuration;
};

#endif /*TCPACKER_HH_*/
