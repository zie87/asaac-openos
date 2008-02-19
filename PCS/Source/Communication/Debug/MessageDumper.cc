#include "MessageDumper.hh"

#include <iostream>
#include <iomanip>

using namespace std;

MessageDumper::MessageDumper() : m_Consumer(0), m_OutputStream(&cout)
{
	m_Name.size = 0;
}


void MessageDumper::initialize()
{
	
}


void MessageDumper::deinitialize()
{
	
}


ASAAC_ReturnStatus MessageDumper::processMessage( ASAAC_PublicId TcId, ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length )
{
	*m_OutputStream << "_______________________ DUMP " << CharSeq(m_Name) << " (TC " << TcId << "/VC " << GlobalVc << ") " <<
		    "_______________________" << endl;

	     
	char* CharData = reinterpret_cast<char*>( Data );
	
	unsigned long RemainingLength = Length;
	unsigned long CurrentIndex = 0;
	
	while ( RemainingLength > 0 )
	{
		unsigned long ThisColumnLength = ( RemainingLength > 16 ) ? 16 : RemainingLength;
		
		unsigned long ColumnCount = 0;
		
		for ( ; ColumnCount < ThisColumnLength; ColumnCount ++ )
		{
			*m_OutputStream << setfill('0') << setw(2) << hex << (short)CharData[ CurrentIndex + ColumnCount ];
			*m_OutputStream << " ";
		}
		
		for ( ; ColumnCount < 16; ColumnCount ++ )
		{
			*m_OutputStream << "   ";
		}
		
		*m_OutputStream << setw(0) << dec << " |  ";

		for ( ColumnCount = 0; ColumnCount < ThisColumnLength; ColumnCount ++ )
		{
			if ( isprint( CharData[ CurrentIndex + ColumnCount ] ) )
			{
				*m_OutputStream << CharData[ CurrentIndex + ColumnCount ];
			}
			else
			{
				*m_OutputStream << ".";
			}
		}
		
		*m_OutputStream << endl;
		
		RemainingLength -= ThisColumnLength;
		CurrentIndex += 16;
	}


	*m_OutputStream << endl << endl;		
		
	if ( m_Consumer != 0 )
	{
		return m_Consumer->processMessage( TcId, GlobalVc, Data, Length );
	}
	
	return ASAAC_SUCCESS;
}


void MessageDumper::setOutputConsumer( MessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void MessageDumper::setDumperName( const ASAAC_CharacterSequence& Name )
{
	m_Name = Name;
}

void MessageDumper::setOutputStream( ostream& Output )
{
	m_OutputStream = &Output;
}


