#ifndef VCMESSAGECONSUMER_HH_
#define VCMESSAGECONSUMER_HH_

#include "PcsHIncludes.hh"


//! Abstract Class (Interface) for objects handling Vc messages 

/*! A MessageConsumer handles messages coming from or intended to go to a Local VirtualChannel interface
 * via the corresponding listener/sender. It does not (no longer) carry information about the corresponding
 * GlobalVc or LocalVc, but rather is indicating its sender or receiver by the respective LocalVc PublicId.
 * 
 * Classes using this interface can be used to generate handler (filter) chains to handle individual processing steps
 * in a well-separated manner.
 */

class VcMessageConsumer {
	
public:
	virtual ASAAC_ReturnStatus processVcMessage( ASAAC_PublicId LocalVc, ASAAC_Address Data, unsigned long Length ) = 0;
	//!< process the message indicated by the parameters.
	/*!< \param[in] LocalVc		PublicId of LocalVc port that this message is meant for or is coming from
	 *   \param[in] Data		Reference to Data
	 *   \param[in] Length		Size of message
	 * 
	 *   \return				SUCCESS if no problems occurred during the processing of the message.
	 *                          ERROR otherwise.
	 * 
	 * \throw PCSException
	 * 
	 * VcMessageConsumers are allowed to throw a PCSException to relay more detailed information to the calling instance.
	 * 
	 * VcMessageConsumers shall not alter the message in its original location, but rather perform changing operations
	 * on a local copy of the message, so the calling instance can trust for the message itself to be unaltered and
	 * ready to be served to another VcMessageConsumer on return of this function.
	 * 
	 * Further, in order to avoid deadlocks, VcMessageConsumers shall not have blocking behavior without timeout. 
	 * If the message can not be processes at the current time, it shall be buffered in the processing instance 
	 * and control shall be handed back to the calling instance.
	 */
	
	
	virtual ~VcMessageConsumer() { };
	
};

#endif /*MESSAGECONSUMER_HH_*/
