#ifndef LocalVcTable_HH_
#define LocalVcTable_HH_

#include "OpenOS.hh"

#include "PcsConstants.hh"

const unsigned long SIZE_OF_VC_TABLE = 512;

//! Table to associate LocalVc and GlobalVc Ids

/*! The LocalVcTable stores a mapping of LocalVc PublicIds and the respective
 *  GlobalVc PublicIds as required to perform the Vc<->Tc mapping via APOS calls
 *  in the PCS.
 */

class LocalVcTable
{
public:
	LocalVcTable();
	virtual ~LocalVcTable();
	
	virtual ASAAC_ReturnStatus	addLocalVc( ASAAC_PublicId LocalVc, ASAAC_PublicId GlobalVc );
	/*!< add a new LocalVc <-> GlobalVc mapping
	 * 
	 * @param[in] LocalVc	LocalVc PublicId corresponding to
	 * @param[in] GlobalVc	GlobalVc PublicId
	 * 
	 * @return				ERROR if there was already a mapping defined for
	 * 						the given LocalVc or the given GlobalVc.
	 * 						SUCCESS otherwise.
	 */
	 
	virtual ASAAC_ReturnStatus	removeLocalVc( ASAAC_PublicId LocalVc );
	/*!< remove LocalVc <-> GlobalVc mapping
	 * 
	 * @param[in] LocalVc	LocalVc PublicId for which the mapping shall
	 * 						be removed.
	 * 
	 * @return				ERROR if there was no mapping defined with
	 *						the given LocalVcId.
	 * 						SUCCESS otherwise.
	 */
	
	virtual ASAAC_PublicId		getLocalVc( ASAAC_PublicId GlobalVc );
	/*!< get GlobalVc for given LocalVc from the registered mappings
	 * 
	 * @param[in] GlobalVc	PublicId of LocalVc
	 * 
	 * @return				PublicId of GlobalVc corresponding to supplied LocalVc.
	 * 						If no matching mapping was found, a value of 0 is returned.
	 */
	 
	virtual ASAAC_PublicId		getGlobalVc( ASAAC_PublicId LocalVc );
	/*!< get LocalVc Id for given GlobalVc Id from the registered mappings
	 * 
	 * @param[in] LocalVc	PublicId of Global Virtual Channel
	 * 
	 * @return				PublicId of LocalVc corresponding to supplied GlobalVc.
	 * 						If no matching mapping was found, a value of 0 is returned.
	 */

	
	virtual ASAAC_PublicId		getFreeLocalVc( ASAAC_PublicId MinValue );
	/*!< find a possible value for a LocalVc Id that is not contained within the mapping table
	 * 
	 * @param[in]	MinValue	Minimum Value of LocalVcId to be returned. LocalVc values below that threshold
	 * 							are most likely used for different communication functionalities.
	 * 
	 * @return				PublicId >= MinValue that is not contained within the mapping table.
	 */
	
	unsigned long getNumberOfVc();

	ASAAC_ReturnStatus getVc(unsigned long n, ASAAC_PublicId & id);
	
	
private:
	
	ASAAC_PublicId		m_AccessSemaphore; 
	//!< PublicId of ASAAC Semaphore object used to ensure non-concurrent access to mapping table
	 
	struct VcMapping {
		ASAAC_PublicId	LocalVc;
		ASAAC_PublicId	GlobalVc;
	};
	
	VcMapping		m_Mapping[  SIZE_OF_VC_TABLE ];
	//!< Table for mapping LocalVcs and GlobalVcs
	
	unsigned long	m_NextFreeMapping;
	//!< Index of next free Cell in mapping table, and hence number of currently present entries
	
    ASAAC_VcToTcMappingDescription dummy_desc;
    ASAAC_PublicIdSet dummy_set;
};

#endif /*LocalVcTable_HH_*/
