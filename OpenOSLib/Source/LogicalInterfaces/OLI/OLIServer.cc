#include "OLIServer.hh"

#include "OpenOSObject.hh"

#include "Managers/FileNameGenerator.hh"

using namespace std;


namespace ASAAC
{
	namespace OLI
	{
		
		Server::Server()
		{
			mRequestVc = OS_OLI_SERVER_VC_REQUEST;
			mReplyVc = OS_OLI_SERVER_VC_REPLY;
			mTimeOut = TimeIntervalInfinity;
		}
		
		Server::~Server()
		{
		}
		
		void Server::setRequestVc(ASAAC_PublicId RequestVc)
		{
			mRequestVc = RequestVc;
		}

		ASAAC_PublicId Server::getRequestVc()
		{
			return mRequestVc;
		}
		
		void Server::setReplyVc(ASAAC_PublicId ReplyVc)
		{
			mReplyVc = ReplyVc;
		}
				
		ASAAC_PublicId Server::getReplyVc()
		{
			return mReplyVc;
		}		

		void Server::setTimeOut(ASAAC_TimeInterval TimeOut)
		{
			mTimeOut = TimeOut;
		}
		
		ASAAC_TimeInterval Server::getTimeOut()
		{
			return mTimeOut;
		}
		
		ASAAC_ReturnStatus Server::handleRequestFileRead(
			ASAAC_OLI_OliMessage* IncomingMessage, 
			ASAAC_OLI_OliMessage* OutgoingMessage)
		{
			ASAAC_PrivateId FileHandle = 0;

			try {
				OutgoingMessage->transfer_id = IncomingMessage->transfer_id;
				OutgoingMessage->unique_message_id = ASAAC_OLI_ReplyFileRead;
				
				unsigned long Size = IncomingMessage->message_parameter._u.request_read_file.size;
				
#ifdef DEBUG_OLI
				cout << "filename: " << CharSeq(IncomingMessage->message_parameter._u.request_read_file.filename).c_str() << endl;
				cout << "offset: " << IncomingMessage->message_parameter._u.request_read_file.offset << endl;
				cout << "size: " << IncomingMessage->message_parameter._u.request_read_file.size << endl;
#endif

				ASAAC_CharacterSequence FileName = FileNameGenerator::getAsaacPath(IncomingMessage->message_parameter._u.request_read_file.filename);
				ASAAC_UseOption UseOption = {ASAAC_READ, ASAAC_SHARE};
				
				if ( ASAAC_APOS_openFile( &FileName, &UseOption, &FileHandle) == ASAAC_ERROR )
				{
					FileHandle = 0;
					throw ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS;
				}
				
				unsigned long Position = 0;
				ASAAC_APOS_seekFile( FileHandle, ASAAC_START_OF_FILE, IncomingMessage->message_parameter._u.request_read_file.offset, &Position );
				
				if (Size > sizeof(ASAAC_OctetSequence))
					throw ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS;
				
				long ReadBytes = 0;
				ASAAC_APOS_readFile( FileHandle, (ASAAC_Address*)&(OutgoingMessage->message_parameter._u.reply_read_file.filedata), Size, &ReadBytes, &TimeIntervalInstant );

				OutgoingMessage->message_parameter._u.reply_read_file.size = ReadBytes;
				
				//OutgoingMessage->message_parameter._d = ASAAC_OLI_ReplyFileRead;
				
				throw ASAAC_READ_FILE_ACK_OK;
			}
			catch( ASAAC_ReadFileResult Result )
			{
				if ( FileHandle > 0 ) 
					ASAAC_APOS_closeFile( FileHandle );
				
				OutgoingMessage->message_parameter._u.reply_read_file.result = Result;
				
				if (Result == ASAAC_READ_FILE_ACK_OK)
					return ASAAC_SUCCESS;
			}
			return ASAAC_ERROR;			
		}
		
    	ASAAC_ReturnStatus Server::handleRequestMliDownload(
			ASAAC_OLI_OliMessage* IncomingMessage, 
			ASAAC_OLI_OliMessage* OutgoingMessage)
    	{
    		return ASAAC_ERROR;
    	}
    	
		ASAAC_ReturnStatus Server::handleRequest()
		{
            try
            {
    			Address Incoming;
    			Address Outgoing;
    		
    			unsigned long Size;
    		
    		    static ASAAC_CharacterSequence ErrorAccessingVC = CharacterSequence("Error accessing VC.").asaac_str();
    			ASAAC_TimedReturnStatus RequestResult = ASAAC_APOS_receiveBuffer( mRequestVc, &mTimeOut, &Incoming, &Size );
    			if (( RequestResult != ASAAC_TM_SUCCESS ) ||
    				( ASAAC_APOS_lockBuffer( mReplyVc, &TimeIntervalInstant, &Outgoing, sizeof(ASAAC_OLI_OliMessage) ) != ASAAC_TM_SUCCESS ))
    			{
    				ASAAC_APOS_logMessage( 
    					&ErrorAccessingVC, 
    					ASAAC_LOG_MESSAGE_TYPE_ERROR );
    				
                    if (RequestResult == ASAAC_TM_ERROR)
                        throw OSException("Error receiving a buffer", LOCATION);

                    if (RequestResult == ASAAC_TM_TIMEOUT)
                        throw TimeoutException("Timeout receiving a buffer", LOCATION);
    			}
    			
    			ASAAC_OLI_OliMessage* IncomingMessage = (ASAAC_OLI_OliMessage*)Incoming;
    			ASAAC_OLI_OliMessage* OutgoingMessage = (ASAAC_OLI_OliMessage*)Outgoing;
    
    			ASAAC_OLI_OliMessageId OliMessageId = (ASAAC_OLI_OliMessageId)IncomingMessage->unique_message_id; 
    						
    #ifdef DEBUG_OLI
    			cout << endl << "### Incoming OLI Message (Server)" << endl;
    			cout << "transfer_id: " << IncomingMessage->transfer_id << endl; 
    			cout << "unique_message_id: " << OliMessageId << endl; 
    #endif
    
    			static ASAAC_CharacterSequence UnexpectedOLIMsg = CharacterSequence("Unexpected OLI Message (Reply instead of Request).").asaac_str();
    			static ASAAC_CharacterSequence UnknownOLIMsg = CharacterSequence("Unknown OLI Message.").asaac_str();
    			
    			switch ( OliMessageId )
    			{
    				case ASAAC_OLI_RequestFileRead:    
    					handleRequestFileRead(IncomingMessage, OutgoingMessage);    
    					break;
        			case ASAAC_OLI_RequestMliDownload: 
        				handleRequestMliDownload(IncomingMessage, OutgoingMessage); 
        				break;
    			
        			case ASAAC_OLI_ReplyFileRead:      
        			case ASAAC_OLI_ReplyMliDownload:   
    					ASAAC_APOS_logMessage( &UnexpectedOLIMsg, ASAAC_LOG_MESSAGE_TYPE_ERROR );
    					break;
    				default:
    					ASAAC_APOS_logMessage( &UnknownOLIMsg, ASAAC_LOG_MESSAGE_TYPE_ERROR );
    					break;
    			}			
    
#ifdef DEBUG_OLI
       			cout << "### End of OLI Message: " << IncomingMessage->unique_message_id << endl << endl; 
#endif
    			
    			ASAAC_APOS_unlockBuffer( mRequestVc, Incoming );
                
    			if (ASAAC_APOS_sendBuffer( mReplyVc, OutgoingMessage, sizeof(ASAAC_OLI_OliMessage) ) != ASAAC_SUCCESS)
                    throw OSException("Error sending the buffer back", LOCATION);
            }
            catch (ASAAC_Exception &e)
            {
                e.addPath("OLIServer::Error handling a message", LOCATION);
                e.raiseError();
                
                return ASAAC_ERROR;   
            }
            
            return ASAAC_SUCCESS;
		}

	} //namespace OLI

}//namesapce ASAAC
