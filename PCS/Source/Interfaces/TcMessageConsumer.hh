#ifndef TCMESSAGECONSUMER_HH_
#define TCMESSAGECONSUMER_HH_

#include "PcsHIncludes.hh"

//! Abstract Class (Interface) for objects handling Tc messages 

/*! A TcMessageConsumer handles tc messages as incoming or outgoing from the NII, wherein only
 * the corresponding TransferChannel is known, and the message itself may be containing a Vc header.
 * 
 * Classes using this interface can be used to generate handler (filter) chains to handle individual processing steps
 * in a well-separated manner.
 */


class TcMessageConsumer {
	
public:
	virtual ASAAC_ReturnStatus processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length ) = 0;
	//!< process the message indicated by the parameters.
	/*!< \param[in] TcId		PublicId of TransferConnection that this message relates to
	 *   \param[in] Data		Reference to Data
	 *   \param[in] Length		Size of message
	 * 
	 *   \return				SUCCESS if no problems occurred during the processing of the message.
	 *                          ERROR otherwise.
	 * 
	 * \throw PCSException
	 * 
	 * TcMessageConsumers are allowed to throw a PCSException to relay more detailed information to the calling instance.
	 * 
	 * TcMessageConsumers shall not alter the message in its original location, but rather perform changing operations
	 * on a local copy of the message, so the calling instance can trust for the message itself to be unaltered and
	 * ready to be served to another TcMessageConsumer on return of this function.
	 * 
	 * Further, in order to avoid deadlocks, TcMessageConsumers shall not have blocking behavior without timeout. 
	 * If the message can not be processes at the current time, it shall be buffered in the processing instance 
	 * and control shall be handed back to the calling instance.
	 */
	
	
	virtual ~TcMessageConsumer() { };
	
};

#endif /*TCMESSAGECONSUMER_HH_*/
