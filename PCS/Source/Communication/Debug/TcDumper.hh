#ifndef TCDUMPER_HH_
#define TCDUMPER_HH_

#include "Interfaces/TcMessageConsumer.hh"

class TcDumper : public TcMessageConsumer
{
public:
	TcDumper();

	void initialize();
	void deinitialize();
	
	virtual ASAAC_ReturnStatus processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( TcMessageConsumer& Consumer );
	
private:
	TcMessageConsumer* m_Consumer;
	
};

#endif /*TCDUMPER_HH_*/
