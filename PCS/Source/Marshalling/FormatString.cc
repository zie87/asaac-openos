#include "FormatString.hh"

#include <ctype.h>

#include <iostream>

using namespace std;

FormatString::FormatString() : m_CurrentPosition(0)
{
	m_String.size = 0;
}


FormatString::FormatString( const ASAAC_CharacterSequence& String ) : m_CurrentPosition(0)
{
	m_String = String;
}


FormatString::~FormatString()
{
}


void FormatString::setString( const ASAAC_CharacterSequence& String )
{
	m_String = String;
	rewind();
}


void FormatString::rewind()
{
	m_CurrentPosition = 0;
}


bool FormatString::findNextElement( char& Identifier, unsigned short& Number, ASAAC_CharacterSequence& Parameters )
{
	Number = 0;
	
	for (;;)
	{
		char currentChar = m_String.data[ m_CurrentPosition ];

		// skip blanks
		if ( currentChar == ' ' )
		{
			m_CurrentPosition++;
			continue;
		}
		
		if ( isdigit( currentChar ) )
		{
			Number *= 10;
			Number += int(currentChar)-int('0');
			
			m_CurrentPosition++;
			continue;
		}
		
		switch ( currentChar ) {
			
			case 'o' :
			case 's' :
			case 'l' :
			case 'L' :
			case 'f' :
			case 'd' :
			case 'D' :
			case 'e' :
						Identifier = currentChar;
						Parameters.size = 0;
						
						if ( Number == 0 ) Number = 1;
						
						m_CurrentPosition ++;
						
						return true;
						break;
						
			case 'S' :
			case 'U' :
			case 'Q' :{
						Identifier = currentChar;
						unsigned short ParameterEnd = findMatchingBracePosition( m_CurrentPosition + 1 );
						if ( ParameterEnd == (m_CurrentPosition+1) ) return false;
						
						Parameters = CharSeq(m_String).asaac_str( m_CurrentPosition+2, ( ParameterEnd - 1 ) - ( m_CurrentPosition + 2 ) + 1 );
						m_CurrentPosition = ParameterEnd + 1;
						
						if ( Number == 0 ) Number = 1;
						
						return true;
						break;
					  }
						
			default:	return false;
		}
	}
	
	return true;
}


unsigned short FormatString::findMatchingBracePosition( unsigned short ThisBracePosition )
{
	char CurrentBrace = m_String.data[ ThisBracePosition ];
	char MatchingBrace;
	
	switch ( CurrentBrace ) {
		case '(' : MatchingBrace = ')'; break;
		case '{' : MatchingBrace = '}'; break;
		case '[' : MatchingBrace = ']'; break;
		default: return ThisBracePosition;
	};
	
	unsigned long CurrentPos = ThisBracePosition + 1;
	unsigned long BraceCount = 1;
	
	for (;;)
	{
		if ( m_String.data[ CurrentPos ] == MatchingBrace ) 
			BraceCount--;
		
		if ( m_String.data[ CurrentPos ] == CurrentBrace  ) 
			BraceCount++;
		
		if ( m_String.data[ CurrentPos ] == '\0'          ) 
			return ThisBracePosition;

		if ( BraceCount == 0 ) 
			return CurrentPos;
		
		CurrentPos ++;
	}
}


