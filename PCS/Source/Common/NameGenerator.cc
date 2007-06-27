#include "NameGenerator.hh"
//COMMENT.SMS> Original code does not work with gcc 4.1.2
//#include <sstream>

NameGenerator::NameGenerator() : m_Counter(0)
{
}

NameGenerator::~NameGenerator()
{
}


ASAAC_CharacterSequence NameGenerator::createUniqueName( const string& Prefix )
{
	ASAAC_CharacterSequence Return;
//COMMENT.SMS> Original code does not work with gcc 4.1.2
//	stringstream Buffer;
//	
//	Buffer << Prefix << " (#" << m_Counter << ")";
//	
//	Return.size = Buffer.str().length();
//	memcpy( Return.data, Buffer.str().c_str(), Return.size );

	char Buffer[256];
	sprintf(Buffer,"%s (#%lu)",Prefix.c_str(),m_Counter);
	Return.size = strlen(Buffer);
	memcpy( Return.data, Buffer, Return.size );

//COMMENT.SMS> end of replaced code
	
	m_Counter++; //pun: Overload is not checked


	
	return Return;
}


NameGenerator* NameGenerator::getInstance()
{
	static NameGenerator ThisGenerator;
	
	return &ThisGenerator;
}

	
	
