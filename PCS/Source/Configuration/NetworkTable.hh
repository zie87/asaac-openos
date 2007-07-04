#ifndef NETWORKTABLE_HH_
#define NETWORKTABLE_HH_

#include "PcsIncludes.hh"


//! Configuration object to hold the available networks


class NetworkTable
{
public:
	NetworkTable();
	virtual ~NetworkTable();
	
	ASAAC_ReturnStatus	addNetworkDescriptor( const ASAAC_NetworkDescriptor& network);
	/*!< create a new entry for a Network Descriptor
	 *\param[in] network is a description of network and port as defined in ASAAC
	 * 
	 *\return SUCCESS if Network could be installed to the list of available networks, where only NUMBER_OF_NETWORKS exist at once
	 * 
	 */
	 
	 unsigned long getNumberOfNetworks();
	 
	 ASAAC_NetworkDescriptor* getNetwork(unsigned long number);
	 
private:
	
	ASAAC_NetworkDescriptor	m_NetworkTable[ PCS_MAX_NUMBER_OF_NETWORKS ]; 
	//!< Table of Networks
	
	unsigned long	m_NextFreeNetwork; 
	//!< Number of Networks currently stored in m_NetworkTable
	
};

#endif /*NETWORKTABLE_HH_*/
