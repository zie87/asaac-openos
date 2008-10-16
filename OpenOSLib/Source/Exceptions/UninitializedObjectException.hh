#ifndef UninitializedObjectException_HH_
#define UninitializedObjectException_HH_

#include "OSException.hh"

class UninitializedObjectException : public OSException
{
public:
	UninitializedObjectException( const char* Function = 0, long Line = 0 ) : OSException( "Uninitialized Object", Function, Line ) {};
	virtual ~UninitializedObjectException() throw() {};
};


#endif /*UninitialzedObjectException_HH_*/
