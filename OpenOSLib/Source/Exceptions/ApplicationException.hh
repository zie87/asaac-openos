#ifndef ApplicationException_HH_
#define ApplicationException_HH_

#include "ASAAC_Exception.hh"

class ApplicationException : public ASAAC_Exception
{
public:
	ApplicationException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	ApplicationException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_APPLICATION_ERROR; }
	virtual ~ApplicationException() throw() {};
};


#endif /*ApplicationException_HH_*/
