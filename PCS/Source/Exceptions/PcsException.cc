#include "PcsException.hh"

PcsException::PcsException()
{
	m_Message.size = 0;
	m_TcId = 0;
	m_VcId = 0;
}


PcsException::PcsException( ASAAC_PublicId TcId, ASAAC_PublicId VcId, char* Message )
{
	m_Message = CharSeq(Message).asaac_str();
	m_TcId = TcId;
	m_VcId = VcId;
}

PcsException::~PcsException()
{
}


const char* PcsException::getFullMessage()
{
	static CharacterSequence Message;
	
	Message.erase();

	char TcVcString[256];
	sprintf(TcVcString,"TC = %d, VC = %d : ", (int) m_TcId, (int) m_VcId);
	
	Message << TcVcString << m_Message;
	
	return Message.c_str();
}
