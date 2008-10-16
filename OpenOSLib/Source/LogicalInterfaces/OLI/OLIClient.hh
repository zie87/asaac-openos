#ifndef ASAAC_OLI_CLIENT_HH_
#define ASAAC_OLI_CLIENT_HH_

#include "OpenOSIncludes.hh"
#include "Communication/TwoWayCommunication.hh"

using namespace std;


namespace ASAAC
{
	namespace OLI
	{
		typedef struct
		{
			unsigned char *ptr;
			unsigned long size;
		} Buffer;
		
		class Client
		{
		private:
			ASAAC_PublicId 		m_RequestVc;
			ASAAC_PublicId 		m_ReplyVc;
			
			ASAAC_TimeInterval 	m_TimeOut;
			unsigned long 		m_Size;
			
			TwoWayCommunication m_TwoWayCommunication;
			
		protected:			
			void establishCommunication();
			
			void receiveReply(
				unsigned long TransferId, 
				ASAAC_OLI_ReplyFileReadPayload &Reply,
                ASAAC_Time Timeout);
			
			void requestFile(
				unsigned long transferId,
				unsigned long size,
				unsigned long offset,
				ASAAC_CharacterSequence filename,
                ASAAC_Time Timeout);
								
		public:
			Client();
			virtual ~Client();
			
			void setRequestVc(ASAAC_PublicId RequestVc);
			ASAAC_PublicId getRequestVc();

			void setReplyVc(ASAAC_PublicId ReplyVc);
			ASAAC_PublicId getReplyVc();

			void setTimeOut(ASAAC_TimeInterval TimeOut);
			ASAAC_TimeInterval getTimeOut();

			void setSize(unsigned long size);
			unsigned long getSize();
			
			unsigned long requestMliDownload(
				ASAAC_CfmDescription cfm_description);
			
			//additional functions
			void storeCompleteFile(
				ASAAC_CharacterSequence LocalPath, 
				ASAAC_CharacterSequence OliPath);
		};
		
	} //namespace: Oli
} //namespace ASAAC

#endif /*CLIENT_H_*/
