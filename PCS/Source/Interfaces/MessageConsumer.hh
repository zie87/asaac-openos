#ifndef MESSAGECONSUMER_HH_
#define MESSAGECONSUMER_HH_

#include "PcsIncludes.hh"

//! Abstract Class (Interface) for objects handling messages 

/*! A MessageConsumer handles messages with a known source and target (GlobalVc and TransferConnection, 
 * wherein the direction (FROM Vc or TO Vc) is not explicitely indicated.
 * 
 * Classes using this interface can be used to generate handler (filter) chains to handle individual processing steps
 * in a well-separated manner.
 */

class MessageConsumer {
	
public:
	virtual ASAAC_ReturnStatus processMessage( ASAAC_PublicId TcId, ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length ) = 0;
	//!< process the message indicated by the parameters.
	/*!< \param[in] TcId		PublicId of TransferConnection that this message relates to
	 *   \param[in] GlobalVc	PublicId of Global VirtualChannel that this message relates to
	 *   \param[in] Data		Reference to Data
	 *   \param[in] Length		Size of message
	 * 
	 *   \return				SUCCESS if no problems occurred during the processing of the message.
	 *                          ERROR otherwise.
	 * 
	 * 
	 * \throw PCSException
	 * 
	 * MessageConsumers are allowed to throw a PCSException to relay more detailed information to the calling instance.
	 * 
	 * MessageConsumers shall not alter the message in its original location, but rather perform changing operations
	 * on a local copy of the message, so the calling instance can trust for the message itself to be unaltered and
	 * ready to be served to another MessageConsumer on return of this function.
	 * 
	 * Further, in order to avoid deadlocks, MessageConsumers shall not have blocking behavior without timeout. 
	 * If the message can not be processes at the current time, it shall be buffered in the processing instance 
	 * and control shall be handed back to the calling instance.
	 * 
	 */
	
	virtual ~MessageConsumer() { };
	
};

#endif /*MESSAGECONSUMER_HH_*/
