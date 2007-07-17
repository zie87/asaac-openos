#ifndef MARSHALLINGPROCESSOR_HH_
#define MARSHALLINGPROCESSOR_HH_

#include "PcsHIncludes.hh"
#include "CDRDataBlock.hh"

using namespace std;

//! Processor object for the packing/unpacking of data into the CDR format

/*! The MarshallingProcessor is designed to evaluate a format string as specified in the
 *  PCS design document, to translate native data into CDR data and vice versa.
 * 
 * In order to determine the packing, the Marshalling Processor supports two methods, the
 * auto-packing and the explicit packing.
 * 
 * For automatic native data packing, the current embodiment assumes the
 * sending application's compiler to use the same data packing as the PCS's compiler.
 * This would be the case where the OS and the applications are compiled on the same
 * platform, with the same version (and option settings) of the C/C++ compiler.
 * 
 * Using different compilers/architectures/options might require the use of manual padding,
 * wherein all padding bytes are indicated just like normal data types would be, with the
 * only difference that padding bytes do not generate and/or originate from data in the
 * CDR data block.
 * 
*/

class MarshallingProcessor
{
public:
	MarshallingProcessor( bool AutoPadding = true );
	virtual ~MarshallingProcessor();

	void		  setAutoPaddingState( bool Flag );
	//!< set the state of the auto-padding setting
	/*!< \param[in] Flag New Auto-Padding setting */
	
	bool		  getAutoPaddingState();
	//!< get the state of the auto-padding setting
	/*!< \returns state of auto-padding setting */
	
	unsigned long getSize( const string& BlockDescription );
	//!< get the native size of a data structure described by a format string
	/*!< \param[in] BlockDescription Data format description, as specified in the PCS design document.
	 * 
	 * \returns The size of a native data structure containing data with a format indicated at function call.
	 */
	
	bool readFromCDR( ASAAC_Address CDRData, unsigned long CDRSize, ASAAC_Address NativeData, unsigned long MaxNativeSize, const string& FormatDescription, unsigned long& ActualSize );
	//!< translate data from CDR representation to native representation
	/*!< \param[in] CDRData Memory location of CDR data block
	 * \param[in] CDRSize Size of CDR data block, required to avoid overflows by misformatted format strings
	 * \param[in] NativeData Memory location of target native data buffer
	 * \param[in] MaxNativeSize Size of native data block, required to avoid overflows by misformatted format strings
	 * \param[in] FormatDescription Format string according to the specification of the PCS design document
	 * \param[out] ActualSize Actual size of native data field according to the FormatDescription
	 * 
	 * \returns true on successful reading of CDR data, false if an error occurred
	 */
	
	bool writeToCDR( ASAAC_Address NativeData, unsigned long NativeSize, ASAAC_Address CDRData, unsigned long MaxCDRSize, const string& FormatDescription, unsigned long& ActualSize );
	//!< translate data from native representation to CDR representation
	/*!< \param[in] NativeData Memory location of target native data buffer
	 *   \param[in] NativeSize Size of native data block, required to avoid overflows by misformatted format strings
	 *   \param[in] CDRData Memory location of CDR data block
	 *   \param[in] MaxCDRSize Size of CDR data block, required to avoid overflows by misformatted format strings
	 *   \param[in] FormatDescription Format string according to the specification of the PCS design document
	 *   \param[out] ActualSize Actual size of CDR field resulting from the origin native data and the Format Description
	 * 
	 *  \returns true on successful translation to CDR data, false if an error occurred
	 */


private:

	bool          readElement( CDRDataBlock& CDR, char Identifier, const string& Parameter, ASAAC_Address NativeData, unsigned long StartingIndex );
	//!< read a single datatype (primitive or compound) from the CDR block
	
	bool		  readStruct( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex );
	//!< read a structure datatype from the CDR block
	
	bool		  readUnion( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex );
	//!< read a union datatype from the CDR block
	

	bool          writeElement( CDRDataBlock& CDR, char Identifier, const string& Parameter, ASAAC_Address NativeData, unsigned long StartingIndex );
	//!< write a single datatype (primitive or compound) to the CDR block

	bool		  writeStruct( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex );
	//!< write a structure datatype to the CDR block

	bool		  writeUnion( CDRDataBlock& CDR, const string& BlockDescription, ASAAC_Address NativeData, unsigned long StartingIndex );
	//!< write a union datatype to the CDR block


	unsigned long alignedOffset( unsigned long UnalignedOffset, unsigned long Alignment );
	//!< compute offset after padding to required alignment

	unsigned long getStructSize( const string& BlockDescription );
	//!< get native size of a structure
	
	unsigned long getStructAlign( const string& BlockDescription );
	//!< get native alignment of a structure


	unsigned long getUnionSize( const string& BlockDescription );
	//!< get native size of a union
	
	unsigned long getUnionAlign( const string& BlockDescription );
	//!< get native alignment of a union


	unsigned long getElementSize( char Identifier, const string& Parameter );
	//!< get native size of any (primitive or compound) datatype
	
	unsigned long getElementAlign( char Identifier, const string& Parameter );
	//!< get native alignment of any (primitive or compound) datatype
	
	bool	m_AutoPadding;
	//!< auto-padding flag

};

#endif /*MARSHALLINGPROCESSOR_HH_*/
