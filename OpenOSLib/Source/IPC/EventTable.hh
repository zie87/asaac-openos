#ifndef EVENTTABLE_HH_
#define EVENTTABLE_HH_

#include "OpenOSIncludes.hh"

#include "Common/Templates/Shared.hh"

//! class for handling a multitude of Events

class EventTable
{

public:
	struct EventData {
		oal_thread_mutex_t	    Mutex;
		oal_thread_cond_t 		Condition;
		oal_thread_condattr_t	ConditionAttribute;
		unsigned long		WaitingThreads;
		unsigned long		Size;
	};


	EventTable( Allocator* ThisAllocator, bool IsMaster = true, unsigned long Size = 1 );
	EventTable( );

	void initialize( Allocator* ThisAllocator, bool IsMaster = true, unsigned long Size = 1 );

	virtual ~EventTable();
	
	void deinitialize();

	void setEvent( unsigned long TableIndex );
	void resetEvent( unsigned long TableIndex );

	
	ASAAC_TimedReturnStatus waitForEvent( unsigned long TableIndex, const ASAAC_Time& Timeout = TimeInfinity );
	ASAAC_TimedReturnStatus waitForEventReset( unsigned long TableIndex, const ASAAC_Time& Timeout = TimeInfinity );
	
	ASAAC_TimedReturnStatus waitForMultipleEvents( unsigned long TableSize, unsigned long* Table, unsigned long MinEvents, const ASAAC_Time& Timeout = TimeInfinity );
	
	unsigned long getWaitCount(); // Return number of waiting clients
	unsigned long getTableSize(); // return size of table
	
	static size_t predictSize( unsigned long TableSize );

	void* operator new( size_t size );
	void operator delete( void* location );

private:
	// prevent implicit copying and assignment
	EventTable( const EventTable& Source );
	EventTable& operator=( const EventTable& Source );

	bool m_IsMaster;
	bool m_IsInitialized;
	Shared<EventData> m_Global;
	Shared<bool> m_EventTable;

	ASAAC_TimedReturnStatus waitForEventStatus( unsigned long TableIndex, bool Status, const ASAAC_Time& Timeout );
	void setEventStatus( unsigned long TableIndex, bool Status );
	
};


#endif /*EVENTTABLE_HH_*/
