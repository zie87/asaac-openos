#ifndef GLOBALVCSENDER_HH_
#define GLOBALVCSENDER_HH_

#include "Interfaces/VcMessageConsumer.hh"

#include "Configuration/PCSConfiguration.hh"

#include "PcsHIncludes.hh"

//! Generate VcMessage from a message

/*! A GlobalVcSender prepares an incoming Message for processing by means of a
 *  VcMessageConsumer, such as a VcSender for sending the message via a local
 *  VirtualChannel interface.
 * 
 * It performs the translation between GlobalVc Id and local Vc Id, and strips
 * the information about the correlating TransferConnection from the message before
 * relaying it to the configured output consumer.
 */

class GlobalVcSender : public VcMessageConsumer
{
public:
	GlobalVcSender();

	void initialize();
	void deinitialize();
	
	virtual ASAAC_ReturnStatus processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length );
	
	void setOutputConsumer( VcMessageConsumer& Consumer );
	//!< set VcMessageConsumer to relay messages to
	
	void setConfiguration( PCSConfiguration& Configuration );
	//!< set PCSConfigurationManager storing information required for the GlobalVc<->LocalVc lookup
	
private:

	VcMessageConsumer*		m_Consumer;
	PCSConfiguration*		m_Configuration;
	
	
};

#endif /*GLOBALVCSENDER_HH_*/
