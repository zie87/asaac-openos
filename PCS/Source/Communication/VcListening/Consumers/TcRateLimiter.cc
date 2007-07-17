#include "TcRateLimiter.hh"

#include "Operators/TimeOperators.hh"

#include "IPC/SemaphoreProtectedScope.hh"

#include "Common/NameGenerator.hh"

#include "PcsCIncludes.hh"


TcRateLimiter::TcRateLimiter()
{
}


TcRateLimiter::~TcRateLimiter()
{
}

void TcRateLimiter::initialize()
{
	ASAAC_CharacterSequence ConfigurationChangedEventName = CharSeq("PCS_RateLimiter_ConfigurationChangedEvent").asaac_str();
	
	if (ASAAC_APOS_createEvent( &ConfigurationChangedEventName, &m_ConfigurationChangedEvent ) != ASAAC_RS_SUCCESS)
		throw PcsException(0,0,"ConfigurationChangedEvent couldn't be created");
	
	ASAAC_Time zeroTime = {0,0};
	
	for ( unsigned long IDX = 0; IDX < PCS_MAX_NUMBER_OF_TCS;  ++IDX )
	{
		m_NextFreeQueue[IDX] = 0;
		m_NextMessage[IDX] = 0;
		m_TcQueueMap[IDX] = 0; //no queue is assigned to a TC at startup
		m_NextMessageTime[IDX] = zeroTime;
		
		for ( unsigned long MSG = 0; MSG < PCS_MAX_SIZE_OF_MESSAGEQUEUE;  ++MSG )
		{
			m_QueuedMessages[IDX][ MSG ].Length = 0;
		}
	}

	m_Consumer = 0;
	m_Configuration = 0;
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
	cout << "TcRateLimiter::processTcMessage(" << TcId<< "," << Data << "," << Length << ")" << endl;fflush(stdout);
#endif	

	if ( m_Consumer == 0 )
	{
		throw PcsException( TcId, 0, "No OutputConsumer set." );
		return ASAAC_ERROR;
	}
	
	ASAAC_Time Now;
	
	unsigned long q = 0;
	
	if ( getQueue(TcId,q) != ASAAC_SUCCESS)
	{
#ifdef _DEBUG_  
		cout << "TcRateLimiter::processTcMessage() process unrated message " << endl;fflush(stdout);
#endif	
		return m_Consumer->processTcMessage( TcId, Data, Length );
	}
	
	ASAAC_APOS_getAbsoluteLocalTime( &Now );

	if(Now > m_NextMessageTime[q] && m_NextFreeQueue[q] == m_NextMessage[q]) //we are allowed to send, and queue is empty
	{
#ifdef _DEBUG_  
		cout << "TcRateLimiter::processTcMessage() process rated message " << endl;fflush(stdout);
#endif	
		setNextMessageTime(q,Now);
		return m_Consumer->processTcMessage( TcId, Data, Length );
	}

#ifdef _DEBUG_       
	cout << "TcRateLimiter::processTcMessage() enqueueMessage()" << endl;
#endif	
	return enqueueMessage( TcId, Data, Length );
}

ASAAC_ReturnStatus TcRateLimiter::setNextMessageTime(unsigned long queue, ASAAC_Time& now)
{
	m_NextMessageTime[queue].sec = now.sec + m_MessageRate[queue].sec;

	m_NextMessageTime[queue].nsec = now.nsec + m_MessageRate[queue].nsec;
	
	if(m_NextMessageTime[queue].nsec > 1000000000)
	{
		m_NextMessageTime[queue].nsec -= 1000000000;
		m_NextMessageTime[queue].sec += 1;
	}
	
	ASAAC_APOS_setEvent( m_ConfigurationChangedEvent );
	
	return ASAAC_SUCCESS;
}

ASAAC_Time TcRateLimiter::getNextMessageTime()
{
	ASAAC_Time Result = TimeInfinity; 

	for ( unsigned long IDX = 0; IDX < PCS_MAX_NUMBER_OF_TCS;  ++IDX )
	{
		if ( (m_TcQueueMap[IDX] != 0) &&
			 (TimeStamp(m_NextMessageTime[IDX]) < TimeStamp(Result)) )
		{
			Result = m_NextMessageTime[IDX]; 
		}		
	}
	
	return Result;
}

ASAAC_ReturnStatus TcRateLimiter::setRateLimit( ASAAC_PublicId TcId, const ASAAC_TimeInterval& MessageRate )
{

	for ( unsigned long q = 0; q < PCS_MAX_NUMBER_OF_TCS;  ++q )
	{
		if(m_TcQueueMap[q] == 0)
		{
			m_TcQueueMap[q] = TcId;
			m_MessageRate[q] = MessageRate;
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;
}

ASAAC_ReturnStatus TcRateLimiter::getQueue(ASAAC_PublicId TcId, unsigned long& index)
{
	for ( index = 0; index < PCS_MAX_NUMBER_OF_TCS; ++index)
	{
		if ( m_TcQueueMap[ index ] == TcId ) return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::removeRateLimit( ASAAC_PublicId TcId )
{
	unsigned long q = 0;
			
	if(getQueue(TcId,q) == ASAAC_SUCCESS)
	{
		m_TcQueueMap[q] = 0;	
		return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::getRateLimit( ASAAC_PublicId TcId , ASAAC_TimeInterval& MessageRate )
{
	unsigned long q = 0;
			
	if(getQueue(TcId,q) == ASAAC_SUCCESS)
	{
		MessageRate = m_MessageRate[q];	
		return ASAAC_SUCCESS;
	}
	
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus TcRateLimiter::enqueueMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
	cout << "TcRateLimiter::enqueueMessage(" << TcId<< "," << Data<< "," << Length << ")" << endl; fflush(stdout);
#endif	

	if ( Length > PCS_MAX_SIZE_OF_NWMESSAGE )
	{
		cerr << "TcRateLimiter::enqueueMessage() cannot queue message due to oversize" << endl;fflush(stdout);
		return ASAAC_ERROR;
	}
	
	unsigned long q = 0;
	
	if(getQueue(TcId,q) != ASAAC_SUCCESS)
	{
		cerr << "TcRateLimiter::enqueueMessage() cannot enqueue message because it is not rate limited" << endl;fflush(stdout);
		return ASAAC_ERROR;
	}
	
	
	ASAAC_Time Now;
	
	ASAAC_APOS_getAbsoluteLocalTime( &Now );
	
	Message* ThisMessage = &m_QueuedMessages[q][ m_NextFreeQueue[q] ];
		
	ThisMessage->Length = Length;
	
	memcpy( ThisMessage->Data, Data, Length );
	
	m_NextFreeQueue[q] = (m_NextFreeQueue[q] + 1) % PCS_MAX_SIZE_OF_MESSAGEQUEUE;
	
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus TcRateLimiter::processNextMessage()
{
	
	if ( m_Consumer == 0 )
	{
		return ASAAC_ERROR;
	}
	
	ASAAC_Time Now;
	
	ASAAC_ReturnStatus Result;
	ASAAC_ReturnStatus ret = ASAAC_ERROR;
	
	for ( unsigned long q = 0; q < PCS_MAX_NUMBER_OF_TCS; ++q)
	{
		if ( m_TcQueueMap[ q ] != 0 )
		{
			ASAAC_TimedReturnStatus ConfigurationChangedStatus = ASAAC_TM_SUCCESS;
			
			do
			{
				ASAAC_TimeInterval Interval = TimeStamp(getNextMessageTime()).asaac_Interval();
				ConfigurationChangedStatus = ASAAC_APOS_waitForEvent( m_ConfigurationChangedEvent, &Interval );
			}
			while ( ConfigurationChangedStatus == ASAAC_TM_SUCCESS );
			
			if ( ConfigurationChangedStatus == ASAAC_TM_ERROR )
				throw PcsException(0,0,"Error while waiting for next message");
			
			ASAAC_APOS_getAbsoluteLocalTime( &Now );
			
			if ( m_NextFreeQueue[q] != m_NextMessage[q] )
			{
				Message* nextMessage = &m_QueuedMessages[q][ m_NextMessage[q]  ];
				Result = m_Consumer->processTcMessage( m_TcQueueMap[ q ], nextMessage->Data, nextMessage->Length );
				
				if(Result == ASAAC_SUCCESS)
				{
					setNextMessageTime( q, Now );
					
					m_NextMessage[q] = ( m_NextMessage[q] + 1 ) % PCS_MAX_SIZE_OF_MESSAGEQUEUE;
					
					#ifdef _DEBUG_       
					cout << "TcRateLimiter::processNextMessage() processed enqueued message" << endl;fflush(stdout);
					#endif
					
					ret = ASAAC_SUCCESS;
				}
				else
				{
					cerr << "TcRateLimiter::processNextMessage() unable to process enqueued message" << endl;fflush(stdout);
				}
			}
			
		}
	}
		
	return ret;
}
