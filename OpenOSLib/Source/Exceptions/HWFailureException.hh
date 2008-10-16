#ifndef HWFailureException_HH_
#define HWFailureException_HH_

#include "ASAAC_Exception.hh"

class HWFailureException : public ASAAC_Exception
{
public:
	HWFailureException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	HWFailureException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_HW_FAILURE; }
	virtual ~HWFailureException() throw() {};
};


#endif /*HWFailureException_HH_*/
