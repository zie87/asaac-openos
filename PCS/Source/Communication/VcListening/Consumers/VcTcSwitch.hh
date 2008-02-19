#ifndef VCTCSWITCH_HH_
#define VCTCSWITCH_HH_

#include "Interfaces/MessageConsumer.hh"
#include "Interfaces/VcMessageConsumer.hh"

#include "Configuration/PCSConfiguration.hh"


class VcTcSwitch : public VcMessageConsumer
{
public:
	VcTcSwitch();

	void initialize();
	void deinitialize();
	
	virtual ASAAC_ReturnStatus processVcMessage( ASAAC_PublicId LocalVc, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( MessageConsumer& OutputConsumer );
	void setConfiguration( PCSConfiguration& Configuration );
	
private:
	MessageConsumer*		m_OutputConsumer;
	PCSConfiguration*		m_Configuration;
};

#endif /*VCTCSWITCH_HH_*/
