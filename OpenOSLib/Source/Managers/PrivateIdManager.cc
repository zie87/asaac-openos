#include "PrivateIdManager.hh"

PrivateIdManager::PrivateIdManager()
{
}

PrivateIdManager::~PrivateIdManager()
{
}

PrivateIdManager* PrivateIdManager::getInstance()
{
	static PrivateIdManager ThisIdManager;
	
	return &ThisIdManager;
}


ASAAC_PrivateId PrivateIdManager::getNextId()
{
    CharacterSequence Counter = getenv(OS_ENV_PRIVATEID_COUNTER);
    
    if ( Counter.empty() )
        Counter = (ASAAC_PublicId)0;
    
    Counter = (ASAAC_PublicId)(Counter.asaac_id() + 1);

    setenv(OS_ENV_PRIVATEID_COUNTER, Counter.c_str(), 1);
    
    return Counter.asaac_id();
}
	
