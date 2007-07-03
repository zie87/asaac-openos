#ifndef PMFILTER_HH_
#define PMFILTER_HH_

#include "Interfaces/VcMessageConsumer.hh"
#include "Configuration/PCSConfiguration.hh"
#include "IPC/SemaphoreProtectedScope.hh"

#include "Exceptions/Exception.hh"

#include "PcsIncludes.hh"

//! causes the message to be redirected via the GSM/SM to perform authentication/encryption if required

/*! The PmFilter checks requirement of authentication and/or encryption of incoming messages
 * and, if applicable, redirects them to a PmDispatcher to be routed via the GSM/SM for processing.
 * 
 * There needs to be one PmFilter both in the Vc-To-Tc filter chain as well as in the Tc-To-Vc filter chain.
 */



class PmFilter : public VcMessageConsumer
{
	
public:

	struct Message {
		ASAAC_PublicId   VcId;
		unsigned long  Length;
		char	 		 Data[ PCS_MAX_SIZE_OF_NWMESSAGE ];
	};
	
	struct Mapping {
		ASAAC_PublicId	VcId;
		PmFilter*		Source;
		
	};
	
	class Queue
	{
		public:
		Queue();
		bool isEmtpy() { return m_NextFreeQueue == m_NextMessage; }
		
		ASAAC_ReturnStatus enqueueMessage(ASAAC_PublicId VcId, ASAAC_Address Data, unsigned long Length, PmFilter* Source);
	
		ASAAC_TimedReturnStatus getMessage(ASAAC_PublicId& VcId, ASAAC_TimeInterval& interval,ASAAC_Address& Data, unsigned long& Length );
		
		ASAAC_ReturnStatus putMessage(ASAAC_PublicId VcId, ASAAC_Address Data, unsigned long Length);
		
		private:
		
		ASAAC_ReturnStatus addMapping(ASAAC_PublicId VcId, PmFilter* Source);
		ASAAC_ReturnStatus removeMapping(ASAAC_PublicId VcId);
		PmFilter* getMapping(ASAAC_PublicId VcId);
		
		Message				m_QueuedMessages[ PCS_MAX_SIZE_OF_PMMESSAGEQUEUE ];
		Mapping				m_PmFilterMapping [ PCS_MAX_NUMBER_OF_TCS ];
		unsigned long		m_NextFreeMapping;
		unsigned long		m_NextFreeQueue;
		unsigned long		m_NextMessage;
		ASAAC_PrivateId	    m_nqEvent;
		ASAAC_PrivateId     m_getSem;
		ASAAC_PrivateId     m_nqSem;
	};


	PmFilter();
	PmFilter( VcMessageConsumer& Consumer, PCSConfiguration& Configuration, Queue& q);
	
	virtual ~PmFilter();
	
	void setQueue(Queue& q);

	void initialize();
	
	virtual ASAAC_ReturnStatus processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( VcMessageConsumer& Consumer );
	//!< set the MessageConsumer which this instance of PmFilter relays its processed messages to
	
	void setConfiguration( PCSConfiguration& Configuration );
	//!< set the instance of PCSConfiguration for lookup of security information
		
	VcMessageConsumer* getConsumer();

	
	
private:


	VcMessageConsumer*		m_Consumer;
	PCSConfiguration*		m_Configuration;
	Queue*					m_Queue;
};

#endif /*PMFILTER_HH_*/
