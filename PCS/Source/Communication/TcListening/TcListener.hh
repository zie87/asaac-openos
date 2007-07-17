#ifndef TCLISTENER_HH_
#define TCLISTENER_HH_

//! listnening object to monitor multiple transfer connections on a network

/*! a VcListener is a facility to monitor multiple transfer connections
 * channels for incoming data in a single thread. According
 * the respective connections, different TcMessageConsumers can
 * be called to process the incoming data.
 * 
 * A TcListener does not start a self-contained thread. Rather,
 * it is required that there is one thread that at least periodically
 * calls TcListener::listen() to relay the incoming messages.
 */
#include "Interfaces/TcMessageConsumer.hh"
#include "Interfaces/MessageConsumer.hh"
#include "Managers/TimeManager.hh"

#include "Configuration/PCSConfiguration.hh"

#include "PcsHIncludes.hh"

class TcListener
{
public:
    TcListener();

    void initialize();
	void deinitialize();
    
    ASAAC_ReturnStatus addListeningConsumer(TcMessageConsumer& Consumer );
    /*!< set up VcListener to monitor a given LocalVc, relaying the data to the respective Consumer
     * 
     * @param[in] Consumer  object inplementing the TcConsumer interface to process the incoming message
     * 
     * @return              result of operation. If another handler is already registered for LocalVc, or no more handler slots are available, 
     *                      ERROR is returned. Otherwise, SUCCESS.
     */

    ASAAC_TimedReturnStatus  listen(const ASAAC_NetworkDescriptor& network_id, const ASAAC_TimeInterval& Timeout );
    /*!< wait for a single incoming message on registered network.
     * 
     * @param[in] Timeout   maximum time interval to spend waiting for incoming messages
     * 
     * @return              TM_TIMEOUT if no message was received within the specified time interval.
     *                      TM_ERROR   if an error occured during the waiting or while processing
     *                                 a message.
     *                      TM_SUCCESS otherwise.
     */
    
private:
    TcMessageConsumer*   m_Consumer;
};

#endif /*TCLISTENER_HH_*/
