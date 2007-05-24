#ifndef ASAAC_OLI_SERVER_HH_
#define ASAAC_OLI_SERVER_HH_

#include "OpenOSIncludes.hh"

namespace ASAAC
{
	namespace OLI
	{
		class Server
		{
		private:
			ASAAC_PublicId mRequestVc;
			ASAAC_PublicId mReplyVc;
			ASAAC_TimeInterval mTimeOut;
			
			ASAAC_ReturnStatus handleRequestFileRead(
				ASAAC_OLI_OliMessage_type* IncomingMessage, 
				ASAAC_OLI_OliMessage_type* OutgoingMessage);
    		ASAAC_ReturnStatus handleRequestMliDownload(
				ASAAC_OLI_OliMessage_type* IncomingMessage, 
				ASAAC_OLI_OliMessage_type* OutgoingMessage);
		public:
			Server();
			virtual ~Server();
			
			ASAAC_ReturnStatus handleRequest();	
			
			void setRequestVc(ASAAC_PublicId RequestVc);
			ASAAC_PublicId getRequestVc();

			void setReplyVc(ASAAC_PublicId ReplyVc);
			ASAAC_PublicId getReplyVc();

			void setTimeOut(ASAAC_TimeInterval TimeOut);
			ASAAC_TimeInterval getTimeOut();
		};
	}//namespace: OLI
}//namespace: ASAAC


#endif /*SERVER_HH_*/
