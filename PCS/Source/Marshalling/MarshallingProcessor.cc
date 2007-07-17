#include "MarshallingProcessor.hh"

#include "FormatString.hh"

#include "PcsCIncludes.hh"

#include <iostream>


MarshallingProcessor::MarshallingProcessor( bool AutoPadding ) : m_AutoPadding(AutoPadding)
{
}

MarshallingProcessor::~MarshallingProcessor()
{
}


unsigned long MarshallingProcessor::getSize( const string& FormatDescription )
{
	return getStructSize( FormatDescription );
}


void MarshallingProcessor::setAutoPaddingState( bool Flag )
{
	m_AutoPadding = Flag;
}


bool MarshallingProcessor::getAutoPaddingState()
{
	return m_AutoPadding;
}


bool MarshallingProcessor::readFromCDR( ASAAC_Address CDRData, unsigned long CDRSize, ASAAC_Address NativeData, unsigned long MaxNativeSize, const string& FormatDescription, unsigned long& ActualSize )
{
	try {
		ActualSize = getSize( FormatDescription );

		if ( ActualSize > MaxNativeSize ) 
			return false;
		
		CDRDataBlock CDR( (char*)CDRData, CDRSize );
	
		return readStruct( CDR, FormatDescription, NativeData, 0 );
	}
	catch ( PcsMemoryOverflowException& e )
	{
		cout << "Memory overflow encountered." << endl;
		return false;
	}
	return true;
}	
	

bool MarshallingProcessor::writeToCDR( ASAAC_Address NativeData, unsigned long NativeSize, ASAAC_Address CDRData, unsigned long MaxCDRSize, const string& FormatDescription, unsigned long& ActualSize )
{
	try {
		if ( getSize( FormatDescription ) > NativeSize ) 
		{
			return false;
		}

		CDRDataBlock CDR( CDRData, MaxCDRSize );
	
		if ( writeStruct( CDR, FormatDescription, NativeData, 0 ) )
		{
			ActualSize = CDR.getCurrentIndex();
			return true;
		}
		else
		{
			return false;
		}
	}
	catch ( PcsMemoryOverflowException& e )
	{
		cout << "Memory overflow encountered." << endl;
		return false;
	}
	return true;
}	




unsigned long MarshallingProcessor::getElementSize( char Identifier, const string& Parameter )
{
	switch ( Identifier ) {
		
		case 'o': return sizeof( char );
		
		case 's': return sizeof( short );
		
		case 'l': return sizeof( long );
		
		case 'L' : return sizeof( long long );
		
		case 'f' : return sizeof( float );
		
		case 'd' : return sizeof( double );
		
		case 'D' : return sizeof( long double );
		
		case 'e' : return sizeof( long );
		
		case 'S' : return getStructSize( Parameter );
		
		case 'U' : return getUnionSize( Parameter );
		
		case 'x' : return 1; // padding byte
		
		case 'Q' : 
		case 'C' : 
		default:
				   throw PcsException( 0, 0, "Sequence and String types not yet supported in marshalling filter." );
				   return 0;
	}
}

unsigned long MarshallingProcessor::getElementAlign( char Identifier, const string& Parameter )
{
	if ( ! m_AutoPadding ) return 1;
	
	switch ( Identifier ) {
		
		case 'o' : return __alignof__( char );
		
		case 's' : return __alignof__( short );
		
		case 'l' : return __alignof__( long );
		
		case 'L' : return __alignof__( long long );
		
		case 'f' : return __alignof__( float );
		
		case 'd' : return __alignof__( double );
		
		case 'D' : return __alignof__( long double );
		
		case 'e' : return __alignof__( long );

		case 'S' : return getStructAlign( Parameter );
		
		case 'U' : return getUnionAlign( Parameter );

		case 'x' : return 1; // padding byte
		
		case 'Q' :
		case 'C' :
		default  :
				   throw PcsException( 0, 0, "Sequence and String types not yet supported in marshalling filter." );
				   return 0;
	}
}


bool MarshallingProcessor::readElement( CDRDataBlock& CDR, char Identifier, const string& Parameter, ASAAC_Address NativeData, unsigned long StartingIndex )
{
	char* NativeCharBuffer = reinterpret_cast<char*>(NativeData);

	switch ( Identifier ) {
		
		case 'o' : NativeCharBuffer[ StartingIndex ] = CDR.getNextChar();
				   return true;
		
		case 's' : *(reinterpret_cast<short*>(NativeCharBuffer + StartingIndex)) = CDR.getNextShort();
				   return true;
		
		case 'l' : *(reinterpret_cast<long*>(NativeCharBuffer + StartingIndex)) = CDR.getNextLong();
				   return true;
		
		case 'L' : *(reinterpret_cast<long long*>(NativeCharBuffer + StartingIndex)) = CDR.getNextLongLong();
				   return true;
		
		case 'f' : *(reinterpret_cast<short*>(NativeCharBuffer + StartingIndex)) = CDR.getNextShort();
				   return true;
		
		case 'd' : *(reinterpret_cast<short*>(NativeCharBuffer + StartingIndex)) = CDR.getNextShort();
				   return true;
		
		case 'D' : *(reinterpret_cast<short*>(NativeCharBuffer + StartingIndex)) = CDR.getNextShort();
				   return true;
		
		
		case 'e' : *(reinterpret_cast<unsigned long*>(NativeCharBuffer + StartingIndex)) = CDR.getNextEnum();
				   return true;
		
		case 'S' : return readStruct( CDR, Parameter, NativeData, StartingIndex );
		
		case 'U' : return readUnion( CDR, Parameter, NativeData, StartingIndex );
		
		case 'x' : NativeCharBuffer[ StartingIndex ] = 0; // zero out padding byte
				   return true;
		
		case 'Q' :
		case 'C' :
		default  :
				   throw PcsException( 0, 0, "Sequence and String types not yet supported in marshalling filter." );
				   return 0;
	}
}


bool MarshallingProcessor::writeElement( CDRDataBlock& CDR, char Identifier, const string& Parameter, ASAAC_Address NativeData, unsigned long StartingIndex )
{
	char* NativeCharBuffer = reinterpret_cast<char*>(NativeData);

	switch ( Identifier ) {
		
		case 'o' : return CDR.setNextChar( NativeCharBuffer[ StartingIndex ] );
		
		case 's' : return CDR.setNextShort( *(reinterpret_cast<short*>(NativeCharBuffer + StartingIndex)) );
		
		case 'l' : return CDR.setNextLong( *(reinterpret_cast<long*>(NativeCharBuffer + StartingIndex)) );
		
		case 'L' : return CDR.setNextLongLong( *(reinterpret_cast<long long*>(NativeCharBuffer + StartingIndex)) );
		
		case 'f' : return false;
		
		case 'd' : return false;
		
		case 'D' : return false;
		
		
		case 'e' : return CDR.setNextEnum( *(reinterpret_cast<unsigned long*>(NativeCharBuffer + StartingIndex)) );
		
		case 'S' : return writeStruct( CDR, Parameter, NativeData, StartingIndex );
		
		case 'U' : return writeUnion( CDR, Parameter, NativeData, StartingIndex );

		case 'x' : return true;
		
		case 'Q' :
		case 'C' :
		default  :
				   throw PcsException( 0, 0, "Sequence and String types not yet supported in marshalling filter." );
				   return 0;
	}
}



unsigned long MarshallingProcessor::getStructSize( const string& DescriptionString )
{
	unsigned long lCurrentAlignment = 0;
	
	FormatString Description( DescriptionString );
	
	char Identifier;
	unsigned short NumberOfElements;
	string Parameters;
	
	while ( Description.findNextElement( Identifier, NumberOfElements, Parameters ) )
	{
		unsigned long ElementSize  = getElementSize ( Identifier, Parameters );
		unsigned long ElementAlign = getElementAlign( Identifier, Parameters );

		
		for ( unsigned short Count = 0; Count < NumberOfElements; Count ++ )
		{
			lCurrentAlignment = alignedOffset( lCurrentAlignment, ElementAlign );
			lCurrentAlignment += ElementSize;
		}
	}
	
	return lCurrentAlignment;
}
			

unsigned long MarshallingProcessor::getStructAlign( const string& DescriptionString )
{
	unsigned long lMaxAlignment = 1;
	
	FormatString Description( DescriptionString );
	
	char Identifier;
	unsigned short NumberOfElements;
	string Parameters;
	
	while ( Description.findNextElement( Identifier, NumberOfElements, Parameters ) )
	{
		unsigned long ElementAlign = getElementAlign( Identifier, Parameters );
	
		if ( ElementAlign > lMaxAlignment )
		{
			lMaxAlignment = ElementAlign;
		}
	}
	
	return lMaxAlignment;
}
			

bool MarshallingProcessor::readStruct( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex )
{
	unsigned long lCurrentIndex = StartingIndex;
	
	FormatString Description( BlockDescription );
	
	char Identifier;
	unsigned short NumberOfElements;
	string Parameters;
	
	while ( Description.findNextElement( Identifier, NumberOfElements, Parameters ) )
	{
		unsigned long ElementSize  = getElementSize ( Identifier, Parameters );
		unsigned long ElementAlign = getElementAlign( Identifier, Parameters );
		
		for ( unsigned short Count = 0; Count < NumberOfElements; Count ++ )
		{
			lCurrentIndex = alignedOffset( lCurrentIndex, ElementAlign );

			if ( ! readElement( CDR, Identifier, Parameters, NativeData, lCurrentIndex ) )
			{
				return false;
			}

			lCurrentIndex += ElementSize;
		}
	}
	
	return true;
}


bool MarshallingProcessor::writeStruct( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex )
{
	unsigned long lCurrentIndex = StartingIndex;

	FormatString Description( BlockDescription );
	
	char Identifier;
	unsigned short NumberOfElements;
	string Parameters;
	
	while ( Description.findNextElement( Identifier, NumberOfElements, Parameters ) )
	{
		unsigned long ElementSize  = getElementSize ( Identifier, Parameters );
		unsigned long ElementAlign = getElementAlign( Identifier, Parameters );
		
		for ( unsigned short Count = 0; Count < NumberOfElements; Count ++ )
		{
			lCurrentIndex = alignedOffset( lCurrentIndex, ElementAlign );

			if ( ! writeElement( CDR, Identifier, Parameters, NativeData, lCurrentIndex ) )
			{
				return false;
			}

			lCurrentIndex += ElementSize;
		}
	}
	
	return true;
}

			
			
unsigned long MarshallingProcessor::getUnionSize( const string& DescriptionString )
{
	unsigned long LargestSize = 0;
	
	string RemainingString = DescriptionString;
	
	while ( RemainingString.length() > 0 )
	{
		long SeparatorPosition = RemainingString.find("|" );

		if ( SeparatorPosition < 0 )
		{
			RemainingString += '.';
			SeparatorPosition = RemainingString.length();
		}
		
		// get string between ||'s
		string SubString = RemainingString.substr(0, SeparatorPosition );
	
		// remove leading '<enum-Value>:' part
		unsigned long ValueSepPosition = SubString.find( ":" );
		SubString.erase( 0, ValueSepPosition + 1 );
		
		
		// compute size INCLUDING the leading enumerator
		unsigned long BranchSize = alignedOffset( sizeof( unsigned long ), getStructAlign( SubString ) ) + 
								   getStructSize( SubString );
		
		if ( BranchSize > LargestSize )
		{
			LargestSize = BranchSize;
		}
		
		RemainingString.erase( 0, SeparatorPosition + 1 );
	}
	
	return LargestSize;
}


unsigned long MarshallingProcessor::getUnionAlign( const string& DescriptionString )
{
	unsigned long LargestAlign = __alignof__(unsigned long);
	
	string RemainingString = DescriptionString + "|";
	
	while ( RemainingString.length() > 0 )
	{
		long SeparatorPosition = RemainingString.find("|" );

		if ( SeparatorPosition < 0 )
		{
			RemainingString += '.';
			SeparatorPosition = RemainingString.length();
		}
		
		string SubString = RemainingString.substr(0, SeparatorPosition );
		
		// remove leading '<enum-Value>:' part
		unsigned long ValueSepPosition = SubString.find( ":" );
		SubString.erase( 0, ValueSepPosition + 1 );
		
		unsigned long BranchAlign = getStructAlign( SubString );
		
		if ( BranchAlign > LargestAlign )
		{
			LargestAlign = BranchAlign;
		}
		
		RemainingString.erase( 0, SeparatorPosition + 1 );
	}
	
	return LargestAlign;
}


bool MarshallingProcessor::readUnion( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex )
{
	char* NativeCharData = reinterpret_cast<char*>(NativeData);
	
	string RemainingString = BlockDescription;
	
	unsigned long BranchIdentifier = CDR.getNextEnum();
	
	*(reinterpret_cast<unsigned long*>(NativeCharData + StartingIndex)) = BranchIdentifier;
	
	
	while ( RemainingString.length() > 0 )
	{
		long SeparatorPosition = RemainingString.find("|" );

		if ( SeparatorPosition < 0 )
		{
			RemainingString += '.';
			SeparatorPosition = RemainingString.length();
		}
		
		// get string between ||'s
		string SubString = RemainingString.substr(0, SeparatorPosition );
		RemainingString.erase( 0, SeparatorPosition + 1 );
	
		// remove leading '<enum-Value>:' part
		unsigned long ValueSepPosition = SubString.find( ":" );
		
		string BranchValueString = SubString.substr( 0, ValueSepPosition );
		
		if ( CharSeq( BranchValueString ).c_ulong() != BranchIdentifier )
		{
			continue;
		}
		
		SubString.erase( 0, ValueSepPosition + 1 );
		
		unsigned long StructIndex = alignedOffset( StartingIndex + sizeof( unsigned long ), getStructAlign( SubString ) );
		
		return readStruct( CDR, SubString, NativeData, StructIndex );
	}
	
	return false;
}


bool MarshallingProcessor::writeUnion( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex )
{
	char* NativeCharData = reinterpret_cast<char*>(NativeData);
	
	string RemainingString = BlockDescription;
	
	unsigned long BranchIdentifier = *(reinterpret_cast<unsigned long*>(NativeCharData + StartingIndex));
	
	if ( ! CDR.setNextEnum( BranchIdentifier ) ) return false;
	
	while ( RemainingString.length() > 0 )
	{
		long SeparatorPosition = RemainingString.find("|" );

		if ( SeparatorPosition < 0 )
		{
			RemainingString += '.';
			SeparatorPosition = RemainingString.length();
		}
		
		// get string between ||'s
		string SubString = RemainingString.substr(0, SeparatorPosition );
		RemainingString.erase( 0, SeparatorPosition + 1 );
	
		// remove leading '<enum-Value>:' part
		unsigned long ValueSepPosition = SubString.find( ":" );
		
		string BranchValueString = SubString.substr( 0, ValueSepPosition );
		
		if ( CharSeq( BranchValueString ).c_ulong() != BranchIdentifier )
		{
			continue;
		}
		
		SubString.erase( 0, ValueSepPosition + 1 );
		
		unsigned long StructIndex = alignedOffset( StartingIndex + sizeof( unsigned long ), getStructAlign( SubString ) );
		
		return writeStruct( CDR, SubString, NativeData, StructIndex );
	}
	
	return false;
}




unsigned long MarshallingProcessor::alignedOffset( unsigned long UnalignedOffset, unsigned long Alignment )
{
	return UnalignedOffset + (( Alignment - ( UnalignedOffset % Alignment ) ) % Alignment );
}

			
