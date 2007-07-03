#ifndef MAPPINGTABLE_HH_
#define MAPPINGTABLE_HH_

#include "PcsIncludes.hh"

//! Configuration object to hold the Vc <-> Tc mapping information

/*! A MappingTable stores information as configured via the SMOS call attachTransferConnectionToVirtualChannel()
 * (STANAG 4626, part II, Section 11.7.3.5). It is required to correlate incoming Vc data in the PCS to 
 *	their respectively connected TransferConnections for filtering, packaging and relaying.
 */

class MappingTable
{
public:
	MappingTable();
	virtual ~MappingTable();
	
	virtual ASAAC_ReturnStatus	addTcMapping( const ASAAC_VcToTcMappingDescription& Description );
	/*!< create a new entry for a Vc<->Tc mapping.
	 *\param[in] Description Mapping description, indicating Vc, Tc and CDR filtering flag for
	 * 					 the connection to be set up.
	 * 
	 *\return			 SUCCESS if parameter is valid and a new connection could be set up,
	 * 					 ERROR if parameters are invalid, and/or a connection with this VC and/or TC
	 * 					 already exists.
	 */
	 
	virtual ASAAC_ReturnStatus	removeTcMapping( ASAAC_PublicId VcId, ASAAC_PublicId TcId );
	/*!< remove an existing Vc<->Tc connection
	 * \param[in] VcId		PublicId of GlobalVc to no longer be relayed from/to
	 * \param[in] TcId		PublicId of TransferConnection to no longer be relayed from/to
	 * 
	 * \return			SUCCESS if a matching connection was found and could be removed.
	 * 				    ERROR otherwise.
	 */
	 
	virtual ASAAC_ReturnStatus	getCDRFlag( ASAAC_PublicId VcId, ASAAC_PublicId TcId, ASAAC_Bool& Flag );
	/*!< get the CDR (Common Data Representation) flag for a channel mapping
	 * 
	 *	\param[in] VcId		PublicId of GlobalVc of the desired mapping
	 *  \param[in] TcId		PublicId of TransferConnection of desired mapping
	 * 	\param[out] Flag	Value of CDR flag for indicated mapping
	 * 
	 *	\return		SUCCESS	if corresponding mapping was found.
	 * 				ERROR otherwise.
	 */
	
	virtual ASAAC_ReturnStatus	getTcMapping( ASAAC_PublicId VcId, ASAAC_PublicIdSet& TcMapping, unsigned long& Number );
	/*!< get a list of all TransferConnections mapped to a given GlobalVc
	 * 
	 * \param[in] VcId		PublicId of GlobalVc
	 * \param[out] TcMapping	PublicIdSet of PublicIds of TransferConnections correlated with the given GlobalVc
	 * \param[out] Number		Number of TransferConnections correlated with given GlobalVc
	 * 
	 * \return				SUCCESS if at least one TransferConnection was found correlated with given GlobalVc.
	 * 						ERROR otherwise.
	 */
	
	virtual ASAAC_ReturnStatus	getVcMapping( ASAAC_PublicId TcId, ASAAC_PublicIdSet& VcMapping, unsigned long& Number );
	/*!< get a list of all GlobalVcsmapped to a given TransferConnection
	 * 
	 * \param[in] TcId		PublicId of TransferConnection
	 * \param[out] VcMapping	PublicIdSet of PublicIds of GlobalVcs correlated with the given TransferConnection
	 * \param[out] Number		Number of GlobalVcs correlated with given TransferConnection
	 * 
	 * \return				SUCCESS if at least one GlobalVc was found correlated with given TransferConnection.
	 * 						ERROR otherwise.
	 */

	
private:
	
	ASAAC_PublicId					m_AccessSemaphore; 
	//!< Synchronization object to prevent concurrent access to mapping table
	
	ASAAC_VcToTcMappingDescription	m_MappingTable[ PCS_MAX_NUMBER_OF_MAPPINGS ]; 
	//!< Table of GlobalVc<->TransferConnection mappings
	
	unsigned long				m_NextFreeMapping; 
	//!< Number of Mappings currently stored in m_MappingTable
	
};

#endif /*MAPPINGTABLE_HH_*/
