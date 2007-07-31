#include "PcsObject.hh"
#include "NII.hh"

#include "PcsCIncludes.hh"

PCS::PCS()
{

}

ASAAC_ReturnStatus PCS::configureInterface( const ASAAC_InterfaceData& if_config )
{
#ifdef _DEBUG_       
	cout << "PCS::configureInterface()  " << endl;
#endif
	
	return ASAAC_ERROR;
};	

ASAAC_ReturnStatus PCS::createTransferConnection( const ASAAC_TcDescription& tc_description )
{
#ifdef _DEBUG_       
	cout << "PCS::createTransferConnection()  " << endl;
#endif
	

	ASAAC_InterfaceConfigurationData configuration_data;
	
	ASAAC_NiiReturnStatus niiRet = ASAAC_MOS_configureInterface(NII_IF_ETHERNET, &tc_description.network_descr, &configuration_data);
	
	if(niiRet == ASAAC_MOS_NII_CALL_COMPLETE)
	{
		#ifdef _DEBUG_       
		cout << "PCS::initialize() configured Ethernet interface with network " << tc_description.network_descr.network << " and port " << tc_description.network_descr.port << endl;
		#endif
		
		if(m_Configuration.addNetwork(tc_description.network_descr) == ASAAC_ERROR)
		{
			cerr << "PCS::createTransferConnection() failed to add network to PCS configuration " << tc_description.network_descr.network << " and port " << tc_description.network_descr.port << endl;
		}		
	}
	else
	{
		cerr << "PCS::createTransferConnection() failed to configure interface with network " << tc_description.network_descr.network << " and port " << tc_description.network_descr.port << endl;
	}			
	
	UdpConfiguration udp_conf;
        
	TcNetworkSpecific nw_spec_conf_data;

	nw_spec_conf_data.udp = udp_conf;

	ASAAC_TC_ConfigurationData tc_configuration_data = { 1, NETWORK_TYPE_UDP, nw_spec_conf_data};
	
	ASAAC_TransferDirection send_receive = tc_description.is_receiver == ASAAC_BOOL_TRUE ? ASAAC_TRANSFER_DIRECTION_RECEIVE : ASAAC_TRANSFER_DIRECTION_SEND;
	
	ASAAC_TransferType message_streaming = tc_description.is_msg_transfer == ASAAC_BOOL_TRUE ? ASAAC_TRANSFER_TYPE_MESSAGE : ASAAC_TRANSFER_TYPE_STREAMING;
	
	niiRet = ASAAC_MOS_configureTransfer(tc_description.tc_id, 
                                          &tc_description.network_descr, 
                                          send_receive, 
                                          message_streaming, 
                                          tc_configuration_data, 
                                          ASAAC_BOOL_FALSE, 
                                          0);
                                              
	if(niiRet == ASAAC_MOS_NII_CALL_COMPLETE)
	{
#ifdef _DEBUG_       
		cout << "PCS::createTransferConnection() configured transfer connection " << tc_description.tc_id << endl;
#endif				
	}
	else
	{
		cerr << "PCS::createTransferConnection() failed to configure transfer connection " << tc_description.tc_id << " with status " << cMosNii::spell(niiRet) << endl;
		return ASAAC_ERROR;
	}			
	
	m_Configuration.addTcDescription(tc_description);
	
	ASAAC_TimeInterval defaultTcRateLimit = {0,20000000}; //COMMENT.SBS> way to specify limits is not known yet
	
	m_Limiter.setRateLimit( tc_description.tc_id, defaultTcRateLimit );
	
	return ASAAC_SUCCESS;
};

ASAAC_ReturnStatus PCS::getTransferConnectionDescription( ASAAC_PublicId tc_id, ASAAC_TcDescription& tc_description )
{
#ifdef _DEBUG_       
	cout << "PCS::getTransferConnectionDescription()  " << endl;
#endif
	
	return m_Configuration.getTcDescription(tc_id, tc_description);
	
	return ASAAC_ERROR;
};

ASAAC_ReturnStatus PCS::destroyTransferConnection( ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor& network_descriptor )
{
#ifdef _DEBUG_       
	cout << "PCS::destroyTransferConnection()  " << endl;
#endif
	
	ASAAC_NiiReturnStatus niiRet = ASAAC_MOS_destroyTransfer(tc_id, &network_descriptor);
	  
  	if(niiRet == ASAAC_MOS_NII_CALL_COMPLETE)
  	{
  		return ASAAC_SUCCESS;
  	}
  	else
  	{
		#ifdef _DEBUG_
		cerr << "PCS handleDestroyTransferConnection() failed in ASAAC_MOS_destroyTransfer" << endl;
		#endif
    	return ASAAC_ERROR;
  	}
	
	return ASAAC_ERROR;
};	

ASAAC_ReturnStatus PCS::getNetworkPortStatus( const ASAAC_NetworkDescriptor& network_descriptor, ASAAC_NetworkPortStatus& status )
{
#ifdef _DEBUG_       
	cout << "PCS::getNetworkPortStatus()  " << endl;
#endif
	
	return ASAAC_ERROR;
};	
ASAAC_ReturnStatus PCS::attachTransferConnectionToVirtualChannel( const ASAAC_VcDescription vc_description, ASAAC_PublicId tc_id, ASAAC_Bool is_data_representation )
{
#ifdef _DEBUG_       
	cout << "PCS::attachTransferConnectionToVirtualChannel()  " << endl;
#endif

	if(m_Configuration.addVcDescription( vc_description ) != ASAAC_SUCCESS)
	{
#ifdef _DEBUG_       
		cerr << "PCS::attachTransferConnectionToVirtualChannel() failed when adding VC description " << endl;
#endif
		return ASAAC_ERROR;
	}
	
	ASAAC_TcDescription tc_desc;
	
	if(m_Configuration.getTcDescription(tc_id, tc_desc) != ASAAC_SUCCESS)
	{
#ifdef _DEBUG_       
		cerr << "PCS::attachTransferConnectionToVirtualChannel() failed when searching TC description " << endl;
#endif
		return ASAAC_ERROR;	
	}
	
	ASAAC_VcToTcMappingDescription TcMapping; //depends on host configuration
	TcMapping.is_data_representation = is_data_representation;
    TcMapping.global_vc_id = vc_description.global_vc_id;
	TcMapping.tc_id        = tc_id;

	if(m_Configuration.addTcMapping(TcMapping) != ASAAC_SUCCESS)
	{
#ifdef _DEBUG_       
		cerr << "PCS::attachTransferConnectionToVirtualChannel() failed when adding TcToVc mapping description " << endl;
#endif
		return ASAAC_ERROR;
	}
	
	m_Configuration.addLocalVc( vc_description.global_vc_id, vc_description.global_vc_id );
	
	if(tc_desc.is_receiver == ASAAC_BOOL_FALSE)
	{
		m_Listener.addListeningConsumer( vc_description.global_vc_id, m_Marshalling );
	}
	
	return ASAAC_SUCCESS;
};

ASAAC_ReturnStatus PCS::detachTransferConnectionFromVirtualChannel( ASAAC_PublicId vc_id, ASAAC_PublicId tc_id )
{
#ifdef _DEBUG_       
	cout << "PCS::detachTransferConnectionFromVirtualChannel()  " << endl;
#endif
	
	return ASAAC_ERROR;
};

ASAAC_TimedReturnStatus PCS::getPMData(unsigned long max_len, ASAAC_Time timeout, ASAAC_PublicId snd_vc, ASAAC_PublicId& vc_id)
{
#ifdef _DEBUG_       
	cout << "PCS::getPMData(" << max_len << ", " << snd_vc << ")" << endl;
#endif
	
	ASAAC_Address data = 0;
	unsigned long length = 0;
	
	ASAAC_TimeInterval int_timeout = TimeStamp(timeout).asaac_Interval();
	
	ASAAC_TimedReturnStatus getMsgStatus = m_PmQueue.getMessage(vc_id, int_timeout, data, length);
	
	if(getMsgStatus == ASAAC_TM_SUCCESS)
	{
		if(length > max_len)
		{
		#ifdef _DEBUG_       
			cerr << "PCS::getPMData() received data length " << length << " is greater than max length " << max_len << endl;		 			
		#endif
			
			return ASAAC_TM_ERROR;	
		}
		
#ifdef _DEBUG_       
		cout << "PCS::getPMData()  sendMessage(" << snd_vc << ", " << data << ", " << length << ")" << endl;
		cout << "PCS::getPMData() int_timeout sec : " << int_timeout.sec << " nsec : " << int_timeout.nsec << endl; 
#endif
		ASAAC_TimedReturnStatus sendStatus = ASAAC_APOS_sendMessage( snd_vc, &TimeIntervalInfinity, data, length );	
		
		if(sendStatus == ASAAC_TM_ERROR)
		{
#ifdef _DEBUG_       
			cerr << "PCS::getPMData() error when sending message on VC " << snd_vc << endl;		 
#endif
		}
		else if(sendStatus == ASAAC_TM_TIMEOUT)
		{
#ifdef _DEBUG_       
			cerr << "PCS::getPMData() timeout when sending message on VC " << snd_vc << endl;		 			
#endif
		}
#ifdef _DEBUG_       
		cout << "PCS::getPMData() message sent back on VC" << snd_vc << endl;
#endif
		
		return sendStatus;
	}
	else if(getMsgStatus == ASAAC_TM_TIMEOUT)
	{
#ifdef _DEBUG_       
		cerr << "PCS::getPMData() timeout when getting message from Queue" << endl;		 			
#endif
	}
	else
	{	
#ifdef _DEBUG_       
		cerr << "PCS::getPMData()  error when getting message from Queue" << endl;		 			
#endif	
	}
	
	return getMsgStatus;
};

ASAAC_ReturnStatus PCS::returnPMData(ASAAC_PublicId vc_id, ASAAC_PublicId rec_vc, ASAAC_ReturnStatus sm_return_status)
{
#ifdef _DEBUG_       
	cout << "PCS::returnPMData(" << vc_id << ", " << rec_vc << ", " << sm_return_status << ")  " << endl;
#endif
	static char msg_buffer[PCS_MAX_SIZE_OF_TCMESSAGE];
	unsigned long actual_size = 0;


#ifdef _DEBUG_       
		cout << "PCS::returnPMData()  receiveMessage(" << rec_vc << ", " << PCS_MAX_SIZE_OF_TCMESSAGE << ", " << (void*) msg_buffer << ")" << endl;
#endif
	ASAAC_TimedReturnStatus receiveStatus = ASAAC_APOS_receiveMessage(rec_vc, &TimeIntervalInstant, PCS_MAX_SIZE_OF_TCMESSAGE, msg_buffer, &actual_size);
	
	if(receiveStatus == ASAAC_TM_SUCCESS)
	{
#ifdef _DEBUG_       
		cout << "PCS::returnPMData()  received message on VC " << rec_vc << endl;
#endif
		
		return m_PmQueue.putMessage(vc_id, msg_buffer, actual_size);
	
	}
	
	return ASAAC_ERROR;
};




void PCS::initialize()
{	
#ifdef _DEBUG_       
	cout << "PCS::PCS() initialization "<< endl;
#endif
	
#ifdef _DEBUG_
	short int word = 0x0001;
  	char *byte = (char *) &word;
  	if(byte[0] != 0)
  	{
  		cout << "PCS runs on LITTLE ENDIAN system" << endl;
  	}
  	else
  	{
   		cout << "PCS runs on BIG ENDIAN system" << endl;
  	}
#endif
  	
	try	
	{	
		m_Configuration.initialize();

		m_Listener.initialize();
		m_PmQueue.initialize();
		m_OutgoingPmFilter.initialize();
		m_Marshalling.initialize();
		m_Switch.initialize();
		m_Limiter.initialize();
		m_Packer.initialize();
	    m_NiiSender.initialize();
		
		m_NiiReceiver.initialize();
		m_UnPacker.initialize();
		m_UnMarshalling.initialize();
		m_IncomingPmFilter.initialize();
		m_GlobalVcSend.initialize();
		m_Sender.initialize();
		
		
		m_Marshalling.setConfiguration( m_Configuration );
		m_Marshalling.setOutputConsumer( m_OutgoingPmFilter );
		
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup Marshalling" << endl;
#endif
		
		m_OutgoingPmFilter.setConfiguration( m_Configuration );
		m_OutgoingPmFilter.setOutputConsumer( m_Switch );
		m_OutgoingPmFilter.setQueue(m_PmQueue);
			
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup outgoing PM Filter" << endl;
#endif		

		m_Switch.setConfiguration( m_Configuration );
		m_Switch.setOutputConsumer( m_Packer );
#ifdef _DEBUG_       
	    cout << "PCS::initialize() Setup Switch" << endl;
#endif
       	
		m_Packer.setConfiguration( m_Configuration );
		m_Packer.setOutputConsumer( m_Limiter );
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup Packer" << endl;
#endif
		
		m_Limiter.setConfiguration( m_Configuration );
		m_Limiter.setOutputConsumer( m_NiiSender );
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup Limiter" << endl;
#endif
		
        
        ////////////////////////////////////////////
        ///////////NII Users Start//////////////////
        ////////////////////////////////////////////

        
        m_NiiReceiver.addListeningConsumer(m_UnPacker);
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup NiiReceiver" << endl;
#endif
        		
        ////////////////////////////////////////////
        ///////////NII Users End ///////////////////
        ////////////////////////////////////////////
        
		m_UnPacker.setConfiguration( m_Configuration );
		m_UnPacker.setOutputConsumer( m_IncomingPmFilter  );
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup UnPacker" << endl;
#endif
		
		m_IncomingPmFilter.setConfiguration( m_Configuration );
		m_IncomingPmFilter.setOutputConsumer( m_UnMarshalling );
		m_IncomingPmFilter.setQueue(m_PmQueue);
		
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup IncomingPmFilter" << endl;
#endif
		
		m_UnMarshalling.setConfiguration( m_Configuration );
		m_UnMarshalling.setOutputConsumer( m_GlobalVcSend );
#ifdef _DEBUG_       
         cout << "PCS::initialize() Setup UnMarshalling" << endl;
#endif
		
		
		m_GlobalVcSend.setConfiguration( m_Configuration );
		m_GlobalVcSend.setOutputConsumer( m_Sender ); 
#ifdef _DEBUG_       
        cout << "PCS::initialize() Setup GlobalVcSend" << endl;
#endif

	}
	catch ( PcsException &e )
	{
		cerr << e.getFullMessage() << endl;
	}
}

void PCS::deinitialize()
{	
	m_Configuration.deinitialize();

	m_Listener.deinitialize();
	m_PmQueue.deinitialize();
	m_OutgoingPmFilter.deinitialize();
	m_Marshalling.deinitialize();
	m_Switch.deinitialize();
	m_Limiter.deinitialize();
	m_Packer.deinitialize();
    m_NiiSender.deinitialize();
	
	m_NiiReceiver.deinitialize();
	m_UnPacker.deinitialize();
	m_UnMarshalling.deinitialize();
	m_IncomingPmFilter.deinitialize();
	m_GlobalVcSend.deinitialize();
	m_Sender.deinitialize();
}

void PCS::loopVcListener()
{
	ASAAC_TimeInterval t = TimeIntervalInfinity;
	
	for(;;)
	{
		try
		{
#ifdef _DEBUG_       
	        //cout << "PCS::vcListener()" << endl; fflush(stdout);
#endif
			if(m_Listener.listen(t) == ASAAC_TM_ERROR)
			{
				cerr << "PCS::vcListener() cannot listen to local VCs. Break loop. Good Bye!" << endl;
				return;
			}
		}
		catch ( PcsException &e )
		{
			cerr << e.getFullMessage() << endl;
		}
	}
};

void PCS::loopTcListener()
{
	ASAAC_NetworkDescriptor* networks = 0;
	unsigned long number = 0;
	
	
	ASAAC_PublicId localNetwork = cMosNii::getInstance()->getLocalNetwork();
	
	ASAAC_TimeInterval network_gap = {0,10000}; //CAUTION.SBS> causes a DELAY between network receives!!!!!!!
	ASAAC_TimeInterval unavailable = {1,0}; //COMMENT.SBS> if their are no networks configured, wait til next try
	
	for(;;)
	{
		try
		{
			m_Configuration.getNetworks(networks, number);
		
			if(networks == 0)
			{
				#ifdef _DEBUG_       
				//cerr << "PCS::tcListener() Invalid pointer to network array" << endl;
				#endif
				ASAAC_APOS_sleep(&unavailable);
				continue;	
			}
			
			for(unsigned long n = 0; n < number; ++n)
			{
				if(networks[n].network == localNetwork)
				{
					#ifdef _DEBUG_       
		        	cout << "PCS::tcListener() on Network " << n << endl;
					#endif
					
					
					if(m_NiiReceiver.listen(networks[n],number == 1 ? TimeIntervalInfinity : network_gap) == ASAAC_TM_SUCCESS)
					{
						#ifdef _DEBUG_       
		        		cout << "PCS::tcListener() successfully processed incoming messages on netork " << n << endl;  fflush(stdout);
						#endif			
					}
				
				}
			}
		}
		catch ( PcsException &e )
		{
			cerr << e.getFullMessage() << endl;
		}
	}
};

void PCS::loopRateLimiter()
{
	ASAAC_ReturnStatus ret;
	for(;;)
	{
		try
		{
			ret = m_Limiter.processNextMessage();
#ifdef _DEBUG_
			if(ret == ASAAC_SUCCESS)
			{       
	    		cout << "PCS::rateLimiter() processed enqueued message " << endl; fflush(stdout);
			}
#endif
		}
		catch ( PcsException &e )
		{
			cerr << e.getFullMessage() << endl;
		}
	}
};
