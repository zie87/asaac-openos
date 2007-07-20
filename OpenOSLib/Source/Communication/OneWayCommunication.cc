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



void OneWayCommunication::assureCommunication() const
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
}



void OneWayCommunication::assureGlobalVc() const
{
    try
    {
    	
        CommunicationManager *CM = CommunicationManager::getInstance();     
        
        GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );
        ASAAC_VcDescription GVc_desc = getReferenceGlobalVcDescription();

        if (GVc == NULL) 
        {
            GVc = CM->createVirtualChannel( GVc_desc );
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
                        
                    invokeOSScope( param_LVc.mapping.global_pid, detachLocalVc, param_LVc );
                }
                CM->destroyVirtualChannel( GVc_desc_old.global_vc_id );
                
                GVc = CM->createVirtualChannel( GVc_desc );
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
}



void OneWayCommunication::assureSenderConnection() const
{
    try
    {
        
        assureGlobalVc();
        
        CommunicationManager *CM = CommunicationManager::getInstance();     

        GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );
        
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
            
            long LVc_sender_idx = GVc->getSendingLocalVcIndex();
            if ( LVc_sender_idx != -1 )
            {
                LVc_sender = GVc->getLocalVcByIndex( LVc_sender_idx );
                if (LVc_sender != NULL)
                {
                    ASAAC_VcMappingDescription *desc = LVc_sender->getDescription();
                    
                    OSScopeData param_false_sender;
                    param_false_sender.mapping = *desc;
                    
                    invokeOSScope( param_false_sender.mapping.global_pid, detachLocalVc, param_false_sender );
                }
            }
            
            invokeOSScope( param_sender.mapping.global_pid, attachLocalVc, param_sender );
        }
        else
        {
            ASAAC_VcMappingDescription LVc_desc_sender;
            LVc_desc_sender = *( LVc_sender->getDescription() );
            
            if ( !isLocalVcAdequate( LVc_desc_sender, param_sender.mapping ) )
            {
                invokeOSScope( param_sender.mapping.global_pid, detachLocalVc, param_sender );
                invokeOSScope( param_sender.mapping.global_pid, attachLocalVc, param_sender );
            }       
        }
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while assuring sender connection", LOCATION);

        throw;
    }
}



void OneWayCommunication::assureReceiverConnection() const
{
    try
    {
   	
        assureGlobalVc();

        CommunicationManager *CM = CommunicationManager::getInstance();     

        GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );
        
        if (GVc == NULL)
            throw OSException("GlobalVC object not found", LOCATION);
        
        LocalVc *LVc_receiver = GVc->getLocalVc( m_Configuration.receiver_process_id, m_Configuration.receiver_local_vc_id );   

        OSScopeData param_receiver;
        param_receiver.mapping = getReferenceReceiverVcDescription();
        
        if (LVc_receiver == NULL)
        {
          invokeOSScope( param_receiver.mapping.global_pid, attachLocalVc, param_receiver );
        }
        else
        {
            ASAAC_VcMappingDescription LVc_desc_receiver;
            LVc_desc_receiver = *( LVc_receiver->getDescription() );
            
            if ( isLocalVcAdequate( LVc_desc_receiver, param_receiver.mapping ) == false )
            {
                invokeOSScope( param_receiver.mapping.global_pid, detachLocalVc, param_receiver );
                invokeOSScope( param_receiver.mapping.global_pid, attachLocalVc, param_receiver );
            }       
        }
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while assuring receiver connection", LOCATION);

        throw;
    }
}



void OneWayCommunication::detach() const
{
    try
    {
	    CommunicationManager *CM = CommunicationManager::getInstance();
	    GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );
	
	    if (GVc != NULL)
        {	
	        detachSender();
	        detachReceiver();
	
	        GVc->removeAllLocalVcs();
                
            CM->destroyVirtualChannel(m_Configuration.global_vc_id);
        }
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while detaching connection path", LOCATION);
        
        throw;
    }
}



void OneWayCommunication::detachSender() const
{
    try
    {
	    CommunicationManager *CM = CommunicationManager::getInstance();
	    GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );
	    
	    if (GVc != NULL)
        {	    
		    OSScopeData param_sender;
		    
		    param_sender.mapping = getReferenceSenderVcDescription();
		    
		    invokeOSScope( param_sender.mapping.global_pid, detachLocalVc, param_sender );
        }	
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while detaching sender process", LOCATION);

        throw;
    }
}



void OneWayCommunication::detachReceiver() const
{
    try
    {
	    CommunicationManager *CM = CommunicationManager::getInstance();
	    GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );
	    
	    if (GVc != NULL)
        {	    
		    OSScopeData param_receiver;
		    
		    param_receiver.mapping = getReferenceReceiverVcDescription();
		    
		    invokeOSScope( param_receiver.mapping.global_pid, detachLocalVc, param_receiver );
        }	        
    }   
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error occured while detaching receiver process", LOCATION);

        throw;
    }
}



bool OneWayCommunication::isAttached() const
{
	//Check Global VC Configuration
	CommunicationManager *CM = CommunicationManager::getInstance();
	
	GlobalVc* GVc = CM->getVirtualChannel( m_Configuration.global_vc_id );

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



void OneWayCommunication::setGlobalVcConfiguration(const ASAAC_PublicId global_vc_id, const unsigned long msg_length, const unsigned long number_of_buffers, ASAAC_VirtualChannelType vc_type)
{
	m_Configuration.global_vc_id = global_vc_id;
	m_Configuration.msg_length = msg_length;
	m_Configuration.number_of_buffers = number_of_buffers;
	m_Configuration.vc_type = vc_type;
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



void OneWayCommunication::getGlobalVcConfiguration(ASAAC_PublicId &global_vc_id, unsigned long &msg_length, unsigned long &number_of_buffers, ASAAC_VirtualChannelType &vc_type) const
{
	global_vc_id = m_Configuration.global_vc_id;
	msg_length = m_Configuration.msg_length;
	number_of_buffers = m_Configuration.number_of_buffers;
	vc_type = m_Configuration.vc_type;
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

	try
	{
		CharacterSequence ErrorString;
		
		if (currentDescription.global_vc_id != referenceDescription.global_vc_id)
			throw OSException( (ErrorString << "current global_vc_id:" << currentDescription.global_vc_id << " reference global_vc_id:" << referenceDescription.global_vc_id).c_str(), LOCATION);
    
		if (currentDescription.max_msg_length < referenceDescription.max_msg_length)
			throw OSException( (ErrorString << "current max_msg_length:" << currentDescription.max_msg_length << " reference max_msg_length:" << referenceDescription.max_msg_length).c_str(), LOCATION);
    
		if (currentDescription.max_number_of_buffers < referenceDescription.max_number_of_buffers)
			throw OSException( (ErrorString << "current max_number_of_buffers:" << currentDescription.max_number_of_buffers << " reference max_number_of_buffers:" << referenceDescription.max_number_of_buffers).c_str(), LOCATION);
    
		if (currentDescription.max_number_of_threads_attached < referenceDescription.max_number_of_threads_attached)
			throw OSException( (ErrorString << "current max_number_of_threads_attached:" << currentDescription.max_number_of_threads_attached << " reference max_number_of_threads_attached:" << referenceDescription.max_number_of_threads_attached).c_str(), LOCATION);
    
		if (currentDescription.max_number_of_TCs_attached != referenceDescription.max_number_of_TCs_attached)
			throw OSException( (ErrorString << "current max_number_of_TCs_attached:" << currentDescription.max_number_of_TCs_attached << " reference max_number_of_TCs_attached:" << referenceDescription.max_number_of_TCs_attached).c_str(), LOCATION);
    
		if (currentDescription.vc_type != referenceDescription.vc_type)
			throw OSException( (ErrorString << "current vc_type:" << (long)currentDescription.vc_type << " reference vc_type:" << (long)referenceDescription.vc_type).c_str(), LOCATION);
    
 	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("GlobalVc not adequate", LOCATION);
		e.raiseError();
		
		return false;
	}
	
	return true;
}



bool OneWayCommunication::isLocalVcAdequate(ASAAC_VcMappingDescription currentDescription, ASAAC_VcMappingDescription referenceDescription) const
{
	try
	{
		CharacterSequence ErrorString;
		
		if (currentDescription.global_pid != referenceDescription.global_pid)
			throw OSException( (ErrorString << "current global_pid:" << currentDescription.global_pid << " reference global_pid:" << referenceDescription.global_pid).c_str(), LOCATION);
			
		if (currentDescription.local_vc_id != referenceDescription.local_vc_id)
			throw OSException( (ErrorString << "current local_vc_id:" << currentDescription.local_vc_id << " reference local_vc_id:" << referenceDescription.local_vc_id).c_str(), LOCATION);
			
		if (currentDescription.global_vc_id != referenceDescription.global_vc_id)
			throw OSException( (ErrorString << "current global_vc_id:" << currentDescription.global_vc_id << " reference global_vc_id:" << referenceDescription.global_vc_id).c_str(), LOCATION);
			
		if (currentDescription.local_thread_id != referenceDescription.local_thread_id)
			throw OSException( (ErrorString << "current local_thread_id:" << currentDescription.local_thread_id << " reference local_thread_id:" << referenceDescription.local_thread_id).c_str(), LOCATION);
			
		if (currentDescription.buffer_size < referenceDescription.buffer_size)
			throw OSException( (ErrorString << "current buffer_size:" << currentDescription.buffer_size << " reference buffer_size:" << referenceDescription.buffer_size).c_str(), LOCATION);
			
		if (currentDescription.number_of_message_buffers < referenceDescription.number_of_message_buffers)
			throw OSException( (ErrorString << "current number_of_message_buffers:" << currentDescription.number_of_message_buffers << " reference number_of_message_buffers:" << referenceDescription.number_of_message_buffers).c_str(), LOCATION);
			
		if (currentDescription.is_reading != referenceDescription.is_reading)
			throw OSException( (ErrorString << "current is_reading:" << (long)currentDescription.is_reading << " reference is_reading:" << (long)referenceDescription.is_reading).c_str(), LOCATION);
			
		if (currentDescription.is_lifo_queue != referenceDescription.is_lifo_queue)
			throw OSException( (ErrorString << "current is_lifo_queue:" << (long)currentDescription.is_lifo_queue << " reference is_lifo_queue:" << (long)referenceDescription.is_lifo_queue).c_str(), LOCATION);
			
		if (currentDescription.is_refusing_queue != referenceDescription.is_refusing_queue)
			throw OSException( (ErrorString << "current is_refusing_queue:" << (long)currentDescription.is_refusing_queue << " reference is_refusing_queue:" << (long)referenceDescription.is_refusing_queue).c_str(), LOCATION);
			
		if (currentDescription.Priority != referenceDescription.Priority)
			throw OSException( (ErrorString << "current Priority:" << currentDescription.Priority << " reference Priority:" << referenceDescription.Priority).c_str(), LOCATION);
			
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("LocalVC not adequate", LOCATION);
		e.raiseError();
		
		return false;
	}
	
	return true;
}



void OneWayCommunication::invokeOSScope( const ASAAC_PublicId process_id, const OSScopeFunction foo, OSScopeData param) const
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
		P->invokeOSScope(foo, param.buffer);
	}
}



//OSScope Functions:
void OneWayCommunication::attachLocalVc(OSScopeCommandBuffer param)
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
			e.raiseError();
		}
		
		if (p_state == PROCESS_RUNNING)
		{ 	
			if (ASAAC_SMOS_runProcess(data->mapping.global_pid) == ASAAC_ERROR)
				throw OSException("target process could not be restarted", LOCATION);
		}
	}
	catch (ASAAC_Exception &e)
	{
		throw;
	}
}



void OneWayCommunication::detachLocalVc(OSScopeCommandBuffer param)
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
			e.raiseError();
		}
		
		if (p_state == PROCESS_RUNNING)
		{ 	
			if (ASAAC_SMOS_runProcess(data->mapping.global_pid) == ASAAC_ERROR)
				throw OSException("target process could not be restarted", LOCATION);
		}
	}
	catch (ASAAC_Exception &e)
	{
		throw;
	}
}
