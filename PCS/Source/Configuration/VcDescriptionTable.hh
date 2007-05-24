#ifndef VCDESCRIPTIONTABLE_HH_
#define VCDESCRIPTIONTABLE_HH_

#include "OpenOS.hh"

#include "PcsConstants.hh"

//! Storage for VcDescription structures for GlobalVcs served in the PCS

class VcDescriptionTable
{
public:
	VcDescriptionTable();
	virtual ~VcDescriptionTable();
	
	virtual ASAAC_ReturnStatus addVcDescription(const  ASAAC_VcDescription& Description );
	/*!< add a VcDescription to the table
	 * 
	 * \param[in] Description VcDescription to add to the table
	 * 
	 * \return SUCCESS if no Description of a GlobalVc with the same
	 * 		   PublicId was found and the Description could be added to the
	 *         Table.
	 * 			ERROR otherwise.
	 */
	
	virtual ASAAC_ReturnStatus removeVcDescription( ASAAC_PublicId GlobalVcId );
	/*!< remove VcDescription for a given GlobalVc
	 * 
	 * \param[in] GlobalVcId	PublicId of GlobalVc that shall have its description
	 *                  		removed from this object's table.
	 * 
	 * \return			SUCCESS if corresponding VcDescription was found in table and
	 *                  could successfully be removed. ERROR otherwise.
	 */
	
	
	ASAAC_ReturnStatus getVcDescription( ASAAC_PublicId GlobalVcId, ASAAC_VcDescription& Description );
	/*!< retrieve VcDescription for a given GlobalVc
	 * 
	 * \param[in] GlobalVcId	PublicId of GlobalVc for which the description field
	 *                  		shall be retrieved
	 * \param[out] Description Structure of Type VcDescription into which the description
	 *                         for the indicated GlobalVc shall be copied
	 * 
	 * \return			SUCCESS if corresponding description of GlobalVc was found.
	 *                  ERROR otherwise.
	 */
	
	
private:
	ASAAC_VcDescription	m_DescriptionTable[ PCS_NUMBER_OF_VCS ];
	//!< Table of stored VcDescriptions
	
	unsigned long	m_NextFreeSlot;
	//!< Number of VcDescriptions stored in m_DescriptionTable
};

#endif /*VCDESCRIPTIONTABLE_HH_*/
