#ifndef GLISUPERNODE_HH_
#define GLISUPERNODE_HH_

#include "GLINode.hh"
#include "OpenOSIncludes.hh"

namespace ASAAC
{
    namespace GLI
    {
        class SuperNode : public Node
        {
        public:
            typedef ASAAC_ReturnStatus(*OnAllocateCFM)( ASAAC_PublicId &cfm_id );
            typedef ASAAC_ReturnStatus(*OnDeallocateCFM)( ASAAC_PublicId &cfm_id );
            typedef ASAAC_ReturnStatus(*OnReportFault)( ASAAC_FaultReport fault );
            typedef ASAAC_ReturnStatus(*OnRequestSecureCommunication)( ASAAC_PublicId tls_id, ASAAC_Bool &response );
            typedef ASAAC_ReturnStatus(*OnRequestKey)( ASAAC_PublicId tls_id, unsigned long encryption_key, unsigned long key_array[10] );
            
        public:
        	SuperNode();
        	virtual ~SuperNode();
            
            ASAAC_TimedReturnStatus loadConfiguration( ASAAC_PublicId &configuration_id  );
            ASAAC_TimedReturnStatus stopConfiguration( ASAAC_PublicId &configuration_id );
            ASAAC_TimedReturnStatus runConfiguration( ASAAC_PublicId &configuration_id );
            ASAAC_TimedReturnStatus changeConfiguration( ASAAC_PublicId configuration_event, ASAAC_PublicId &new_configuration );
            
            ASAAC_TimedReturnStatus getBITResult( ASAAC_BitType type, ASAAC_BitResult &result );
            
            ASAAC_TimedReturnStatus getAliveStatus( ASAAC_GLI_GliAliveParameter &paramter );
            
            ASAAC_ReturnStatus registerAllocateCFMHandler( OnAllocateCFM AllocateCFMHandler );
            ASAAC_ReturnStatus registerDeallocateCFMHandler( OnDeallocateCFM DeallocateCFMHandler);
            ASAAC_ReturnStatus registerReportFaultHandler( OnReportFault ReportFaultHandler );
            ASAAC_ReturnStatus registerRequestSecureCommunicationHandler( OnRequestSecureCommunication RequestSecureCommunicationHandler );
            ASAAC_ReturnStatus registerRequestKeyHandler( OnRequestKey RequestKeyHandler );

            virtual ASAAC_TimedReturnStatus handleOneRequest( ASAAC_TimeInterval p_Timeout );
            virtual unsigned long handleBufferedRequests( );
            virtual unsigned long handleRequests( ASAAC_TimeInterval p_Timeout );
            
        protected:
            void implementSecureCommunication( ASAAC_Bool response );

            static ASAAC_ReturnStatus handleAllocateCFM( ASAAC_PublicId &cfm_id );
            static ASAAC_ReturnStatus handleDeallocateCFM( ASAAC_PublicId &cfm_id );
            static ASAAC_ReturnStatus handleReportFault( ASAAC_FaultReport fault );
            static ASAAC_ReturnStatus handleRequestSecureCommunication( ASAAC_PublicId tls_id, ASAAC_Bool &response );
            static ASAAC_ReturnStatus handleRequestKey( ASAAC_PublicId tls_id, unsigned long encryption_key, unsigned long key_array[10] );
            
            virtual void handleSubNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout );
            
        private:
            OnAllocateCFM                   m_OnAllocateCFMHandler;
            OnDeallocateCFM                 m_OnDeallocateCFMHandler;    
            OnReportFault                   m_OnReportFaultHandler;
            OnRequestSecureCommunication    m_OnRequestSecureCommunicationHandler;
            OnRequestKey                    m_OnRequestKeyHandler;
            
            unsigned long                   m_EncryptionKey;
        };
    }
}

#endif /*GLISUPERNODE_HH_*/
