#ifndef VCLISTENER_HH_
#define VCLISTENER_HH_

#include "PcsIncludes.hh"
#include "Interfaces/VcMessageConsumer.hh"


//! listnening object to monitor multiple virtual channels

/*! a VcListener is a facility to monitor multiple virtual
 * channels for incoming data in a single thread. According
 * the respective channels, different VcMessageConsumers can
 * be called to process the incoming data.
 * 
 * A VcListener does not start a self-contained thread. Rather,
 * it is required that there is one thread that at least periodically
 * calls VcListener::listen() to relay the incoming messages.
 */

class VcListener
{
public:
	VcListener();
	virtual ~VcListener();
	
	ASAAC_ReturnStatus addListeningConsumer( ASAAC_PublicId LocalVc, VcMessageConsumer& Consumer );
	/*!< set up VcListener to monitor a given LocalVc, relaying the data to the respective Consumer
	 * 
	 * @param[in] LocalVc	number of process-local LocalVc to be monitored for incoming messages
	 * @param[in] Consumer  object inplementing the VcConsumer interface to process the incoming message
	 * 
	 * @return 				result of operation. If another handler is already registered for LocalVc, or no more handler slots are available, 
	 * 						ERROR is returned. Otherwise, SUCCESS.
	 */
	 
	ASAAC_ReturnStatus removeListeningConsumer( ASAAC_PublicId LocalVc );
	/*!< remove given LocalVc from the list of monitored Virtual Channels
	 * 
	 * @param[in] LocalVc	LocalVc to be removed from monitoring
	 * 
	 * @return 				ERROR if LocalVc is not contained in the list of currently monitored Virtual Channels
	 * 						SUCCESS otherwise.
	 */
	
	ASAAC_TimedReturnStatus listen( const ASAAC_TimeInterval& Timeout );
	/*!< wait for a single incoming message on registered Virtual Channels.
	 * 
	 * @param[in] Timeout	maximum time interval to spend waiting for incoming messages
	 * 
	 * @return				TM_TIMEOUT if no message was received within the specified time interval.
	 * 						TM_ERROR   if an error occured during the waiting or while processing
	 *                                 a message.
	 * 						TM_SUCCESS otherwise.
	 */
	
private:
	
	/*! Storage of registered virtual channel message handlers */
	struct {
		ASAAC_PublicId			LocalVc;	/*!< LocalVc number of registered Virtual Channel */
		VcMessageConsumer*	Consumer;	/*!< object implementing the VcMessageConsumer interface
										 *   to handle incoming messages on said virtual channel
										 */
	} m_ListeningVcInfo[32];
	
	
	unsigned long m_NextFreeSlot;		//!< Next free slot in VcListener::m_ListeningVcInfo
	
	
};

#endif /*VCLISTENER_HH_*/
