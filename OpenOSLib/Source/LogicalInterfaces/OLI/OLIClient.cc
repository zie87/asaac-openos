#include "OLIClient.hh"

#include "OpenOSObject.hh"

#include "Communication/CommunicationManager.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "ProcessManagement/ThreadManager.hh"

#include "Managers/FileManager.hh"

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
				sizeof(ASAAC_OLI_OliMessage),	//msg_length
				16,
				ASAAC_OLI_VC);
				
			m_TwoWayCommunication.setReplyGlobalVc(
				OS_OLI_GLOBAL_VC_REPLY, 		//vc_id
				sizeof(ASAAC_OLI_OliMessage),	//msg_length
				16,
				ASAAC_OLI_VC);

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
				cs << (unsigned long)sizeof(ASAAC_OctetSequence) << ")";
				throw OSException(cs.c_str(), LOCATION);
			}			
			m_Size = size;
		}
		

		unsigned long Client::getSize()
		{
			return m_Size;
		}


		void Client::establishCommunication()
		{
			try
			{
				ProcessManager *PM = ProcessManager::getInstance();
				ThreadManager *TM = ThreadManager::getInstance();
				
				m_TwoWayCommunication.setClientConfiguration( 
					PM->getCurrentProcessId(), 	//process_id
					TM->getCurrentThreadId(),	//thread_id
					getRequestVc(), 			//request_local_vc_id
					getReplyVc());				//reply_local_vc_id
								
				m_TwoWayCommunication.attach();
			}
			catch (ASAAC_Exception &e)
			{
				e.addPath("Establishing communication channel to OLI server failed", LOCATION);
				
				throw;
			}
		}			
		

		void Client::receiveReply(
            unsigned long TransferId, 
            ASAAC_OLI_ReplyFileReadPayload &Reply,
            ASAAC_Time Timeout)
		{
			Address Incoming;
			unsigned long Size;

			try
			{
                CharacterSequence ErrorString;
                
    			establishCommunication();

			    CommunicationManager::getInstance()->receiveBuffer( getReplyVc(), TimeStamp(Timeout).asaac_Interval(), Incoming, Size );
				
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

				CommunicationManager::getInstance()->unlockBuffer( getReplyVc(), Incoming );
			}
			catch ( ASAAC_Exception &e)
			{
				e.addPath("Error receiving an oli message", LOCATION);

				NO_EXCEPTION( CommunicationManager::getInstance()->unlockBuffer( getReplyVc(), Incoming ) );
				
				throw;
			}					
		}		
		

		void Client::requestFile(
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
				
				CommunicationManager::getInstance()->lockBuffer( getRequestVc(), TimeStamp(Timeout).asaac_Interval(), Outgoing, sizeof(ASAAC_OLI_OliMessage) );
					
				ASAAC_OLI_OliMessage* OutgoingMessage = (ASAAC_OLI_OliMessage*)Outgoing;
				
				OutgoingMessage->unique_message_id = ASAAC_OLI_RequestFileRead;
				OutgoingMessage->transfer_id = transferId;
				OutgoingMessage->message_parameter._u.request_read_file.size = size;
				OutgoingMessage->message_parameter._u.request_read_file.offset = offset;
				OutgoingMessage->message_parameter._u.request_read_file.filename = filename;
				
				CommunicationManager::getInstance()->sendBuffer( getRequestVc(), OutgoingMessage, sizeof(ASAAC_OLI_OliMessage) );
			}
			catch ( ASAAC_Exception &e )
			{
				e.addPath("Error requesting file", LOCATION);

				NO_EXCEPTION( CommunicationManager::getInstance()->unlockBuffer( getRequestVc(), Outgoing ) );
                
				throw;
			}
		}
		

		unsigned long Client::requestMliDownload(ASAAC_CfmDescription cfm_description)
		{
			return 0;
		}
		

		void Client::storeCompleteFile(
			ASAAC_CharacterSequence LocalPath, 
			ASAAC_CharacterSequence OliPath)
		{
			TimeStamp Timeout(m_TimeOut);
			ASAAC_TimeInterval TimeoutInterval;
			
			unsigned long transferId;
			unsigned long offset = 0;
			bool ready = false;
			
			ASAAC_OLI_ReplyFileReadPayload Incoming;
			
			ASAAC_PrivateId FileHandle;
			
			try
			{
				FileManager *FM = FileManager::getInstance();
				
				FM->createFile( LocalPath, ASAAC_RWD, 0 );

				const ASAAC_UseOption UseOption = {ASAAC_READWRITE, ASAAC_SHARE};
				FM->openFile( LocalPath, UseOption, FileHandle );
				
				while ( ready == false ) 
				{	
#ifdef DEBUG                    
					static unsigned long counter = 1;
                    cout << ".";
                    if (div(counter, 80).rem == 0)
                        cout << endl; 
                    fflush(stdout);
                    counter++;
#endif            
                    
					if ( TimeStamp::Now() >= Timeout )
						throw TimeoutException(LOCATION);
					
					transferId = rand();
					
					requestFile(transferId, m_Size, offset, OliPath, Timeout.asaac_Time());
		
					receiveReply(transferId, Incoming, Timeout.asaac_Time());
                    					
 					if (Incoming.result == ASAAC_READ_FILE_ACK_FAILURE_NO_FILE)
						throw OSException("OLI server answered 'ASAAC_READ_FILE_ACK_FAILURE_NO_FILE'", LOCATION);	

  					if (Incoming.result == ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS)					
						throw OSException("OLI server answered 'ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS'", LOCATION);	
						
					if (Incoming.size > 0)
					{						
						unsigned long BytesWritten = 0;
						
						TimeoutInterval = Timeout.asaac_Interval();
						FM->writeFile( FileHandle, Incoming.filedata, Incoming.size, BytesWritten, TimeoutInterval );
											    
						if (BytesWritten != Incoming.size)
							throw OSException("Error while writing data to local device", LOCATION);
					} 
					
					if (Incoming.size < m_Size)
						ready = true;
					else offset = offset + Incoming.size;
				}
	
				FM->closeFile(FileHandle);
				
				if (ready == false)
				{
					TimeoutInterval = Timeout.asaac_Interval();
					FM->deleteFile(LocalPath, ASAAC_IMMEDIATELY, TimeoutInterval);
				}
			}
			catch ( ASAAC_Exception &e )
			{
				e.addPath("Error storing complete file", LOCATION);
				
				throw;			
			}

#ifdef DEBUG                    
                cout << endl;
#endif            			
		}
	}
}
