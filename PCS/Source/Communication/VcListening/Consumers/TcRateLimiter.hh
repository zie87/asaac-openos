#ifndef TCRATELIMITER_HH_
#define TCRATELIMITER_HH_


#include "PcsHIncludes.hh"

#include "Interfaces/TcMessageConsumer.hh"
#include "Configuration/PCSConfiguration.hh"



class TcRateLimiter : public TcMessageConsumer
{

public:
	
	TcRateLimiter();

	void initialize();
	void deinitialize();
	
	virtual ASAAC_ReturnStatus processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( TcMessageConsumer& Consumer );
	void setConfiguration( PCSConfiguration& Configuration );

	ASAAC_ReturnStatus setRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& MessageRate );
	ASAAC_ReturnStatus removeRateLimit( ASAAC_PublicId TcId );
	
	ASAAC_ReturnStatus processNextMessage( );
	
private:
	ASAAC_ReturnStatus setNextMessageTime(unsigned long queue, ASAAC_Time& now);
	
	long			   getNextMessageIndex();
	
	ASAAC_ReturnStatus getRateLimit( ASAAC_PublicId TcId , ASAAC_TimeInterval& MessageRate );
	ASAAC_ReturnStatus getQueue(ASAAC_PublicId TcId, unsigned long& index);
	
	ASAAC_ReturnStatus enqueueMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	ASAAC_PublicId	   m_TcQueueMap[ PCS_MAX_NUMBER_OF_TCS ];
	ASAAC_TimeInterval m_MessageRate[ PCS_MAX_NUMBER_OF_TCS ];

	struct Message {
		unsigned long  Length;
		char	 	   Data[ PCS_MAX_SIZE_OF_NWMESSAGE]; 
	};
	
	Message			   m_QueuedMessages[PCS_MAX_NUMBER_OF_TCS][ PCS_MAX_SIZE_OF_MESSAGEQUEUE ];
	ASAAC_Time		   m_NextMessageTime[PCS_MAX_NUMBER_OF_TCS];
	unsigned long	   m_NextFreeQueue[PCS_MAX_NUMBER_OF_TCS];
	unsigned long	   m_NextMessage[PCS_MAX_NUMBER_OF_TCS];

	TcMessageConsumer* m_Consumer;
	PCSConfiguration*  m_Configuration;
	
	ASAAC_PrivateId	   m_ConfigurationChangedEvent;
};


#endif /*TCRATELIMITER_HH_*/
