#ifndef BLOCKINGSCOPE_HH_
#define BLOCKINGSCOPE_HH_

#include "OpenOSIncludes.hh"

class BlockingScope
{
public:
	BlockingScope();
	virtual ~BlockingScope();
private:
	ASAAC_PublicId m_ThreadId;
};

#endif /*BLOCKINGSCOPE_HH_*/
