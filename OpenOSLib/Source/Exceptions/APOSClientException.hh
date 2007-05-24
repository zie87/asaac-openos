#ifndef APOSCLIENTEXCEPTION_HH_
#define APOSCLIENTEXCEPTION_HH_

#include "ASAAC_Exception.hh"

class APOSClientException : public ASAAC_Exception
{
public:
	APOSClientException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	APOSClientException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_APOS_CLIENT_ERROR; }
	virtual ~APOSClientException() throw() {};
};

#endif /*APOSClientEXCEPTION_HH_*/
