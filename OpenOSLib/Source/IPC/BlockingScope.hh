#ifndef BLOCKINGSCOPE_HH_
#define BLOCKINGSCOPE_HH_

#include "OpenOSIncludes.hh"

class BlockingScope
{
public:
	BlockingScope();
	virtual ~BlockingScope();
	
	ASAAC_PublicId getThreadId();
private:
	ASAAC_PublicId m_ThreadId;
};

#endif /*BLOCKINGSCOPE_HH_*/
