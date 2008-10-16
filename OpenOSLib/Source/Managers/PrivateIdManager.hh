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
};

#endif /*PRIVATEIDMANAGER_HH_*/
