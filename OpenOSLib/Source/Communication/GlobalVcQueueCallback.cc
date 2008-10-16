#include "GlobalVcQueueCallback.hh"

GlobalVcQueueCallback::GlobalVcQueueCallback() : m_ParentGlobalVc(0)
{
}


void GlobalVcQueueCallback::initialize( GlobalVc* ParentGlobalVc )
{
	m_ParentGlobalVc = ParentGlobalVc;
}


void GlobalVcQueueCallback::call( void* ValuePtr )
{
	if ( m_ParentGlobalVc != 0 )
	{
#ifdef DEBUG_BUFFER
		cout << "GVC-Id: " << m_ParentGlobalVc->getId() << "GlobalVcQueueCallback::call() -> releaseBuffer()"  << endl;
#endif        
		m_ParentGlobalVc->releaseBuffer( *(static_cast<unsigned long*>(ValuePtr)) );
	}
}


GlobalVcQueueCallback::~GlobalVcQueueCallback()
{
}
