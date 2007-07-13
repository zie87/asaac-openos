#include "TwoWayCommunication.hh"

#include "Communication/OneWayCommunication.hh"


TwoWayCommunication::TwoWayCommunication()
{
}



TwoWayCommunication::~TwoWayCommunication()
{
}



void TwoWayCommunication::attach() const
{
	m_RequestChannel.assureCommunication();
	m_ReplyChannel.assureCommunication();
}


void TwoWayCommunication::attachClient() const
{
    m_RequestChannel.assureSenderConnection();
    m_ReplyChannel.assureReceiverConnection();
}


void TwoWayCommunication::attachServer() const
{
   m_RequestChannel.assureReceiverConnection();
   m_ReplyChannel.assureSenderConnection();
}


void TwoWayCommunication::detach() const
{
	m_RequestChannel.detach();
	m_ReplyChannel.detach();
}


	
bool TwoWayCommunication::isAttached() const
{
	return ( m_RequestChannel.isAttached() &&
	     	 m_ReplyChannel.isAttached() );
}	


	
void TwoWayCommunication::setClientConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId send_local_vc_id, const ASAAC_PublicId receive_local_vc_id)
{
	m_RequestChannel.setSenderConfiguration(process_id, thread_id, send_local_vc_id);
	m_ReplyChannel.setReceiverConfiguration(process_id, thread_id, receive_local_vc_id);
}


 
void TwoWayCommunication::setServerConfiguration(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, const ASAAC_PublicId send_local_vc_id, const ASAAC_PublicId receive_local_vc_id) 
{
	m_RequestChannel.setReceiverConfiguration(process_id, thread_id, receive_local_vc_id);
	m_ReplyChannel.setSenderConfiguration(process_id, thread_id, send_local_vc_id);
}



void TwoWayCommunication::setRequestGlobalVc(const ASAAC_PublicId vc_id, const unsigned long msg_length, const unsigned long number_of_buffers)
{
	m_RequestChannel.setGlobalVcConfiguration(vc_id, msg_length, number_of_buffers);
}



void TwoWayCommunication::setReplyGlobalVc(const ASAAC_PublicId vc_id, const unsigned long msg_length, const unsigned long number_of_buffers)
{
	m_ReplyChannel.setGlobalVcConfiguration(vc_id, msg_length, number_of_buffers);
}

 
 
void TwoWayCommunication::getClientConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &send_local_vc_id, ASAAC_PublicId &receive_local_vc_id) const 
{
	m_RequestChannel.getSenderConfiguration(process_id, thread_id, send_local_vc_id);
	m_ReplyChannel.getReceiverConfiguration(process_id, thread_id, receive_local_vc_id);
}


 
void TwoWayCommunication::getServerConfiguration(ASAAC_PublicId &process_id, ASAAC_PublicId &thread_id, ASAAC_PublicId &send_local_vc_id, ASAAC_PublicId &receive_local_vc_id) const
{
	m_RequestChannel.getReceiverConfiguration(process_id, thread_id, receive_local_vc_id);
	m_ReplyChannel.getSenderConfiguration(process_id, thread_id, send_local_vc_id);
}


 
void TwoWayCommunication::getRequestGlobalVc(ASAAC_PublicId &vc_id, unsigned long &msg_length, unsigned long &number_of_buffers) const
{
	m_RequestChannel.getGlobalVcConfiguration(vc_id, msg_length, number_of_buffers);
}



void TwoWayCommunication::getReplyGlobalVc(ASAAC_PublicId &vc_id, unsigned long &msg_length, unsigned long &number_of_buffers) const
{
	m_ReplyChannel.getGlobalVcConfiguration(vc_id, msg_length, number_of_buffers);
}



OneWayCommunication &TwoWayCommunication::operator[](const long index)
{
	switch (index)
	{
		case 0: return m_RequestChannel;
		case 1: return m_ReplyChannel;
		default: throw FatalException("Operator [] was called with a false index", LOCATION);
	}
}
