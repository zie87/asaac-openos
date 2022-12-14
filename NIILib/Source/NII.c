#include "NII.h"

/***********************************************************************************/
/*                          DATA DEFINITIONS                                       */      
/***********************************************************************************/

Network 			m_NwList[NII_MAX_NUMBER_OF_NETWORKS]; 
long				m_NwListSize = 0;

TransferConnection  m_TcList[NII_MAX_NUMBER_OF_TCS];
long				m_TcListSize = 0;

TcPacketData    	m_TcBufferArray[NII_MAX_NUMBER_OF_TCS];
char				m_TcBufferArrayAllocated[NII_MAX_NUMBER_OF_TCS];

pthread_t       	m_ServiceThread;
int					m_ServiceFileDescriptor;

pthread_cond_t		m_ServiceThreadCondition;
pthread_mutex_t		m_ServiceThreadMutex;

pthread_cond_t		m_NewDataCondition;
pthread_mutex_t		m_NewDataMutex;
long				m_NewDataTcId = NII_UNUSED_ID;


/***********************************************************************************/
/*                              DATA ACCESS                                        */      
/***********************************************************************************/

void lockAccess();
void releaseAccess();

// Service synchronization
ASAAC_NiiReturnStatus hasTcData( const ASAAC_PublicId tc_id );
ASAAC_NiiReturnStatus hasNetworkData( const ASAAC_NetworkDescriptor network_id, ASAAC_PublicId *tc_id );
ASAAC_NiiReturnStatus waitForData( ASAAC_Time time_out, ASAAC_PublicId *tc_id  );
ASAAC_NiiReturnStatus receiveData( 		
		const ASAAC_PublicId tc_id, 
		const ASAAC_CharAddress receive_data, 
		const unsigned long data_length_available, 
		unsigned long *data_length);


// Network data access
char addNw(Network Data);
char removeNw(const ASAAC_PublicId port);
void removeAllNw();

char getNw(const ASAAC_PublicId port, Network *Data);
char setNw(const ASAAC_PublicId port, const Network Data);

long countNws();

// Transfer Connection data access
char addTc(TransferConnection Data);
char removeTc(const ASAAC_PublicId tc_id);
void removeAllTc();

char getTc(const ASAAC_PublicId tc_id, TransferConnection *Data);
char setTc(const ASAAC_PublicId tc_id, const TransferConnection Data);

long countTcs();
long countTcsByPort(const ASAAC_PublicId port);

// Buffer handling
char allocateBuffer(ASAAC_PrivateId *buffer_id);
void releaseBuffer(ASAAC_PrivateId buffer_id);

TcPacketData *getBuffer(ASAAC_PrivateId buffer_id);

long getIndexOfNw(const ASAAC_PublicId port);
long getIndexOfTc(const ASAAC_PublicId tc_id);

ASAAC_NiiReturnStatus receiveFromNetwork( const int fd, ASAAC_NetworkDescriptor *network_id, ASAAC_PublicId *buffer_id );
ASAAC_NiiReturnStatus sendToNetwork( const int fd, const ASAAC_NetworkDescriptor network_id, const ASAAC_PublicId tc_id, const char* data, const unsigned long length, ASAAC_TimeInterval time_out );


/***********************************************************************************/
/*                              SERVICES                                           */      
/***********************************************************************************/

void handleStreamingTc(const ASAAC_PublicId port, ASAAC_PublicId *tc_id);
void handleMessageTc(const ASAAC_PublicId port, ASAAC_PublicId *tc_id);

static void* ServiceThread(void* Data);

void configureServices();


/***********************************************************************************/
/*                           INITIALIZATION                                        */      
/***********************************************************************************/

void initialize()
{
	static char Initialized = 0;
	
	if (Initialized == 1)
		return;
	
	Initialized = 1;
	
	long Index;
	for (Index = 0; Index < NII_MAX_NUMBER_OF_TCS; Index++ )
		m_TcBufferArrayAllocated[Index] = 0;
	                         
	pthread_condattr_t CondAttr;
	pthread_condattr_init( &CondAttr );
	pthread_condattr_setpshared( &CondAttr, PTHREAD_PROCESS_PRIVATE );

	pthread_mutexattr_t MutexAttr;
	pthread_mutexattr_init( &MutexAttr );    
	pthread_mutexattr_setpshared( &MutexAttr, PTHREAD_PROCESS_PRIVATE );
	
	if (pthread_cond_init( &m_ServiceThreadCondition, &CondAttr ) == -1)
		perror("pthread_cond_init: ");
	
	if (pthread_mutex_init( &m_ServiceThreadMutex, &MutexAttr ) == -1)
		perror("pthread_mutex_init: ");

	if (pthread_cond_init( &m_NewDataCondition, &CondAttr ) == -1)
		perror("pthread_cond_init: ");

	if (pthread_mutex_init( &m_NewDataMutex, &MutexAttr ) == -1)
		perror("pthread_mutex_init: ");
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(0xffff);
	
	m_ServiceFileDescriptor = socket( AF_INET, SOCK_DGRAM, 0 );
	if (bind( m_ServiceFileDescriptor, (struct sockaddr *)&addr, sizeof(addr) ) == -1)
		perror("bind: ");
	
	if (pthread_create( &m_ServiceThread, NULL, ServiceThread, NULL ) == -1)
		perror("pthread_create: ");
}



/***********************************************************************************/
/*                            NII INTERFACE                                        */      
/***********************************************************************************/

ASAAC_NiiReturnStatus NII_configureInterface(
		const ASAAC_PublicId interface_id,
		const ASAAC_NetworkDescriptor *network_id,
		const ASAAC_InterfaceConfigurationData *configuration_data)
{
	initialize();
	
#ifdef _DEBUG_
	printf("cMosNii::configureInterface with port: %lu \n", network_id->port);
#endif

	if (countTcsByPort(network_id->port) > 0)
	{		
#ifdef _DEBUG_
		printf("cMosNii::configureInterface with port: %lu : ASAAC_MOS_NII_OPEN_TCS\n", network_id->port);
#endif
		return ASAAC_MOS_NII_OPEN_TCS;
	}
	
	if ( interface_id == 0 )
	{
		if ( removeNw( network_id->port ) == 0 )
		{		
#ifdef _DEBUG_
			printf("cMosNii::configureInterface with port: %lu, interface_id == 0: ASAAC_MOS_NII_INVALID_NETWORK\n", network_id->port);
#endif
			return ASAAC_MOS_NII_INVALID_NETWORK;
		}
		
#ifdef _DEBUG_
		printf("cMosNii::configureInterface with port: %lu, interface_id == 0: ASAAC_MOS_NII_CALL_COMPLETE\n", network_id->port);
#endif
		return ASAAC_MOS_NII_CALL_COMPLETE;
	}
	
	if ( network_id->network == 0 )
	{
		if ( countTcs() > 0 )
		{
#ifdef _DEBUG_
			printf("cMosNii::configureInterface with port: %lu, network_id == 0: ASAAC_MOS_NII_OPEN_TCS\n", network_id->port);
#endif
			return ASAAC_MOS_NII_OPEN_TCS;
		}
		
		removeAllNw();
		
#ifdef _DEBUG_
		printf("cMosNii::configureInterface with port: %lu, network_id == 0, ASAAC_MOS_NII_CALL_COMPLETE\n", network_id->port);
#endif
		return ASAAC_MOS_NII_CALL_COMPLETE;
	}
	
	Network Nw;

	Nw.port               = network_id->port;
	Nw.configuration_data = *configuration_data;
	Nw.fd                 = -1;
	Nw.is_streaming       = 0;
	Nw.tc_id_with_data	  = NII_UNUSED_ID;

	if (setNw( network_id->port, Nw ) == 0)
	{
		if ( addNw( Nw ) == 0 )
		{
#ifdef _DEBUG_
			printf("cMosNii::configureInterface with port: %lu ASAAC_MOS_NII_STORAGE_FAULT\n", network_id->port);
#endif
			return ASAAC_MOS_NII_STORAGE_FAULT;
		}
	}

	#ifdef _DEBUG_
	printf("cMosNii::configureInterface with port: %lu, ASAAC_MOS_NII_CALL_COMPLETE\n", network_id->port);
#endif

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


// Configure the local resource to handle the transmission or reception of information over a Transfer Channel (TC)
ASAAC_NiiReturnStatus NII_configureTransfer(
		const ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor network_id, 
		const ASAAC_TransferDirection send_receive,
		const ASAAC_TransferType message_streaming,
		const ASAAC_TC_ConfigurationData configuration_data, 
		const ASAAC_Bool trigger_callback,
		const ASAAC_PublicId callback_id)
{
	initialize();
	
#ifdef _DEBUG_
	printf("cMosNii::configureTransfer(%lu) setup\n", tc_id);
#endif
	
	Network Nw;
	
	if ( getNw(network_id.port, &Nw) == 0 )	
	{
		return ASAAC_MOS_NII_INVALID_NETWORK;
	}

	TransferConnection Tc;

	if ( getTc( tc_id, &Tc ) == 1 )
	{
		return ASAAC_MOS_NII_ALREADY_CONFIGURED;
	}

	Tc.tc_id            = tc_id;
	Tc.network_id       = network_id;
	Tc.direction        = send_receive;
	Tc.type             = message_streaming;
	
	Tc.trigger_callback = trigger_callback;
	Tc.callback_id      = callback_id;
	
	Tc.buffer_id        = NII_UNUSED_ID;

	struct sockaddr_in NetworkAddr;	
	NetworkAddr.sin_family = AF_INET;
	NetworkAddr.sin_addr.s_addr = INADDR_ANY;
	NetworkAddr.sin_port = htons(Nw.port);
	
	switch ( Tc.type )
	{
		case ASAAC_TRANSFER_TYPE_MESSAGE:
		{
			if (Nw.fd != -1)
				break;

			if (Tc.direction == ASAAC_TRANSFER_DIRECTION_SEND)
				break;
			
			Nw.is_streaming = 0;
			Nw.fd = socket( AF_INET, SOCK_DGRAM, 0 );
			
			if (Nw.fd == -1)
			{
				perror("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) : ");
				return ASAAC_MOS_NII_CALL_FAILED;
			}
			
			if ( bind( Nw.fd, (struct sockaddr *)&NetworkAddr, sizeof(NetworkAddr)) == -1 )
			{
				perror("bind() on socket : ");
				close(Nw.fd);
				return ASAAC_MOS_NII_CALL_FAILED;
			}

			//Necessary for sendTransfer()
			int val = fcntl(Nw.fd, F_GETFL, 0);
			fcntl(Nw.fd, F_SETFL, val | O_NONBLOCK);
		}
		break;
		
		case ASAAC_TRANSFER_TYPE_STREAMING:
		{
			if (Nw.fd != -1)
				return ASAAC_MOS_NII_INVALID_CONFIG;
			
			Nw.is_streaming = 1;
			Nw.fd = socket( AF_INET, SOCK_STREAM, 0 );
			
			if ( Nw.fd == -1)
			{
				perror("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) : " );
				return ASAAC_MOS_NII_CALL_FAILED;
			}
			
			if ( bind( Nw.fd, (struct sockaddr *)&NetworkAddr, sizeof(NetworkAddr)) == -1 )
			{
				perror("bind() on socket : ");
				close(Nw.fd);
				return ASAAC_MOS_NII_CALL_FAILED;
			}
		}
		break;
		
		default:
		{
			printf("configureTransfer() unknown TransferType: %d\n", message_streaming);
			return ASAAC_MOS_NII_CALL_FAILED;
		}
		break;
	}

	setNw(network_id.port, Nw);

	// Now the TC can be stored
	if ( addTc( Tc ) == 0 )
		return ASAAC_MOS_NII_STORAGE_FAULT;
	
	configureServices();			

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


// Send a block of data on the given TC
ASAAC_NiiReturnStatus NII_sendTransfer(
		const ASAAC_PublicId tc_id,
		const char* transmit_data, 
		const unsigned long data_length, 
		ASAAC_Time time_out)
{
	initialize();
	
	TransferConnection Tc;
	
	if ( getTc( tc_id, &Tc ) == 0)
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	if (Tc.type != ASAAC_TRANSFER_TYPE_MESSAGE)
	{
#ifdef _DEBUG_
		printf("ERROR: not a message TC \n");
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	if (Tc.direction != ASAAC_TRANSFER_DIRECTION_SEND)
	{
#ifdef _DEBUG_
		printf("ERROR: not a sending TC\n ");
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	Network Nw;
	
	if ( getNw( Tc.network_id.port, &Nw ) == 0)
		return ASAAC_MOS_NII_CALL_FAILED;

	ASAAC_Time now;
	ASAAC_TimeInterval Interval;
	
	ASAAC_MOS_getAbsoluteLocalTime( &now );
	Time_subtractTime( &time_out, &now, &Interval );
	
	Tc.event_info_data.comms_ev_buffer_sent.status = sendToNetwork(m_ServiceFileDescriptor, Tc.network_id, tc_id, transmit_data, data_length, Interval);
	Tc.event_info_data.comms_ev_buffer_sent.tc_id  = Tc.tc_id;

	if ( Tc.trigger_callback == ASAAC_BOOL_TRUE )
		ASAAC_MOS_callbackHandler( ASAAC_COMMS_EV_BUFFER_SEND, Tc.callback_id, &Tc.event_info_data );

	if ( Tc.event_info_data.comms_ev_buffer_sent.status != ASAAC_MOS_NII_CALL_OK )
		return Tc.event_info_data.comms_ev_buffer_sent.status;
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


// Receive a block of data on the given TC
ASAAC_NiiReturnStatus NII_receiveTransfer(
		const ASAAC_PublicId tc_id, 
		ASAAC_CharAddress *receive_data,
		const unsigned long data_length_available, 
		unsigned long *data_length, 
		const ASAAC_Time time_out)
{
	initialize();
	
	ASAAC_NiiReturnStatus Result = hasTcData( tc_id );

	if ( (Result != ASAAC_MOS_NII_CALL_OK) &&
	     (Result != ASAAC_MOS_NII_BUFFER_EMPTY) )
		return Result;
	
	if ( Result ==  ASAAC_MOS_NII_BUFFER_EMPTY )
	{
		ASAAC_PublicId ReceivedTcId;
		
		do
		{
			ASAAC_NiiReturnStatus Result = waitForData( time_out, &ReceivedTcId );
			
			if ( Result != ASAAC_MOS_NII_CALL_OK )
				return Result;
		}	
		while ( ReceivedTcId != tc_id );
	}
	
	Result = receiveData(tc_id, *receive_data, data_length_available, data_length);

	if ( Result != ASAAC_MOS_NII_CALL_OK )
	    return Result;
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


ASAAC_NiiReturnStatus NII_receiveNetwork(
		const ASAAC_NetworkDescriptor network_id,
		ASAAC_CharAddress *receive_data, 
		const unsigned long data_length_available, 
		unsigned long *data_length,
		ASAAC_PublicId *tc_id, 
		const ASAAC_Time time_out)
{
	initialize();
	
	ASAAC_NiiReturnStatus Result = hasNetworkData( network_id, tc_id );

	if ( (Result != ASAAC_MOS_NII_CALL_OK) &&
	     (Result != ASAAC_MOS_NII_BUFFER_EMPTY) )
		return Result;
	
	if ( Result ==  ASAAC_MOS_NII_BUFFER_EMPTY )
	{
		TransferConnection Tc;
		
		do
		{
			ASAAC_NiiReturnStatus Result = waitForData( time_out, tc_id );
			
			if ( Result != ASAAC_MOS_NII_CALL_OK )
				return Result;
			
			if ( getTc( *tc_id, &Tc ) )
				return ASAAC_MOS_NII_CALL_FAILED;
		}	
		while ( ( Tc.network_id.network != network_id.network ) || 
				( Tc.network_id.port != network_id.port ) );
	}	
	
	Result = receiveData(*tc_id, *receive_data, data_length_available, data_length);

	if ( Result != ASAAC_MOS_NII_CALL_OK )
	    return Result;
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


// Release local resources previously allocated to handle the transmission or reception of information over a TC
ASAAC_NiiReturnStatus NII_destroyTransfer(
		const ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor network_id)
{
	initialize();
	
	TransferConnection Tc;
	
	if ( getTc( tc_id, &Tc ) == 0 ) //TC not established yet
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	if ( removeTc( tc_id ) == 0 )
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	Network Nw;
	
	if (getNw( Tc.network_id.port, &Nw ) == 1)
	{
		if (countTcsByPort( Tc.network_id.port ) == 0)
		{
			close(Nw.fd);
			
			Nw.fd              = -1;
			Nw.is_streaming    = 0;
			Nw.tc_id_with_data = NII_UNUSED_ID;
			
			setNw(Nw.port, Nw);
		}
	}
	
	configureServices();

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


/*************************************************************************/
/*                 N E T W O R K   A C C E S S                           */
/*************************************************************************/

ASAAC_NiiReturnStatus receiveFromNetwork( const int fd, ASAAC_NetworkDescriptor *network_id, ASAAC_PublicId *buffer_id )
{
	struct sockaddr_in NetworkAddr;
	
	socklen_t NetworkAddrLength = (socklen_t)sizeof(NetworkAddr);
	
	if ( allocateBuffer( buffer_id ) == 0 )
	{
#ifdef _DEBUG_
		printf("receiveFromNetwork: Error allocating a buffer \n");
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	
	TcPacketData *Buffer = getBuffer( *buffer_id );
	
	if ( Buffer == NULL )
	{		
#ifdef _DEBUG_
		printf("receiveFromNetwork: buffer_id is invalid \n");
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	
	long ReceivedBytes = recvfrom( fd, &Buffer->packet, sizeof(TcPacket), 0, (struct sockaddr *)&NetworkAddr, &NetworkAddrLength);

	if ( ReceivedBytes == -1 )
	{
#ifdef _DEBUG_
		perror("recvfrom()");
#endif
		releaseBuffer( *buffer_id );
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	
	network_id->network = NetworkAddr.sin_addr.s_addr;
	network_id->port    = ntohs(NetworkAddr.sin_port);
	
	Buffer->data_length = ReceivedBytes /*- sizeof(NetworkHeader)*/ - sizeof(TcHeader);
	Buffer->packet.tc_header.tc_id = ntohl( Buffer->packet.tc_header.tc_id );

	//TODO: depending on a network header, data shall be checked for completelyness
	//maybe further several recvfrom's must be executed

#ifdef _DEBUG_
	printf("receiveFromNetwork: %ld received.\n", ReceivedBytes);
#endif
	
	return ASAAC_MOS_NII_CALL_OK;
}


ASAAC_NiiReturnStatus sendToNetwork( const int fd, const ASAAC_NetworkDescriptor network_id, const ASAAC_PublicId tc_id, const char* data, const unsigned long length, ASAAC_TimeInterval time_out )
{
	if (length > NII_MAX_SIZE_OF_BUFFER)
		return ASAAC_MOS_NII_CALL_FAILED;
	
	struct sockaddr_in NetworkAddr;
	NetworkAddr.sin_family        = AF_INET;
	NetworkAddr.sin_addr.s_addr   = network_id.network;
	NetworkAddr.sin_port          = htons(network_id.port);
	
	size_t NetworkAddrLength = sizeof(NetworkAddr);

	static TcPacketData Buffer;
	
	Buffer.packet.tc_header.tc_id = htonl( tc_id );
	memcpy(&Buffer.packet.data, data, length);
    //TODO: memcpy can may be exchanged by several sendto calls

	Buffer.data_length = /*sizeof(NetworkHeader) + */sizeof(TcHeader) + length;

	long SendBytes = Buffer.data_length;
	char *BufferArray = (char*)&Buffer.packet;

	long SentBytes = 0;
	long Offset = 0;

	fd_set wfds;
	FD_ZERO( &wfds );
	FD_SET( fd, &wfds );

	struct timeval tv;
	
	TimeInterval_convertToTimeval( &time_out, &tv );
	
	select(fd+1, NULL, &wfds, NULL, &tv);
	
	do
	{
		SentBytes = sendto( fd, &BufferArray[Offset], SendBytes, 0, (struct sockaddr *) &NetworkAddr, NetworkAddrLength);
		
		if (SentBytes == -1)
		{
			if( errno == EWOULDBLOCK )
				SentBytes = Offset;
			else
			{
				perror("sendto() : ");
				return ASAAC_MOS_NII_CALL_FAILED;
			}
		}
		
		SendBytes -= SentBytes;
		Offset += SentBytes;
	}
	while ( SendBytes > 0 );

#ifdef _DEBUG_
	printf("sendToNetwork: %ld sent.\n", SentBytes);
#endif
	
	return ASAAC_MOS_NII_CALL_OK;
}


/*************************************************************************/
/*                       D A T A   A C C E S S                           */
/*************************************************************************/

void lockAccess()
{
	
}


void releaseAccess()
{
	
}


long getIndexOfNw(const ASAAC_PublicId port)
{
	long Index;
	for (Index = 0; Index < m_NwListSize; Index++)
	{
		if ( m_NwList[Index].port == port )
		{ 
#ifdef _DEBUG_
			printf("Nw found: %ld with Index: %lu\n", port, Index);
#endif
			return Index;
		}
	}

#ifdef _DEBUG_
	printf("Nw not found: %lu\n", port);
#endif
	
	return -1;
}


char addNw(Network Data)
{
#ifdef _DEBUG_
	printf("addNw: %lu\n",  Data.port);
#endif

	if ( getIndexOfNw(Data.port) != -1 )
		return 0;
	
	if ( m_NwListSize == NII_MAX_NUMBER_OF_NETWORKS)
		return 0;
	
	m_NwList[m_NwListSize] = Data;
	m_NwListSize++;

#ifdef _DEBUG_
	printf("addNw was successful\n");
#endif
	
	return 1;
}


char removeNw(const ASAAC_PublicId port)
{
	long Index = getIndexOfNw(port);
	
	if ( Index == -1)
		return 0;
	
	memmove(&(m_NwList[Index]), &(m_NwList[Index+1]), ( (m_NwListSize-1) - Index) * sizeof(Network));
	
	m_NwListSize--;
	
	return 1;
}


void removeAllNw()
{
	m_NwListSize = 0;
}


long countNws()
{
	return m_NwListSize;
}


char getNw(const ASAAC_PublicId port, Network *Data)
{
#ifdef _DEBUG_
	printf("getNw: %lu\n", port);
#endif

	long Index = getIndexOfNw( port );
	
	if ( Index == -1 )
		return 0;
		
	*Data = m_NwList[Index];
	
	return 1;
}


char setNw(const ASAAC_PublicId port, const Network Data)
{
	if ( Data.port != port )
		return 0;

	long Index = getIndexOfNw(port);
	
	if ( Index == -1)
		return 0;
	
	m_NwList[Index] = Data;
	
	return 1;
}


long getIndexOfTc(const ASAAC_PublicId tc_id)
{
	long Index;
	for (Index = 0; Index < m_TcListSize; Index++)
	{
		if ( m_TcList[Index].tc_id == tc_id )
			return Index;
	}
	
#ifdef _DEBUG_
	printf("Tc not found: %lu\n", tc_id);
#endif

	return -1;
}


char addTc(TransferConnection Data)
{
#ifdef _DEBUG_
	printf("addTc: %lu\n", Data.tc_id);
#endif

	if ( getIndexOfTc(Data.tc_id) != -1 )
		return 0;
	
	if ( m_TcListSize == NII_MAX_NUMBER_OF_TCS)
		return 0;
	
	m_TcList[m_TcListSize] = Data;
	m_TcListSize++;

#ifdef _DEBUG_
	printf("addTc was successful.\n");
#endif
	
	return 1;
}


char removeTc(const ASAAC_PublicId tc_id)
{
	long Index = getIndexOfTc(tc_id);
	
	if ( Index == -1)
		return 0;
	
	memmove(&(m_TcList[Index]), &(m_TcList[Index+1]), ( (m_TcListSize-1) - Index) * sizeof(TransferConnection));
	
	m_TcListSize--;
	
	return 1;
}


void removeAllTc()
{
	m_TcListSize = 0;
}


long countTcs()
{
	return m_TcListSize;
}


long countTcsByPort(const ASAAC_PublicId port)
{
	long Result = 0;
	long Index;
	
	for (Index = 0; Index < m_TcListSize; Index++)
	{
		if (m_TcList[Index].network_id.port == port)
			Result++;
	}
	
	return Result;
}


char getTc(const ASAAC_PublicId tc_id, TransferConnection *Data)
{
	long Index = getIndexOfTc( tc_id );
	
	if ( Index == -1 )
		return 0;
		
	*Data = m_TcList[Index];
	
	return 1;
}


char setTc(const ASAAC_PublicId tc_id, const TransferConnection Data)
{
	if ( Data.tc_id != tc_id )
		return 0;

	long Index = getIndexOfTc(tc_id);
	
	if ( Index == -1)
		return 0;

	m_TcList[Index] = Data;
	
	return 1;
}


char allocateBuffer(ASAAC_PublicId *buffer_id)
{
	unsigned long Index;
	
	for ( Index = 0; Index < NII_MAX_NUMBER_OF_TCS; Index++ )
	{
		if (m_TcBufferArrayAllocated[ Index ] == 0)
		{
			m_TcBufferArrayAllocated[ Index ] = 1;
			*buffer_id = Index;
			
			return 1;
		}
	}
	
	return 0;
}


void releaseBuffer(ASAAC_PublicId buffer_id)
{
	if ( buffer_id < 0 )
		return;

	if ( buffer_id >= NII_MAX_NUMBER_OF_TCS )
		return;
	
	m_TcBufferArrayAllocated[ buffer_id ] = 0;
}


TcPacketData *getBuffer(ASAAC_PublicId buffer_id)
{
	if ( buffer_id < 0 )
		return NULL;

	if ( buffer_id >= NII_MAX_NUMBER_OF_TCS )
		return NULL;
	
	return &m_TcBufferArray[ buffer_id ];
}


/*************************************************************************/
/*                         S E R V I C E S                               */
/*************************************************************************/

void handleStreamingTc(const ASAAC_PublicId port, ASAAC_PublicId *tc_id)
{
#ifdef _DEBUG_
	printf("handle streaming tc: not completely implemented !!!\n");
#endif
	
	*tc_id = NII_UNUSED_ID;
	
	Network Nw;	
	if (getNw( port, &Nw ) == 0)
		return;
	
	ASAAC_NetworkDescriptor NetworkId;
	ASAAC_PublicId BufferId;
	
	ASAAC_NiiReturnStatus Result = receiveFromNetwork( Nw.fd, &NetworkId, &BufferId );
	//TODO: StreamingTcs may not have a header
	
	if (Result != ASAAC_MOS_NII_CALL_OK)
		return;
	
	TcPacketData *Buffer = getBuffer(BufferId);
	
	if (Buffer == NULL)
		return;
	
	TransferConnection Tc;
	if ( getTc(Buffer->packet.tc_header.tc_id, &Tc) == 0 )
	{
		releaseBuffer(BufferId);
		return;
	}

	//Now the tc_id is validated
	*tc_id = Buffer->packet.tc_header.tc_id;
	
	Tc.buffer_id = BufferId;	
	Tc.event_info_data.comms_ev_buffer_received.tc_id = Tc.tc_id;
	Tc.event_info_data.comms_ev_buffer_received.status = ASAAC_MOS_NII_CALL_OK;

	setTc(Tc.tc_id, Tc);

	// TODO: Now a stream of the application shall be filled
}


void handleMessageTc(const ASAAC_PublicId port, ASAAC_PublicId *tc_id)
{
#ifdef _DEBUG_
	printf("handle message tc\n");
#endif
	
	*tc_id = NII_UNUSED_ID;
	
	Network Nw;	
	if (getNw( port, &Nw ) == 0)
	{
#ifdef _DEBUG_
		printf("handle message tc: Network not found\n");
#endif
		return;
	}
	
	ASAAC_NetworkDescriptor NetworkId;
	ASAAC_PublicId BufferId;
	
	ASAAC_NiiReturnStatus Result = receiveFromNetwork( Nw.fd, &NetworkId, &BufferId );
	
	if (Result != ASAAC_MOS_NII_CALL_OK)
	{
#ifdef _DEBUG_
		printf("handle message tc: Data could not received\n");
#endif
		return;
	}
	
	TcPacketData *Buffer = getBuffer(BufferId);
	
	if (Buffer == NULL)
		return;
	
	//DEBUG CODE
	//Buffer->packet.tc_header.tc_id = 8;
	//DEBUG CODE
	
	TransferConnection Tc;
	if ( getTc(Buffer->packet.tc_header.tc_id, &Tc) == 0 )
	{
#ifdef _DEBUG_
		printf("handle message tc: Tc not found\n");
#endif
		releaseBuffer(BufferId);
		return;
	}

	//Now the tc_id is validated
	*tc_id = Buffer->packet.tc_header.tc_id;
	Tc.event_info_data.comms_ev_buffer_received.tc_id = Tc.tc_id;

	if (Tc.direction == ASAAC_TRANSFER_DIRECTION_SEND)
	{
#ifdef _DEBUG_
		printf("handle message tc: Tc is configured for sending\n");
#endif
		Tc.event_info_data.comms_ev_buffer_received.status = ASAAC_MOS_NII_INVALID_TC;
		setTc(Tc.tc_id, Tc);
		
		releaseBuffer(BufferId);
		
		return;		
	}
	
	Tc.buffer_id = BufferId;	
	Tc.event_info_data.comms_ev_buffer_received.status = ASAAC_MOS_NII_CALL_OK;

	setTc(Tc.tc_id, Tc);
}


void* ServiceThread(void* Data)
{
#ifdef _DEBUG_
	printf("ServiceThread: Entry\n");
#endif

	for (;;)
	{
		fd_set rfds;
		int max_fd = m_ServiceFileDescriptor;
	
		FD_ZERO( &rfds );
		FD_SET( m_ServiceFileDescriptor, &rfds );

#ifdef _DEBUG_
		printf("ServiceThread: Collect file descriptors for select()\n");
#endif
		
		lockAccess();
		
		long Index;
		for (Index = 0; Index < m_NwListSize; Index++)
		{			
			if ( m_NwList[Index].fd == -1 )
				continue;
			
			if ( m_NwList[Index].tc_id_with_data != NII_UNUSED_ID )
				continue;
			
#ifdef _DEBUG_
			printf("ServiceThread: Collect file descriptors for select(): index:%ld fd:%d\n", Index, m_NwList[Index].fd );
#endif			

			FD_SET( m_NwList[Index].fd, &rfds );
			
			if ( max_fd < m_NwList[Index].fd )
				max_fd = m_NwList[Index].fd;
		}	
		
		releaseAccess();
		
#ifdef _DEBUG_
		printf("ServiceThread: execute select()\n");
#endif

		ASAAC_TimeInterval interval;
		struct timeval tv;
		
		TimeInterval_assignInfinity( &interval );
		TimeInterval_convertToTimeval( &interval, &tv );
		
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		int retval = select( max_fd+1, &rfds, NULL, NULL, &tv );
		
#ifdef _DEBUG_
		printf("ServiceThread: return from select()\n");
#endif

		if (retval == -1)
		{
			printf("cMosNii::listen thread() ERROR on select()\n");
		}

		if ( FD_ISSET(m_ServiceFileDescriptor, &rfds) )
		{

#ifdef _DEBUG_
			printf("ServiceThread: ServiceFileDescriptor triggered this event\n");
#endif

			ASAAC_NetworkDescriptor NetworkId;
			ASAAC_PublicId BufferId;
			
			receiveFromNetwork( m_ServiceFileDescriptor, &NetworkId, &BufferId);
			releaseBuffer( BufferId );
		}
		else
		{
#ifdef _DEBUG_
			printf("ServiceThread: A TC triggered this event\n");
#endif

			lockAccess();
			
			long Index;
			for (Index = 0; Index < m_NwListSize; Index++)
			{
				if ( m_NwList[Index].fd == -1 )
				    continue;

				if ( m_NwList[Index].tc_id_with_data != NII_UNUSED_ID )
				    continue;
				
				if ( FD_ISSET(m_NwList[Index].fd, &rfds) == 0 )
				    continue;
				
				ASAAC_PublicId TcId = NII_UNUSED_ID;
				
				if (m_NwList[Index].is_streaming == 1)
					handleStreamingTc( m_NwList[Index].port, &TcId );
				else handleMessageTc( m_NwList[Index].port, &TcId );
				
				if (TcId == NII_UNUSED_ID)
					continue;
				
				TransferConnection Tc;
				if (getTc(TcId, &Tc) == 0)
					continue;
				
				if (Tc.trigger_callback == ASAAC_BOOL_TRUE)
					ASAAC_MOS_callbackHandler( ASAAC_COMMS_EV_BUFFER_RECEIVED, Tc.callback_id, &(Tc.event_info_data) );

				m_NewDataTcId = TcId;
				pthread_cond_broadcast(&m_NewDataCondition);
			}
			
			releaseAccess();
		}
	}
	
	return NULL;
}


ASAAC_NiiReturnStatus hasTcData( const ASAAC_PublicId tc_id )
{
	TransferConnection Tc;
	
	if ( getTc( tc_id, &Tc ) == 0)
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;
	
	if ( Tc.buffer_id == NII_UNUSED_ID )
		return ASAAC_MOS_NII_BUFFER_EMPTY;
	
	return ASAAC_MOS_NII_CALL_OK;
}


ASAAC_NiiReturnStatus hasNetworkData( const ASAAC_NetworkDescriptor network_id, ASAAC_PublicId *tc_id )
{
	Network Nw;
	
	if ( getNw( network_id.port, &Nw ) )
		return ASAAC_MOS_NII_INVALID_NETWORK;

	if (Nw.tc_id_with_data == NII_UNUSED_ID)
		return ASAAC_MOS_NII_BUFFER_EMPTY;
	
	*tc_id = Nw.tc_id_with_data;
	
	return ASAAC_MOS_NII_CALL_OK;
}


ASAAC_NiiReturnStatus waitForData( ASAAC_Time time_out, ASAAC_PublicId *tc_id )
{
#ifdef _DEBUG_
	printf("waitForData: wait for new_data condition\n");
#endif

	struct timespec TimespecTimeout;
	
	Time_convertToTimespec( &time_out, &TimespecTimeout );
	
	int iErrorCode = pthread_cond_timedwait( &m_NewDataCondition, &m_NewDataMutex, &TimespecTimeout );
	
	if ( iErrorCode == ETIMEDOUT )
		return ASAAC_MOS_NII_BUFFER_EMPTY;

	if ( iErrorCode == -1 )
		return ASAAC_MOS_NII_CALL_FAILED;
	
	if (m_NewDataTcId == -1)
		return ASAAC_MOS_NII_CALL_FAILED;
	
	*tc_id = m_NewDataTcId;
	
	return ASAAC_MOS_NII_CALL_OK;
}


ASAAC_NiiReturnStatus receiveData( 		
		const ASAAC_PublicId tc_id, 
		const ASAAC_CharAddress receive_data, 
		const unsigned long data_length_available, 
		unsigned long *data_length)
{
	TransferConnection Tc;
	
	if ( getTc(tc_id, &Tc) == 0 )
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	if (Tc.type != ASAAC_TRANSFER_TYPE_MESSAGE)
	{
#ifdef _DEBUG_
		printf("ERROR: not a message TC \n");
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	if (Tc.direction != ASAAC_TRANSFER_DIRECTION_RECEIVE)
	{
#ifdef _DEBUG_
		printf("ERROR: not a receiving TC \n");
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	
	Network Nw;
	
	if ( getNw( Tc.network_id.port, &Nw ) == 0 )
		return ASAAC_MOS_NII_CALL_FAILED;
	
	TcPacketData *Buffer = getBuffer(Tc.buffer_id);
	
	if (Buffer == NULL)
		return ASAAC_MOS_NII_CALL_FAILED;

	memcpy(receive_data, &Buffer->packet.data, Buffer->data_length);
	*data_length = Buffer->data_length;
	
	Nw.tc_id_with_data = NII_UNUSED_ID;
	releaseBuffer(Tc.buffer_id);
	Tc.buffer_id = NII_UNUSED_ID;
	
	setNw( Nw.port, Nw );
	setTc( Tc.tc_id, Tc );
	
	configureServices();	
	
#ifdef _DEBUG_
	printf("RECEIVE %lu BYTES ON TC %lu\n", *data_length, tc_id );
#endif

	return ASAAC_MOS_NII_CALL_OK;
}


void configureServices()
{	
#ifdef _DEBUG_
	printf("configureServices: entry\n");
#endif

	struct in_addr NetworkAddr;
	inet_aton("127.0.0.1", &NetworkAddr);

	ASAAC_NetworkDescriptor NetworkId;

	NetworkId.network = NetworkAddr.s_addr;
	NetworkId.port = 0xffff;

#ifdef _DEBUG_
	printf("configureServices: send data via ServiceFileDescriptor\n");
#endif

	ASAAC_TimeInterval interval;
	TimeInterval_assignInstant( &interval );
	
	sendToNetwork(m_ServiceFileDescriptor, NetworkId, NII_UNUSED_ID, NULL, 0, interval);
	
#ifdef _DEBUG_
	printf("configureServices: exit\n");
#endif
}

