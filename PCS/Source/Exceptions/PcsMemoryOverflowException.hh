#ifndef PCSMEMORYOVERFLOWEXCEPTION_HH_
#define PCSMEMORYOVERFLOWEXCEPTION_HH_

#include "PcsException.hh"

class PcsMemoryOverflowException: PcsException
{
public:
	PcsMemoryOverflowException();
	virtual ~PcsMemoryOverflowException();
};

#endif /*MEMORYOVERFLOWEXCEPTION_HH_*/
