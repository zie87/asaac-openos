#include "NameGenerator.hh"
//COMMENT.SMS> Original code does not work with gcc 4.1.2
//#include <sstream>

NameGenerator::NameGenerator() : m_Counter(0)
{
}

NameGenerator::~NameGenerator()
{
}


ASAAC_CharacterSequence NameGenerator::createUniqueName( const ASAAC_CharacterSequence& Prefix )
{
	ASAAC_CharacterSequence Return;

	char Buffer[256];
	sprintf(Buffer,"%s (#%lu)",CharSeq(Prefix).c_str(),m_Counter);
	Return.size = strlen(Buffer);
	memcpy( Return.data, Buffer, Return.size );

	m_Counter++; 
	
	return Return;
}


NameGenerator* NameGenerator::getInstance()
{
	static NameGenerator ThisGenerator;
	
	return &ThisGenerator;
}

	
	
