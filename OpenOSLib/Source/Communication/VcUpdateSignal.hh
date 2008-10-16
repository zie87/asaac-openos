#ifndef VCUPDATESIGNAL_HH_
#define VCUPDATESIGNAL_HH_

#include "IPC/Trigger.hh"
#include "Allocator/Allocator.hh"

//! singleton class encapsulating the signal called for every update on a Virtual Channel
/*! The VcUpdateSignal is required to implement the function of the waitOnMultiChannel()
 *  (see STANAG 4626, Part II, Section 11.4.7.9)
 * 
 *  It instantiates a trigger that shall be released on any write operation to a virtual
 *  channel, so waiting threads can re-evaluate their list of virtual channels to
 *  wait on.
 */

class VcUpdateSignal
{
public:
	//! get Trigger instance handling the VC updates
	static Trigger* getInstance();
	/*! \return reference to Vc Update trigger instance
	 */

	//! initialize the VC update signal for use
	static void initialize( bool IsMaster, Allocator *ParentAllocator );
	/*!< \param[in] IsMaster  Flag indicating whether the calling thread shall set up
	 *                        the shared memory object and control structures required for
	 *                        the operation of the trigger, or whether it shall merely
	 *                        act as user of these structures. There shall always be exactly one
	 *                        master instance.
	 */
	static void deinitialize();
	
	virtual ~VcUpdateSignal();

	static size_t predictSize();

private:
	VcUpdateSignal();
	
	
};

#endif /*VCUPDATESIGNAL_HH_*/
