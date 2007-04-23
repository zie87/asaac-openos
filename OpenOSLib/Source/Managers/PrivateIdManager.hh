#ifndef PRIVATEIDMANAGER_HH_
#define PRIVATEIDMANAGER_HH_

#include "OpenOSIncludes.hh"

class PrivateIdManager
{
public:
	static PrivateIdManager* getInstance();

	ASAAC_PrivateId getNextId();

	virtual ~PrivateIdManager();
	
protected:
	PrivateIdManager();
	
	ASAAC_PrivateId m_NextPrivateId;

};

#endif /*PRIVATEIDMANAGER_HH_*/
