#ifndef ASAAC_EXCEPTION_HH_
#define ASAAC_EXCEPTION_HH_

#include "ASAAC.h"
#include "OAL/oal.h"
#include "Common/CharacterSequence.hh"


#define THROW(x,y)		throw x(y,__PRETTY_FUNCTION__,__LINE__);


typedef struct
{
	ASAAC_CharacterSequence message;
	ASAAC_CharacterSequence function;
	int          	  		line;			
} ExceptionPathType;


class ASAAC_Exception {
	public:
		ASAAC_Exception( );
		ASAAC_Exception( const char* Message, const char* Function = 0, int Line = 0 );
		virtual ~ASAAC_Exception();
		
		virtual bool isTimeout() const;
		virtual ASAAC_ErrorCode getErrorCode() const;
		virtual ASAAC_ErrorType getErrorType() const;
		virtual ASAAC_PublicId getVcId() const;
		virtual ASAAC_PublicId getTcId() const;
		virtual ASAAC_NetworkDescriptor getNetwork() const;
		virtual ASAAC_Address getLocation() const;
		
		virtual void addPath( const char* Message, const char* Function = 0, int Line = 0 );
		
		virtual const char * getMessage();

		virtual const char * getMessageItem(unsigned short index);

		void printMessage();
		
		void logMessage();
		
	protected:
		void initialize();
	
		ASAAC_Time		  		m_Time;
		ASAAC_PublicId	  		m_ProcessId;
		ASAAC_PublicId	  		m_ThreadId;

		ExceptionPathType 		m_Path[OS_MAX_NUMBER_OF_EXCEPTIONS];
		unsigned short			m_PathSize;
};



#endif /*ASAAC_EXCEPTION_HH_*/
