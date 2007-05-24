#ifndef TCUNPACKER_HH_
#define TCUNPACKER_HH_

#include "Interfaces/TcMessageConsumer.hh"
#include "Interfaces/VcMessageConsumer.hh"

#include "Configuration/PCSConfiguration.hh"

#include "Exception.hh"

class TcUnpacker : public TcMessageConsumer
{
public:
	TcUnpacker();
	TcUnpacker( VcMessageConsumer& Consumer, PCSConfiguration& Configuration );
	
	
	virtual ~TcUnpacker();
	
	virtual ASAAC_ReturnStatus processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( VcMessageConsumer& Consumer );
	void setConfiguration( PCSConfiguration& Configuration );
	
private:
	VcMessageConsumer*		m_Consumer;
	PCSConfiguration*		m_Configuration;
	
};

#endif /*TCUNPACKER_HH_*/
