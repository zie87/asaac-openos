#ifndef PCSCONFIGURATION_HH_
#define PCSCONFIGURATION_HH_

#include "PcsHIncludes.hh"

#include "Configuration/MappingTable.hh"
#include "Configuration/VcDescriptionTable.hh"
#include "Configuration/TcDescriptionTable.hh"
#include "Configuration/LocalVcTable.hh"
#include "Configuration/NetworkTable.hh"

//! Central configuration object of the PCS

/*! The PCSConfiguration is a facade for all configuration objects required in the PCS,
 *  so other objects can be relayed to one single configuration object rather than a complete
 * set, while still keeping the different configuration subinformations seperate.
 * 
 * Information on the individual functions can be found long he corresponding
 * child objects.
 */

class PCSConfiguration 
{
public:
	PCSConfiguration();

	void initialize();
	void deinitialize();
	
	// TC-Vc Mapping
	
	ASAAC_ReturnStatus	addTcMapping( const ASAAC_VcToTcMappingDescription& Description );
	ASAAC_ReturnStatus	removeTcMapping( ASAAC_PublicId VcId, ASAAC_PublicId TcId );
	ASAAC_ReturnStatus	getTcMapping( ASAAC_PublicId VcId, ASAAC_PublicIdSet& TcMapping, unsigned long& Number );
	ASAAC_ReturnStatus	getVcMapping( ASAAC_PublicId TcId, ASAAC_PublicIdSet& VcMapping, unsigned long& Number );
	ASAAC_ReturnStatus	getCDRFlag( ASAAC_PublicId VcId, ASAAC_PublicId TcId, ASAAC_Bool& Flag );
	
	// Vc Descriptions
	
	ASAAC_ReturnStatus	addVcDescription( const ASAAC_VcDescription& Description );
	ASAAC_ReturnStatus	removeVcDescription( ASAAC_PublicId GlobalVcId );
	ASAAC_ReturnStatus	getVcDescription( ASAAC_PublicId GlobalVcId, ASAAC_VcDescription& Description );

	// Tc Descriptions

	ASAAC_ReturnStatus	addTcDescription( const ASAAC_TcDescription& Description );
	ASAAC_ReturnStatus	removeTcDescription( ASAAC_PublicId TcId );
	ASAAC_ReturnStatus	getTcDescription( ASAAC_PublicId TcId, ASAAC_TcDescription& Description );
	
	// LocalVc <-> GlobalVc Mapping
	
	ASAAC_ReturnStatus	addLocalVc( ASAAC_PublicId LocalVc, ASAAC_PublicId GlobalVc );
	ASAAC_ReturnStatus	removeLocalVc( ASAAC_PublicId LocalVc );
	
	
	ASAAC_PublicId		getLocalVc( ASAAC_PublicId GlobalVc );
	ASAAC_PublicId		getGlobalVc( ASAAC_PublicId LocalVc );
	
	unsigned long getNumberOfVc();

	ASAAC_ReturnStatus getVc(unsigned long n, ASAAC_PublicId & id);
	
	ASAAC_PublicId		getFreeLocalVc( ASAAC_PublicId MinValue );
	
	unsigned long getNumberOfTc();

	ASAAC_ReturnStatus getTcDescription(ASAAC_TcDescription& Description,unsigned long index);
	
	
	ASAAC_ReturnStatus addNetwork(const ASAAC_NetworkDescriptor& descriptor);
	ASAAC_ReturnStatus getNetworks(ASAAC_NetworkDescriptor* &networks, unsigned long& number);

	ASAAC_ReturnStatus addTcRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& RateLimit);
	
	ASAAC_ReturnStatus getTcRateLimit( ASAAC_TimeInterval& RateLimit, ASAAC_PublicId& TcId, unsigned long Index);
	
private:
	MappingTable			m_MappingTable;
	VcDescriptionTable		m_VcDescriptionTable;
	TcDescriptionTable		m_TcDescriptionTable;
	LocalVcTable			m_LocalVcTable;
	NetworkTable			m_NetworkTable;
};

#endif /*PCSCONFIGURATION_HH_*/
