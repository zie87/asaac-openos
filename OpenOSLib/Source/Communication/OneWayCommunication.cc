#include "OneWayCommunication.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Communication/CommunicationManager.hh"
#include "Communication/LocalVc.hh"



OneWayCommunication::OneWayCommunication()
{
	m_Configuration.global_vc_id 			= OS_UNUSED_ID;
	
	m_Configuration.sender_process_id 		= OS_UNUSED_ID;
	m_Configuration.receiver_process_id 	= OS_UNUSED_ID;

	m_Configuration.sender_local_vc_id 		= OS_UNUSED_ID;
	m_Configuration.receiver_local_vc_id 	= OS_UNUSED_ID;
	
	m_Configuration.msg_length 				= 1024;
	m_Configuration.number_of_buffers 		= 16;
	
											// for every thread two buffers shall be enough
	m_Configuration.number_of_threads 		= div(m_Configuration.number_of_buffers, 2).quot;

	m_Configuration.vc_type 				= ASAAC_Application_Raw_VC;
	
	m_Configuration.priority				= 0;
}



OneWayCommunication::~OneWayCommunication()
{

}



ASAAC_ReturnStatus OneWayCommunication::assureCommunication() const
{
	try
	{            
		assureGlobalVc();
        assureSenderConnection();
        assureReceiverConnection();		
	}	
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error occured while establishing a communication path", LOCATION);
		throw;
	}
		
	return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus OneWayCommunication::assureGlobalVc() const
{
    try
    {
        CommunicationManager *CM = CommunicationManager::getInstance();     
        
        GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );
        ASAAC_VcDescription GVc_desc = getReferenceGlobalVcDescription();

        if (GVc == NULL) 
        {
            GVc = CM->createGlobalVirtualChannel( GVc_desc );
        }       
        else
        {
            //Before we destroy the global Vc all attached local Vcs shall be removed
            ASAAC_VcDescription GVc_desc_old = *(GVc->getDescription());
            if ( isGlobalVcAdequate( GVc_desc_old, GVc_desc ) == false )
            {
                for (unsigned long GVc_idx = 0; GVc_idx < GVc_desc_old.max_number_of_threads_attached; GVc_idx++ )
                {
                    LocalVc *LVc = GVc->getLocalVcByIndex(GVc_idx);
                    
                    if (LVc == NULL)
                        continue;

                    if (LVc->isInitialized() == false)
                        continue;

                    OSScopeData param_LVc;
                    param_LVc.mapping = *LVc->getDescription();
                        
                    if (invokeOSScope( param_LVc.mapping.global_pid, detachLocalVc, param_LVc ) == ASAAC_ERROR)
                        throw OSException("An error occured while detaching local vcs from inadequate global vc", LOCATION);
                }
                CM->destroyGlobalVirtualChannel( GVc_desc_old.global_vc_id );
                
                GVc = CM->createGlobalVirtualChannel( GVc_desc );
            }
        }
        
        if (GVc == NULL)
            throw OSException("GlobalVC could not be created", LOCATION);
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while assuring global vc", LOCATION);
        throw;
    }
        
    return ASAAC_SUCCESS;    
}



ASAAC_ReturnStatus OneWayCommunication::assureSenderConnection() const
{
    try
    {
        assureGlobalVc();
        
        CommunicationManager *CM = CommunicationManager::getInstance();     

        GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );
        
        if (GVc == NULL)
            throw OSException("GlobalVC object not found", LOCATION);
        
        LocalVc *LVc_sender = GVc->getLocalVc( m_Configuration.sender_process_id, m_Configuration.sender_local_vc_id );
             
        OSScopeData param_sender;
        param_sender.mapping = getReferenceSenderVcDescription();
        
        if (LVc_sender == NULL)
        {
            //before we create a new sending local vc, we have to check, whether another receiving vc
            //in another process is blocking the global vc with a sending local vc, because only one 
            //writing local vc per global vc is allowed (Multicast requirement)
            
            int LVc_sender_idx = GVc->getSendingLocalVcIndex();
            if ( LVc_sender_idx != -1 )
            {
                LVc_sender = GVc->getLocalVcByIndex( LVc_sender_idx );
                if (LVc_sender != NULL)
                {
                    ASAAC_VcMappingDescription *desc = LVc_sender->getDescription();
                    
                    OSScopeData param_false_sender;
                    param_false_sender.mapping = *desc;
                    
                    if (invokeOSScope( param_false_sender.mapping.global_pid, detachLocalVc, param_false_sender ) == ASAAC_ERROR)
                        throw OSException("An error occured while detaching inadequat sending local vc from sending process", LOCATION);
                }
            }
            
            if (invokeOSScope( param_sender.mapping.global_pid, attachLocalVc, param_sender ) == ASAAC_ERROR)
                throw OSException("An error occured while attaching local vc to sender process", LOCATION);
        }
        else
        {
            ASAAC_VcMappingDescription LVc_desc_sender;
            LVc_desc_sender = *( LVc_sender->getDescription() );
            
            if ( !isLocalVcAdequate( LVc_desc_sender, param_sender.mapping ) )
            {
                if (invokeOSScope( param_sender.mapping.global_pid, detachLocalVc, param_sender ) == ASAAC_ERROR)
                    throw OSException("An error occured while detaching inadequat local vc from sending process", LOCATION);
                
                if (invokeOSScope( param_sender.mapping.global_pid, attachLocalVc, param_sender ) == ASAAC_ERROR)
                    throw OSException("An error occured while attaching adequat local vc to sending process", LOCATION);
            }       
        }
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while assuring sender connection", LOCATION);
        throw;
    }
        
    return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus OneWayCommunication::assureReceiverConnection() const
{
    try
    {
        assureGlobalVc();

        CommunicationManager *CM = CommunicationManager::getInstance();     

        GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );
        
        if (GVc == NULL)
            throw OSException("GlobalVC object not found", LOCATION);
        
        LocalVc *LVc_receiver = GVc->getLocalVc( m_Configuration.receiver_process_id, m_Configuration.receiver_local_vc_id );   

        OSScopeData param_receiver;
        param_receiver.mapping = getReferenceReceiverVcDescription();
        
        if (LVc_receiver == NULL)
        {
            if (invokeOSScope( param_receiver.mapping.global_pid, attachLocalVc, param_receiver ) == ASAAC_ERROR)
                throw OSException("An error occured while attaching local vc to receiving process", LOCATION);
        }
        else
        {
            ASAAC_VcMappingDescription LVc_desc_receiver;
            LVc_desc_receiver = *( LVc_receiver->getDescription() );
            
            if ( isLocalVcAdequate( LVc_desc_receiver, param_receiver.mapping ) == false )
            {
                if (invokeOSScope( param_receiver.mapping.global_pid, detachLocalVc, param_receiver ) == ASAAC_ERROR)
                    throw OSException("An error occured while detaching inadequat local vc from receiving process", LOCATION);
                
                if (invokeOSScope( param_receiver.mapping.global_pid, attachLocalVc, param_receiver ) == ASAAC_ERROR)
                    throw OSException("An error occured while attaching adequat local vc to receiving process", LOCATION);
            }       
        }
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while assuring receiver connection", LOCATION);
        throw;
    }
        
    return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus OneWayCommunication::detach() const
{
    try
    {
	    CommunicationManager *CM = CommunicationManager::getInstance();
	    GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );
	
	    if (GVc != NULL)
        {	
	        detachSender();
	        detachReceiver();
	
	        if (GVc->removeAllLocalVcs() == ASAAC_ERROR)
                throw OSException("Error removing other local vcs", LOCATION);
                
            if (CM->destroyGlobalVirtualChannel(m_Configuration.global_vc_id) == ASAAC_ERROR)
                throw OSException("Error destroying global vc", LOCATION);;
        }
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while detaching connection path", LOCATION);
        throw;
    }
        
    return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus OneWayCommunication::detachSender() const
{
    try
    {
	    CommunicationManager *CM = CommunicationManager::getInstance();
	    GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );
	    
	    if (GVc != NULL)
        {	    
		    OSScopeData param_sender;
		    
		    param_sender.mapping = getReferenceSenderVcDescription();
		    
		    if (invokeOSScope( param_sender.mapping.global_pid, detachLocalVc, param_sender ) == ASAAC_ERROR)
		        throw OSException("An error occured while detaching local vc from sender process", LOCATION);
        }	
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while detaching sender process", LOCATION);
        throw;
    }
        
    return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus OneWayCommunication::detachReceiver() const
{
    try
    {
	    CommunicationManager *CM = CommunicationManager::getInstance();
	    GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );
	    
	    if (GVc != NULL)
        {	    
		    OSScopeData param_receiver;
		    
		    param_receiver.mapping = getReferenceReceiverVcDescription();
		    
		    if (invokeOSScope( param_receiver.mapping.global_pid, detachLocalVc, param_receiver ) == ASAAC_ERROR)
		        throw OSException("An error occured while detaching local vc from receiver process", LOCATION);
        }	        
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while detaching receiver process", LOCATION);
        throw;
    }
        
    return ASAAC_SUCCESS;
}



bool OneWayCommunication::isAttached() const
{
	//Check Global VC Configuration
	CommunicationManager *CM = CommunicationManager::getInstance();
	
	GlobalVc* GVc = CM->getGlobalVirtualChannel( m_Configuration.global_vc_id );

	if (GVc == NULL)
		return false;
	
	try
	{	
		ASAAC_VcDescription *GVc_desc;
		GVc_desc = GVc->getDescription();
		
		if ( isGlobalVcAdequate( *GVc_desc, getReferenceGlobalVcDescription() ) == false )
			throw false;
		
		//Check local VC Configuration (sender)
		LocalVc *SLVc = GVc->getLocalVc( m_Configuration.sender_process_id, m_Configuration.sender_local_vc_id );
		
		if (SLVc == NULL)
			throw false;
		
		ASAAC_VcMappingDescription *LVc_desc_sender;
		LVc_desc_sender = SLVc->getDescription();
		
		if ( isLocalVcAdequate( *LVc_desc_sender, getReferenceSenderVcDescription() ) == false )
			throw false;
		
		
		//Check local VC Configuration (receiver)
		LocalVc *RLVc = GVc->getLocalVc( m_Configuration.sender_process_id, m_Configuration.sender_local_vc_id );
		
		if (RLVc == NULL)
			throw false;
		
		ASAAC_VcMappingDescription *LVc_desc_receiver;
		LVc_desc_receiver = RLVc->getDescription();
		
		if ( isLocalVcAdequate( *LVc_desc_receiver, getReferenceReceiverVcDescription() ) == false )
			throw false;
	}
	catch (bool e)
	{
		return e;
	}
		
	return true;
}



void OneWayCommunication::setConfiguration(const OneWayConfiguration config)
{
	m_Configuration = config;
}



void OneWayCommunication::setSenderConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId local_vc_id) 
{
	m_Configuration.sender_thread_id = thread_id;
	m_Configuration.sender_process_id = process_id;
	m_Configuration.sender_local_vc_id = local_vc_id;
	
}



void OneWayCommunication::setReceiverConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId local_vc_id) 
{
	m_Configuration.receiver_thread_id = thread_id;
	m_Configuration.receiver_process_id = process_id;
	m_Configuration.receiver_local_vc_id = local_vc_id;
}



void OneWayCommunication::setGlobalVcConfiguration(const ASAAC_PublicId global_vc_id, const unsigned long msg_length, const unsigned long number_of_buffers)
{
	m_Configuration.global_vc_id = global_vc_id;
	m_Configuration.msg_length = msg_length;
	m_Configuration.number_of_buffers = number_of_buffers;
}



OneWayConfiguration OneWayCommunication::getConfiguration() const
{
	return m_Configuration;
}



void OneWayCommunication::getSenderConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &local_vc_id) const 
{
	thread_id = m_Configuration.sender_thread_id;
	process_id = m_Configuration.sender_process_id;
	local_vc_id = m_Configuration.sender_local_vc_id;
}



void OneWayCommunication::getReceiverConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &local_vc_id) const 
{
	thread_id = m_Configuration.receiver_thread_id;
	process_id = m_Configuration.receiver_process_id;
	local_vc_id = m_Configuration.receiver_local_vc_id;
}



void OneWayCommunication::getGlobalVcConfiguration(ASAAC_PublicId &global_vc_id, unsigned long &msg_length, unsigned long &number_of_buffers) const
{
	global_vc_id = m_Configuration.global_vc_id;
	msg_length = m_Configuration.msg_length;
	number_of_buffers = m_Configuration.number_of_buffers;
}



ASAAC_VcDescription OneWayCommunication::getReferenceGlobalVcDescription() const
{
	ProcessManager *PM = ProcessManager::getInstance();
			
	Process *P = PM->getCurrentProcess();
	if (P == NULL)
		throw OSException("'Current Process' could not be found", LOCATION);
	
	ASAAC_VcDescription description;
				
	description.global_vc_id 							= m_Configuration.global_vc_id;
	description.max_msg_length 		  					= m_Configuration.msg_length;
	description.max_number_of_buffers 					= m_Configuration.number_of_buffers;  
	description.max_number_of_threads_attached 			= m_Configuration.number_of_threads;
	description.max_number_of_TCs_attached 				= 0; //tc's shall not be attached
	description.security_rating.classification_level 	= ASAAC_UNCLASSIFIED;
	description.security_rating.security_category 		= ASAAC_LEVEL_1;
	description.security_info							= ASAAC_Unmarked;
	description.vc_type									= m_Configuration.vc_type;
	description.cpu_id									= P->getId(); 
	description.is_typed_message						= ASAAC_BOOL_FALSE;
	description.data_representation_format				= 0;

	return description;
}



ASAAC_VcMappingDescription OneWayCommunication::getReferenceSenderVcDescription() const
{
	ASAAC_VcMappingDescription description;

	description.global_pid 				  	= m_Configuration.sender_process_id;
	description.local_vc_id 				= m_Configuration.sender_local_vc_id;
	description.global_vc_id 				= m_Configuration.global_vc_id;
	description.local_thread_id 			= m_Configuration.sender_thread_id;
	description.buffer_size 				= m_Configuration.msg_length;
	description.number_of_message_buffers 	= div(m_Configuration.number_of_buffers, (long)m_Configuration.number_of_threads).quot;
	description.is_reading 					= ASAAC_BOOL_FALSE;
	description.is_lifo_queue 				= ASAAC_BOOL_FALSE;
	description.is_refusing_queue 			= ASAAC_BOOL_TRUE;
	description.Priority 					= m_Configuration.priority;
	
	return description;
}



ASAAC_VcMappingDescription OneWayCommunication::getReferenceReceiverVcDescription() const
{
	ASAAC_VcMappingDescription description;

	description = getReferenceSenderVcDescription();

	description.global_pid 					= m_Configuration.receiver_process_id;
	description.local_vc_id 				= m_Configuration.receiver_local_vc_id;
	description.local_thread_id 			= m_Configuration.receiver_thread_id;
	description.is_reading 					= ASAAC_BOOL_TRUE;
	
	return description;
}



bool OneWayCommunication::isGlobalVcAdequate(ASAAC_VcDescription currentDescription, ASAAC_VcDescription referenceDescription) const
{
	return ( (currentDescription.global_vc_id 					== referenceDescription.global_vc_id)					&&
			 (currentDescription.max_msg_length 				>= referenceDescription.max_msg_length)	 				&&
		     (currentDescription.max_number_of_buffers 			>= referenceDescription.max_number_of_buffers) 			&&
		     (currentDescription.max_number_of_threads_attached >= referenceDescription.max_number_of_threads_attached) &&
	    	 (currentDescription.max_number_of_TCs_attached 	== referenceDescription.max_number_of_TCs_attached) 	&&
		     (currentDescription.vc_type						== m_Configuration.vc_type)	);
}



bool OneWayCommunication::isLocalVcAdequate(ASAAC_VcMappingDescription currentDescription, ASAAC_VcMappingDescription referenceDescription) const
{
	return ( (currentDescription.global_pid 				== referenceDescription.global_pid) 				&&
		 	 (currentDescription.local_vc_id 				== referenceDescription.local_vc_id) 				&&
		 	 (currentDescription.global_vc_id 				== referenceDescription.global_vc_id) 				&&
		  	 (currentDescription.local_thread_id 			== referenceDescription.local_thread_id) 			&&
		 	 (currentDescription.buffer_size 				>= referenceDescription.buffer_size)				&&
		 	 (currentDescription.number_of_message_buffers 	>= referenceDescription.number_of_message_buffers)	&&
		 	 (currentDescription.is_reading 				== referenceDescription.is_reading) 				&&
		 	 (currentDescription.is_lifo_queue 				== referenceDescription.is_lifo_queue) 				&&
		 	 (currentDescription.is_refusing_queue 			== referenceDescription.is_refusing_queue) 			&&
		 	 (currentDescription.Priority 					== referenceDescription.Priority) );
}



ASAAC_ReturnStatus OneWayCommunication::invokeOSScope( const ASAAC_PublicId process_id, const OSScopeFunction foo, OSScopeData param) const
{
	ProcessManager *PM = ProcessManager::getInstance();
	
	Process *P = PM->getCurrentProcess();
	if (P == NULL)
		throw OSException("'current process' is not available", LOCATION);
	
	if (P->getId() != process_id)
	{
		//In this case "invoking OSScope" is not needed, because target process can be stopped easily
		//This call is much faster
		return foo( param.buffer );
	}
	else
	{	
		//In this case the calling process must stop itself to attach/detach local Vcs
		//This call takes more time, but necessary
		return P->invokeOSScope(foo, param.buffer);
	}
}



//OSScope Functions:
ASAAC_ReturnStatus OneWayCommunication::attachLocalVc(OSScopeCommandBuffer param)
{
	try
	{
		OSScopeData *data = (OSScopeData *)param;
		
		ProcessManager *PM = ProcessManager::getInstance();	
		Process *P = PM->getProcess( data->mapping.global_pid );
			
		if (P == NULL)
			throw OSException("process is not available", LOCATION);
	
		ProcessStatus p_state = P->getState(); 
		
		if (p_state == PROCESS_RUNNING)
		{				
			if (ASAAC_SMOS_stopProcess(data->mapping.global_pid) == ASAAC_ERROR)
				throw OSException("target process could not be stopped", LOCATION);	
		}
		
		try
		{	
			if (ASAAC_SMOS_attachChannelToProcessOrThread(&(data->mapping)) == ASAAC_ERROR)
				throw OSException("channel could not be attached", LOCATION);
		}
		catch (ASAAC_Exception &e)
		{
			e.logMessage();
		}
		
		if (p_state == PROCESS_RUNNING)
		{ 	
			if (ASAAC_SMOS_runProcess(data->mapping.global_pid) == ASAAC_ERROR)
				throw OSException("target process could not be restarted", LOCATION);
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.logMessage();
		return ASAAC_ERROR;	
	}
		
	return ASAAC_SUCCESS;
}



ASAAC_ReturnStatus OneWayCommunication::detachLocalVc(OSScopeCommandBuffer param)
{
	try
	{
		OSScopeData *data = (OSScopeData *)param;
		
		ProcessManager *PM = ProcessManager::getInstance();	
		Process *P = PM->getProcess( data->mapping.global_pid );
			
		if (P == NULL)
			throw OSException("process is not available", LOCATION);
	
		ProcessStatus p_state = P->getState(); 
						
		if (ASAAC_SMOS_stopProcess(data->mapping.global_pid) == ASAAC_ERROR)
			throw OSException("target process could not be stopped", LOCATION);	
		
		try
		{	
			if (ASAAC_SMOS_detachAllThreadsOfProcessFromVc(data->mapping.global_vc_id, data->mapping.global_pid) == ASAAC_ERROR)
				throw OSException("channel could not be detached", LOCATION);
		}
		catch (ASAAC_Exception &e)
		{
			e.logMessage();
		}
		
		if (p_state == PROCESS_RUNNING)
		{ 	
			if (ASAAC_SMOS_runProcess(data->mapping.global_pid) == ASAAC_ERROR)
				throw OSException("target process could not be restarted", LOCATION);
		}
	}
	catch (ASAAC_Exception &e)
	{
		e.logMessage();
		return ASAAC_ERROR;	
	}
		
	return ASAAC_SUCCESS;
}
