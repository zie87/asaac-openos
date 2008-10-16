#ifndef COMMUNICATIONMANAGER_HH_
#define COMMUNICATIONMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "Allocator/AllocatedArea.hh"
#include "IPC/Semaphore.hh"

#include "Communication/GlobalVc.hh"

#include "Communication/TwoWayCommunication.hh"
#include "LogicalInterfaces/asaac_pcs.hh"


//! singleton class providing methods to create and configure communication longerfaces

class CommunicationManager
{
public:
	void                   initialize( bool IsMaster, Allocator *ParentAllocator );

	
	void                   deinitialize();


	virtual                ~CommunicationManager();
	
	//! get reference to the instance of the communication manager
	static CommunicationManager*	getInstance();
	
	bool                    isInitialized();

    void                    configurePCS();

	long 					getVirtualChannelIndex( ASAAC_PublicId GlobalVcId );	

	//! get reference to a GlobalVc instance
	GlobalVc*				getVirtualChannel( ASAAC_PublicId GlobalVcId );
	/*!< \param[in] GlobalVcId ASAAC_PublicId of the GlobalVc.
	 * 
	 *   \return    reference to GlobalVc object instance handling the designated Global Vc.
	 *              If no GlobalVc object for said Global Vc has been instantiated so far,
	 *              it returns NULL.
	 */
	
	//! get reference to a LocalVc instance
	LocalVc*				getLocalVirtualChannel( ASAAC_PublicId ProcessId, ASAAC_PublicId GlobalVcId, ASAAC_PublicId LocalVcId);
	/*!< \param[in] GlobalVcId ASAAC_PublicId of the GlobalVc.
	 * 
	 *   \param[in] LocalVcId ASAAC_PublicId of the LocalVc.
	 * 
	 *   \return    reference to LocalVc object instance handling the designated Local Vc.
	 *              If no LocalVc object for said Local Vc has been instantiated so far,
	 *              it returns NULL.
	 */
	
	 
	//! create a GlobalVc (as SMOS)
	GlobalVc*				createVirtualChannel( const ASAAC_VcDescription& Description );
	/*!< This function creates a shared memory object and required data structures to communicate
	 *   via this shared memory object as a Global Vc, as is required for the setup of a virtual channel
	 *   via the SMOS call createVirtualChannel().
	 * 
	 *   \param[in] Description ASAAC_VcDescription of the virtual channel as supplied to the SMOS call,
	 *                          containing all data required to instantiate and set up a GlobalVc object
	 *                          that acts as a master for a shared memory object handling that virtual channel.
	 * 
	 *   \return    reference to the instantiated GlobalVc object now handling the designated GlobalVc.
	 *              0, if said GlobalVc could not successfully be created.
	 */

	
	//! delete a GlobalVc (as SMOS)
	void					destroyVirtualChannel( const ASAAC_PublicId vc_id );
	/*!< This function destroy a shared memory object.
	 * 
	 *   \param[in] vc_id is the identification number for the destroying global vc. 
	 * 
	 *   \return    refer to STANAG 4626, 11.7.3.2.
	 */

	void					destroyAllVirtualChannels();
    
    void                    releaseVirtualChannel( ASAAC_PublicId GlobalVcId );

    void                    releaseAllVirtualChannels();
	
	//! SMOS call to attach a process or thread to a global virtual channels while implicit creating a local virtual channel. 
	void			 		attachChannelToProcessOrThread(const ASAAC_VcMappingDescription vc_mapping);

	//! SMOS call to detach all threads from global virtual channel. 
	void			  		detachAllThreadsOfProcessFromVc(const ASAAC_PublicId vc_id, const ASAAC_PublicId process_id);
	
	//! SMOS call to configure a network longerface. 
	void			 		configureInterface( const ASAAC_InterfaceData& if_config );
	
	//! SMOS call to configure a transfer connection.
	void			 		createTransferConnection( const ASAAC_TcDescription& tc_desc );

	//! SMOS call to remove a transfer connection. 
	void			 		destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descr );
	
	//! SMOS call to longerface with NII
	void			 		getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_desc, ASAAC_NetworkPortStatus& network_status );
	
	//! SMOS call to control the switching of the PCS 
	void			 		attachTransferConnectionToVirtualChannel( const ASAAC_VcToTcMappingDescription& vc_to_tc_mapping );

	//! SMOS call to control the switching of the PCS 
	void			 		detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id );

	//! SMOS call to obtain data to Security Manager 
	void			 		getPMData(ASAAC_PublicId &vc_id, ASAAC_Address &message_buffer_reference, const unsigned long max_msg_length, unsigned long &msg_length, const ASAAC_TimeInterval timeout);

	//! SMOS call to send data back from Security Manager 
	void			  		returnPMData(const ASAAC_PublicId vc_id, const ASAAC_Address message_buffer_reference, const unsigned long msg_length, const ASAAC_ReturnStatus sm_return_status);

	//! APOS call to send a message nonblocking 
	void			 		sendMessageNonblocking(const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference, const unsigned long actual_size);
	
	//! APOS call to receive a message nonblocking 
	void			 		receiveMessageNonblocking(const ASAAC_PublicId local_vc_id, const unsigned long maximum_size, const ASAAC_Address message_buffer_reference, unsigned long &actual_size);

	//! APOS call to send a message 
	void			 		sendMessage(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, const ASAAC_Address message_buffer_reference, const unsigned long actual_size);

	//! APOS call to receive a message 
	void			 		receiveMessage(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, const unsigned long maximum_size, const ASAAC_Address message_buffer_reference, unsigned long& actual_size);

	//! APOS call to lock a buffer 
	void			 		lockBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, ASAAC_Address& message_buffer_reference, const unsigned long maximum_size);

	//! APOS call to send a buffer 
	void			 		sendBuffer( const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference, const unsigned long maximum_size );

	//! APOS call to receive a buffer 
	void			 		receiveBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, ASAAC_Address &message_buffer_reference, unsigned long &actual_size);

	//! APOS call to unlock a buffer 
	void			 		unlockBuffer ( const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference );

	//! APOS call to wait for data on multiple virtual channels 
	void			 		waitOnMultiChannel(const ASAAC_PublicIdSet vc_set_in, const unsigned long min_no_vc, ASAAC_PublicIdSet &vc_set_out, const ASAAC_TimeInterval timeout);

	//! predict the amount of memory for control and data structures to be allocated via an allocator
	static size_t	predictSize();
	
	//SBS: synchronize with GSM via PCS server/client
	void letPcsListenToAttachedChannels();
	
private:
	friend class LocalVc;

	CommunicationManager();
	
	GlobalVc*				getVirtualChannel( ASAAC_PublicId GlobalVcId, long &Index );

    typedef struct
    {
        ASAAC_PublicId  GlobalVcId;
        SessionId       GlobalVcSessionId;
    } GlobalVcIndex;

	bool						m_IsInitialized;
	bool						m_IsMaster;
	//SBS: synchronize with GSM via PCS server/client
	bool						m_IsPcsListening;
	
	Semaphore					m_Semaphore;
	AllocatedArea				m_Allocator;
	
	GlobalVc	 				m_GlobalVcObject[OS_MAX_NUMBER_OF_GLOBALVCS];
	Shared<GlobalVcIndex>		m_GlobalVcIndex;
	
	ASAAC::PCS::Client 			m_PCSClient;
	TwoWayCommunication 		m_SMCommunication;
	ASAAC_Address				m_SMLastMessageBufferReference;
};

#endif /*COMMUNICATIONMANAGER_HH_*/
