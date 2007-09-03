#include "TcRateLimiter.hh"

#include "Operators/TimeOperators.hh"

#include "IPC/SemaphoreProtectedScope.hh"

#include "Common/NameGenerator.hh"

#include "PcsCIncludes.hh"


TcRateLimiter::TcRateLimiter()
{
}


void TcRateLimiter::initialize()
{
	ASAAC_CharacterSequence ConfigurationChangedEventName = CharSeq("PCS_RateLimiter_ConfigurationChangedEvent").asaac_str();
	ASAAC_CharacterSequence MessageProcessedEventName = CharSeq("PCS_RateLimiter_MessageProcessedEvent").asaac_str();
	
	if (ASAAC_APOS_createEvent( &ConfigurationChangedEventName, &m_ConfigurationChangedEvent ) != ASAAC_RS_SUCCESS)
		throw PcsException(0,0,"ConfigurationChangedEvent couldn't be created");
	
	if (ASAAC_APOS_createEvent( &MessageProcessedEventName, &m_MessageProcessedEvent ) != ASAAC_RS_SUCCESS)
		throw PcsException(0,0,"MessageProcessedEvent couldn't be created");

	ASAAC_APOS_resetEvent( m_ConfigurationChangedEvent );
	ASAAC_APOS_resetEvent( m_MessageProcessedEvent );
	
	for ( unsigned long Index = 0; Index < PCS_MAX_NUMBER_OF_TCS;  ++Index )
		m_Queues[Index].tc_id = OS_UNUSED_ID;

	m_Consumer = NULL;
	m_Configuration = NULL;
}


void TcRateLimiter::deinitialize()
{
	ASAAC_APOS_setEvent( m_ConfigurationChangedEvent );
	ASAAC_APOS_deleteEvent( m_ConfigurationChangedEvent );

	ASAAC_APOS_deleteEvent( m_MessageProcessedEvent );
}


void TcRateLimiter::setOutputConsumer( TcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void TcRateLimiter::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}


ASAAC_ReturnStatus TcRateLimiter::processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
	cout << "TcRateLimiter::processTcMessage(" << TcId<< ", " << Data << ", " << Length << ")" << endl;fflush(stdout);
#endif	

	if ( m_Consumer == 0 )
	{
		throw PcsException( TcId, 0, "No OutputConsumer set." );
		return ASAAC_ERROR;
	}
	
	unsigned long q = 0;
	
	if ( getQueue( TcId, q ) == ASAAC_ERROR )
	{
#ifdef _DEBUG_  
		cout << "TcRateLimiter::processTcMessage() process unrated message " << endl; fflush(stdout);
#endif	
		return m_Consumer->processTcMessage( TcId, Data, Length );
	}
	
	if ( ( TimeStamp::Now() >= TimeStamp(m_Queues[q].time) ) && 
		 ( m_Queues[q].counter == 0 ) ) //we are allowed to send, and queue is empty
	{
#ifdef _DEBUG_  
		cout << "TcRateLimiter::processTcMessage() process rated message " << endl; fflush(stdout);
#endif	
		setNextMessageTime(q);
		
		return m_Consumer->processTcMessage( TcId, Data, Length );
	}

#ifdef _DEBUG_       
	cout << "TcRateLimiter::processTcMessage() enqueueMessage()" << endl;
#endif	
	
	return enqueueMessage( TcId, Data, Length );
}


ASAAC_ReturnStatus TcRateLimiter::setNextMessageTime(unsigned long index)
{
#ifdef _DEBUG_       
	cout << "TcRateLimiter::setNextMessageTime()" << endl;
	cout << "TcRateLimiter::setNextMessageTime() m_Queues[" << index << "].time: " << CharSeq(m_Queues[index].time) << endl;
	cout << "TcRateLimiter::setNextMessageTime() m_Queues[" << index << "].rate: " << m_Queues[index].rate.sec << ":" << m_Queues[index].rate.nsec << endl;
#endif	

	m_Queues[index].time = (TimeStamp(m_Queues[index].time) + m_Queues[index].rate).asaac_Time();
	
	if ( TimeStamp(m_Queues[index].time) < TimeStamp::Now() )
		m_Queues[index].time = TimeStamp::Now().asaac_Time();

#ifdef _DEBUG_       
	cout << "TcRateLimiter::setNextMessageTime() m_Queues[" << index << "].time: " << CharSeq(m_Queues[index].time) << endl;
#endif	
	
	ASAAC_APOS_setEvent( m_ConfigurationChangedEvent );
	
	return ASAAC_SUCCESS;
}


long TcRateLimiter::getNextMessageIndex()
{
	ASAAC_Time ClosestTime = TimeInfinity; 
	long Result = -1;
	
	for ( long Index = 0; Index < PCS_MAX_NUMBER_OF_TCS;  ++Index )
	{
		if ( (m_Queues[Index].tc_id != OS_UNUSED_ID) &&
			 (m_Queues[Index].counter != 0) &&
			 (TimeStamp(m_Queues[Index].time) < TimeStamp(ClosestTime)) )
		{
			ClosestTime = m_Queues[Index].time;
			Result = Index; 
		}		
	}
	
	return Result;
}


ASAAC_ReturnStatus TcRateLimiter::setRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& MessageRate )
{

	for ( unsigned long Index = 0; Index < PCS_MAX_NUMBER_OF_TCS;  Index++ )
	{
		if ( m_Queues[Index].tc_id == OS_UNUSED_ID )
		{
			m_Queues[Index].tc_id         = TcId;
			m_Queues[Index].counter       = 0;
			m_Queues[Index].position      = 0;
			m_Queues[Index].free_position = 0;
			m_Queues[Index].time          = TimeZero;
			m_Queues[Index].rate          = MessageRate;
			
			return ASAAC_SUCCESS;
		}
	}

	ASAAC_APOS_setEvent( m_ConfigurationChangedEvent );

	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::getQueue(ASAAC_PublicId TcId, unsigned long& index)
{
	for ( index = 0; index < PCS_MAX_NUMBER_OF_TCS; index++ )
	{
		if ( m_Queues[ index ].tc_id == TcId ) 
			return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::removeRateLimit( ASAAC_PublicId TcId )
{
	unsigned long Index = 0;
			
	if(getQueue(TcId, Index) == ASAAC_SUCCESS)
	{
		m_Queues[Index].tc_id = OS_UNUSED_ID;	
		return ASAAC_SUCCESS;
	}

	ASAAC_APOS_setEvent( m_ConfigurationChangedEvent );

	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::getRateLimit( ASAAC_PublicId TcId , ASAAC_TimeInterval& MessageRate )
{
	unsigned long q = 0;
			
	if( getQueue(TcId, q) == ASAAC_SUCCESS )
	{
		MessageRate = m_Queues[q].rate;	
		return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::enqueueMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
	cout << "TcRateLimiter::enqueueMessage(" << TcId << ", " << Data << ", " << Length << ")" << endl; fflush(stdout);
#endif	

	if ( Length > PCS_MAX_SIZE_OF_NWMESSAGE )
	{
		cerr << "TcRateLimiter::enqueueMessage() cannot queue message due to oversize" << endl;fflush(stdout);
		return ASAAC_ERROR;
	}
	
	unsigned long Index = 0;
	
	if ( getQueue( TcId, Index ) == ASAAC_ERROR )
	{
		cerr << "TcRateLimiter::enqueueMessage() cannot enqueue message because it is not rate limited" << endl;fflush(stdout);
		return ASAAC_ERROR;
	}
	
	while ( m_Queues[Index].counter == PCS_MAX_SIZE_OF_MESSAGEQUEUE )
	{
#ifdef _DEBUG_       
		cout << "TcRateLimiter::enqueueMessage() - Waiting for a free buffer." << endl; fflush(stdout);
#endif	
		
		ASAAC_APOS_waitForEvent( m_MessageProcessedEvent, &TimeIntervalInfinity );
		ASAAC_APOS_resetEvent( m_MessageProcessedEvent );
	}
	
	Message* ThisMessage = &m_Queues[Index].buffer[ m_Queues[Index].free_position ];
		
	ThisMessage->Length = Length;
	memcpy( ThisMessage->Data, Data, Length );
	
	m_Queues[Index].free_position = (m_Queues[Index].free_position + 1) % PCS_MAX_SIZE_OF_MESSAGEQUEUE;
	m_Queues[Index].counter++;

	ASAAC_APOS_setEvent( m_ConfigurationChangedEvent );

	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus TcRateLimiter::processNextMessage()
{		
#ifdef _DEBUG_       
	cout << "TcRateLimiter::processNextMessage()" << endl; fflush(stdout);
#endif	

	long Index = -1;
	ASAAC_TimedReturnStatus ConfigurationChangedStatus = ASAAC_TM_SUCCESS;

	ASAAC_APOS_resetEvent( m_ConfigurationChangedEvent );
	
	ASAAC_ReturnStatus Result = ASAAC_ERROR;
	
	do
	{
		ASAAC_TimeInterval WaitInterval;
		Index = getNextMessageIndex();
	
		if (Index == -1)
			WaitInterval = TimeIntervalInfinity;
		else WaitInterval = TimeStamp(m_Queues[Index].time).asaac_Interval();
		
#ifdef _DEBUG_       
		cout << "TcRateLimiter::processNextMessage() Index: " << Index << endl;		
		cout << "TcRateLimiter::processNextMessage() Time: " << CharSeq(m_Queues[Index].time) << endl;		
		cout << "TcRateLimiter::processNextMessage() WaitInterval: " << WaitInterval.sec << ":" << WaitInterval.nsec << endl;
#endif	
		
		ConfigurationChangedStatus = ASAAC_APOS_waitForEvent( m_ConfigurationChangedEvent, &WaitInterval );
		
#ifdef _DEBUG_       
		cout << "TcRateLimiter::ConfigurationChangedStatus: " << ConfigurationChangedStatus << endl;
#endif	
		
		ASAAC_APOS_resetEvent( m_ConfigurationChangedEvent );
	}
	while (( ConfigurationChangedStatus == ASAAC_TM_SUCCESS ) || ( Index == -1 ));
	
	if ( ConfigurationChangedStatus == ASAAC_TM_ERROR )
		throw PcsException( m_Queues[ Index ].tc_id, 0, "Error while waiting for next message" );
			
	if ( m_Consumer == NULL )
		throw PcsException( m_Queues[ Index ].tc_id, 0, "A consumer is not configured" );

#ifdef _DEBUG_       
	cout << "TcRateLimiter::processNextMessage() m_Queues[" << Index << "].counter:       " << m_Queues[Index].counter << endl; 
	cout << "TcRateLimiter::processNextMessage() m_Queues[" << Index << "].free_position: " << m_Queues[Index].free_position << endl; 
	cout << "TcRateLimiter::processNextMessage() m_Queues[" << Index << "].position:      " << m_Queues[Index].position << endl; 
#endif

	if ( m_Queues[Index].counter > 0 ) //Message Queue is empty
	{
		Message* nextMessage = &m_Queues[Index].buffer[ m_Queues[Index].position ];
		Result = m_Consumer->processTcMessage( m_Queues[ Index ].tc_id, nextMessage->Data, nextMessage->Length );
		
		m_Queues[Index].position = ( m_Queues[Index].position + 1 ) % PCS_MAX_SIZE_OF_MESSAGEQUEUE;
		m_Queues[Index].counter--;
	}
	
	setNextMessageTime( Index );
	
	ASAAC_APOS_setEvent( m_MessageProcessedEvent );
		
	return Result;
}
