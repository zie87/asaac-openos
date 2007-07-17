#ifndef UNMARSHALLINGFILTER_HH_
#define UNMARSHALLINGFILTER_HH_

#include "Interfaces/VcMessageConsumer.hh"
#include "Configuration/PCSConfiguration.hh"

#include "Marshalling/MarshallingProcessor.hh"

#include "PcsHIncludes.hh"

class UnMarshallingFilter : public VcMessageConsumer
{
public:
	UnMarshallingFilter();
	UnMarshallingFilter( VcMessageConsumer& OutputConsumer, PCSConfiguration& Configuration );
	
	virtual ~UnMarshallingFilter();
	
	virtual ASAAC_ReturnStatus processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( VcMessageConsumer& OutputConsumer );
	
	void setConfiguration( PCSConfiguration& Configuration );
		
		
private:

	VcMessageConsumer*		m_OutputConsumer;
	PCSConfiguration*		m_Configuration;
	
	MarshallingProcessor	m_Processor;

};



#endif /*UNMARSHALLINGFILTER_HH_*/
