#include "CommunicationManager.hh"

#include "Communication/VcUpdateSignal.hh"

#include "Managers/FileNameGenerator.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"
#include "TwoWayCommunication.hh"

CommunicationManager::CommunicationManager()
{
	m_IsInitialized = false;
	m_IsMaster = false;
}


size_t	CommunicationManager::predictSize()
{
	size_t CumulativeSize = 0;

	// m_GlobalVcId
	CumulativeSize += Shared<GlobalVcIndex>::predictSize(OS_MAX_NUMBER_OF_GLOBALVCS);

	// m_Semaphore
	CumulativeSize += Semaphore::predictSize();

	// VcUpdateSignal
	CumulativeSize += VcUpdateSignal::predictSize();

	return CumulativeSize;
}


void CommunicationManager::initialize( bool IsMaster, Allocator *ParentAllocator )
{
	if (m_IsInitialized)
		throw DoubleInitializationException(LOCATION);

    try
    {
        m_IsInitialized = true;

		m_IsMaster = IsMaster;

		//SBS: synchronize with GSM via PCS server/client
		m_IsPcsListening = false;

		m_Allocator.initialize( ParentAllocator, predictSize() );

		m_Semaphore.initialize( &m_Allocator, IsMaster );
		m_GlobalVcIndex.initialize( &m_Allocator, OS_MAX_NUMBER_OF_GLOBALVCS );

		m_PCSClient.setRequestVc( OS_PCS_CLIENT_VC_REQUEST );
		m_PCSClient.setReplyVc( OS_PCS_CLIENT_VC_REPLY );

		VcUpdateSignal::initialize( IsMaster, &m_Allocator );

		m_SMLastMessageBufferReference = 0;

		if ( m_IsMaster )
		{
			for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_GLOBALVCS; Index ++ )
	        {
	            m_GlobalVcIndex[ Index ].GlobalVcId         = OS_UNUSED_ID;
	            m_GlobalVcIndex[ Index ].GlobalVcSessionId  = OS_UNUSED_ID;
	        }
		}
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error initializing CommunicationManager", LOCATION);

        deinitialize();

        throw;
    }
}


void CommunicationManager::deinitialize()
{
	if (m_IsInitialized == false)
		return;

	try
	{
		releaseAllVirtualChannels();

		VcUpdateSignal::deinitialize();

		m_Semaphore.deinitialize();
		m_GlobalVcIndex.deinitialize();

		m_Allocator.deinitialize();
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error while deinitializing CommunicationManager", LOCATION);
		e.raiseError();
	}

	m_IsInitialized = false;
}


CommunicationManager::~CommunicationManager()
{
}


CommunicationManager* CommunicationManager::getInstance()
{
	static CommunicationManager ThisManager;

	return &ThisManager;
}

bool CommunicationManager::isInitialized()
{
	return m_IsInitialized;
}


void CommunicationManager::configurePCS()
{
    m_PCSClient.configurePCS();
}


//***********************************************************************************************
//* Global Virtual Channel functions															*
//***********************************************************************************************

long CommunicationManager::getVirtualChannelIndex( ASAAC_PublicId GlobalVcId )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_GLOBALVCS; Index ++ )
	{
		if ( m_GlobalVcIndex[ Index ].GlobalVcId == GlobalVcId )
			return Index;
	}

	return -1;
}


GlobalVc* CommunicationManager::getVirtualChannel( ASAAC_PublicId GlobalVcId, long &Index )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

    GlobalVc* obj = NULL;

    try
    {
	    ProtectedScope Access( "Searching a global vc", m_Semaphore );

		if (GlobalVcId == OS_UNUSED_ID)
			return NULL;

		Index = getVirtualChannelIndex( GlobalVcId );

		if ( Index == -1 )
			return NULL;

		obj = &m_GlobalVcObject[Index];

		//This is the smallest configuration, we could predict
		ASAAC_VcDescription Description;
		Description.max_number_of_buffers = 1;
		Description.max_number_of_threads_attached = 2;
		Description.max_number_of_TCs_attached = 0;
		Description.max_msg_length = 1;

		if (obj->isInitialized())
		{
			if (obj->getSessionId() != m_GlobalVcIndex[ Index ].GlobalVcSessionId)
			{
				obj->deinitialize();
				obj->initialize(GlobalVcId, false, Description, m_GlobalVcIndex[ Index].GlobalVcSessionId);
			}
		}
		else
		{
			obj->initialize(GlobalVcId, false, Description, m_GlobalVcIndex[ Index ].GlobalVcSessionId);
		}
	}
	catch (ASAAC_Exception &e)
	{
		destroyVirtualChannel( GlobalVcId );

		e.addPath("GlobalVc has been destroyed.", LOCATION);

		obj = NULL;

		throw;
	}

	return obj;
}


LocalVc* CommunicationManager::getLocalVirtualChannel( ASAAC_PublicId ProcessId, ASAAC_PublicId GlobalVcId, ASAAC_PublicId LocalVcId)
{
	GlobalVc* GVc = getVirtualChannel( GlobalVcId );

	if (GVc == 0)
		return NULL;

	return GVc->getLocalVc( ProcessId, LocalVcId );
}


GlobalVc* CommunicationManager::getVirtualChannel( ASAAC_PublicId GlobalVcId )
{
	long dummy;
	return getVirtualChannel( GlobalVcId, dummy );
}


GlobalVc* CommunicationManager::createVirtualChannel( const ASAAC_VcDescription& Description)
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

	try
	{
		ProtectedScope Access( "Creating a global vc", m_Semaphore );

		long Index = getVirtualChannelIndex( Description.global_vc_id );

		if (Index != -1)
			throw OSException("GlobalVC is already created with dedicated id", LOCATION);

		Index = getVirtualChannelIndex( OS_UNUSED_ID );

		if ( Index == -1 )
			throw OSException("Maximum number of global Vcs reached. No more free slots.", LOCATION);

		//Maybe an old object is still initialized
		if ( m_GlobalVcObject[Index].isInitialized() )
			m_GlobalVcObject[Index].deinitialize();

		//Initialie global data
        m_GlobalVcIndex[Index].GlobalVcId        = Description.global_vc_id;
        m_GlobalVcIndex[Index].GlobalVcSessionId = rand();

		//Initialze local data
		m_GlobalVcObject[Index].initialize( Description.global_vc_id, true, Description, m_GlobalVcIndex[Index].GlobalVcSessionId );

		return &m_GlobalVcObject[Index];
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath( (ErrorString << "Error while creating Global VC: " << CharSeq(Description.global_vc_id)).c_str(), LOCATION);

		throw;
	}
}


void	CommunicationManager::destroyVirtualChannel( const ASAAC_PublicId vc_id )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

	try
    {
		ProtectedScope Access( "Destroying a global vc", m_Semaphore );

		long Index = getVirtualChannelIndex( vc_id );

		if (Index == -1)
			throw OSException( (ErrorString << "Global VC is not available:" << vc_id).c_str(), LOCATION);

		m_GlobalVcIndex[Index].GlobalVcId         = OS_UNUSED_ID;
        m_GlobalVcIndex[Index].GlobalVcSessionId  = OS_UNUSED_ID;

		if ( m_GlobalVcObject[Index].isInitialized() )
			m_GlobalVcObject[Index].deinitialize();
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error while destroying Global VC", LOCATION);

		throw;
	}
}


void	CommunicationManager::destroyAllVirtualChannels()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_GLOBALVCS; Index ++ )
	{
		if (m_GlobalVcIndex[ Index ].GlobalVcId != OS_UNUSED_ID)
		{
			destroyVirtualChannel( m_GlobalVcIndex[ Index ].GlobalVcId );
		}
	}
}


void CommunicationManager::releaseVirtualChannel( ASAAC_PublicId GlobalVcId )
{
    long Index = getVirtualChannelIndex( GlobalVcId );

    if (Index != -1)
    {
    	m_GlobalVcObject[Index].deinitialize();
	}
}


void CommunicationManager::releaseAllVirtualChannels()
{
	for ( unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_GLOBALVCS; Index ++ )
	{
		if ( m_GlobalVcObject[ Index ].isInitialized() )
			m_GlobalVcObject[ Index ].deinitialize();
	}
}


void CommunicationManager::attachChannelToProcessOrThread(const ASAAC_VcMappingDescription vc_mapping)
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

	try
	{
		GlobalVc* ThisVc = CommunicationManager::getInstance()->getVirtualChannel( vc_mapping.global_vc_id );

		if ( ThisVc == 0 )
			throw OSException( (ErrorString << "Global VC is not available: " << CharSeq(vc_mapping.global_vc_id)).c_str(), LOCATION);

		ThisVc->createLocalVc( vc_mapping );
	}
	catch ( ASAAC_Exception &e)
	{
		e.addPath("Error attaching channel to process or thread", LOCATION);

		throw;
	}
}


void CommunicationManager::detachAllThreadsOfProcessFromVc(const ASAAC_PublicId vc_id, const ASAAC_PublicId process_id)
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		CharacterSequence ErrorString;

		GlobalVc* ThisVc = CommunicationManager::getInstance()->getVirtualChannel( vc_id );

		if ( ThisVc == NULL )
			throw OSException( (ErrorString << "Global VC is not available: " << vc_id).c_str(), LOCATION);

		ThisVc->removeLocalVcsFromProcess( process_id );
	}
	catch ( ASAAC_Exception &e)
	{
		//HR: Modified message to be consistent
		e.addPath("Error detaching all threads from VCs", LOCATION);

		throw;
	}
}


//***********************************************************************************************
//* TransferChannel functions																	*
//***********************************************************************************************

void CommunicationManager::configureInterface( const ASAAC_InterfaceData& if_config )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

    try
    {
	    m_PCSClient.configureInterface( if_config );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error configuring interface", LOCATION);

        throw;
    }
}


void CommunicationManager::createTransferConnection( const ASAAC_TcDescription& tc_desc )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

    try
    {
        m_PCSClient.createTransferConnection( tc_desc );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error creating TransferConnection", LOCATION);

        throw;
    }
}


void CommunicationManager::destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descr )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

    try
    {
        m_PCSClient.destroyTransferConnection( tc_id, network_descr );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error destroying TransferConnection", LOCATION);

        throw;
    }
}


void CommunicationManager::getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_desc, ASAAC_NetworkPortStatus& network_status )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

    try
    {
	    m_PCSClient.getNetworkPortStatus( network_desc, network_status );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error retrieving NetworkPortStatus", LOCATION);

        throw;
    }
}


void CommunicationManager::attachTransferConnectionToVirtualChannel( const ASAAC_VcToTcMappingDescription& vc_to_tc_mapping )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	ASAAC_VcDescription *vc_description;
	GlobalVcStatus vc_status;
	ASAAC_PublicId tc_id;
	ASAAC_Bool is_data_representation;
	ASAAC_VcMappingDescription vc_mapping;
	ASAAC_TcDescription tc_description;
	unsigned long number_of_message_buffers;

	try
	{
		GlobalVc* ThisVc = getVirtualChannel( vc_to_tc_mapping.global_vc_id );

		if (ThisVc == NULL)
			throw OSException("Virtual channel not found", LOCATION);

		vc_description = ThisVc->getDescription();
		vc_status = ThisVc->getStatus();

		tc_id = vc_to_tc_mapping.tc_id;
		is_data_representation = vc_to_tc_mapping.is_data_representation;

		m_PCSClient.getTransferConnectionDescription( tc_id, tc_description );

		if ((vc_description->max_number_of_buffers > LONG_MAX) ||
		    (vc_description->max_number_of_threads_attached > LONG_MAX))
		    throw OSException("Values are out of bounds (max_number_of_buffers, max_number_of_threads_attached).", LOCATION);

		// Calculate fair number of buffers for PCS thread
		//(suppose, every attached thread will get the same number of buffers)
		number_of_message_buffers = div(
			(long)vc_description->max_number_of_buffers,
			(long)vc_description->max_number_of_threads_attached).quot;

		unsigned long free_buffers = vc_description->max_number_of_buffers - vc_status.NumberOfBuffers;

		if (number_of_message_buffers > free_buffers)
			number_of_message_buffers = free_buffers;

		if (number_of_message_buffers == 0)
			throw OSException("Not enough free buffer size for TC.", LOCATION);

/**************************** PROBLEM WITH VC/TC MAPPING WAS HERE *******************************************/

    /* The status of the global VC has to be considered in order to solve VC/TC mapping problem -SBS */
    if(!ThisVc->isPcsAttached())
    {
		  vc_mapping.global_pid     			 = OS_PROCESSID_PCS;
	    vc_mapping.local_vc_id				 = vc_to_tc_mapping.global_vc_id;
	    vc_mapping.global_vc_id				 = vc_to_tc_mapping.global_vc_id;
	    vc_mapping.buffer_size				 = vc_description->max_msg_length;
	    vc_mapping.number_of_message_buffers = number_of_message_buffers;
	    vc_mapping.is_reading				 = BoolNot(tc_description.is_receiver);
	    vc_mapping.is_lifo_queue			 = ASAAC_BOOL_FALSE;
	    vc_mapping.is_refusing_queue		 = ASAAC_BOOL_TRUE;

	   	Process *PCSProcess = ProcessManager::getInstance()->getProcess(OS_PROCESSID_PCS);
	   	if (PCSProcess == NULL)
	   		throw OSException("PCS Process was not found.", LOCATION);

	   	//Stop PCS Process now
	    //PCSProcess->stop();

	    try //This inner exception block is needed to surely restart PCS Process
	    {
			ThisVc->createLocalVc( vc_mapping );
	    }
	    catch (ASAAC_Exception &e)
	    {
            //Restart PCS Process
	    	try
	    	{
	    		//PCSProcess->run();
	    	}
	    	catch (ASAAC_Exception &e2)
	    	{
                e.addPath("PCS Process couldn't be restarted.", LOCATION);
	    	}

            throw;
	    }
      /* Now set status of the global VC to solve VC/TC mapping problem -SBS */
      ThisVc->attachPcs();
      
		//Restart PCS Process
	    //PCSProcess->run();
    } 
       
/**************************** PROBLEM WITH VC/TC MAPPING WAS HERE *******************************************/
    
    m_PCSClient.attachTransferConnectionToVirtualChannel( *vc_description, tc_id, is_data_representation);
	}
	catch (ASAAC_Exception &e)
	{
        e.addPath("Error attaching TransferConnection to VirtualChannel", LOCATION);

        throw;
	}
}


void CommunicationManager::detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id )
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

    try
    {
    	m_PCSClient.detachTransferConnectionFromVirtualChannel( vc_id, tc_id );

		GlobalVc* ThisVc = getVirtualChannel( vc_id );

		if ( ThisVc == NULL )
			throw OSException("Global VC was not found.", LOCATION);

	   	Process *PCSProcess = ProcessManager::getInstance()->getProcess(OS_PROCESSID_PCS);
	   	if (PCSProcess == NULL)
	   		throw OSException("PCS Process was not found.", LOCATION);

	   	//Stop PCS Process now
	    //PCSProcess->stop();

	    try //This inner exception block is needed to surely restart PCS Process
	    {
			ThisVc->removeLocalVcsFromProcess( OS_PROCESSID_PCS );
	    }
	    catch (ASAAC_Exception &e)
	    {
            //Restart PCS Process
            //PCSProcess->run();

	    	throw;
	    }

		//Restart PCS Process
	    //PCSProcess->run();
    }
	catch (ASAAC_Exception &e)
	{
        e.addPath("Error detaching TransferConnection from VirtualChannel", LOCATION);

        throw;
	}
}


//***********************************************************************************************
//* Security Manager functions																	*
//***********************************************************************************************

void CommunicationManager::getPMData(ASAAC_PublicId &vc_id, ASAAC_Address &message_buffer_reference, const unsigned long max_msg_length, unsigned long &msg_length, const ASAAC_TimeInterval timeout)
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	CharacterSequence ErrorString;

	try
	{
		//generate timeout stamp
		ASAAC_Time abs_timeout = TimeStamp(timeout).asaac_Time();

		//Check current process environment
		ProcessManager *PM = ProcessManager::getInstance();
		ThreadManager *TM = ThreadManager::getInstance();

		Process *P = PM->getCurrentProcess();
		Thread *T = TM->getCurrentThread();

		ASAAC_PublicId currentProcessId = P->getId();
		ASAAC_PublicId currentThreadId = T->getId();

		//Check and prepare communication environment
		m_SMCommunication[0].setSenderConfiguration(   OS_PROCESSID_PCS, 1,               OS_SM_SERVER_VC_SEND );
		m_SMCommunication[0].setReceiverConfiguration( currentProcessId, currentThreadId, OS_SM_CLIENT_VC_RECEIVE );
		m_SMCommunication[0].setGlobalVcConfiguration( OS_SM_GLOBAL_VC_REQUEST, max_msg_length );

		m_SMCommunication[0].assureCommunication();

		//Free buffer of last getPMData call
		LocalVc* LVc = P->getAttachedVirtualChannel( OS_SM_CLIENT_VC_RECEIVE );

		if ( LVc == NULL )
			throw FatalException( (ErrorString << "Local VC not found (" << CharSeq((unsigned long)OS_SM_CLIENT_VC_RECEIVE) << ")").c_str(), LOCATION);

		if (m_SMLastMessageBufferReference != 0)
		{
			LVc->unlockBuffer(m_SMLastMessageBufferReference);
			m_SMLastMessageBufferReference = 0;
		}

		//Send request to PCS to send data via vc
		m_PCSClient.getPMData( max_msg_length, abs_timeout, OS_SM_SERVER_VC_SEND, vc_id );

		//Now receive this data on local vc
	    LVc->receiveBuffer( message_buffer_reference, msg_length, abs_timeout );

		//ReceiveBuffer method returned successfully. Store the buffer reference.
	    m_SMLastMessageBufferReference = message_buffer_reference;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error retrieving PM data", LOCATION);

		throw;
	}
}


void CommunicationManager::returnPMData(const ASAAC_PublicId vc_id, const ASAAC_Address message_buffer_reference, const unsigned long msg_length, const ASAAC_ReturnStatus sm_return_status)
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	try
	{
		//Check current process environment
		ProcessManager *PM = ProcessManager::getInstance();
		ThreadManager *TM = ThreadManager::getInstance();

		Process *P = PM->getCurrentProcess();
		Thread *T = TM->getCurrentThread();

		ASAAC_PublicId currentProcessId = P->getId();
		ASAAC_PublicId currentThreadId = T->getId();

		//Check and prepare communication environment
		m_SMCommunication[1].setReceiverConfiguration( OS_PROCESSID_PCS, 1,               OS_SM_SERVER_VC_RECEIVE );
		m_SMCommunication[1].setSenderConfiguration(   currentProcessId, currentThreadId, OS_SM_CLIENT_VC_SEND );
		m_SMCommunication[1].setGlobalVcConfiguration( OS_SM_GLOBAL_VC_REPLY, msg_length );
		m_SMCommunication[1].assureCommunication();

		//Send data on local vc
		LocalVc* LVc = P->getAttachedVirtualChannel( OS_SM_CLIENT_VC_SEND );

		if ( LVc == 0 )
			throw FatalException("Local VC not found", LOCATION);

	    LVc->sendMessage( message_buffer_reference, msg_length, TimeStamp::Infinity().asaac_Time() );

		//Send now reply to PCS to get the data from dedicated vc
		m_PCSClient.returnPMData( vc_id, OS_SM_SERVER_VC_RECEIVE, sm_return_status );

	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error returning PM data", LOCATION);

        throw;
	}
}

//***********************************************************************************************
//* Local Virtual Channel functions																*
//***********************************************************************************************

void CommunicationManager::sendMessageNonblocking(const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference, const unsigned long actual_size)
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

        ThisLocalVc->sendMessageNonblocking( message_buffer_reference, actual_size );
    }
    catch ( ASAAC_Exception &e )
    {
        if (e.isTimeout())
            throw ResourceException("", LOCATION);
        else
	        throw;
    }
}


void CommunicationManager::receiveMessageNonblocking(const ASAAC_PublicId local_vc_id, const unsigned long maximum_size, const ASAAC_Address message_buffer_reference, unsigned long &actual_size)
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

        ThisLocalVc->receiveMessageNonblocking( message_buffer_reference, maximum_size, actual_size );
    }
    catch ( ASAAC_Exception &e )
    {
        if (e.isTimeout())
            throw ResourceException("", LOCATION);
        else
	        throw;
    }
}


void CommunicationManager::sendMessage(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, const ASAAC_Address message_buffer_reference, const unsigned long actual_size)
{
    try
    {
		Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

		if (ThisProcess == NULL)
			throw FatalException("Process object not found", LOCATION);

		LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

		if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

		ThisLocalVc->sendMessage( message_buffer_reference,
		                          actual_size,
								  TimeStamp(timeout).asaac_Time() );
    }
    catch ( ASAAC_Exception &e )
    {
        throw;
    }
}


void CommunicationManager::receiveMessage(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, const unsigned long maximum_size, const ASAAC_Address message_buffer_reference, unsigned long& actual_size)
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

		ThisLocalVc->receiveMessage( message_buffer_reference,
									 maximum_size,
									 actual_size,
									 TimeStamp(timeout).asaac_Time() );
    }
    catch ( ASAAC_Exception &e )
    {
        throw;
    }
}


void CommunicationManager::lockBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, ASAAC_Address& message_buffer_reference, const unsigned long maximum_size)
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

        ThisLocalVc->lockBuffer( message_buffer_reference,
	                             maximum_size,
	                             TimeStamp(timeout).asaac_Time() );
    }
    catch ( ASAAC_Exception &e )
    {
        throw;
    }
}


void CommunicationManager::sendBuffer( const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference, const unsigned long maximum_size )
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

	   ThisLocalVc->sendBuffer( message_buffer_reference, maximum_size, TimeInfinity );
    }
    catch ( ASAAC_Exception &e )
    {
        throw;
    }
}


void CommunicationManager::receiveBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval timeout, ASAAC_Address &message_buffer_reference, unsigned long &actual_size)
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

	    ThisLocalVc->receiveBuffer( message_buffer_reference,
		        	                actual_size,
					                TimeStamp(timeout).asaac_Time() );
    }
    catch ( ASAAC_Exception &e )
    {
        throw;
    }
}


void CommunicationManager::unlockBuffer ( const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference )
{
    try
    {
        Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

        if (ThisProcess == NULL)
            throw FatalException("Process object not found", LOCATION);

        LocalVc* ThisLocalVc = ThisProcess->getAttachedVirtualChannel( local_vc_id );

        if ( ThisLocalVc == NULL )
            throw FatalException("LocalVC object not found", LOCATION);

	    ThisLocalVc->unlockBuffer( message_buffer_reference );
    }
    catch ( ASAAC_Exception &e )
    {
        throw;
    }
}


void CommunicationManager::waitOnMultiChannel(const ASAAC_PublicIdSet vc_set_in, const unsigned long min_no_vc, ASAAC_PublicIdSet &vc_set_out, const ASAAC_TimeInterval timeout)
{
	try
	{
		LocalVc* LocalVcObjects[ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE];

		// generate list of objects for which to wait,
		// checkinf whether all VC's are receiving, too.
		for ( unsigned long Index = 0; Index < ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE; Index++ )
		{
			if ( vc_set_in[ Index ] == 0 )
			{
				LocalVcObjects[ Index ] = 0;

				//continue; pun: NULL terminates the vc set. Use break instead.
				break;
			}

			Process *ThisProcess = ProcessManager::getInstance()->getCurrentProcess();

			if (ThisProcess == NULL)
				throw OSException("Process not found", LOCATION);

			LocalVcObjects[Index] = ThisProcess->getAttachedVirtualChannel( vc_set_in[ Index ] );

			if ( LocalVcObjects[Index] == NULL )
			{
				throw OSException("Virtual channel not found", LOCATION);
			}

			if ( LocalVcObjects[Index]->getDescription()->is_reading == false )
			{
				throw OSException("Virtual channel is not a receiving one", LOCATION);
			}

			for ( unsigned long CompareIndex = 0; CompareIndex < Index; CompareIndex ++ )
			{
				if ( vc_set_in[ CompareIndex ] == vc_set_in[ Index ] )
				{
	//				return ASAAC_TM_ERROR;
				}
			}
		}


		Trigger* UpdateSignal = VcUpdateSignal::getInstance();

		// Waiting loop
		for (;;)
		{
			unsigned long TriggerState = UpdateSignal->getTriggerState();
			unsigned long AvailableVcs  = 0;

			//pun: Initialize field 0 to 0, if no objects are available in LocalVcObjects array.
			vc_set_out[ 0 ] = 0;

			for ( unsigned long Index = 0; Index < ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE; Index++ )
			{
				//vc_set_out[ Index ] = 0; pun: Index is not the index variable for vc_set_out

				if ( LocalVcObjects[ Index ] != NULL )
				{
					try
					{
				    	LocalVcObjects[ Index ]->waitForAvailableData( TimeStamp::Instant().asaac_Time() );

						vc_set_out[ AvailableVcs ] = vc_set_in[ Index ];

						AvailableVcs ++;
					}
					catch ( ASAAC_Exception &e )
					{
						vc_set_out[ AvailableVcs ] = 0;
					}
				}
				else break;
			}

			if ( AvailableVcs >= min_no_vc )
				break;

			VcUpdateSignal::getInstance()->waitForTrigger( TriggerState, timeout );
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error occured while waiting on multiple channels", LOCATION);

		throw;
	}
}

//SBS: synchronize with GSM via PCS server/client
void CommunicationManager::letPcsListenToAttachedChannels()
{
	if (m_IsInitialized == false)
		throw UninitializedObjectException(LOCATION);

	if(m_IsPcsListening)
		return;

    try
    {
	    m_PCSClient.listenAttachedChannels();

		m_IsPcsListening = true;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error when letting PCS listen to attached channels", LOCATION);

        throw;
    }
}

