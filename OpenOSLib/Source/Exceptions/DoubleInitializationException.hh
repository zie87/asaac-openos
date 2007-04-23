#ifndef DoubleInitializationException_HH_
#define DoubleInitializationException_HH_

#include "OSException.hh"

class DoubleInitializationException : public OSException
{
public:
	DoubleInitializationException( const char* Function = 0, int Line = 0 ) : OSException( "Double Initialization", Function, Line ) {};
	virtual ~DoubleInitializationException() {};
};


#endif /*DoubleInitializationException_HH_*/
