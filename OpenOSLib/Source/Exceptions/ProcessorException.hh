#ifndef ProcessorException_HH_
#define ProcessorException_HH_

#include "ASAAC_Exception.hh"

class ProcessorException : public ASAAC_Exception
{
public:
	ProcessorException( const char* Message = 0, const char* Function = 0, long Line = 0 ) : ASAAC_Exception( Message, Function, Line ) {};
	ProcessorException( const char* Function, long Line ) : ASAAC_Exception( 0, Function, Line ) {};
	virtual ASAAC_ErrorType getErrorType() const { return ASAAC_PROCESSOR_ERROR; }
	virtual ~ProcessorException() throw() {};
};


#endif /*ProcessorException_HH_*/
