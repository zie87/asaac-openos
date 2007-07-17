#ifndef VCSENDER_HH_
#define VCSENDER_HH_

#include "Interfaces/VcMessageConsumer.hh"

#include "PcsHIncludes.hh"

class VcSender : public VcMessageConsumer
{
public:
	VcSender();

	void initialize();
	void deinitialize();
	
	virtual ASAAC_ReturnStatus processVcMessage( ASAAC_PublicId LocalVc, ASAAC_Address Data, unsigned long Length );
	
	void setTimeout( const ASAAC_TimeInterval& Timeout );
	
private:
	ASAAC_TimeInterval		m_Timeout;
	
};

#endif /*VCSENDER_HH_*/
