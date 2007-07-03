#include "PmFilter.hh"
#include "Common/Asaac/TimeInterval.hh"

PmFilter::PmFilter() : m_Consumer(0), m_Configuration(0),  m_Queue(0)
{
	initialize();
}


PmFilter::PmFilter( VcMessageConsumer& Consumer, PCSConfiguration& Configuration, Queue& q) :
	m_Consumer( &Consumer ), m_Configuration( &Configuration ),  m_Queue(&q)
{
	
	initialize();
}


PmFilter::~PmFilter()
{
	
}

void PmFilter::setQueue(Queue& q)
{
	m_Queue = &q;	
}

void PmFilter::initialize()
{
	
	
}

PmFilter::Queue::Queue() : m_NextFreeQueue(0), m_NextMessage(0)
{
		
	ASAAC_CharacterSequence nqSemName = CharacterSequence("NqSemaphore").asaac_str();
	
	ASAAC_ResourceReturnStatus semCreateStatus = ASAAC_APOS_createSemaphore(&nqSemName, &m_nqSem, 1, 1, ASAAC_QUEUING_DISCIPLINE_PRIORITY);

	if(semCreateStatus == ASAAC_RS_ERROR)
	{
		cerr << "PmFilter::Queue::Queue() could not create enqueueMessage semaphore " << endl;
		throw PCSException( 0, 0, "Could not create enqueueMessage semaphore" );
	}
	
	ASAAC_CharacterSequence eventName = CharacterSequence("EnqueueEvent").asaac_str();
	ASAAC_ResourceReturnStatus eventCreateStatus = ASAAC_APOS_createEvent(&eventName, &m_nqEvent);
	
	if(eventCreateStatus == ASAAC_RS_ERROR)
	{
		cerr << "PmFilter::Queue::Queue() could not create enqueue event " << endl;
		throw PCSException( 0, 0, "Could not create Enqueue Event" );
	}
	
}

VcMessageConsumer* PmFilter::getConsumer()
{
	return m_Consumer;	
};

ASAAC_ReturnStatus PmFilter::Queue::addMapping(ASAAC_PublicId VcId, PmFilter* Source)
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping; ++ Index )
	{
		if ( m_PmFilterMapping[ Index ].VcId == VcId )
		{
			return ASAAC_ERROR;
		}
	}
	
	m_PmFilterMapping[ m_NextFreeMapping ].VcId = VcId;
	m_PmFilterMapping[ m_NextFreeMapping ].Source = Source;
	
	 ++m_NextFreeMapping;
	
	return ASAAC_SUCCESS;
};

ASAAC_ReturnStatus PmFilter::Queue::removeMapping(ASAAC_PublicId VcId)
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping;  ++Index )
	{
		if ( m_PmFilterMapping[ Index ].VcId == VcId )
		{
			m_PmFilterMapping[ Index ] = m_PmFilterMapping[ m_NextFreeMapping - 1 ];
			 --m_NextFreeMapping;
			
			return ASAAC_SUCCESS;
		}
	}
	
	return ASAAC_ERROR;	
};

PmFilter* PmFilter::Queue::getMapping(ASAAC_PublicId VcId)
{
	for ( unsigned long Index = 0; Index < m_NextFreeMapping;  ++Index )
	{
		if ( m_PmFilterMapping[ Index ].VcId == VcId )
		{
			return m_PmFilterMapping[ Index ].Source;
		}
	}
	
	return 0;
};

ASAAC_ReturnStatus PmFilter::Queue::putMessage(ASAAC_PublicId VcId, ASAAC_Address Data, unsigned long Length)
{
	
#ifdef _DEBUG_       
	cout << "PmFilter::Queue::putMessage(" << VcId<< ","<< Data<< "," << Length << ") "<< endl; fflush(stdout);
#endif
	
	PmFilter* pmFilter = getMapping(VcId);
	
	if(pmFilter == 0)
	{
		cerr << "PmFilter::Queue::putMessage() there is no PmFilter mapped for VC " << VcId << ", so the data cannot be processed further" <<endl;
		return ASAAC_ERROR;
	}	
	
	VcMessageConsumer* consumer = pmFilter->getConsumer();
	
#ifdef _DEBUG_       
	cout << "PmFilter::Queue::putMessage() consumer" << (void*) consumer << " processes message " << Data << " for VC " << VcId << endl; fflush(stdout);
#endif
	return consumer->processVcMessage(VcId, Data, Length);
};

ASAAC_TimedReturnStatus PmFilter::Queue::getMessage(ASAAC_PublicId& VcId, ASAAC_TimeInterval& interval, ASAAC_Address& Data, unsigned long& Length )
{
	
#ifdef _DEBUG_       
	cout << "PmFilter::Queue::getMessage() next message : " << m_NextMessage << " next free q : " << m_NextFreeQueue << endl; fflush(stdout);
#endif

	ASAAC_ReturnStatus eventResetStatus = ASAAC_APOS_resetEvent(m_nqEvent);
	
	if(eventResetStatus == ASAAC_ERROR)
	{
		cerr << "PmFilter::Queue::getMessage() could not reset Enqueue Event " << endl;
		return ASAAC_TM_ERROR;
	}
	
	if(m_NextFreeQueue == m_NextMessage) //no messages are enqueued to be released as PM data
	{
#ifdef _DEBUG_       
		cout << "PmFilter::Queue::getMessage() wait for next message to be enqueued" << endl; fflush(stdout);
#endif	
		ASAAC_TimedReturnStatus eventWaitStatus = ASAAC_APOS_waitForEvent(m_nqEvent, &interval);
		if(eventWaitStatus == ASAAC_TM_ERROR)
		{
			cerr << "PmFilter::Queue::getMessage() could not wait for Enqueue Event " << endl;
			return ASAAC_TM_ERROR;
		}
		else if(eventWaitStatus == ASAAC_TM_TIMEOUT)
		{
			return ASAAC_TM_TIMEOUT;	
		}
		else//enqueue event occurred inside time interval
		{
			if(m_NextFreeQueue == m_NextMessage)
			{
				cerr << "PmFilter::Queue::getMessage() there should be at least one message enqueued after the event arised " << endl;
				return ASAAC_TM_ERROR;
			}
			else
			{
				#ifdef _DEBUG_       
				cout << "PmFilter::Queue::getMessage() a message has been enqueued" << endl; fflush(stdout);
				#endif	
			}
		
		}
	}
		
	Message* ThisMessage = &m_QueuedMessages[ m_NextMessage ];
	
	VcId = 	ThisMessage->VcId;
	Data = ThisMessage->Data;
	Length = ThisMessage->Length;
	
	m_NextMessage = (m_NextMessage + 1) % PCS_MAX_SIZE_OF_PMMESSAGEQUEUE;

#ifdef _DEBUG_       
	cout << "PmFilter::Queue::getMessage(" << VcId<< ","<< Data<< "," << Length << ") returns with success" << endl; fflush(stdout);
#endif	
		
	return ASAAC_TM_SUCCESS;
};

ASAAC_ReturnStatus PmFilter::processVcMessage(ASAAC_PublicId GlobalVc, ASAAC_Address Data, unsigned long Length )
{

#ifdef _DEBUG_       
	cout << "PmFilter::processVcMessage(" << GlobalVc<< ","<< Data<< "," << Length << ")" << endl; fflush(stdout);
#endif	

	ASAAC_VcDescription VcDesc;
	
	if ( m_Configuration->getVcDescription( GlobalVc, VcDesc ) != ASAAC_SUCCESS )
	{
		//throw PCSException( 0, GlobalVc, "No VcDescription found" );
		cerr << "PmFilter::processVcMessage(" << GlobalVc << ",...) No VcDescription found " << endl;
		return ASAAC_ERROR;
	}
	
	if ( VcDesc.security_info == ASAAC_Marked )
	{
	// If security rating of Vc exceeds that of Tc, encrypt/authenticate or do the like
		return m_Queue->enqueueMessage(GlobalVc, Data, Length, this);
	}
	
	return m_Consumer->processVcMessage(GlobalVc, Data, Length );
}


ASAAC_ReturnStatus PmFilter::Queue::enqueueMessage(ASAAC_PublicId VcId, ASAAC_Address Data, unsigned long Length , PmFilter* Source)
{
#ifdef _DEBUG_       
	cout << "PmFilter::Queue::enqueueMessage(" << VcId<< ","<< Data<< "," << Length << ")" << endl; fflush(stdout);
#endif	

	if ( Length > PCS_MAX_SIZE_OF_NWMESSAGE )
	{
		cerr << "PmFilter::Queue::enqueueMessage() cannot queue message due to oversize" << endl;fflush(stdout);
		return ASAAC_ERROR;
	}

#ifdef _DEBUG_       
	cout << "PmFilter::Queue::enqueueMessage() wait for access semaphore" << endl; fflush(stdout);
#endif	
	
	ASAAC_TimedReturnStatus SemWaitResult = ASAAC_APOS_waitForSemaphore( m_nqSem, &TimeIntervalInfinity );
	
	Message* ThisMessage = &m_QueuedMessages[ m_NextFreeQueue ];
	
	if (SemWaitResult == ASAAC_TM_TIMEOUT )
	{
		 cerr << "PmFilter::Queue::enqueueMessage() this is quite impossible to happen" << endl; fflush(stdout);
		 return ASAAC_ERROR;
	}
	else if (SemWaitResult == ASAAC_TM_ERROR   )
	{
		 cerr << "PmFilter::Queue::enqueueMessage() error while waiting for access semaphore" << endl; fflush(stdout);
		 return ASAAC_ERROR;
	}
	
	if(addMapping(VcId, Source) == ASAAC_SUCCESS)
	{
#ifdef _DEBUG_       
		cout << "PmFilter::Queue::enqueueMessage() added new mapping for VC " << VcId << " and PmFilter " << Source << endl; fflush(stdout);
#endif	
		
	}
	
	ThisMessage->Length = Length;
	ThisMessage->VcId = VcId;
	memcpy( ThisMessage->Data, Data, Length );
	
	#ifdef _DEBUG_       
	cout << "PmFilter::Queue::enqueueMessage() copy  " <<  Length << " bytes of data to " << (void* ) ThisMessage->Data << endl; fflush(stdout);
	#endif	
	//cout << "PM Queue at " << m_NextFreeQueue << endl;
	m_NextFreeQueue = (m_NextFreeQueue + 1) % PCS_MAX_SIZE_OF_PMMESSAGEQUEUE;
	
	ASAAC_APOS_setEvent(m_nqEvent);
	
	#ifdef _DEBUG_       
	cout << "PmFilter::Queue::enqueueMessage() returns with success next free q: " <<  m_NextFreeQueue << endl; fflush(stdout);
	#endif	
	
	ASAAC_APOS_postSemaphore( m_nqSem );
	
	return ASAAC_SUCCESS;
}


void PmFilter::setOutputConsumer( VcMessageConsumer& Consumer )
{
	m_Consumer = &Consumer;
}


void PmFilter::setConfiguration( PCSConfiguration& Configuration )
{
	m_Configuration = &Configuration;
}


