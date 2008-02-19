#ifndef MESSAGEDUMPER_HH_
#define MESSAGEDUMPER_HH_

#include "Interfaces/MessageConsumer.hh"

#include <iostream>

using namespace std;

//! Class to dump the content of a message to console

/*! The MessageDumper is used to display the contents of a message on the console, or
 *  any other ostream. It shows both a hexadecimal representation, as well as displays
 *  any printable characters of the message.
 */

class MessageDumper : public MessageConsumer {
public:

	MessageDumper();

	void initialize();
	void deinitialize();
	
	virtual ASAAC_ReturnStatus processMessage( ASAAC_PublicId TcId, ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length );
	//!< dump contents of the message to configured output stream and, if applicable, forward message
	/*!< Parameters and Return Value and Conditions are identical to those as defined in the abstract interface
	 * class MessageDumper.
	 */
	
	void setOutputConsumer( MessageConsumer& Consumer );
	//!< set the MessageConsumer to forward messages to after dumping
	
	void setDumperName( const ASAAC_CharacterSequence& Name );
	//!< set the name of the dump to be prepended to any dump
	
	void setOutputStream( ostream& Output );
	//!< change the output stream to dump messages to (default: cout)
	
private:
	MessageConsumer*		m_Consumer;
	ASAAC_CharacterSequence	m_Name;
	
	ostream*				m_OutputStream;
	
};

#endif /*MESSAGEDUMPER_HH_*/
