#ifndef RESOURCEEXCEPTION_HH_
#define RESOURCEEXCEPTION_HH_

#include "ASAAC_Exception.hh"

class ResourceException : public ASAAC_Exception
{
public:
	ResourceException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_RESOURCE_ERROR; }
	virtual ~ResourceException() throw() {};
};



#endif /*RESOURCEEXCEPTION_HH_*/
