#include "FormatString.hh"

#include <ctype.h>

#include <iostream>

using namespace std;

FormatString::FormatString() : m_String(""), m_CurrentPosition(0)
{
}


FormatString::FormatString( const string& String ) : m_String( String ), m_CurrentPosition(0)
{
}


FormatString::~FormatString()
{
}


void FormatString::setString( const string& String )
{
	m_String = String;
	rewind();
}


void FormatString::rewind()
{
	m_CurrentPosition = 0;
}


bool FormatString::findNextElement( char& Identifier, unsigned short& Number, string& Parameters )
{
	const char* cString = m_String.c_str();
	
	Number = 0;
	
	for (;;)
	{
		char currentChar = cString[ m_CurrentPosition ];

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
						Parameters = string("");
						
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
						
						Parameters = m_String.substr( m_CurrentPosition+2, ( ParameterEnd - 1 ) - ( m_CurrentPosition + 2 ) + 1 );
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
	const char* cString = m_String.c_str();
	
	char CurrentBrace = cString[ ThisBracePosition ];
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
		if ( cString[ CurrentPos ] == MatchingBrace ) BraceCount--;
		if ( cString[ CurrentPos ] == CurrentBrace  ) BraceCount++;
		
		if ( cString[ CurrentPos ] == '\0'          ) return ThisBracePosition;

		if ( BraceCount == 0 ) return CurrentPos;
		
		CurrentPos ++;
	}
}


