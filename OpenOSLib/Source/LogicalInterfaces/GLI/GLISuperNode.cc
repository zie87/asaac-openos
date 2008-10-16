#include "GLISuperNode.hh"

#include "OpenOSObject.hh"

#include "Common/Algorithms/Primes.hh"
#include "Common/Algorithms/DiffieHellman.hh"

namespace ASAAC
{
    namespace GLI
    {
        
        SuperNode::SuperNode()
        {
            registerAllocateCFMHandler( NULL );
            registerDeallocateCFMHandler( NULL );
            registerReportFaultHandler( NULL );
            registerRequestSecureCommunicationHandler( NULL );
            registerRequestKeyHandler( NULL );
            
            m_EncryptionKey = 0;
        }
        
        
        SuperNode::~SuperNode()
        {
        }
        
        
        ASAAC_TimedReturnStatus SuperNode::loadConfiguration( ASAAC_PublicId &configuration_id  )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.config_to_be_loaded = configuration_id;        
                sendAndReceive( ASAAC_GLI_Load_Configuration, ASAAC_GLI_Configuration_Loaded, Param);
                
                if (Param._u.config_loaded != configuration_id)
                {
                    configuration_id = Param._u.config_loaded; 
                    throw OSException("Loaded configuration differs from configuration to load", LOCATION);
                }    
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::loadConfiguration", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SuperNode::stopConfiguration( ASAAC_PublicId &configuration_id )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.config_to_be_acquired = configuration_id;        
                sendAndReceive( ASAAC_GLI_Stop_Configuration, ASAAC_GLI_Configuration_Stopped, Param);
                
                if (Param._u.config_acquired != configuration_id)
                {
                    configuration_id = Param._u.config_acquired; 
                    throw OSException("Stopped configuration differs from configuration to load", LOCATION);
                }    
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::stopConfiguration", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SuperNode::runConfiguration( ASAAC_PublicId &configuration_id )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.config_to_be_run = configuration_id;        
                sendAndReceive( ASAAC_GLI_Run_Configuration, ASAAC_GLI_Configuration_Running, Param);
                
                if (Param._u.config_run != configuration_id)
                {
                    configuration_id = Param._u.config_run; 
                    throw OSException("Running configuration differs from configuration to run", LOCATION);
                }    
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::runConfiguration", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SuperNode::changeConfiguration( ASAAC_PublicId configuration_event, ASAAC_PublicId &new_configuration )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.configuration_event = configuration_event;        
                sendAndReceive( ASAAC_GLI_Change_Configuration, ASAAC_GLI_Configuration_Changed, Param);        
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::changeConfiguration", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            new_configuration = Param._u.new_configuration;
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SuperNode::getBITResult( ASAAC_BitType type, ASAAC_BitResult &result )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.type = type;        
                sendAndReceive( ASAAC_GLI_Request_BIT_Result, ASAAC_GLI_Report_BIT_Result, Param);
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::getBITResult", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            result = Param._u.result;
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_TimedReturnStatus SuperNode::getAliveStatus( ASAAC_GLI_GliAliveParameter &parameter )
        {
            ASAAC_GLI_GliMessageParameter Param;           
            
            try
            {
                Param._u.alive_param = parameter;        
                sendAndReceive( ASAAC_GLI_Are_You_Alive, ASAAC_GLI_I_Am_Alive, Param);
                
                if ( (Param._u.alive_param.function_id != parameter.function_id) || 
                     (Param._u.alive_param.status_id != parameter.status_id) )
                {
                    parameter = Param._u.alive_param; 
                    throw OSException("Expected function or status differs from requested one", LOCATION);
                }    
            }
            catch ( ASAAC_Exception &e )
            {
                e.addPath( "GLI::getAliveStatus", LOCATION );
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
            }
            
            return  ASAAC_TM_SUCCESS;
        }
        
        
        ASAAC_ReturnStatus SuperNode::registerAllocateCFMHandler( OnAllocateCFM AllocateCFMHandler )
        {
            return registerHandler( m_OnAllocateCFMHandler, AllocateCFMHandler, handleAllocateCFM); 
        }
        
        
        ASAAC_ReturnStatus SuperNode::registerDeallocateCFMHandler( OnDeallocateCFM DeallocateCFMHandler )
        {
            return registerHandler( m_OnDeallocateCFMHandler, DeallocateCFMHandler, handleDeallocateCFM); 
        }
        
        
        ASAAC_ReturnStatus SuperNode::registerReportFaultHandler( OnReportFault ReportFaultHandler )
        {
            return registerHandler( m_OnReportFaultHandler, ReportFaultHandler, handleReportFault); 
        }

        
        ASAAC_ReturnStatus SuperNode::registerRequestSecureCommunicationHandler( OnRequestSecureCommunication RequestSecureCommunicationHandler )
        {
            return registerHandler( m_OnRequestSecureCommunicationHandler, RequestSecureCommunicationHandler, handleRequestSecureCommunication); 
        }

        
        ASAAC_ReturnStatus SuperNode::registerRequestKeyHandler( OnRequestKey RequestKeyHandler )
        {
            return registerHandler( m_OnRequestKeyHandler, RequestKeyHandler, handleRequestKey ); 
        }
        

        ASAAC_ReturnStatus SuperNode::handleAllocateCFM( ASAAC_PublicId &cfm_id )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SuperNode::handleDeallocateCFM( ASAAC_PublicId &cfm_id )
        {
            return ASAAC_ERROR;
        }
        
        
        ASAAC_ReturnStatus SuperNode::handleReportFault( ASAAC_FaultReport fault )
        {
            return ASAAC_ERROR;
        }


        ASAAC_ReturnStatus SuperNode::handleRequestSecureCommunication( ASAAC_PublicId tls_id, ASAAC_Bool &response )
        {
            response = ASAAC_BOOL_FALSE;
            return ASAAC_SUCCESS;
        }

        
        ASAAC_ReturnStatus SuperNode::handleRequestKey( ASAAC_PublicId tls_id, unsigned long encryption_key, unsigned long key_array[10] )
        {
            return ASAAC_ERROR;
        }

        
        void SuperNode::implementSecureCommunication( ASAAC_Bool response )
        {
            try
            {
                ASAAC_GLI_GliMessageParameter Param;
                Param._u.response = response;           
                sendRequest( ASAAC_GLI_SC_Response, Param );
                    
                if ( response == ASAAC_BOOL_TRUE )
                {
                    receiveReply(ASAAC_GLI_DH_Send_M, Param);
                    
                    unsigned long j = rand();
                    unsigned long M = Param._u.key;
                    unsigned long X = 2; //TODO: calculate a random value here
                    
                    Param._u.key = X;      
                    sendAndReceive(ASAAC_GLI_DH_Send_X, ASAAC_GLI_DH_Send_XimodM, Param);
            
                    m_EncryptionKey = acmodm(Param._u.key, j, M);
                    
                    Param._u.key = acmodm(X, j, M);
                    sendRequest(ASAAC_GLI_DH_Send_XjmodM, Param);
                }
            }
            catch ( ASAAC_Exception &e )
            {
                m_EncryptionKey = 0;
                
                e.addPath("Implementing a secure communication link failed", LOCATION);
                e.raiseError();
            }    
        }

        
        void SuperNode::handleSubNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout )
        {
            try
            {
                bool Reply = false;
                ASAAC_TimedReturnStatus Status;
                size_t Size = sizeof(ASAAC_GLI_GliMessage);
                
                switch (Message.unique_message_id)
                {
                    case ASAAC_GLI_Request_New_Cfm:
                    { 
                        ASAAC_PublicId cfm_id = OS_UNUSED_ID;
                        
                        m_OnAllocateCFMHandler( cfm_id );
                        
                        Message.message_parameter._u.Allocated_Cfm_Id = cfm_id;
                        Message.unique_message_id = ASAAC_GLI_Cfm_Allocated;
                        Reply = true;
                    }
                    break;
                        
                    case ASAAC_GLI_Deallocate_Cfm:
                    {                    
                        ASAAC_PublicId cfm_id = Message.message_parameter._u.Deallocate_Cfm_Id;
                        
                        m_OnDeallocateCFMHandler( cfm_id );
                        
                        Message.message_parameter._u.Deallocated_Cfm_Id = cfm_id;
                        Message.unique_message_id = ASAAC_GLI_Cfm_Deallocated;
                        Reply = true;
                    }
                    break;

                    case ASAAC_GLI_Fault_Report:
                    {                     
                        ASAAC_FaultReport fault = Message.message_parameter._u.the_fault;
                        
                        m_OnReportFaultHandler( fault );
                    }
                    break;

                    case ASAAC_GLI_Request_SC:
                    {                    
                        ASAAC_PublicId tls_id = Message.message_parameter._u.request_sc_tls_id;
                        ASAAC_Bool response   = ASAAC_BOOL_FALSE;
                        
                        m_OnRequestSecureCommunicationHandler( tls_id, response );
                        
                        implementSecureCommunication(response);
                    }
                    break;

                    case ASAAC_GLI_Request_Key:
                    {
                        ASAAC_PublicId tls_id = Message.message_parameter._u.request_key_tls_id;
                        unsigned long key_array[10] = {0,0,0,0,0,0,0,0,0,0};
                        
                        m_OnRequestKeyHandler( tls_id, m_EncryptionKey, key_array );

                        for (long Index = 0; Index < 10; Index++)
                            Message.message_parameter._u.key_array[Index] = key_array[Index];
                            
                        Message.unique_message_id = ASAAC_GLI_Cfm_Allocated;
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
        }       
    }
}
