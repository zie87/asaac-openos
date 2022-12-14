#ifndef FatalException_HH_
#define FatalException_HH_

#include "ASAAC_Exception.hh"

class FatalException : public ASAAC_Exception
{
public:
	FatalException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	FatalException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_FATAL_ERROR; }
	virtual ~FatalException() throw() {};
};


#endif /*FatalException_HH_*/
