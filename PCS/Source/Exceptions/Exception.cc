#include "Exception.hh"

//COMMENT.SMS> Original code does not work with gcc 4.1.2
//#include <sstream>

PCSException::PCSException() : m_Message(""), m_TcId(0), m_VcId(0)
{
}


PCSException::PCSException( ASAAC_PublicId TcId, ASAAC_PublicId VcId, const string& Message ) : m_Message(Message), m_TcId(TcId), m_VcId(VcId)
{
}

PCSException::~PCSException()
{
}


string PCSException::getFullMessage()
{
//COMMENT.SMS> Original code does not work with gcc 4.1.2
//	ostringstream Buffer;
//	
//	Buffer << "Exception for Tc " << m_TcId << " / VC " << m_VcId << " : '" << m_Message << "'";
//	
//	return Buffer.str();

	char TcVcString[256];
	
	sprintf(TcVcString,"TC = %d, VC = %d : ", (int) m_TcId, (int) m_VcId);
	
	return string(TcVcString) + m_Message;
//COMMENT.SMS> end of replaced code

}
