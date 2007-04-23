#ifndef EVENTTABLE_HH_
#define EVENTTABLE_HH_

#include "OpenOSIncludes.hh"

#include "Common/Shared.hh"

//! class for handling a multitude of Events

class EventTable
{

public:
	struct EventData {
		oal_thread_mutex_t	    Mutex;
		oal_thread_cond_t 		Condition;
		oal_thread_condattr_t	ConditionAttribute;
		unsigned int		WaitingThreads;
		unsigned int		Size;
	};


	EventTable( Allocator* ThisAllocator, bool IsMaster = true, unsigned int Size = 1 );
	EventTable( );

	void initialize( Allocator* ThisAllocator, bool IsMaster = true, unsigned int Size = 1 );

	virtual ~EventTable();
	
	void deinitialize();

	void setEvent( unsigned int TableIndex );
	void resetEvent( unsigned int TableIndex );

	
	ASAAC_TimedReturnStatus waitForEvent( unsigned int TableIndex, const ASAAC_Time& Timeout = TimeInfinity );
	ASAAC_TimedReturnStatus waitForEventReset( unsigned int TableIndex, const ASAAC_Time& Timeout = TimeInfinity );
	
	ASAAC_TimedReturnStatus waitForMultipleEvents( unsigned int TableSize, unsigned int* Table, unsigned int MinEvents, const ASAAC_Time& Timeout = TimeInfinity );
	
	unsigned int getWaitCount(); // Return number of waiting clients
	unsigned int getTableSize(); // return size of table
	
	static size_t predictSize( unsigned int TableSize );

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

	ASAAC_TimedReturnStatus waitForEventStatus( unsigned int TableIndex, bool Status, const ASAAC_Time& Timeout );
	void setEventStatus( unsigned int TableIndex, bool Status );
	
};


#endif /*EVENTTABLE_HH_*/
