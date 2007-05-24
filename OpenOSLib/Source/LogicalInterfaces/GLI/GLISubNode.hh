#ifndef GLISUBNODE_HH_
#define GLISUBNODE_HH_

#include "GLINode.hh"
#include "OpenOSIncludes.hh"

namespace ASAAC
{
    namespace GLI
    {
        class SubNode : public Node
        {
        public:
            typedef ASAAC_ReturnStatus(*OnLoadConfiguration)( ASAAC_PublicId &configuration_id );
            typedef ASAAC_ReturnStatus(*OnStopConfiguration)( ASAAC_PublicId &configuration_id );
            typedef ASAAC_ReturnStatus(*OnRunConfiguration)( ASAAC_PublicId &configuration_id );
            typedef ASAAC_ReturnStatus(*OnChangeConfiguration)( ASAAC_PublicId configuration_event, ASAAC_PublicId &new_configuration );
            typedef ASAAC_ReturnStatus(*OnGetBITResult)( ASAAC_BitType type, ASAAC_BitResult &result );
            typedef ASAAC_ReturnStatus(*OnGetAliveStatus)( ASAAC_GLI_GliAliveParameter &paramter );
        
        public:
        	SubNode();
        	virtual ~SubNode();
            
            ASAAC_TimedReturnStatus allocateCFM( ASAAC_PublicId &cfm_id );
            ASAAC_TimedReturnStatus deallocateCFM( ASAAC_PublicId &cfm_id );
            
            ASAAC_ReturnStatus      reportFault( ASAAC_FaultReport fault );
            
            ASAAC_TimedReturnStatus requestKey( ASAAC_PublicId tls_id, unsigned long &decryption_key, unsigned long *key_array[10] );
        
            ASAAC_ReturnStatus registerLoadConfiguration( OnLoadConfiguration LoadConfigurationHandler );
            ASAAC_ReturnStatus registerStopConfiguration( OnStopConfiguration StopConfigurationHandler );
            ASAAC_ReturnStatus registerRunConfiguration( OnRunConfiguration RunConfigurationHandler );
            ASAAC_ReturnStatus registerChangeConfiguration( OnChangeConfiguration ChangeConfigurationHandler );
            ASAAC_ReturnStatus registerGetBITResult( OnGetBITResult GetBITResultHandler );
            ASAAC_ReturnStatus registerGetAliveStatus ( OnGetAliveStatus GetAliveStatusHandler );
        
            virtual ASAAC_TimedReturnStatus handleOneRequest( ASAAC_TimeInterval p_Timeout );
            virtual unsigned long handleBufferedRequests( );
            virtual unsigned long handleRequests( ASAAC_TimeInterval p_Timeout );
        
        protected:
            void implementSecureCommunication( ASAAC_PublicId tls_id, unsigned long &decryption_key );

            static ASAAC_ReturnStatus handleLoadConfiguration( ASAAC_PublicId &configuration_id );
            static ASAAC_ReturnStatus handleStopConfiguration( ASAAC_PublicId &configuration_id );
            static ASAAC_ReturnStatus handleRunConfiguration( ASAAC_PublicId &configuration_id );
            static ASAAC_ReturnStatus handleChangeConfiguration( ASAAC_PublicId configuration_event, ASAAC_PublicId &new_configuration );
            static ASAAC_ReturnStatus handleGetBITResult( ASAAC_BitType type, ASAAC_BitResult &result );
            static ASAAC_ReturnStatus handleGetAliveStatus( ASAAC_GLI_GliAliveParameter &paramter );

            virtual void handleSuperNodeMessage( ASAAC_GLI_GliMessage Message, ASAAC_Time p_Timeout );
            
        private:
            OnLoadConfiguration     m_OnLoadConfigurationHandler;
            OnStopConfiguration     m_OnStopConfigurationHandler;
            OnRunConfiguration      m_OnRunConfigurationHandler;
            OnChangeConfiguration   m_OnChangeConfigurationHandler;
            OnGetBITResult          m_OnGetBITResultHandler;
            OnGetAliveStatus        m_OnGetAliveStatusHandler;
            
            unsigned long           m_DecryptionKey;
        };
    }
}

#endif /*GLISUBNODE_HH_*/
