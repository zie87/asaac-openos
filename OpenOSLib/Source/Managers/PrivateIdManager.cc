#include "PrivateIdManager.hh"

PrivateIdManager::PrivateIdManager()
{
	m_NextPrivateId = 1;
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
	
