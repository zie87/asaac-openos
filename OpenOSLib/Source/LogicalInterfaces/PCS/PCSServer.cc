#include "PCSServer.hh"
#include "PCSMessages.hh"

#include "OpenOS.hh"

#include "Managers/FileNameGenerator.hh"

using namespace std;


namespace ASAAC
{
	namespace PCS
	{
		
		Server::Server()
		{
			mRequestVc = 0;
			mReplyVc = 1;
			mTimeOut = TimeIntervalInfinity;
			
			for (int i = 0; i < ASAAC_PCS_FunctionId_MAX; i++)
				m_Handler[i] = DummyFunction;
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

		ASAAC_ReturnStatus Server::DummyFunction( ASAAC_PCS_MessageParameter &Parameter )
		{
			return ASAAC_ERROR;
		}

		void Server::registerHandler(ASAAC_PCS_FunctionId functionId, Handler handler)
		{
			if ((functionId >= 0) and (functionId < ASAAC_PCS_FunctionId_MAX))
				m_Handler[ functionId ] = handler;
		}
		
		void Server::unregisterHandler(ASAAC_PCS_FunctionId functionId)
		{
			if ((functionId >= 0) and (functionId < ASAAC_PCS_FunctionId_MAX))
				m_Handler[ functionId ] = DummyFunction;
		}
		
		ASAAC_TimedReturnStatus Server::handleOneRequest()
		{
			ASAAC_TimedReturnStatus status;					
			size_t max_size = sizeof(ASAAC_PCS_Message);
			unsigned long size;
			ASAAC_PCS_Message Message;
			
			status = ASAAC_APOS_receiveMessage( getRequestVc(), &mTimeOut, max_size, &Message, &size );
			
			static ASAAC_CharacterSequence UnknownPCSMsg = CharacterSequence("Unknown PCS Message.").asaac_str();
			static ASAAC_CharacterSequence UncexpectedPCSMsg = CharacterSequence("Unexpected PCS Message (Reply instead of Request).").asaac_str();
			
			if (status != ASAAC_TM_SUCCESS)
				return status;
			
			if ((Message.unique_message_id.function_id < 0) || 
				(Message.unique_message_id.function_id >= ASAAC_PCS_FunctionId_MAX))
			{
				ASAAC_APOS_logMessage( 
					&UnknownPCSMsg, 
					ASAAC_LOG_MESSAGE_TYPE_ERROR );						
				return ASAAC_TM_ERROR;				
			}						
			
			if (Message.unique_message_id.direction == ASAAC_PCS_Reply)
			{
				ASAAC_APOS_logMessage( 
					&UncexpectedPCSMsg, 
					ASAAC_LOG_MESSAGE_TYPE_ERROR );					
				return ASAAC_TM_ERROR;
			}
			
			ASAAC_PCS_FunctionId FunctionId = Message.unique_message_id.function_id;
			ASAAC_PCS_MessageParameter Param = Message.message_parameter;
			
			ASAAC_ReturnStatus handlerStatus;
			
			try
			{
				handlerStatus = m_Handler[FunctionId](Param);
			}
			catch ( ASAAC_Exception &e)
			{
				e.logMessage();
				handlerStatus = ASAAC_ERROR;
			}
			catch (...)
			{
				handlerStatus = ASAAC_ERROR;
			}
			
			if (handlerStatus == ASAAC_SUCCESS)
			{
				Message.message_parameter = Param;
				
				Message.message_parameter._d.direction = ASAAC_PCS_Reply;
				Message.unique_message_id.direction = ASAAC_PCS_Reply;
							
				return ASAAC_APOS_sendMessage( getReplyVc(), &mTimeOut, &Message, max_size );
			}
			
			return ASAAC_TM_ERROR;
		}

	} //namespace OLI

}//namesapce ASAAC
