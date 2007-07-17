#include "NetworkTable.hh"

NetworkTable::NetworkTable()
{
	m_NextFreeNetwork = 0;
};


void NetworkTable::initialize()
{
	m_NextFreeNetwork = 0;
}


void NetworkTable::deinitialize()
{
	m_NextFreeNetwork = 0;
}

	
ASAAC_ReturnStatus	NetworkTable::addNetworkDescriptor( const ASAAC_NetworkDescriptor& network)
{
	for(unsigned long search = 0; search < m_NextFreeNetwork; ++search)
	{
		if(m_NetworkTable[search].network == network.network && m_NetworkTable[search].port == network.port)
			return ASAAC_ERROR;
	}
		
	if(m_NextFreeNetwork >= PCS_MAX_NUMBER_OF_NETWORKS)
		return ASAAC_ERROR;
		
	m_NetworkTable[m_NextFreeNetwork] = network;
	
	++m_NextFreeNetwork;
	
	return ASAAC_SUCCESS;
};

unsigned long NetworkTable::getNumberOfNetworks()
{
	return m_NextFreeNetwork;
};
	 
ASAAC_NetworkDescriptor* NetworkTable::getNetwork(unsigned long number)
{
	if(number >= m_NextFreeNetwork)
	{
		#ifdef _DEBUG_
		//cerr << "NetworkTable::getNetwork() invalid argument " << number << endl;
		#endif
		return 0;
	}
	return &(m_NetworkTable[number]);
};
