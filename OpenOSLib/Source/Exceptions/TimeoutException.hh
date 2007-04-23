#ifndef TIMEOUTEXCEPTION_HH_
#define TIMEOUTEXCEPTION_HH_

#include "OSException.hh"

class TimeoutException : public OSException
{
public:
	TimeoutException( const char* Message = 0, const char* Function = 0, int Line = 0 ) : OSException( Message, Function, Line ) {};
	TimeoutException( const char* Function = 0, int Line = 0 ) : OSException( "Timeout", Function, Line ) {};
	virtual ~TimeoutException() {};
	virtual bool isTimeout() const { return true; };
};

#endif /*TIMEOUTEXCEPTION_HH_*/
