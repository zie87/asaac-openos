#include "CDRDataBlock.hh"

#include <arpa/inet.h>
#include <byteswap.h>

#include "PcsCIncludes.hh"

inline void swapBytes( char& Val1, char& Val2 ) { char Store = Val1; Val1 = Val2; Val2 = Store; }


CDRDataBlock::CDRDataBlock( ASAAC_Address Data, unsigned long Size ) : m_CurrentIndex(0)
{
	m_Data = (char*)Data;
	m_Size = Size;
	
	union 
	{
		short Integer;
		bool  Bool;
	} IsLittleEndian;
	
	IsLittleEndian.Integer = 1;
	
	m_LittleEndian = IsLittleEndian.Bool;
}


CDRDataBlock::~CDRDataBlock()
{
}


void CDRDataBlock::rewind()
{
	m_CurrentIndex = 0;
}


bool CDRDataBlock::seek( unsigned long Index )
{
	if ( Index >= m_Size ) 
		return false;
	
	m_CurrentIndex = Index;
	
	return true;
}


unsigned long CDRDataBlock::getCurrentIndex()
{
	return m_CurrentIndex;
}


bool CDRDataBlock::jumpToAlign( size_t Alignment )
{
	m_CurrentIndex += (( Alignment - ( m_CurrentIndex % Alignment ) ) % Alignment );
	
	// in CDR, Alignment == Size. Use Alignment to check for overflows.
	if ( m_CurrentIndex + Alignment > m_Size ) 
		throw PcsMemoryOverflowException();
	
	return true;
}


char CDRDataBlock::getNextChar()
{
	jumpToAlign( 1 );
	
	return m_Data[ m_CurrentIndex ++ ];
}


bool CDRDataBlock::setNextChar( char Value )
{
	jumpToAlign( 1 );
	
	m_Data[ m_CurrentIndex++ ] = Value;
	
	return true;
}



short CDRDataBlock::getNextShort()
{
	jumpToAlign( 2 );

	short ReturnVal = *( reinterpret_cast<short*>(& m_Data[ m_CurrentIndex ]) );
	m_CurrentIndex += 2;
	
	return htons(ReturnVal);
}


bool CDRDataBlock::setNextShort( short Value )
{
	jumpToAlign( 2 );

	*( reinterpret_cast<short*>(& m_Data[ m_CurrentIndex ]) ) = htons( Value );
	m_CurrentIndex += 2;
	
	return true;
}


long CDRDataBlock::getNextLong()
{
	if ( !jumpToAlign( 4 ) ) return 0;

	long ReturnVal = *( reinterpret_cast<long*>(& m_Data[ m_CurrentIndex ]) );
	m_CurrentIndex += 4;
	
	return htonl(ReturnVal);
}


bool CDRDataBlock::setNextLong( long Value )
{
	if ( !jumpToAlign( 4 ) ) return false;
	
	*( reinterpret_cast<long*>(& m_Data[ m_CurrentIndex ]) ) = htonl( Value );
	m_CurrentIndex += 4;
	
	return true;
}



long long CDRDataBlock::getNextLongLong()
{
	long long Value;
	
	if ( !jumpToAlign( 8 ) ) return 0;

	Value = *( reinterpret_cast<long long*>(& m_Data[ m_CurrentIndex ]) );
	m_CurrentIndex += 8;
	
	if ( m_LittleEndian )
	{
		return bswap_64( Value );
	}
	
	return Value;
}


bool CDRDataBlock::setNextLongLong( long long Value )
{
	if ( !jumpToAlign( 8 ) ) return 0;

	if ( m_LittleEndian )
	{
		*( reinterpret_cast<long long*>(& m_Data[ m_CurrentIndex ]) ) = bswap_64( Value );
	}
	else
	{
		*( reinterpret_cast<long long*>(& m_Data[ m_CurrentIndex ]) ) = Value;
	}
	
	m_CurrentIndex += 8;
	
	return true;
}


unsigned long 	CDRDataBlock::getNextEnum()
{
	if ( !jumpToAlign( 2 ) ) return 0;
	
	short ReturnVal = *( reinterpret_cast<short*>(& m_Data[ m_CurrentIndex ]) );
	
	m_CurrentIndex += 2;
	
	return htons(ReturnVal);
}	


bool			CDRDataBlock::setNextEnum( unsigned long Value )
{
	if ( !jumpToAlign( 2 ) ) return 0;

	*( reinterpret_cast<short*>(& m_Data[ m_CurrentIndex ]) ) = htons( Value );
	
	m_CurrentIndex += 2;
	
	return true;
}

