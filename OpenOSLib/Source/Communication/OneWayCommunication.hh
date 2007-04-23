#ifndef ONEWAYCOMMUNICATION_HH_
#define ONEWAYCOMMUNICATION_HH_

#include "OpenOSIncludes.hh"
#include "ProcessManagement/Process.hh"

typedef struct OneWayConfiguration 
{
	ASAAC_PublicId global_vc_id;
	
	ASAAC_PublicId sender_process_id;
	ASAAC_PublicId sender_thread_id;
	ASAAC_PublicId sender_local_vc_id;
	
	ASAAC_PublicId receiver_process_id;
	ASAAC_PublicId receiver_thread_id;
	ASAAC_PublicId receiver_local_vc_id;
	
	unsigned long msg_length;
	unsigned long number_of_buffers;
	unsigned long number_of_threads;
	
	ASAAC_VirtualChannelType vc_type; 
	
	unsigned long priority;
};



class OneWayCommunication
{
public:
	OneWayCommunication();
	virtual ~OneWayCommunication();

    ASAAC_ReturnStatus assureCommunication() const;
    ASAAC_ReturnStatus assureGlobalVc() const;
    ASAAC_ReturnStatus assureSenderConnection() const;
    ASAAC_ReturnStatus assureReceiverConnection() const;
    
    ASAAC_ReturnStatus detach() const;
    ASAAC_ReturnStatus detachSender() const;
    ASAAC_ReturnStatus detachReceiver() const;

	bool isAttached() const;
	
	void setConfiguration(const OneWayConfiguration config);
	void setSenderConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId local_vc_id); 
	void setReceiverConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId local_vc_id); 
	void setGlobalVcConfiguration(const ASAAC_PublicId global_vc_id, const unsigned long msg_length, const unsigned long number_of_buffers = 16);
	
	OneWayConfiguration getConfiguration() const;
	void getSenderConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &local_vc_id) const; 
	void getReceiverConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &local_vc_id) const; 
	void getGlobalVcConfiguration(ASAAC_PublicId &global_vc_id, unsigned long &msg_length, unsigned long &number_of_buffers) const;
	
private:
	OneWayConfiguration m_Configuration;

	typedef union {
		OSScopeCommandBuffer buffer;
		ASAAC_VcMappingDescription mapping;
	} OSScopeData;

	ASAAC_VcDescription 		getReferenceGlobalVcDescription() const;
	ASAAC_VcMappingDescription 	getReferenceSenderVcDescription() const;
	ASAAC_VcMappingDescription 	getReferenceReceiverVcDescription() const;
	
	bool						isGlobalVcAdequate(const ASAAC_VcDescription currentDescription, const ASAAC_VcDescription referenceDescription) const;
	bool						isLocalVcAdequate(const ASAAC_VcMappingDescription currentDescription, const ASAAC_VcMappingDescription referenceDescription) const;
	
	ASAAC_ReturnStatus 			invokeOSScope( const ASAAC_PublicId process_id, const OSScopeFunction foo, OSScopeData param) const;

	//OSScope Functions (necessary if one of the processes is equal to the process asking for this configuration
	static ASAAC_ReturnStatus 	attachLocalVc( OSScopeCommandBuffer param);
	
	static ASAAC_ReturnStatus 	detachLocalVc( OSScopeCommandBuffer param);

};

#endif /*ONEWAYCOMMUNICATION_HH_*/
