#ifndef MESSAGEINTERFACE_HH_
#define MESSAGEINTERFACE_HH_

#include "OpenOSIncludes.hh"

//! abstract longerface for the functionality of sending and receiving messages

class MessageInterface
{
	public:

    //! Copy a message longo a buffer and send on the longerface without waiting
    virtual void sendMessageNonblocking( ASAAC_Address BufferReference, unsigned long Size ) = 0;
    
    //! read the next available buffer associated with the longerface without waiting
    virtual void receiveMessageNonblocking( ASAAC_Address BufferReference, 
    												unsigned long MaxSize, 
    												unsigned long& ActualSize ) = 0;

	//! copy a message longo a buffer and send
    virtual void sendMessage( ASAAC_Address BufferReference, 
    							   unsigned long Size, 
    							   const ASAAC_Time& Timeout ) = 0;

	//! receive a message 
    virtual void receiveMessage( ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout ) = 0;
    					     	  
  	virtual ~MessageInterface() { };
};




#endif /*MESSAGEINTERFACE_HH_*/
