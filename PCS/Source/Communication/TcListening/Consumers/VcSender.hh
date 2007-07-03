#ifndef VCSENDER_HH_
#define VCSENDER_HH_

#include "Interfaces/VcMessageConsumer.hh"

#include "Exceptions/Exception.hh"

#include "PcsIncludes.hh"

class VcSender : public VcMessageConsumer
{
public:
	VcSender();
	VcSender( const ASAAC_TimeInterval& Timeout );
	virtual ~VcSender();
	
	virtual ASAAC_ReturnStatus processVcMessage( ASAAC_PublicId LocalVc, ASAAC_Address Data, unsigned long Length );
	
	void setTimeout( const ASAAC_TimeInterval& Timeout );
	
private:
	ASAAC_TimeInterval		m_Timeout;
	
};

#endif /*VCSENDER_HH_*/
