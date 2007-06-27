#ifndef TCRATELIMITER_HH_
#define TCRATELIMITER_HH_

#include "Interfaces/TcMessageConsumer.hh"
#include "Configuration/PCSConfiguration.hh"

#include "Exceptions/Exception.hh"



class TcRateLimiter : public TcMessageConsumer
{

public:
	
	TcRateLimiter();
	
	TcRateLimiter( TcMessageConsumer& Consumer, PCSConfiguration& Configuration );
	
	virtual ~TcRateLimiter();
	
	virtual ASAAC_ReturnStatus processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( TcMessageConsumer& Consumer );
	
	void setConfiguration( PCSConfiguration& Configuration );
	
	ASAAC_ReturnStatus setRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& MessageRate );
	
	ASAAC_ReturnStatus removeRateLimit( ASAAC_PublicId TcId );
	
	ASAAC_ReturnStatus processNextMessage( );
	
private:
	
	void init();
	
	ASAAC_ReturnStatus setNextMessageTime(unsigned long queue, ASAAC_Time& now);
	
	ASAAC_ReturnStatus getRateLimit( ASAAC_PublicId TcId , ASAAC_TimeInterval& MessageRate );
	
	ASAAC_ReturnStatus getQueue(ASAAC_PublicId TcId, unsigned long& index);
	
	ASAAC_ReturnStatus enqueueMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	ASAAC_PublicId		m_TcQueueMap[ PCS_NUMBER_OF_TCS ];
	
	ASAAC_TimeInterval m_MessageRate[ PCS_NUMBER_OF_TCS ];

	struct Message {
		unsigned long  Length;
		char	 		Data[ PCS_MAXIMUM_MESSAGE_LENGTH]; 
	};
	
	Message				m_QueuedMessages[PCS_NUMBER_OF_TCS][ PCS_MAXIMUM_QUEUED_MESSAGES ];
	ASAAC_Time			m_NextMessageTime[PCS_NUMBER_OF_TCS];
	unsigned long		m_NextFreeQueue[PCS_NUMBER_OF_TCS];
	unsigned long		m_NextMessage[PCS_NUMBER_OF_TCS];
	
	TcMessageConsumer*	m_Consumer;
	PCSConfiguration*	m_Configuration;
};


#endif /*TCRATELIMITER_HH_*/
