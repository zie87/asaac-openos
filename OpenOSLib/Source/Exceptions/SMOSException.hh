#ifndef SMOSException_HH_
#define SMOSException_HH_

#include "ASAAC_Exception.hh"

class SMOSException : public ASAAC_Exception
{
public:
	SMOSException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	SMOSException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_SMOS_ERROR; }
	virtual ~SMOSException() throw() {};
};


#endif /*SMOSException_HH_*/
