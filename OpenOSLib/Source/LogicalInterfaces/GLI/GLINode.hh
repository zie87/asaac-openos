#ifndef GLINODE_HH_
#define GLINODE_HH_

#include "OpenOSIncludes.hh"

namespace ASAAC
{
    namespace GLI
    {
        class Node
        {
        public:
        	Node();
        	virtual ~Node();
        
            void setSenderVc(ASAAC_PublicId SenderVc);
            ASAAC_PublicId getSenderVc();
        
            void setReceiverVc(ASAAC_PublicId ReceiverVc);
            ASAAC_PublicId getReceiverVc();
        
            void setTimeout(ASAAC_TimeInterval Timeout);
            ASAAC_TimeInterval getTimeout();
            
            ASAAC_TimedReturnStatus handleOneRequest( ASAAC_TimeInterval p_Timeout );
            ASAAC_TimedReturnStatus handleOneRequest( );
            
            unsigned long handleBufferedRequests( );
            
            unsigned long handleRequests( ASAAC_TimeInterval p_Timeout );
            unsigned long handleRequests( );
        
        protected:    
            void sendRequest( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter Request, ASAAC_Time Timeout );
            void sendRequest( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter Request );
        
            void receiveReply( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter &Reply, ASAAC_Time Timeout );
            void receiveReply( ASAAC_GLI_GliMessageId MessageId, ASAAC_GLI_GliMessageParameter &Reply );
        
            void sendAndReceive( ASAAC_GLI_GliMessageId SendMessageId, ASAAC_GLI_GliMessageId ReceiveMessageId, ASAAC_GLI_GliMessageParameter &Param, ASAAC_Time Timeout );
            void sendAndReceive( ASAAC_GLI_GliMessageId SendMessageId, ASAAC_GLI_GliMessageId ReceiveMessageId, ASAAC_GLI_GliMessageParameter &Param );
        
            void handleMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout );
            virtual void handleSubNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout );
            virtual void handleSuperNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout );
            
            template<class T> ASAAC_ReturnStatus registerHandler( T &Member, T Handler, T DefaultHandler );
        
        private:
            ASAAC_PublicId      m_SenderVc;
            ASAAC_PublicId      m_ReceiverVc;
            ASAAC_TimeInterval  m_Timeout;
        };
    }
}

#endif /*GLINODE_HH_*/
