#include "OLIClient.hh"

#include "OpenOSObject.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"

namespace ASAAC
{

	namespace OLI
	{
	
		Client::Client()
		{
			m_RequestVc = OS_OLI_CLIENT_VC_REQUEST;
			m_ReplyVc = OS_OLI_CLIENT_VC_REPLY;
			
			m_TimeOut = OS_COMPLEX_COMMAND_TIMEOUT;
			m_Size = sizeof(ASAAC_OctetSequence);
			
			m_TwoWayCommunication.setServerConfiguration(
				OS_PROCESSID_OLI, 				//process_id
				1, 								//thread_id
				OS_OLI_SERVER_VC_REPLY, 		//request_local_vc_id
				OS_OLI_SERVER_VC_REQUEST);		//reply_local_vc_id
							 
			m_TwoWayCommunication.setRequestGlobalVc(
				OS_OLI_GLOBAL_VC_REQUEST, 		//vc_id
				sizeof(ASAAC_OLI_OliMessage));	//msg_length
				
			m_TwoWayCommunication.setReplyGlobalVc(
				OS_OLI_GLOBAL_VC_REPLY, 		//vc_id
				sizeof(ASAAC_OLI_OliMessage));	//msg_length			
		}
		

		Client::~Client()
		{
		}
		

		void Client::setRequestVc(ASAAC_PublicId RequestVc)
		{
			m_RequestVc = RequestVc;
		}


		ASAAC_PublicId Client::getRequestVc()
		{
			return m_RequestVc;
		}
		

		void Client::setReplyVc(ASAAC_PublicId ReplyVc)
		{
			m_ReplyVc = ReplyVc;
		}
				

		ASAAC_PublicId Client::getReplyVc()
		{
			return m_ReplyVc;
		}		


		void Client::setTimeOut(ASAAC_TimeInterval TimeOut)
		{
			m_TimeOut = TimeOut;
		}
		

		ASAAC_TimeInterval Client::getTimeOut()
		{
			return m_TimeOut;
		}


		void Client::setSize(unsigned long size)
		{
			if (size > sizeof(ASAAC_OctetSequence))
			{
				CharacterSequence cs;
				cs << "OLI-MessageSize(" << size;
				cs << ") was too high (maximum is sizeof(ASAAC_OctetSequence) = ";
				cs << sizeof(ASAAC_OctetSequence) << ")";
				throw OSException(cs.c_str(), LOCATION);
			}			
			m_Size = size;
		}
		

		unsigned long Client::getSize()
		{
			return m_Size;
		}


		ASAAC_ReturnStatus Client::establishCommunication()
		{
			try
			{
				ASAAC_PublicId currentProcessId = OS_UNUSED_ID;
				ASAAC_PublicId currentThreadId = OS_UNUSED_ID;
				
				ProcessManager *PM = ProcessManager::getInstance();
				ThreadManager *TM = ThreadManager::getInstance();
			
				Process *P = PM->getCurrentProcess();
			
				currentProcessId = P->getId();

				Thread *T = TM->getCurrentThread(false);
				if (T != NULL)
					currentThreadId = T->getId();
				
				m_TwoWayCommunication.setClientConfiguration( 
					currentProcessId, 	//process_id
					currentThreadId,	//thread_id
					getRequestVc(), 	//request_local_vc_id
					getReplyVc());		//reply_local_vc_id
								
				m_TwoWayCommunication.attach();
			}
			catch (ASAAC_Exception &e)
			{
				e.addPath("Establishing communication channel to OLI server failed", LOCATION);
				throw;
			}
			
			return ASAAC_SUCCESS;
		}			
		

		ASAAC_TimedReturnStatus Client::receiveReply(
            unsigned long TransferId, 
            ASAAC_OLI_ReplyFileReadPayload &Reply,
            ASAAC_Time Timeout)
		{
			establishCommunication();

			Address Incoming;
			unsigned long Size;

			
			try
			{
                CharacterSequence ErrorString;
                
                ASAAC_TimeInterval TimeoutInterval = TimeStamp(Timeout).asaac_Interval();
                
			    ASAAC_TimedReturnStatus ReceiveStatus = ASAAC_APOS_receiveBuffer( m_ReplyVc, &m_TimeOut, &Incoming, &Size );
				
				if (ReceiveStatus == ASAAC_TM_ERROR)
					throw OSException("Error receiving a buffer", LOCATION);
				 
				if (ReceiveStatus == ASAAC_TM_TIMEOUT)
					throw TimeoutException("Timeout receiving a buffer", LOCATION);
				 
				ASAAC_OLI_OliMessage* IncomingMessage = (ASAAC_OLI_OliMessage*)Incoming;

#ifdef DEBUG_OLI
				cout << endl << "### Incoming OLI Message (Client)" << endl;
				cout << "transfer_id: " << IncomingMessage->transfer_id << endl; 
				cout << "unique_message_id: " << IncomingMessage->unique_message_id << endl; 
				cout << "size: " << IncomingMessage->message_parameter._u.reply_read_file.size << endl;
				cout << "checksum: " << IncomingMessage->message_parameter._u.reply_read_file.checksum << endl;
				cout << "result: " << IncomingMessage->message_parameter._u.reply_read_file.result << endl;
				//cout << "filedata: " << IncomingMessage->message_parameter._u.reply_read_file.filedata << endl;
				cout << "### End of OLI Message: " << IncomingMessage->unique_message_id << endl << endl; 
#endif
				
				if (IncomingMessage->unique_message_id != ASAAC_OLI_ReplyFileRead)
					throw OSException("unique_message_id is not equal to ASAAC_OLI_ReplyFileRead", LOCATION);
				     
				if (IncomingMessage->transfer_id != TransferId)
					throw OSException( (ErrorString << "TransferId is not correct. Expected:" << TransferId << " Received:" << IncomingMessage->transfer_id).c_str(), LOCATION);
					
				Reply = IncomingMessage->message_parameter._u.reply_read_file;

                ASAAC_APOS_unlockBuffer( m_ReplyVc, Incoming );
			}
			catch ( ASAAC_Exception &e)
			{
				e.addPath("Error receiving an oli message", LOCATION);
				e.raiseError();
				ASAAC_APOS_unlockBuffer( m_ReplyVc, Incoming );
				
				return e.isTimeout() ? ASAAC_TM_TIMEOUT : ASAAC_TM_ERROR;
			}					
			
			return ASAAC_TM_SUCCESS;
		}		
		

		ASAAC_TimedReturnStatus Client::requestFile(
				unsigned long transferId,
				unsigned long size,
				unsigned long offset,
				ASAAC_CharacterSequence filename,
                ASAAC_Time Timeout)
		{
            Address Outgoing;
            
			try
			{
				establishCommunication();
				
				if (ASAAC_APOS_lockBuffer( m_RequestVc, &TimeIntervalInstant, &Outgoing, sizeof(ASAAC_OLI_OliMessage) ) != ASAAC_TM_SUCCESS )
					throw OSException("Error locking a buffer", LOCATION);
					
				ASAAC_OLI_OliMessage* OutgoingMessage = (ASAAC_OLI_OliMessage*)Outgoing;
				
				OutgoingMessage->unique_message_id = ASAAC_OLI_RequestFileRead;
				OutgoingMessage->transfer_id = transferId;
				OutgoingMessage->message_parameter._u.request_read_file.size = size;
				OutgoingMessage->message_parameter._u.request_read_file.offset = offset;
				OutgoingMessage->message_parameter._u.request_read_file.filename = filename;
				
				if (ASAAC_APOS_sendBuffer( getRequestVc(), OutgoingMessage, sizeof(ASAAC_OLI_OliMessage) ) == ASAAC_ERROR)
					throw OSException("Error sending a buffer", LOCATION);
			}
			catch ( ASAAC_Exception &e )
			{
				e.addPath("Error requesting file", LOCATION);
				ASAAC_APOS_unlockBuffer( m_RequestVc, Outgoing );
                
				throw;
			}
			
			return ASAAC_TM_SUCCESS;
		}
		

		unsigned long Client::requestMliDownload(ASAAC_CfmDescription cfm_description)
		{
			return 0;
		}
		

		ASAAC_TimedReturnStatus Client::storeCompleteFile(
			ASAAC_CharacterSequence LocalPath, 
			ASAAC_CharacterSequence OliPath)
		{
			TimeStamp Timeout(m_TimeOut);
			
			unsigned long transferId;
			unsigned long offset = 0;
			bool ready = false;
			
			ASAAC_OLI_ReplyFileReadPayload Incoming;
			
			try
			{
				oal_remove( LocalPath.data );
				long File = oal_creat( CharSeq(LocalPath).c_str(), O_WRONLY | O_CREAT | O_TRUNC | S_IRWXU | S_IRWXG | S_IRWXO );
				
				if (File <= 0) 
					throw OSException("Error creating local file", LOCATION);
				
				while ( ready == false ) 
				{	
#ifdef DEBUG                    
                    cout << "."; fflush(stdout);
#endif            
                    
					if ( TimeStamp::Now() >= Timeout )
						throw TimeoutException(LOCATION);
					
					transferId = rand();
					
					if (requestFile(transferId, m_Size, offset, OliPath, Timeout.asaac_Time()) != ASAAC_TM_SUCCESS)
						throw OSException("Error requesting file", LOCATION);
		
					if (receiveReply(transferId, Incoming, Timeout.asaac_Time()) == ASAAC_TM_ERROR)
                    {
                        ASAAC_TimedReturnStatus ReceiveResult;
                        do 
                        {
                            ReceiveResult = receiveReply(transferId, Incoming, TimeStamp::Instant().asaac_Time());
                        }
                        while ( ReceiveResult == ASAAC_TM_ERROR);
                        
                        if (ReceiveResult == ASAAC_TM_TIMEOUT)
                            continue;
                    }
                    					
 					if (Incoming.result == ASAAC_READ_FILE_ACK_FAILURE_NO_FILE)
						throw OSException("OLI server answered 'ASAAC_READ_FILE_ACK_FAILURE_NO_FILE'", LOCATION);	

  					if (Incoming.result == ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS)					
						throw OSException("OLI server answered 'ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS'", LOCATION);	
						
					if (Incoming.size > 0)
					{						
						unsigned long BytesWritten = oal_write( File, 
											    Incoming.filedata,
											    Incoming.size );
											    
						if (BytesWritten != Incoming.size)
							throw OSException("Error while writing data to local device", LOCATION);
					} 
					
					if (Incoming.size < m_Size)
						ready = true;
					else offset = offset + Incoming.size;
				}
	
				close(File);
				
				if (ready == false)
				{
					//TODO: delete created file 
				}
			}
			catch ( ASAAC_Exception &e )
			{
#ifdef DEBUG                    
                cout << endl;
#endif            
				e.addPath("Error storing complete file", LOCATION);
				
				throw;			
			}
			catch ( ... )
			{
#ifdef DEBUG                    
                cout << endl;
#endif            
				throw FatalException("Unknown exception while executing OLI protocol", LOCATION);
			}

#ifdef DEBUG                    
                cout << endl;
#endif            			
			return ASAAC_TM_SUCCESS;
		}
	}
}
