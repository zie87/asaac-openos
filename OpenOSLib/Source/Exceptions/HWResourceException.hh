#ifndef HWResourceException_HH_
#define HWResourceException_HH_

#include "ASAAC_Exception.hh"

class HWResourceException : public ASAAC_Exception
{
public:
	HWResourceException( const char* Message = 0, const char* Function = 0, int Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	HWResourceException( const char* Function, int Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_HW_RESOURCE_ERROR; }
	virtual ~HWResourceException() {};
};


#endif /*HWResourceException_HH_*/
