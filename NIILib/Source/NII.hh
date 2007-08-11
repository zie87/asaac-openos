#ifndef MOSNII_HH_
#define MOSNII_HH_

#include "NIIIncludes.h"

class cMosNii
{
public:

	/////////////////////////////////////////////
	/// Returns the reference to singletion instance

	static cMosNii* getInstance();

	/////////////////////////////////////////////
	/// Destroys all configurations and open transfer channels
	~cMosNii();

	/////////////////////////////////////////////
	/// Configure a local communication interface 
	/// (STANAG 4626 Part II - 11.5.1.3.1)
	/// @param interface_id a unique value used to identify the local physical interface to be configured
	/// @param network_id a unique value used to identify the network to be configured
	/// @param configuration_data a structure containing the configuration data for the communications interface
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus configureInterface(
			const ASAAC_PublicId interface_id,
			const ASAAC_NetworkDescriptor *network_id,
			const ASAAC_InterfaceConfigurationData *configuration_data);

	/////////////////////////////////////////////
	/// Configure the local resource to handle the transmission or reception of information over a Transfer Channel (TC)
	/// (STANAG 4626 Part II - 11.5.1.3.2)
	/// @param tc_id a unique value to identify the transfer
	/// @param network_id a unique value to identify the network
	/// @param send_receive defines the direction of data transfers on the TC
	/// @param message_streaming defines the type of transfer, message or stream based
	/// @param configuration_data a structure containing the configuration data for the TC
	/// @param trigger_callback ASAAC_TRUE causes the associated callback to be invoked when an event occurs for this tc_id. ASAAC_FALSE disables the callback activation for this tc_id
	/// @param callback_id a unique callback identifier used by the OS and the MSL.
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus configureTransfer(
			const ASAAC_PublicId tc_id,
			const ASAAC_NetworkDescriptor network_id,
			const ASAAC_TransferDirection send_receive, 
			const ASAAC_TransferType message_streaming,
			const ASAAC_TC_ConfigurationData configuration_data, 
			const ASAAC_Bool trigger_callback,
			const ASAAC_PublicId callback_id);

	/////////////////////////////////////////////
	/// Send a block of data on the given TC
	/// (STANAG 4626 Part II - 11.5.1.3.3)
	/// @param tc_id a unique value to identify the TC to be used for the transfer
	/// @param transmit_data the address of the data to be transmitted
	/// @param data_length length of the data to be transmitted in Bytes
	/// @param time_out Time out value for the service call
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus sendTransfer(
			const ASAAC_PublicId tc_id, 
			const char *transmit_data,
			const unsigned long data_length, 
			const ASAAC_Time time_out);

	/////////////////////////////////////////////
	/// Receive a block of data on the given TC
	/// (STANAG 4626 Part II - 11.5.1.3.4)
	/// @param tc_id a unique value to identify the TC on which a message is going to be received
	/// @param receive_data the address of the buffer for the data to be received
	/// @param data_length_available The available data length on calling side in Bytes
	/// @param data_length the actual length of the message
	/// @param time_out Time out value for the service call
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus receiveTransfer(
			const ASAAC_PublicId tc_id, 
			ASAAC_CharAddress *receive_data,
			const unsigned long data_length_available, 
			unsigned long *data_length, 
			const ASAAC_Time time_out);

	/////////////////////////////////////////////
	/// Release local resources previously allocated to handle the transmission or reception of information over a TC
	/// (STANAG 4626 Part II - 11.5.1.3.5)
	/// @param tc_id a unique value to identify the transfer
	/// @param network_id a unique value to identify the network
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus destroyTransfer(
			const ASAAC_PublicId tc_id,
			const ASAAC_NetworkDescriptor network_id);

	/////////////////////////////////////////////
	/// TBD
	/// (STANAG 4626 Part II - 11.5.1.3.6)
	/// @param 
	/// @param 
	/// @param 
	/// @return 

	//NiiReturnStatus getNetworkPortStatus();


	/////////////////////////////////////////////
	/// Receive data on every TC connected to a specific network
	/// (STANAG 4626 Part II - 11.5.1.3.7)
	/// @param network_id a unique value to identify the network
	/// @param tc_id a unique value to identify the TC on which data was received
	/// @param receive_data the address of the buffer for the data to be received
	/// @param data_length_available The available data length on calling side in Bytes
	/// @param data_length the actual length of the message
	/// @param time_out Time out value for the service call
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus receiveNetwork(
			const ASAAC_NetworkDescriptor network_id,
			ASAAC_CharAddress *receive_data, 
			const unsigned long data_length_available,
			unsigned long *data_length, 
			ASAAC_PublicId *tc_id, 
			const ASAAC_Time time_out);

protected:

	typedef struct {
		pthread_cond_t	CallingThreadCondition;
		pthread_mutex_t	CallingThreadMutex;
	} CallingThreadData;

	/////////////////////////////////////////////
	/// This function is called as a new thread, which writes/reads data to/from a stream-based TC
	/// The thread is executed as long as the valid flag in TcData is TRUE
	/// @param pTcData si a pointer to TcData which is passed as void*
	/// @see TcData

	void handleStreamingTc(NwData *Nw);
	void handleMessageTc(NwData *Nw);

	static void* ServiceThread(void* Data);

	ASAAC_NiiReturnStatus hasData( const ASAAC_PublicId *tc_id );
	ASAAC_NiiReturnStatus hasData( const ASAAC_NetworkDescriptor network_id, ASAAC_PublicId *tc_id );
	ASAAC_NiiReturnStatus waitForData( const ASAAC_Time time_out, ASAAC_PublicId *tc_id  );
	ASAAC_NiiReturnStatus receiveData( 		
			const ASAAC_PublicId tc_id, 
			const ASAAC_CharAddress receive_data, 
			const unsigned long data_length_available, 
			unsigned long& data_length);
	
	void configureServices();

	
	char addNw(NwData Data);
	char removeNw(const ASAAC_NetworkDescriptor network_id);
	void removeAllNw();
	
	char getNw(const ASAAC_NetworkDescriptor network_id, NwData *Data);
	char setNw(const ASAAC_NetworkDescriptor network_id, const NwData Data);
	
	long countNws();

	
	char addTc(TcData Data);
	char removeTc(const ASAAC_PublicId tc_id);
	void removeAllTc();
	
	char getTc(const ASAAC_PublicId tc_id, TcData *Data);
	char setNw(const ASAAC_PublicId tc_id, const TcData Data);
	
	long countTcs();
	long countTcs(const ASAAC_NetworkDescriptor network_id);

	
	char allocateBuffer(ASAAC_PrivateId *buffer_id);
	void releaseBuffer(ASAAC_PrivateId buffer_id);

	TcPacketData *getBuffer(ASAAC_PrivateId buffer_id);
	
private:
	cMosNii();

	long getIndexOfNw(const ASAAC_NetworkDescriptor network_id);
	long getIndexOfTc(const ASAAC_PublicId tc_id);
	
	long receiveFromNetwork( const int fd, ASAAC_NetworkDescriptor *network_id, char* data, const unsigned long length );
	long sendToNetwork( const int fd, const ASAAC_NetworkDescriptor network_id, char* data, const unsigned long length );
	
	NwData 			m_NwList[NII_MAX_NUMBER_OF_NETWORKS]; 
	long			m_NwListSize;

	TcData 			m_TcList[NII_MAX_NUMBER_OF_TCS];
	long			m_TcListSize;
	
	TcPacketData    m_TcBufferArray[NII_MAX_NUMBER_OF_TCS];
	char			m_TcBufferArrayAllocated[NII_MAX_NUMBER_OF_TCS];
	
	pthread_t       m_ServiceThread;
	int				m_ServiceFileDescriptor;
	
	pthread_cond_t	m_ServiceThreadCondition;
	pthread_mutex_t	m_ServiceThreadMutex;
	
	pthread_cond_t	m_NewDataCondition;
	pthread_mutex_t	m_NewDataMutex;
	long			m_NewDataTcIndex;
	
	int				m_IsListening;	
};

#endif /*MOSNII_HH_*/
