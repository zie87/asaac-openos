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
	ASAAC_ReturnStatus setNextMessageTime(unsigned long index);
	
	long			   getNextMessageIndex();
	
	ASAAC_ReturnStatus getRateLimit( ASAAC_PublicId TcId , ASAAC_TimeInterval& MessageRate );
	ASAAC_ReturnStatus getQueue(ASAAC_PublicId TcId, unsigned long& index);
	
	ASAAC_ReturnStatus enqueueMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length );
	
	typedef struct {
		unsigned long  Length;
		char	 	   Data[ PCS_MAX_SIZE_OF_NWMESSAGE]; 
	} Message;
	
	typedef struct {
		ASAAC_PublicId     tc_id;
		Message	           buffer[ PCS_MAX_SIZE_OF_MESSAGEQUEUE ];
		unsigned long      counter;
		unsigned long      position;
		unsigned long      free_position;
		ASAAC_Time         time;
		ASAAC_TimeInterval rate;
	} Queue;
	
	Queue			   m_Queues[PCS_MAX_NUMBER_OF_TCS];

	TcMessageConsumer* m_Consumer;
	PCSConfiguration*  m_Configuration;
	
	ASAAC_PrivateId	   m_ConfigurationChangedEvent;
	ASAAC_PrivateId	   m_MessageProcessedEvent;
};


#endif /*TCRATELIMITER_HH_*/
