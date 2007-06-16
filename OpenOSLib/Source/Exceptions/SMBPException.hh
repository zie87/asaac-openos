#ifndef SMBPException_HH_
#define SMBPException_HH_

#include "ASAAC_Exception.hh"

class SMBPException : public ASAAC_Exception
{
public:
	SMBPException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	SMBPException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_SMBP_ERROR; }
	virtual ~SMBPException() throw() {};
};


#endif /*SMBPException_HH_*/
