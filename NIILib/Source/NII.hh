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
			const ASAAC_NetworkDescriptor& network_id,
			const ASAAC_InterfaceConfigurationData& configuration_data);

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
			ASAAC_PublicId tc_id,
			const ASAAC_NetworkDescriptor& network_id,
			ASAAC_TransferDirection send_receive, 
			ASAAC_TransferType message_streaming,
			ASAAC_TC_ConfigurationData configuration_data, 
			ASAAC_Bool trigger_callback,
			ASAAC_PublicId callback_id);

	/////////////////////////////////////////////
	/// Send a block of data on the given TC
	/// (STANAG 4626 Part II - 11.5.1.3.3)
	/// @param tc_id a unique value to identify the TC to be used for the transfer
	/// @param transmit_data the address of the data to be transmitted
	/// @param data_length length of the data to be transmitted in Bytes
	/// @param time_out Time out value for the service call
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus sendTransfer(
			ASAAC_PublicId tc_id, 
			const char* transmit_data,
			unsigned long data_length, 
			ASAAC_Time time_out);

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
			ASAAC_PublicId tc_id, 
			ASAAC_CharAddress& receive_data,
			unsigned long data_length_available, 
			unsigned long& data_length, 
			ASAAC_Time time_out);

	/////////////////////////////////////////////
	/// Release local resources previously allocated to handle the transmission or reception of information over a TC
	/// (STANAG 4626 Part II - 11.5.1.3.5)
	/// @param tc_id a unique value to identify the transfer
	/// @param network_id a unique value to identify the network
	/// @return MOS_NII_CALL_FAILED if call did not succeed

	ASAAC_NiiReturnStatus destroyTransfer(
			ASAAC_PublicId tc_id,
			const ASAAC_NetworkDescriptor& network_id);

	/////////////////////////////////////////////
	/// TBD
	/// (STANAG 4626 Part II - 11.5.1.3.6)
	/// @param 
	/// @param 
	/// @param 
	/// @return 

	//NiiReturnStatus getNetworkPortStatus();

	ASAAC_NiiReturnStatus listenNetwork(
			const ASAAC_NetworkDescriptor& network_id,
			ASAAC_PublicId& tc_id);
	
	
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
			const ASAAC_NetworkDescriptor& network_id,
			ASAAC_CharAddress& receive_data, 
			unsigned long data_length_available,
			unsigned long& data_length, 
			ASAAC_PublicId& tc_id, 
			ASAAC_Time time_out);

	/////////////////////////////////////////////
	/// Returns a character string representation of the return status code

	static char* spell(ASAAC_NiiReturnStatus status);

	/////////////////////////////////////////////
	/// Returns a multicast address based on a network identifier between 3-255

	ASAAC_PublicId networkGroup(char address);

	/////////////////////////////////////////////
	/// Returns a network address based on an IP4 address format X.X.X.X

	ASAAC_PublicId networkAddress(const char* ip_addr);

	/////////////////////////////////////////////
	/// Returns a network address based on an IP4 address 127,0.0.1

	ASAAC_PublicId getLocalNetwork();

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

	static void* streamTcThread(void* pTcData);

	static void* listenThread(void* Data);

	ASAAC_TimedReturnStatus waitForData( const ASAAC_Time time_out, ASAAC_PublicId &tc_id  );
	ASAAC_TimedReturnStatus waitForDataOnTc( const ASAAC_Time time_out, const ASAAC_PublicId tc_id );
	ASAAC_TimedReturnStatus waitForDataOnNw( const ASAAC_Time time_out, const ASAAC_NetworkDescriptor& network_id );
	
	void startListening();
	void stopListening();

	/////////////////////////////////////////////
	/// Returns TRUE, if network descriptor is available and associated with a valid interface and socket
	/// @param Index that is where a valid index to an entry is stored to
	/// @param network_id a unique value to identify the network
	/// @return TRUE if network_id was already configured and index could be specified

	long getNwIndex(const ASAAC_NetworkDescriptor& network_id);

	/////////////////////////////////////////////
	/// Returns TRUE, if the transfer connection is available and associated with a valid network interface
	/// @param Index that is where a valid index to an entry is stored to
	/// @param tc_id a unique value to identify the TC
	/// @return TRUE if tc_id was already configured and index could be specified

	long getTcIndex(const ASAAC_PublicId tc_id);

	/////////////////////////////////////////////
	/// Returns TRUE, if an invalid, empty slot for a new connection was found and index is given as argument
	/// @param Index that is where an invalid index to an entry is stored to
	/// @return TRUE if another empty slot for storage of a new TC could be specified

	long getEmptyTc();

	/////////////////////////////////////////////
	/// Returns TRUE, if an invalid, empty slot for a new network socket was found and index is given as argument
	/// @param Index that is where an invalid index to an entry is stored to
	/// @return TRUE if another empty slot for storage of a new Network could be specified

	long getEmptyNw();

private:

	/////////////////////////////////////////////
	/// Constructs a default object with no configured interfaces and no open transfer channels
	cMosNii();

	///< Used to associate a Transfer Connection with a Network
	TcData 			m_TcData[NII_MAX_NUMBER_OF_TC_CONNECTIONS];
	///< Used to associate an Interface with a Network
	NwData 			m_NwData[NII_MAX_NUMBER_OF_NETWORKS]; 

	///< This is the current local port to open new sockets. It is inkremented from each time and initialized in constructor @see cMosNii()
	ASAAC_PublicId 	m_NiiLocalPort; 

	//< Buffer for received data
	char 			m_ReceiveBuffer[NII_MAX_NUMBER_OF_RECEIVEBUFFERS][NII_MAX_SIZE_OF_RECEIVEBUFFER]; 
	//< Currently used buffer for received data
	unsigned int 	m_CurrentBuffer; 
	
	pthread_t       m_ListeningThread;
	int				m_LoopBackFileDescriptor;
	
	pthread_cond_t	m_ListeningThreadCondition;
	pthread_mutex_t	m_ListeningThreadMutex;
	
	pthread_cond_t	m_NewDataCondition;
	pthread_mutex_t	m_NewDataMutex;
	
	int				m_IsListening;
	
	long			m_NewDataTcIndex;
};

#endif /*MOSNII_HH_*/