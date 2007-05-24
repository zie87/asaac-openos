#ifndef ASAAC_PCS_SERVER_HH_
#define ASAAC_PCS_SERVER_HH_

#include "OpenOSIncludes.hh"
#include "PCSMessages.hh"

namespace ASAAC
{
	namespace PCS
	{
		typedef ASAAC_ReturnStatus(*Handler)( ASAAC_PCS_MessageParameter &Parameter );
		
		class Server
		{
		private:
			ASAAC_PublicId mRequestVc;
			ASAAC_PublicId mReplyVc;
			ASAAC_TimeInterval mTimeOut;
			
			static ASAAC_ReturnStatus DummyFunction( ASAAC_PCS_MessageParameter &Parameter );
						
			Handler	m_Handler[ ASAAC_PCS_FunctionId_MAX ];
			
		public:
			Server();
			virtual ~Server();
			
			void registerHandler(ASAAC_PCS_FunctionId functionId, Handler handler);
			void unregisterHandler(ASAAC_PCS_FunctionId functionId);
			
			ASAAC_TimedReturnStatus handleOneRequest();	
			
			void setRequestVc(ASAAC_PublicId RequestVc);
			ASAAC_PublicId getRequestVc();

			void setReplyVc(ASAAC_PublicId ReplyVc);
			ASAAC_PublicId getReplyVc();

			void setTimeOut(ASAAC_TimeInterval TimeOut);
			ASAAC_TimeInterval getTimeOut();
		};
	}//namespace: PCS
}//namespace: ASAAC

#endif /*SERVER_HH_*/
