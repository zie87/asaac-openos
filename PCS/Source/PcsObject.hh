#ifndef PCSOBJECT_HH_
#define PCSOBJECT_HH_


#include "Interfaces/VcMessageConsumer.hh"
#include "Interfaces/TcMessageConsumer.hh"

#include "Communication/Debug/TcDumper.hh"
#include "Communication/Debug/MessageDumper.hh"

#include "Communication/PmFilter/PmFilter.hh"

#include "Communication/TcListening/TcListener.hh"
#include "Communication/TcListening/Consumers/TcUnpacker.hh"
#include "Communication/TcListening/Consumers/UnMarshallingFilter.hh"
#include "Communication/TcListening/Consumers/GlobalVcSender.hh"
#include "Communication/TcListening/Consumers/VcSender.hh"

#include "Communication/VcListening/VcListener.hh"
#include "Communication/VcListening/Consumers/VcTcSwitch.hh"
#include "Communication/VcListening/Consumers/MarshallingFilter.hh"
#include "Communication/VcListening/Consumers/TcPacker.hh"
#include "Communication/VcListening/Consumers/TcRateLimiter.hh"
#include "Communication/VcListening/Consumers/TcSender.hh"

#include "Configuration/PCSConfiguration.hh"

#include "Operators/TimeOperators.hh"

#include "Marshalling/MarshallingProcessor.hh"


class PCS
{
	public:
		static PCS *getInstance();

		void initialize();
		void deinitialize();

		void loopVcListener();
		void loopRateLimiter();

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
		static void TcCallback( ASAAC_Address event_info_data );

		ASAAC_Address getBuffer();

	private:
		PCS();

		PCSConfiguration	m_Configuration;

		VcListener 			m_Listener;
		VcTcSwitch			m_Switch;

#ifndef DISABLE_MARSHALLING
		MarshallingFilter	m_Marshalling;
#endif
		PmFilter			m_OutgoingPmFilter;

		TcPacker			m_Packer;
		TcRateLimiter		m_Limiter;

	    TcSender            m_NiiSender;

	    TcListener 			m_NiiReceiver;
		TcUnpacker			m_UnPacker;
		PmFilter			m_IncomingPmFilter;
#ifndef DISABLE_MARSHALLING
		UnMarshallingFilter	m_UnMarshalling;
#endif

		GlobalVcSender		m_GlobalVcSend;
		VcSender			m_Sender;

		PmFilter::Queue		m_PmQueue;

		char 				m_MessageBuffer[PCS_MAX_NUMBER_OF_BUFFER][PCS_MAX_SIZE_OF_NWMESSAGE];
		unsigned long		m_MessageBufferIndex;
};

#endif /*PCSOBJECT_HH_*/
