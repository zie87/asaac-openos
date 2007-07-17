#ifndef NAMEGENERATOR_HH_
#define NAMEGENERATOR_HH_

#include "PcsHIncludes.hh"

#include <string>

using namespace std;


//! Facility to provide unique names for ASAAC objects

/*! The NameGenerator provides a functionality to generate unique
 *  names for ASAAC objects to be requested, such as Semaphores
 *  and Events. The specification of STANAG 4626 (Part II, Draft 1)
 *  Section 11.4.3.1 and 11.4.4.1 respectively require these
 *  synchronization objects to be named.
 *  In order to avoid unintended double-use of names when using
 *  synchronization objects in multiple instances of an object type,
 *  the use of uniquely generated names is advisable there.
 */
class NameGenerator
{
public:
	static NameGenerator* getInstance();
	/*!< request the pointer to an instance of NameGenerator.
	 *   The NameGenerator class is a singleton, thus only
	 *   allowing for one single NameGenerator object to
	 *   be instantiated at any given time, so double-use
	 *   of names is avoided.
	 */

	virtual ~NameGenerator();
	
	ASAAC_CharacterSequence createUniqueName( const string& Prefix );
	/*!< create a unique name string
	 * @param[in] Prefix shall, if possible, denote the requesting instance and the use of the object that shall be named
	 * 
	 * The output string will be in the format:
	 * "<Prefix> (#<unique number>)"
	 */

protected:
	NameGenerator();
	
	unsigned long m_Counter; //!< incremental value for the unique number to be used in unique name strings

};

#endif /*NAMEGENERATOR_HH_*/
