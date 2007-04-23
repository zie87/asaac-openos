#ifndef OSEXCEPTION_HH_
#define OSEXCEPTION_HH_

#include "ASAAC_Exception.hh"

class OSException : public ASAAC_Exception
{
public:
	OSException( const char* Message = 0, const char* Function = 0, int Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	OSException( const char* Function, int Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_OS_ERROR; }
	virtual ~OSException() {};
};


#endif /*OSEXCEPTION_HH_*/
