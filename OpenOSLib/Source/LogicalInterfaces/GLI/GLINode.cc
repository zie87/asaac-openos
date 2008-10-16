#include "GLINode.hh"

#include "OpenOSObject.hh"

namespace ASAAC
{
    namespace GLI
    {
        
        Node::Node()
        {
            m_Timeout = TimeIntervalInfinity;
            
        }
        
        
        Node::~Node()
        {
        }
        
        
        void Node::setSenderVc(ASAAC_PublicId SenderVc)
        {
            m_SenderVc = SenderVc;
        }
        
        
        ASAAC_PublicId Node::getSenderVc()
        {
            return m_SenderVc;
        }
        
        
        void Node::setReceiverVc(ASAAC_PublicId ReceiverVc)
        {
            m_ReceiverVc = ReceiverVc;
        }
        
                
        ASAAC_PublicId Node::getReceiverVc()
        {
            return m_ReceiverVc;
        }       
        
        
        void Node::setTimeout(ASAAC_TimeInterval Timeout)
        {
            m_Timeout = Timeout;
        }
        
        
        ASAAC_TimeInterval Node::getTimeout()
        {
            return m_Timeout;
        }
        
        
        ASAAC_TimedReturnStatus Node::handleOneRequest( ASAAC_TimeInterval p_Timeout )
        {
            ASAAC_Time Timeout = TimeStamp(p_Timeout).asaac_Time();
    
            ASAAC_TimedReturnStatus Status;
            size_t MaxSize = sizeof(ASAAC_GLI_GliMessage);
            unsigned long Size;
            ASAAC_GLI_GliMessage Message;
            
            try
            {
                Status = ASAAC_APOS_receiveMessage( getSenderVc(), &p_Timeout, MaxSize, &Message, &Size );
                
                if (Status == ASAAC_TM_ERROR)
                    throw OSException( "Error receiving a request", LOCATION );

                if (Status == ASAAC_TM_TIMEOUT)
                    throw TimeoutException( "Timeout receiving a request", LOCATION );

                if (Size > MaxSize)
                    throw OSException( "Message size is too small", LOCATION );

                handleMessage( Message, Timeout );
            }
            catch (ASAAC_Exception &e)
            {
                e.addPath("Error handling a GLI request", LOCATION);
                e.raiseError();
                
                return (e.isTimeout()) ? ASAAC_TM_ERROR : ASAAC_TM_ERROR;
            }
            
            return ASAAC_TM_SUCCESS;
        }


        ASAAC_TimedReturnStatus Node::handleOneRequest( )
        {
            return handleOneRequest( m_Timeout );
        }
        
        
        unsigned long Node::handleBufferedRequests( )
        {
            unsigned long MessageCounter = 0;
    
            ASAAC_ResourceReturnStatus Status = ASAAC_RS_SUCCESS;
            size_t MaxSize = sizeof(ASAAC_GLI_GliMessage);
            unsigned long Size;
            ASAAC_GLI_GliMessage Message;
            
            do
            {
                try
                {
                    Status = ASAAC_APOS_receiveMessageNonblocking( getSenderVc(), MaxSize, &Message, &Size );
                    
                    if (Status != ASAAC_RS_RESOURCE)
                    {       
                        if (Status == ASAAC_RS_ERROR)
                            throw OSException( "Error receiving a request", LOCATION );
        
                        if (Size > MaxSize)
                            throw OSException( "Message size is too small", LOCATION );
        
                        ASAAC_Time Timeout = TimeStamp(m_Timeout).asaac_Time();
                        handleMessage( Message, Timeout );
                        
                        MessageCounter++;
                    }
                }
                catch (ASAAC_Exception &e)
                {
                    e.addPath("Error handling a GLI request", LOCATION);
                    e.raiseError();
                }
            }
            while (Status == ASAAC_RS_SUCCESS);
            
            return MessageCounter;
        }
        
        
        unsigned long Node::handleRequests( ASAAC_TimeInterval p_Timeout )
        {
            ASAAC_TimedReturnStatus Status = ASAAC_TM_SUCCESS;
            ASAAC_Time Timeout = TimeStamp(m_Timeout).asaac_Time();
            unsigned long MessageCounter = 0;

            while ((TimeStamp::Now() < Timeout) && (Status == ASAAC_TM_SUCCESS))
            {
                ASAAC_TimeInterval Interval = TimeStamp(Timeout).asaac_Interval();
                Status = handleOneRequest(Interval);
                
                if (Status == ASAAC_TM_SUCCESS)
                    MessageCounter++;
            }
            
            return MessageCounter;
        }
        

        unsigned long Node::handleRequests( )
        {
            return handleRequests( m_Timeout );
        }
        

        void Node::handleMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout )
        {
            static ASAAC_CharacterSequence UnknownPCSMsg = CharacterSequence("Unknown PCS Message.").asaac_str();
    
            switch (Message.unique_message_id)
            {
                case ASAAC_GLI_Load_Configuration:
                case ASAAC_GLI_Stop_Configuration:
                case ASAAC_GLI_Run_Configuration:
                case ASAAC_GLI_Change_Configuration:
                case ASAAC_GLI_Cfm_Allocated:
                case ASAAC_GLI_Cfm_Deallocated:
                case ASAAC_GLI_Request_BIT_Result:
                case ASAAC_GLI_Are_You_Alive:
                case ASAAC_GLI_SC_Response:
                case ASAAC_GLI_DH_Send_X:
                case ASAAC_GLI_DH_Send_XjmodM:
                case ASAAC_GLI_Send_Key:    
                    handleSuperNodeMessage( Message, p_Timeout ); break;

                case ASAAC_GLI_Configuration_Loaded:
                case ASAAC_GLI_Configuration_Stopped:
                case ASAAC_GLI_Configuration_Running:
                case ASAAC_GLI_Configuration_Changed:
                case ASAAC_GLI_Request_New_Cfm:
                case ASAAC_GLI_Deallocate_Cfm:                    
                case ASAAC_GLI_Fault_Report:                    
                case ASAAC_GLI_Report_BIT_Result:                    
                case ASAAC_GLI_I_Am_Alive:                    
                case ASAAC_GLI_Request_SC:                    
                case ASAAC_GLI_DH_Send_M:
                case ASAAC_GLI_DH_Send_XimodM:
                case ASAAC_GLI_Request_Key: 
                    handleSubNodeMessage( Message, p_Timeout ); break;
                 
                default: ASAAC_APOS_logMessage( &UnknownPCSMsg, ASAAC_LOG_MESSAGE_TYPE_ERROR );                                          
            }
        }


        void Node::handleSubNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout )
        {
            throw OSException("Unexpected GLI message (Reply instead of Request).", LOCATION);            
        }
        
        
        void Node::handleSuperNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout )
        {
            throw OSException("Unexpected GLI message (Reply instead of Request).", LOCATION);            
        }

        
        void Node::sendRequest( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter Request, ASAAC_Time Timeout )
        {
            handleBufferedRequests();
            
            size_t max_size = sizeof(ASAAC_GLI_GliMessage);
            ASAAC_GLI_GliMessage Message;
        
            Message.unique_message_id     = MessageId;
            Message.message_parameter._d  = MessageId; 
            Message.message_parameter._u  = Request._u;
        
            ASAAC_TimeInterval int_timeout = TimeStamp( Timeout ).asaac_Interval();
        
            if (ASAAC_APOS_sendMessage( getSenderVc(), &int_timeout, &Message, max_size) != ASAAC_TM_SUCCESS)
                throw OSException("Error sending a message", LOCATION);;
        }
        
        
        void Node::sendRequest( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter Request )
        {
            sendRequest( MessageId, Request, TimeStamp(m_Timeout).asaac_Time() );
        }
        
        
        void Node::receiveReply( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter &Reply, ASAAC_Time Timeout )
        {
            size_t max_size = sizeof(ASAAC_GLI_GliMessage);
            long unsigned size = 0;
            ASAAC_GLI_GliMessage Message;
        
            do
            {
                ASAAC_TimeInterval int_timeout = TimeStamp( Timeout ).asaac_Interval();
                
                if (ASAAC_APOS_receiveMessage( getReceiverVc(), &int_timeout, max_size, &Message, &size) != ASAAC_TM_SUCCESS)
                    throw OSException("Error receiving a message", LOCATION);
            }               
            while (Message.unique_message_id != MessageId);
            
            if (size < max_size)
                throw OSException("Size of message is too small", LOCATION);;
            
            if (Message.message_parameter._d != MessageId)
                throw OSException("MessageId is not valid", LOCATION);
                
            Reply = Message.message_parameter;
        }
        
        
        void Node::receiveReply( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter &Reply )
        {
            receiveReply( MessageId, Reply, TimeStamp(m_Timeout).asaac_Time() );
        }
        
        
        void Node::sendAndReceive( ASAAC_GLI_GliMessageId SendMessageId, ASAAC_GLI_GliMessageId ReceiveMessageId, ASAAC_GLI_GliMessageParameter &Param, ASAAC_Time Timeout )
        {
            sendRequest( SendMessageId, Param, Timeout );
            receiveReply( ReceiveMessageId, Param, Timeout );
        }
        
        void Node::sendAndReceive( ASAAC_GLI_GliMessageId SendMessageId, ASAAC_GLI_GliMessageId ReceiveMessageId, ASAAC_GLI_GliMessageParameter &Param )
        {
            sendAndReceive( SendMessageId, ReceiveMessageId, Param, TimeStamp(m_Timeout).asaac_Time() );
        }
    }
}
