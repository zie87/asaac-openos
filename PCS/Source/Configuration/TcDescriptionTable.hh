#ifndef TCDESCRIPTIONTABLE_HH_
#define TCDESCRIPTIONTABLE_HH_

#include "PcsHIncludes.hh"

//! Storage for TcDescription structures for TransferConnections served in the PCS

class TcDescriptionTable
{
public:
	TcDescriptionTable();
	virtual ~TcDescriptionTable();
	
	virtual ASAAC_ReturnStatus addTcDescription(const  ASAAC_TcDescription& Description );
	/*!< add a TcDescription to the table
	 * 
	 * \param[in] Description TcDescription to add to the table
	 * 
	 * \return SUCCESS if no Description of a TransferConnection with the same
	 * 		   PublicId was found and the Description could be added to the
	 *         Table.
	 * 			ERROR otherwise.
	 */
	 
	virtual ASAAC_ReturnStatus removeTcDescription( ASAAC_PublicId TcId );
	/*!< remove TcDescription for a given TransferConnection
	 * 
	 * \param[in] TcId	PublicId of TransferConnection that shall have its description
	 *                  removed from this object's table.
	 * 
	 * \return			SUCCESS if corresponding TcDescription was found in table and
	 *                  could successfully be removed. ERROR otherwise.
	 */
	
	virtual ASAAC_ReturnStatus getTcDescription( ASAAC_PublicId TcId, ASAAC_TcDescription& Description );
	/*!< retrieve TcDescription for a given TransferConnection
	 * 
	 * \param[in] TcId	PublicId of TransferConnection for which the description field
	 *                  shall be retrieved
	 * \param[out] Description Structure of Type TcDescription into which the description
	 *                         for the indicated TransferConnection shall be copied
	 * 
	 * \return			SUCCESS if corresponding TransferConnection was found.
	 *                  ERROR otherwise.
	 */
	
	unsigned long getNumberOfTc();

	ASAAC_ReturnStatus getTcDescription( ASAAC_TcDescription& Description, unsigned long Index);
	
	ASAAC_ReturnStatus getTcRateLimit( ASAAC_TimeInterval& RateLimit, ASAAC_PublicId& TcId, unsigned long Index);
	
	ASAAC_ReturnStatus addTcRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& RateLimit);
	
	ASAAC_ReturnStatus getTcIndex( ASAAC_PublicId TcId, unsigned long & TcIndex );
	
private:
	ASAAC_TcDescription	m_DescriptionTable[ PCS_MAX_NUMBER_OF_TCS ];
	//!< Table of TcDescriptions for TransferChannels
	
	ASAAC_TimeInterval m_RateLimits[ PCS_MAX_NUMBER_OF_TCS ];
	
	unsigned long	m_NextFreeSlot;
	//!< Number of TcDescriptions stored in m_DescriptionTable
};

#endif /*TCDESCRIPTIONTABLE_HH_*/
