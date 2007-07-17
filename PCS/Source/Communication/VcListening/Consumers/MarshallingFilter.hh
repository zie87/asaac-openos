#ifndef MARSHALLINGFILTER_HH_
#define MARSHALLINGFILTER_HH_

#include "Interfaces/VcMessageConsumer.hh"
#include "Configuration/PCSConfiguration.hh"

#include "Marshalling/MarshallingProcessor.hh"

#include "PcsHIncludes.hh"

class MarshallingFilter : public VcMessageConsumer
{
public:
	MarshallingFilter();
	MarshallingFilter( VcMessageConsumer& OutputConsumer, PCSConfiguration& Configuration );
	
	virtual ~MarshallingFilter();
	
	virtual ASAAC_ReturnStatus processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( VcMessageConsumer& OutputConsumer );
	
	void setConfiguration( PCSConfiguration& Configuration );
		
		
private:

	VcMessageConsumer*		m_OutputConsumer;
	PCSConfiguration*		m_Configuration;
	
	MarshallingProcessor	m_Processor;

};



#endif /*MARSHALLINGFILTER_HH_*/
