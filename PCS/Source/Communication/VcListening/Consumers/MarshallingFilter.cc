#include "MarshallingFilter.hh"

#include "PcsCIncludes.hh"

#include <iostream>

using namespace std;


MarshallingFilter::MarshallingFilter() : m_OutputConsumer(0), m_Configuration(0)
{
}


void MarshallingFilter::initialize()
{

}


void MarshallingFilter::deinitialize()
{
	
}


ASAAC_ReturnStatus MarshallingFilter::processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
		cout << "MarshallingFilter::processVcMessage(" << GlobalVc << "," << Data << "," << Length << ")" << endl;
#endif	
	
	ASAAC_VcDescription 			 Description;
//	VcToTcMappingDescription MappingDesc;

	if ( m_Configuration->getVcDescription( GlobalVc, Description ) == ASAAC_ERROR )
	{
#ifdef _DEBUG_       
		cerr << "MarshallingFilter::processVcMessage() No VcDescription found" << endl;
#endif	
		throw PcsException( 0, GlobalVc, "No VcDescription found" );
	}
	
	if ( m_OutputConsumer == 0 ) 
	{
#ifdef _DEBUG_       
		cerr << "MarshallingFilter::processVcMessage() No OutputConsumer set" << endl;
#endif	
		throw PcsException( 0, GlobalVc, "No OutputConsumer set." );
	}

	if(Description.is_typed_message == ASAAC_BOOL_FALSE)
	{
#ifdef _DEBUG_       
		cout << "MarshallingFilter::processVcMessage() Processing untyped message" << endl;
#endif	
		return m_OutputConsumer->processVcMessage( GlobalVc, Data, Length );
	}
		
	// The CDR-Marshalled data can at most be 16/12 = 4/3 times the size of the unmarshalled data
	unsigned long MaxLength = (4 * Length / 3) + 1;
	unsigned long ActualSize;
	
	char Buffer[ MaxLength ];
	
	CharacterSequence DataRepresentationFormat = Description.data_representation_format;
	
	if ( m_Processor.writeToCDR( Data, Length, Buffer, MaxLength, 
			DataRepresentationFormat.asaac_str(), ActualSize ) == false )
	{
		throw PcsException( 0, GlobalVc, "Marshalling: Error converting into CDR." );
		return ASAAC_ERROR;
	}
	
#ifdef _DEBUG_       
	cout << "MarshallingFilter::processVcMessage() Processing typed message" << endl;
#endif	
	return m_OutputConsumer->processVcMessage(GlobalVc, Buffer, ActualSize );
}


void MarshallingFilter::setOutputConsumer( VcMessageConsumer& OutputConsumer )
{
	m_OutputConsumer = &OutputConsumer;
}


void MarshallingFilter::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}
