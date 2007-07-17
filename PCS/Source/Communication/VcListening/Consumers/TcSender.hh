#ifndef TCSENDER_HH_
#define TCSENDER_HH_

#include "Interfaces/TcMessageConsumer.hh"
#include "Interfaces/MessageConsumer.hh"

#include "Configuration/PCSConfiguration.hh"

#include "PcsHIncludes.hh"

#include "NII.hh"

class TcSender : public TcMessageConsumer
{
public:
	TcSender();

	void initialize();
	void deinitialize();

    virtual ASAAC_ReturnStatus processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
    
    void setConfiguration( PCSConfiguration& Configuration );
    
private:
    PCSConfiguration*       m_Configuration;
    
};

#endif /*TCSENDER_HH_*/
