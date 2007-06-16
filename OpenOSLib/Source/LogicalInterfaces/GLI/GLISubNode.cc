#include "GLISubNode.hh"

#include "OpenOS.hh"

#include "Common/Algorithms/Primes.hh"
#include "Common/Algorithms/DiffieHellman.hh"


namespace ASAAC
{
    namespace GLI
    {

        SubNode::SubNode()
        {
            registerLoadConfiguration( NULL );
            registerStopConfiguration( NULL );
            registerRunConfiguration( NULL );
            registerChangeConfiguration( NULL );
            registerGetBITResult( NULL );
            registerGetAliveStatus ( NULL );
            
            m_DecryptionKey = 0;
        }
        
        
        SubNode::~SubNode()
        {
        }
        
        
        ASAAC_TimedReturnStatus SubNode::allocateCFM( ASAAC_PublicId &cfm_id )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.new_configuration = cfm_id;        
                sendAndReceive( ASAAC_GLI_Request_New_Cfm, ASAAC_GLI_Cfm_Allocated, Param );
                
                if (Param._u.Allocated_Cfm_Id != cfm_id)
                {
                    cfm_id = Param._u.Allocated_Cfm_Id;
                    throw OSException("Allocated cfm_id differs from cfm_id to allocate", LOCATION);
                }    
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::allocateCFM", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SubNode::deallocateCFM( ASAAC_PublicId &cfm_id )
        {
            ASAAC_GLI_GliMessageParameter Param;    
                   
            try
            {
                Param._u.Deallocate_Cfm_Id = cfm_id;    
                sendAndReceive( ASAAC_GLI_Deallocate_Cfm, ASAAC_GLI_Cfm_Deallocated, Param);
                
                if (Param._u.Deallocated_Cfm_Id != cfm_id)
                {
                    cfm_id = Param._u.Deallocated_Cfm_Id;
                    throw OSException("Deallocated cfm_id differs from cfm_id to deallocate", LOCATION);
                }    
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::deallocateCFM", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            return ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_ReturnStatus SubNode::reportFault( ASAAC_FaultReport fault )
        {
            ASAAC_GLI_GliMessageParameter Param;           
        
            try
            {
                Param._u.the_fault = fault;
                sendRequest( ASAAC_GLI_Fault_Report, Param);
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::reportFault", LOCATION );
                e.raiseError();
                
                return ASAAC_ERROR;
            }
        
            return ASAAC_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SubNode::requestKey( ASAAC_PublicId tls_id, unsigned long &decryption_key, unsigned long *key_array[10] )
        {
            ASAAC_GLI_GliMessageParameter Param;
                       
            try
            {
                implementSecureCommunication(tls_id, decryption_key);
        
                Param._u.request_key_tls_id = tls_id;        
                sendAndReceive( ASAAC_GLI_Request_Key, ASAAC_GLI_Send_Key, Param);        
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::deallocateCFM", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            *key_array = Param._u.key_array; 
            return ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_ReturnStatus SubNode::registerLoadConfiguration( OnLoadConfiguration LoadConfigurationHandler )
        {
            return registerHandler( m_OnLoadConfigurationHandler, LoadConfigurationHandler, handleLoadConfiguration); 
        }
        
        
        ASAAC_ReturnStatus SubNode::registerStopConfiguration( OnStopConfiguration StopConfigurationHandler )
        {
            return registerHandler( m_OnStopConfigurationHandler, StopConfigurationHandler, handleStopConfiguration); 
        }
        
        
        ASAAC_ReturnStatus SubNode::registerRunConfiguration( OnRunConfiguration RunConfigurationHandler )
        {
            return registerHandler( m_OnRunConfigurationHandler, RunConfigurationHandler, handleRunConfiguration); 
        }
        
        
        ASAAC_ReturnStatus SubNode::registerChangeConfiguration( OnChangeConfiguration ChangeConfigurationHandler )
        {
            return registerHandler( m_OnChangeConfigurationHandler, ChangeConfigurationHandler, handleChangeConfiguration); 
        }
        
        
        ASAAC_ReturnStatus SubNode::registerGetBITResult( OnGetBITResult GetBITResultHandler )
        {
            return registerHandler( m_OnGetBITResultHandler, GetBITResultHandler, handleGetBITResult); 
        }
        
        
        ASAAC_ReturnStatus SubNode::registerGetAliveStatus ( OnGetAliveStatus GetAliveStatusHandler )
        {
            return registerHandler( m_OnGetAliveStatusHandler, GetAliveStatusHandler, handleGetAliveStatus); 
        }
        
        
        void SubNode::implementSecureCommunication( ASAAC_PublicId tls_id, unsigned long &decryption_key )
        {
            ASAAC_GLI_GliMessageParameter Param;
                       
            try
            {
                Param._u.request_sc_tls_id = tls_id;        
                sendAndReceive( ASAAC_GLI_Request_SC, ASAAC_GLI_SC_Response, Param);
                
                if (Param._u.response == ASAAC_BOOL_FALSE)
                    throw OSException("Request has been rejected", LOCATION);            
                
                unsigned long i = rand();
                unsigned long M = prime();
                
                Param._u.key = M;        
                sendAndReceive( ASAAC_GLI_DH_Send_M, ASAAC_GLI_DH_Send_X, Param);
                
                Param._u.key = acmodm(Param._u.key, i, M);        
                sendAndReceive( ASAAC_GLI_DH_Send_XimodM, ASAAC_GLI_DH_Send_XjmodM, Param);
                
                m_DecryptionKey = acmodm(Param._u.key, i, M);
                decryption_key = m_DecryptionKey;
            }
            catch ( ASAAC_Exception &e )
            {
                m_DecryptionKey = 0;
                
                e.addPath( "Implementing a secure communication link failed", LOCATION );
                throw;
            }
        }
        
        
        ASAAC_ReturnStatus SubNode::handleLoadConfiguration( ASAAC_PublicId &configuration_id )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SubNode::handleStopConfiguration( ASAAC_PublicId &configuration_id )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SubNode::handleRunConfiguration( ASAAC_PublicId &configuration_id )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SubNode::handleChangeConfiguration( ASAAC_PublicId configuration_event, ASAAC_PublicId &new_configuration )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SubNode::handleGetBITResult( ASAAC_BitType type, ASAAC_BitResult &result )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SubNode::handleGetAliveStatus( ASAAC_GLI_GliAliveParameter &paramter )
        {
            return ASAAC_ERROR;
        }
        
        
        void SubNode::handleSuperNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout )
        {
            try
            {
                bool Reply = false;
                ASAAC_TimedReturnStatus Status;
                size_t Size = sizeof(ASAAC_GLI_GliMessage);
                
                switch (Message.unique_message_id)
                {
                    case ASAAC_GLI_Load_Configuration:
                    { 
                        ASAAC_PublicId configuration_id = Message.message_parameter._u.config_to_be_loaded;
                        
                        m_OnLoadConfigurationHandler( configuration_id );
                        
                        Message.message_parameter._u.config_loaded = configuration_id;
                        Message.unique_message_id = ASAAC_GLI_Configuration_Loaded;
                        Reply = true;
                    }
                    break;

                    case ASAAC_GLI_Stop_Configuration:
                    { 
                        ASAAC_PublicId configuration_id = Message.message_parameter._u.config_to_be_acquired;
                        
                        m_OnStopConfigurationHandler( configuration_id );
                        
                        Message.message_parameter._u.config_acquired = configuration_id;
                        Message.unique_message_id = ASAAC_GLI_Configuration_Stopped;
                        Reply = true;
                    }
                    break;

                    case ASAAC_GLI_Run_Configuration:
                    { 
                        ASAAC_PublicId configuration_id = Message.message_parameter._u.config_to_be_run;
                        
                        m_OnRunConfigurationHandler( configuration_id );
                        
                        Message.message_parameter._u.config_run = configuration_id;
                        Message.unique_message_id = ASAAC_GLI_Configuration_Running;
                        Reply = true;
                    }
                    break;

                    case ASAAC_GLI_Change_Configuration:
                    { 
                        ASAAC_PublicId configuration_event = Message.message_parameter._u.configuration_event;
                        ASAAC_PublicId new_configuration = OS_UNUSED_ID;
                        
                        m_OnChangeConfigurationHandler( configuration_event, new_configuration );
                        
                        Message.message_parameter._u.new_configuration = new_configuration;
                        Message.unique_message_id = ASAAC_GLI_Configuration_Changed;
                        Reply = true;
                    }
                    break;

                    case ASAAC_GLI_Request_BIT_Result:
                    { 
                        ASAAC_BitType type = Message.message_parameter._u.type;
                        ASAAC_BitResult result;
                                                
                        m_OnGetBITResultHandler( type, result );
                        
                        Message.message_parameter._u.result = result;
                        Message.unique_message_id = ASAAC_GLI_Report_BIT_Result;
                        Reply = true;
                    }
                    break;

                    case ASAAC_GLI_Are_You_Alive:
                    { 
                        ASAAC_GLI_GliAliveParameter parameter = Message.message_parameter._u.alive_param; 
                        
                        m_OnGetAliveStatusHandler( parameter );
                        
                        Message.message_parameter._u.alive_param = parameter;
                        Message.unique_message_id = ASAAC_GLI_I_Am_Alive;
                        Reply = true;
                    }
                    break;

                    default: break;
                } 
                
                if (Reply == true)
                { 
                    ASAAC_TimeInterval Timeout = TimeStamp(p_Timeout).asaac_Interval();
                    
                    Status = ASAAC_APOS_sendMessage( getSenderVc(), &Timeout, &Message, Size );
    
                    if (Status == ASAAC_TM_ERROR)
                        throw OSException( "Error sending a reply", LOCATION );

                    if (Status == ASAAC_TM_TIMEOUT)
                        throw TimeoutException( "Timeout sending a reply", LOCATION );
                }                
            }
            catch ( ASAAC_Exception &e)
            {
                e.addPath("Error handling a SubNodeMessage", LOCATION);
                throw;
            }
            catch (...)
            {
                throw OSException("Unknown error handling a SubNodeMessage", LOCATION);
            }                
        }       
        
    }
}        
