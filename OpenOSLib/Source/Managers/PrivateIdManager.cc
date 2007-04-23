#include "PrivateIdManager.hh"

PrivateIdManager::PrivateIdManager()
{
	printf("Initializing PrivateIdManager..."); fflush(stdout);
	m_NextPrivateId = 1;
	printf("done\n"); fflush(stdout);
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
	return ( this->m_NextPrivateId++ );
}
	
