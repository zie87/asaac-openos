#include "UnMarshallingFilter.hh"

#include "PcsCIncludes.hh"

#include <string.h>

#include <iostream>

using namespace std;


UnMarshallingFilter::UnMarshallingFilter() : m_OutputConsumer(0), m_Configuration(0)
{
}


UnMarshallingFilter::UnMarshallingFilter( VcMessageConsumer& OutputConsumer, PCSConfiguration& Configuration ) : 
	m_OutputConsumer(&OutputConsumer),
	m_Configuration(&Configuration)
{
}



UnMarshallingFilter::~UnMarshallingFilter()
{
}




ASAAC_ReturnStatus UnMarshallingFilter::processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
		cout << "UnMarshallingFilter::processVcMessage(" << GlobalVc<< "," << Data<< "," << Length << ")" << endl;
#endif

	ASAAC_VcDescription 			 Description;
//	VcToTcMappingDescription MappingDesc;

	if ( m_Configuration->getVcDescription( GlobalVc, Description ) == ASAAC_ERROR )
	{
		throw PcsException( 0, GlobalVc, "No VcDescription found" );
		return ASAAC_ERROR;
	}

	if ( m_OutputConsumer == 0 ) 
	{
		throw PcsException( 0, GlobalVc, "No OutputConsumer set." );
		return ASAAC_ERROR;
	}
	
	if(Description.vc_type == ASAAC_Application_Raw_VC)
	{
#ifdef _DEBUG_       
		cerr << "UnMarshallingFilter::processMessage() Processing Raw VC Message" << endl;
#endif	
		return m_OutputConsumer->processVcMessage(GlobalVc, Data, Length );
	}
		
	// The CDR-Marshalled data can at most be 16/12 = 4/3 times the size of the unmarshalled data
	unsigned long MaxLength = m_Processor.getSize( Description.data_representation_format );
	unsigned long ActualSize;
	
	char Buffer[ MaxLength ];
	
	if ( ! m_Processor.readFromCDR( Data, Length, Buffer, MaxLength, Description.data_representation_format, ActualSize ) )
	{
		throw PcsException( 0, GlobalVc, "Marshalling: Error converting into CDR." );
		return ASAAC_ERROR;
	}
	
	return m_OutputConsumer->processVcMessage(GlobalVc, Buffer, ActualSize );
}


void UnMarshallingFilter::setOutputConsumer( VcMessageConsumer& OutputConsumer )
{
	m_OutputConsumer = &OutputConsumer;
}


void UnMarshallingFilter::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}
