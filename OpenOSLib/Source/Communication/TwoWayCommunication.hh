#ifndef TWOWAYCOMMUNICATION_HH_
#define TWOWAYCOMMUNICATION_HH_

#include "OpenOSIncludes.hh"

#include "Communication/OneWayCommunication.hh"


class TwoWayCommunication
{
public:
	TwoWayCommunication();
	virtual ~TwoWayCommunication();
	
    ASAAC_ReturnStatus attach() const;
    ASAAC_ReturnStatus attachClient() const;
    ASAAC_ReturnStatus attachServer() const;
	ASAAC_ReturnStatus detach() const;

	bool isAttached() const;
	
	void setClientConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId request_local_vc_id, const ASAAC_PublicId reply_local_vc_id); 
	void setServerConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId request_local_vc_id, const ASAAC_PublicId reply_local_vc_id); 
	void setRequestGlobalVc(const ASAAC_PublicId vc_id, const unsigned long msg_length, const unsigned long number_of_buffers = 16);
	void setReplyGlobalVc(const ASAAC_PublicId vc_id, const unsigned long msg_length, const unsigned long number_of_buffers = 16);
	
	void getClientConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &send_local_vc_id, ASAAC_PublicId &receive_local_vc_id) const; 
	void getServerConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &send_local_vc_id, ASAAC_PublicId &receive_local_vc_id) const; 
	void getRequestGlobalVc(ASAAC_PublicId &vc_id, unsigned long &msg_length, unsigned long &number_of_buffers) const;
	void getReplyGlobalVc(ASAAC_PublicId &vc_id, unsigned long &msg_length, unsigned long &number_of_buffers) const;

	OneWayCommunication &operator[](const int index);

private:	
	OneWayCommunication m_RequestChannel;
	OneWayCommunication m_ReplyChannel;
};

#endif /*TWOWAYCOMMUNICATION_HH_*/
