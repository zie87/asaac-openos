#include "PCSConfiguration.hh"

PCSConfiguration::PCSConfiguration()
{
}


void PCSConfiguration::initialize()
{
	m_MappingTable.initialize();
	m_VcDescriptionTable.initialize();
	m_TcDescriptionTable.initialize();
	m_LocalVcTable.initialize();
	m_NetworkTable.initialize();
}


void PCSConfiguration::deinitialize()
{
	m_MappingTable.deinitialize();
	m_VcDescriptionTable.deinitialize();
	m_TcDescriptionTable.deinitialize();
	m_LocalVcTable.deinitialize();
	m_NetworkTable.deinitialize();
}


ASAAC_ReturnStatus	PCSConfiguration::addTcMapping( const ASAAC_VcToTcMappingDescription & Description )
{
	return m_MappingTable.addTcMapping( Description );
}


ASAAC_ReturnStatus	PCSConfiguration::removeTcMapping( ASAAC_PublicId VcId, ASAAC_PublicId TcId )
{
	return m_MappingTable.removeTcMapping( VcId, TcId );
}


ASAAC_ReturnStatus	PCSConfiguration::getTcMapping( ASAAC_PublicId VcId, ASAAC_PublicIdSet& TcMapping, unsigned long& Number )
{
	return m_MappingTable.getTcMapping( VcId, TcMapping, Number );
}


ASAAC_ReturnStatus	PCSConfiguration::getVcMapping( ASAAC_PublicId TcId, ASAAC_PublicIdSet& VcMapping, unsigned long& Number )
{
	return m_MappingTable.getVcMapping( TcId, VcMapping, Number );
}


ASAAC_ReturnStatus	PCSConfiguration::getCDRFlag( ASAAC_PublicId VcId, ASAAC_PublicId TcId, ASAAC_Bool& Flag )
{
	return m_MappingTable.getCDRFlag( VcId, TcId, Flag );
}



// Vc Descriptions
	
ASAAC_ReturnStatus	PCSConfiguration::addVcDescription( const ASAAC_VcDescription& Description )
{
	return m_VcDescriptionTable.addVcDescription( Description );
}


ASAAC_ReturnStatus	PCSConfiguration::removeVcDescription( ASAAC_PublicId GlobalVcId )
{
	return m_VcDescriptionTable.removeVcDescription( GlobalVcId );
}


ASAAC_ReturnStatus	PCSConfiguration::getVcDescription( ASAAC_PublicId GlobalVcId, ASAAC_VcDescription& Description )
{
	return m_VcDescriptionTable.getVcDescription( GlobalVcId, Description );
}



// Tc Descriptions

ASAAC_ReturnStatus	PCSConfiguration::addTcDescription( const ASAAC_TcDescription& Description )
{
	return m_TcDescriptionTable.addTcDescription( Description );
}


ASAAC_ReturnStatus	PCSConfiguration::removeTcDescription( ASAAC_PublicId TcId )
{
	return m_TcDescriptionTable.removeTcDescription( TcId );
}

	
ASAAC_ReturnStatus	PCSConfiguration::getTcDescription( ASAAC_PublicId TcId, ASAAC_TcDescription& Description )
{
	return m_TcDescriptionTable.getTcDescription( TcId, Description );
}

unsigned long PCSConfiguration::getNumberOfVc()
{
	return m_LocalVcTable.getNumberOfVc();
}

ASAAC_ReturnStatus PCSConfiguration::getVc(unsigned long n, ASAAC_PublicId & id)
{
	return m_LocalVcTable.getVc(n,id);
}

ASAAC_ReturnStatus	PCSConfiguration::addLocalVc( ASAAC_PublicId LocalVc, ASAAC_PublicId GlobalVc )
{
	return m_LocalVcTable.addLocalVc( LocalVc, GlobalVc );
}

ASAAC_ReturnStatus	PCSConfiguration::removeLocalVc( ASAAC_PublicId LocalVc )
{
	return m_LocalVcTable.removeLocalVc( LocalVc );
}

	
ASAAC_PublicId		PCSConfiguration::getLocalVc( ASAAC_PublicId GlobalVc )
{
	return m_LocalVcTable.getLocalVc( GlobalVc );
}


ASAAC_PublicId		PCSConfiguration::getGlobalVc( ASAAC_PublicId LocalVc )
{
	return m_LocalVcTable.getGlobalVc( LocalVc );
}

	
ASAAC_PublicId		PCSConfiguration::getFreeLocalVc( ASAAC_PublicId MinValue )
{
	return m_LocalVcTable.getFreeLocalVc( MinValue );
}


ASAAC_ReturnStatus PCSConfiguration::addNetwork(const ASAAC_NetworkDescriptor& descriptor)
{
	return m_NetworkTable.addNetworkDescriptor(descriptor);
};

ASAAC_ReturnStatus PCSConfiguration::getNetworks(ASAAC_NetworkDescriptor* &networks, unsigned long& number)
{
	number = m_NetworkTable.getNumberOfNetworks();
	networks = m_NetworkTable.getNetwork(0);
	
	return ASAAC_SUCCESS;
};

unsigned long PCSConfiguration::getNumberOfTc()
{
	return m_TcDescriptionTable.getNumberOfTc();	
}

ASAAC_ReturnStatus PCSConfiguration::getTcDescription(ASAAC_TcDescription& Description,unsigned long index)
{
	return m_TcDescriptionTable.getTcDescription(Description, index);
}

ASAAC_ReturnStatus PCSConfiguration::addTcRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& RateLimit)
{
	return m_TcDescriptionTable.addTcRateLimit(TcId, RateLimit);
};
	
ASAAC_ReturnStatus PCSConfiguration::getTcRateLimit( ASAAC_TimeInterval& RateLimit, ASAAC_PublicId& TcId, unsigned long Index)
{
	return m_TcDescriptionTable.getTcRateLimit(RateLimit, TcId, Index);
};
