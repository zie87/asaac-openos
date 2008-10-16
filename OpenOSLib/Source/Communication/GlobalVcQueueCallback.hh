#ifndef GLOBALVCQUEUECALLBACK_HH_
#define GLOBALVCQUEUECALLBACK_HH_

#include "AbstractInterfaces/Callback.hh"

class GlobalVc; // forward definition of this class type

//! callback object handling the release of overwritten buffers in queues
/*! The GlobalVcQueueCallback is to be set as the overwrite-callback for the
 *  local vc queues in a global vc. It takes care that all buffers that are
 *  overwritten in the individual receiver queues are properly registered
 *  as no longer used in the respective queue, and might, if applicable,
 *  be returned to the free buffers queue.
 */
class GlobalVcQueueCallback : public Callback
{
public:
	GlobalVcQueueCallback();
	
	//! initialization
	void initialize( GlobalVc* ParentGlobalVc );
	/*!< \param[in] ParentGlobalVc Reference to the Global Vc that instantiated
	 *                             this callback object.
	 */
	
	virtual void call( void* Data );
	
	virtual ~GlobalVcQueueCallback();
	
private:

	GlobalVc* m_ParentGlobalVc;
};

#include "GlobalVc.hh"

#endif /*GLOBALVCQUEUECALLBACK_HH_*/
