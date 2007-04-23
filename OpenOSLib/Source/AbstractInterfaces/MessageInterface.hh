#ifndef MESSAGEINTERFACE_HH_
#define MESSAGEINTERFACE_HH_

#include "OpenOSIncludes.hh"

//! abstract interface for the functionality of sending and receiving messages

class MessageInterface
{
	public:

    //! Copy a message into a buffer and send on the interface without waiting
    virtual ASAAC_ResourceReturnStatus sendMessageNonblocking( ASAAC_Address BufferReference, unsigned long Size ) = 0;
    
    //! read the next available buffer associated with the interface without waiting
    virtual ASAAC_ResourceReturnStatus receiveMessageNonblocking( ASAAC_Address BufferReference, 
    												unsigned long MaxSize, 
    												unsigned long& ActualSize ) = 0;

	//! copy a message into a buffer and send
    virtual ASAAC_TimedReturnStatus sendMessage( ASAAC_Address BufferReference, 
    							   unsigned long Size, 
    							   const ASAAC_Time& Timeout ) = 0;

	//! receive a message 
    virtual ASAAC_TimedReturnStatus receiveMessage( ASAAC_Address BufferReference, 
    								  unsigned long MaxSize, 
    								  unsigned long& ActualSize, 
    								  const ASAAC_Time& Timeout ) = 0;
    					     	  
  	virtual ~MessageInterface() { };
};




#endif /*MESSAGEINTERFACE_HH_*/
