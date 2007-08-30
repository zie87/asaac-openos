#ifndef ASAAC_EXCEPTION_HH_
#define ASAAC_EXCEPTION_HH_

#include "ASAAC.h"
#include "OAL/oal.h"
#include "Common/Asaac/CharacterSequence.hh"


typedef struct
{
	ASAAC_CharacterSequence message;
	ASAAC_CharacterSequence function;
	long          	  		line;			
} ExceptionPathType;


class ASAAC_Exception: public exception {
	public:
		ASAAC_Exception( );
		ASAAC_Exception( const char* Message, const char* Function = 0, long Line = 0 );
		virtual ~ASAAC_Exception() throw();
		
        virtual const char *what() const throw(); 
        
        void setProcessId( ASAAC_PublicId ProcessId );
        void setThreadId( ASAAC_PublicId ThreadId );
        
        ASAAC_PublicId getProcessId();
        ASAAC_PublicId getThreadId();
        
		virtual bool isTimeout() const;
		virtual bool isResource() const;
		virtual ASAAC_ErrorCode getErrorCode() const;
		virtual ASAAC_ErrorType getErrorType() const;
		virtual ASAAC_PublicId getVcId() const;
		virtual ASAAC_PublicId getTcId() const;
		virtual ASAAC_NetworkDescriptor getNetwork() const;
		virtual ASAAC_Address getLocation() const;
		
		virtual void addPath( const char* Message, const char* Function = 0, long Line = 0 );
		
		virtual const char * getMessage() const;

		virtual const char * getMessageItem(unsigned short index) const;

		void printMessage() const;
		
        void logMessage( ASAAC_LogMessageType message_type = ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE ) const;
        void raiseError( ) const;
		
	protected:
		void initialize();
	
		ASAAC_Time		  		m_Time;
		ASAAC_PublicId	  		m_ProcessId;
		ASAAC_PublicId	  		m_ThreadId;

		ExceptionPathType 		m_Path[OS_MAX_NUMBER_OF_EXCEPTIONS];
		unsigned short			m_PathSize;
};



#endif /*ASAAC_EXCEPTION_HH_*/
