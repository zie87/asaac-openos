#ifndef EXCEPTION_HH_
#define EXCEPTION_HH_

#include "OpenOS.hh"
#include <string>

using namespace std;

/*! Objects serving as MessageConsumer, VcMessageConsumer and TcMessageConsumer
 *  shall throw an Exception in case of an unexpected behaviour.
 *  They shall supply Vc number and Tc number where available, and a
 *  message denoting the type of unexpected behaviour encountered.
 */
class PCSException
{
public:
	PCSException();
	PCSException( ASAAC_PublicId TcId, ASAAC_PublicId VcId, const string& Message );
	
	virtual ~PCSException();
	
	string	getFullMessage();
	/*!< return formatted string that includes TcId, VcId and error message for
	 *   outputting or relaying to an error logging facility
	 */
	
private:
	string		m_Message; 	//!< information about the encountered unexpected behaviour
	ASAAC_PublicId	m_TcId;		//!< PublicId of the Tc for which the exception had to be raised
	ASAAC_PublicId	m_VcId;		//!< PublicId of the (preferably global) Vc for which the exception was raised
	
};

#endif /*EXCEPTION_HH_*/
