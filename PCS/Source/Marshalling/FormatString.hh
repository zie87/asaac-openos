#ifndef FORMATSTRING_HH_
#define FORMATSTRING_HH_


//! Class for obtaining format data from a string

/*! This class encapsulates the finding of format string elements from a given string,
 * a central part of the calculation of sizes and alignments and of reading and writing
 * processing of data contained in the described structures.
 */

#include "PcsHIncludes.hh"

class FormatString
{
public:
	FormatString();
	FormatString( const ASAAC_CharacterSequence& String );
	
	void setString( const ASAAC_CharacterSequence& String );
	
	void rewind();
	bool findNextElement( char& Identifier, unsigned short& Number, ASAAC_CharacterSequence& Parameters );
	//!< get the next foremost element from a format string
	/*!< 
	 * \param[out] Identifier one-character identifier of the data type described by the
	 * 							next element of the format string
	 * 
	 * \param[out] Number	  number of identical elements denoted in format string
	 * \param[out] Parameters child format string contained in braces following the
	 * 							identifier of a compound data type. If DataType is
	 * 							not a compound type, Parameters is not changed.
	 * 
	 * \returns					true, if an element could be found. false otherwise.
	 */
	
	virtual ~FormatString();
	
private:
	unsigned short findMatchingBracePosition( unsigned short CurrentPosition );
	/*!< find the position of the closing brace matching the opening brace at the given
	 * location of the format string. Required for recursively evaluating the size and
	 * alignment of structures, unions and other compound types.
	 */

	ASAAC_CharacterSequence m_String;
	unsigned short 			m_CurrentPosition;
};

#endif /*FORMATSTRING_HH_*/
