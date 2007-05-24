#ifndef PCSCLASS_HH_
#define PCSCLASS_HH_



#include "Interfaces/VcMessageConsumer.hh"
#include "Interfaces/TcMessageConsumer.hh"

#include "Consumers/VcSender.hh"

#include "Consumers/TcDumper.hh"

#include "Consumers/MessageDumper.hh"
#include "VcListener.hh"
#include "TcListener.hh"

#include "Consumers/MarshallingFilter.hh"
#include "Consumers/UnMarshallingFilter.hh"

#include "Consumers/VcTcSwitch.hh"
#include "Consumers/TcUnpacker.hh"
#include "Consumers/TcPacker.hh"
#include "Consumers/TcSender.hh"

#include "Consumers/GlobalVcSender.hh"

#include "Configuration/PCSConfiguration.hh"

#include "PmFilter/PmFilter.hh"

#include "TimeOperations.hh"

#include "Consumers/TcRateLimiter.hh"

#include "Marshalling/MarshallingProcessor.hh"


class PCS 
{
	
	public:
		PCS();
		
		bool initialize();
		
		void vcListener();
		void tcListener();
		void rateLimiter();
		
		ASAAC_ReturnStatus configureInterface( const ASAAC_InterfaceData& if_config );	
		ASAAC_ReturnStatus createTransferConnection( const ASAAC_TcDescription& tc_description );
		ASAAC_ReturnStatus getTransferConnectionDescription( ASAAC_PublicId tc_id, ASAAC_TcDescription& tc_description );
		ASAAC_ReturnStatus destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descriptor );	
		ASAAC_ReturnStatus getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_descriptor, ASAAC_NetworkPortStatus& status );	
		ASAAC_ReturnStatus attachTransferConnectionToVirtualChannel( const ASAAC_VcDescription vc_description, ASAAC_PublicId tc_id, ASAAC_Bool is_data_representation );
		ASAAC_ReturnStatus detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id );
		ASAAC_TimedReturnStatus getPMData(unsigned long max_len, ASAAC_Time timeout, ASAAC_PublicId snd_vc, ASAAC_PublicId& vc_id);
		ASAAC_ReturnStatus returnPMData(ASAAC_PublicId vc_id, ASAAC_PublicId rec_vc, ASAAC_ReturnStatus sm_return_status);
		
		
	protected:
	
	
	private:	
	
		PCSConfiguration	m_Configuration;
	
		VcListener 			m_Listener;
		VcTcSwitch			m_Switch;

		MarshallingFilter	m_Marshalling;
		PmFilter			m_OutgoingPmFilter;

		TcPacker			m_Packer;
		TcRateLimiter		m_Limiter;

	    TcSender            m_NiiSender;
	    
	    TcListener 			m_NiiReceiver;       
		TcUnpacker			m_UnPacker;
		PmFilter			m_IncomingPmFilter;
		UnMarshallingFilter	m_UnMarshalling;

		GlobalVcSender		m_GlobalVcSend;
		VcSender			m_Sender;
		
		PmFilter::Queue		m_PmQueue;
	
};

#endif /*PCSCLASS_HH_*/
