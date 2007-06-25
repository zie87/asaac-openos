#ifndef MESSAGEQUEUE_HH_
#define MESSAGEQUEUE_HH_

#include "OpenOSIncludes.hh"

#include "AbstractInterfaces/MessageInterface.hh"

using namespace std;

enum QueueDirection {
	
	CLIENTS_SEND,
	CLIENTS_RECEIVE
	
};


//! MessageInterface - class for communication via POSIX message queues
/*! The MessageQueue provides all the functionality required to set up uni-directional
 *  POSIX message queues (optional module MSG, IEEE 1003.1, 2004 Edition) 
 *  that can be used for longer-process communication without the
 *  need to resort to virtual channels. A big advantage of uni-directional message queues is
 *  that the message queues can provide a communication of multiple senders and one receiver, as
 *  e.g. is required for the transmission of log and error messages to the GSM.
 */

class MessageQueue : public MessageInterface
{
public:
	MessageQueue();
	virtual ~MessageQueue();
	
	//! create a POSIX message queue for communication
	void initialize( const bool IsMaster, const ASAAC_CharacterSequence Name, const QueueDirection Direction, const unsigned long QueueSize = 0, const unsigned long MessageSize = 0 );
	/*!< this call creates a POSIX message queue and sets up its parameters for communication
	 *   as indicated by the parameters. This operation in many environments requires special
	 *   (root) privileges.
	 * 
	 *   \param[in] Name      Name of the message queue object to be created
	 *   \param[in] Direction Communications direction of the message queue. Indicates what communications
	 *                        direction may be used by clients connecting with open().
	 *   \param[in] QueueSize Maximum number of messages stored in the queue
	 *   \param[in] MessageSize Maximum length of an individual message in the queue
	 * 
	 *   \returns ASAAC_SUCCESS on successful creation of the message queue. ASAAC_ERROR if an error occurred.
	 */
	
	
	//! close/disconnect from a POSIX message queue
	void deinitialize();
	/*!< closes the connection to the POSIX message queue handled by this object instance.
	 *   If the current instance was the creator of the message queue, the message queue is removed.
	 *   However, due to the specifications of the POSIX environment, the actual OS resources of the queue
	 *   will only be freed once ALL connected instances have closed their connection to the object.
	 */
	
	
	virtual ASAAC_ResourceReturnStatus sendMessageNonblocking( ASAAC_Address BufferReference, unsigned long Size );
	
    virtual ASAAC_ResourceReturnStatus receiveMessageNonblocking( ASAAC_Address BufferReference, 
    												unsigned long MaxSize, 
    												unsigned long& ActualSize );

    virtual ASAAC_TimedReturnStatus sendMessage( ASAAC_Address BufferReference, 
    							   unsigned long Size, 
    							   const ASAAC_Time& Timeout );

    virtual ASAAC_TimedReturnStatus receiveMessage( const ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout );

	
private:
	
	bool					m_IsInitialized;
	ASAAC_PrivateId    		m_QueueHandle;
	
	bool					m_IsMaster;
	
	ASAAC_CharacterSequence	m_Name;
	
};

#endif /*MESSAGEQUEUE_HH_*/
