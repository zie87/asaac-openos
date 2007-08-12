#include "NII.hh"

using namespace std;


cMosNii* cMosNii::getInstance()
{
	static cMosNii Instance;

	return &Instance;
}


// Constructs a default object with no configured interfaces and no open transfer channels
cMosNii::cMosNii()
{
	m_NewDataTcId = NII_UNUSED_ID;
	
	m_IsListening = 0;
	
	m_NwListSize = 0;
	m_TcListSize = 0;
	
	for (long Index = 0; Index < NII_MAX_NUMBER_OF_TCS; Index++ )
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
	
	m_IsListening = 1;
	if (pthread_create( &m_ServiceThread, NULL, ServiceThread, NULL ) == -1)
		perror("pthread_create: ");
}


// Destroys all configurations and open transfer channels
cMosNii::~cMosNii()
{
	pthread_cancel( m_ServiceThread );
	close(m_ServiceFileDescriptor);

	pthread_cond_destroy( &m_ServiceThreadCondition );
	pthread_mutex_destroy( &m_ServiceThreadMutex );
	pthread_cond_destroy( &m_NewDataCondition );
	pthread_mutex_destroy( &m_NewDataMutex );
}


/////////////////////////////////////////////////////////////////////////////
///////////////////////COMMUNICATION SERVICE INTERFACE///////////////////////
/////////////////////////////////////////////////////////////////////////////

// Configure a local communication interface 
ASAAC_NiiReturnStatus cMosNii::configureInterface(
		const ASAAC_PublicId interface_id,
		const ASAAC_NetworkDescriptor *network_id,
		const ASAAC_InterfaceConfigurationData *configuration_data)
{
	if (countTcs(*network_id) > 0)
		return ASAAC_MOS_NII_OPEN_TCS;
	
	if ( interface_id == 0 )
	{
		if ( removeNw( *network_id ) == 0 )
			return ASAAC_MOS_NII_INVALID_NETWORK;
		
		return ASAAC_MOS_NII_CALL_COMPLETE;
	}
	
	if ( network_id->network == 0 )
	{
		if ( countTcs() > 0 )
			return ASAAC_MOS_NII_OPEN_TCS;
		
		removeAllNw();
		
		return ASAAC_MOS_NII_CALL_COMPLETE;
	}
	
	Network Nw;

	Nw.interface_id       = interface_id;
	Nw.network_id         = *network_id;
	Nw.configuration_data = *configuration_data;
	Nw.fd                 = -1;
	Nw.is_streaming       = 0;
	Nw.tc_id_with_data	  = NII_UNUSED_ID;

	if (setNw( *network_id, Nw ) == 0)
	{
		if ( addNw( Nw ) == 0 )
			return ASAAC_MOS_NII_STORAGE_FAULT;
	}

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


// Configure the local resource to handle the transmission or reception of information over a Transfer Channel (TC)
ASAAC_NiiReturnStatus cMosNii::configureTransfer(
		const ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor network_id, 
		const ASAAC_TransferDirection send_receive,
		const ASAAC_TransferType message_streaming,
		const ASAAC_TC_ConfigurationData configuration_data, 
		const ASAAC_Bool trigger_callback,
		const ASAAC_PublicId callback_id)
{
#ifdef _DEBUG_
	cerr << "cMosNii::configureTransfer(" << tc_id << ") setup TC" << endl;
#endif
	
	Network Nw;
	
	if ( getNw(network_id, &Nw) == 0 )	
		return ASAAC_MOS_NII_INVALID_NETWORK;

	TransferConnection Tc;

	if ( getTc( tc_id, &Tc ) == 1 )
		return ASAAC_MOS_NII_ALREADY_CONFIGURED;

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
	NetworkAddr.sin_port = htons(Nw.network_id.port);
	
	switch ( Tc.type )
	{
		case ASAAC_TRANSFER_TYPE_MESSAGE:
		{
			if (Nw.fd == -1)
			{
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
					return ASAAC_MOS_NII_CALL_FAILED;
				}

				//Necessary for sendTransfer()
				int val = fcntl(Nw.fd, F_GETFL, 0);
				fcntl(Nw.fd, F_SETFL, val | O_NONBLOCK);
			}
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
				return ASAAC_MOS_NII_CALL_FAILED;
			}
		}
		break;
		
		default:
		{
			cerr << "cMosNii::configureTransfer() unknown TransferType: " << message_streaming << endl;
			return ASAAC_MOS_NII_CALL_FAILED;
		}
		break;
	}

	// Now the TC can be stored
	if ( addTc( Tc ) == 0 )
		return ASAAC_MOS_NII_STORAGE_FAULT;

	setNw(network_id, Nw);
	
	configureServices();			

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


// Send a block of data on the given TC
ASAAC_NiiReturnStatus cMosNii::sendTransfer(
		const ASAAC_PublicId tc_id,
		const char* transmit_data, 
		const unsigned long data_length, 
		const ASAAC_Time time_out)
{
	TransferConnection Tc;
	
	if ( getTc( tc_id, &Tc ) == 0)
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	if (Tc.type != ASAAC_TRANSFER_TYPE_MESSAGE)
	{
#ifdef _DEBUG_
		cerr << "ERROR: not a message TC " << endl;
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	if (Tc.direction != ASAAC_TRANSFER_DIRECTION_SEND)
	{
#ifdef _DEBUG_
		cerr << "ERROR: not a sending TC " << endl;
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	Network Nw;
	
	if ( getNw( Tc.network_id, &Nw ) == 0)
		return ASAAC_MOS_NII_CALL_FAILED;

	Tc.event_info_data.comms_ev_buffer_sent.status = sendToNetwork(Nw.fd, Nw.network_id, tc_id, transmit_data, data_length, time_out);
	Tc.event_info_data.comms_ev_buffer_sent.tc_id  = Tc.tc_id;

	if ( Tc.trigger_callback == ASAAC_BOOL_TRUE )
		ASAAC_MOS_callbackHandler( ASAAC_COMMS_EV_BUFFER_SEND, Tc.callback_id, &Tc.event_info_data );

	return Tc.event_info_data.comms_ev_buffer_sent.status;
}


// Receive a block of data on the given TC
ASAAC_NiiReturnStatus cMosNii::receiveTransfer(
		const ASAAC_PublicId tc_id, 
		ASAAC_CharAddress *receive_data,
		const unsigned long data_length_available, 
		unsigned long *data_length, 
		const ASAAC_Time time_out)
{
	ASAAC_NiiReturnStatus Result = hasData( tc_id );

	if ( (Result != ASAAC_MOS_NII_CALL_COMPLETE) &&
	     (Result != ASAAC_MOS_NII_BUFFER_EMPTY) )
		return Result;
	
	if ( Result ==  ASAAC_MOS_NII_BUFFER_EMPTY )
	{
		ASAAC_PublicId ReceivedTcId;
		
		do
		{
			ASAAC_NiiReturnStatus Result = waitForData( time_out, &ReceivedTcId );
			
			if ( Result != ASAAC_MOS_NII_CALL_COMPLETE )
				return Result;
		}	
		while ( ReceivedTcId != tc_id );
	}
	
	return receiveData(tc_id, *receive_data, data_length_available, data_length);
}


ASAAC_NiiReturnStatus cMosNii::receiveNetwork(
		const ASAAC_NetworkDescriptor network_id,
		ASAAC_CharAddress *receive_data, 
		const unsigned long data_length_available, 
		unsigned long *data_length,
		ASAAC_PublicId *tc_id, 
		const ASAAC_Time time_out)
{
	ASAAC_NiiReturnStatus Result = hasData( network_id, tc_id );

	if ( (Result != ASAAC_MOS_NII_CALL_COMPLETE) &&
	     (Result != ASAAC_MOS_NII_BUFFER_EMPTY) )
		return Result;
	
	if ( Result ==  ASAAC_MOS_NII_BUFFER_EMPTY )
	{
		TransferConnection Tc;
		
		do
		{
			ASAAC_NiiReturnStatus Result = waitForData( time_out, tc_id );
			
			if ( Result != ASAAC_MOS_NII_CALL_COMPLETE )
				return Result;
			
			if ( getTc( *tc_id, &Tc ) )
				return ASAAC_MOS_NII_CALL_FAILED;
		}	
		while ( ( Tc.network_id.network != network_id.network ) || 
				( Tc.network_id.port != network_id.port ) );
	}	
	
	return receiveData(*tc_id, *receive_data, data_length_available, data_length);
}


// Release local resources previously allocated to handle the transmission or reception of information over a TC
ASAAC_NiiReturnStatus cMosNii::destroyTransfer(
		const ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor network_id)
{
	TransferConnection Tc;
	
	if ( getTc( tc_id, &Tc ) == -1 ) //TC not established yet
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	if ( removeTc( tc_id ) == 0 )
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;

	Network Nw;
	
	if (getNw( Tc.network_id, &Nw ) == 1)
	{
		if (countTcs( Tc.network_id ) == 0)
			close(Nw.fd);
	}
	
	configureServices();

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


/*************************************************************************/
/*                       D A T A   A C C E S S                           */
/*************************************************************************/

ASAAC_NiiReturnStatus cMosNii::receiveFromNetwork( const int fd, ASAAC_NetworkDescriptor *network_id, ASAAC_PublicId *buffer_id )
{
	struct sockaddr_in NetworkAddr;
	
	size_t NetworkAddrLength = sizeof(NetworkAddr);
	
	if ( allocateBuffer( buffer_id ) == 0 )
		return ASAAC_MOS_NII_CALL_FAILED;

	TcPacketData *Buffer = getBuffer( *buffer_id );
	
	if ( Buffer == NULL )
		return ASAAC_MOS_NII_CALL_FAILED;		
	
	long bytes_received = recvfrom( fd, &Buffer->packet, sizeof(TcPacket), 0, (sockaddr*)&NetworkAddr, &NetworkAddrLength);

	if ( bytes_received == -1 )
	{
		releaseBuffer( *buffer_id );
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	
	network_id->network = NetworkAddr.sin_addr.s_addr;
	network_id->port    = ntohs(NetworkAddr.sin_port);
	
	Buffer->data_length = bytes_received - sizeof(NetworkHeader) - sizeof(TcHeader);
	Buffer->packet.tc_header.tc_id = ntohl( Buffer->packet.tc_header.tc_id );
	
	//TODO: depending on a network header, data shall be checked for completelyness
	//maybe further several recvfrom's must be executed
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


ASAAC_NiiReturnStatus cMosNii::sendToNetwork( const int fd, const ASAAC_NetworkDescriptor network_id, const ASAAC_PublicId tc_id, const char* data, const unsigned long length, const ASAAC_Time time_out )
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
	
	Buffer.data_length = sizeof(NetworkHeader) + sizeof(TcHeader) + length;
	
	long SendBytes = Buffer.data_length;
	char *BufferArray = (char*)&Buffer.packet.data;

	long SentBytes = 0;
	long Offset = 0;
	
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
	while (( SendBytes > 0 ) && ( lower(TimeInstant(), time_out) ));
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


/*************************************************************************/
/*                       D A T A   A C C E S S                           */
/*************************************************************************/

long cMosNii::getIndexOfNw(const ASAAC_NetworkDescriptor network_id)
{
	for (long Index = 0; Index < m_NwListSize; Index++)
	{
		if ( (m_TcList[Index].network_id.port == network_id.port) &&
			 (m_TcList[Index].network_id.network == network_id.network) )
			return Index;
	}
	
	return -1;
}


char cMosNii::addNw(Network Data)
{
	if ( getIndexOfNw(Data.network_id) != -1 )
		return 0;
	
	if ( m_NwListSize == NII_MAX_NUMBER_OF_NETWORKS)
		return 0;
	
	m_NwListSize++;
	m_NwList[m_NwListSize] = Data;
	
	return 1;
}


char cMosNii::removeNw(const ASAAC_NetworkDescriptor network_id)
{
	long Index = getIndexOfNw(network_id);
	
	if ( Index == -1)
		return 0;
	
	memmove(&(m_NwList[Index]), &(m_NwList[Index+1]), ( (m_NwListSize-1) - Index) * sizeof(Network));
	
	m_NwListSize--;
	
	return 1;
}


void cMosNii::removeAllNw()
{
	m_NwListSize = 0;
}


long cMosNii::countNws()
{
	return m_NwListSize;
}


char cMosNii::getNw(const ASAAC_NetworkDescriptor network_id, Network *Data)
{
	long Index = getIndexOfNw( network_id );
	
	if ( Index == -1 )
		return 0;
		
	*Data = m_NwList[Index];
	
	return 1;
}


char cMosNii::setNw(const ASAAC_NetworkDescriptor network_id, const Network Data)
{
	if ( ( Data.network_id.network != network_id.network ) ||
	     ( Data.network_id.port != network_id.port ) )
		return 0;

	long Index = getIndexOfNw(network_id);
	
	if ( Index == -1)
		return 0;

	m_NwList[Index] = Data;
	
	return 1;
}


long cMosNii::getIndexOfTc(const ASAAC_PublicId tc_id)
{
	for (long Index = 0; Index < m_TcListSize; Index++)
	{
		if ( m_TcList[Index].tc_id == tc_id )
			return Index;
	}
	
	return -1;
}


char cMosNii::addTc(TransferConnection Data)
{
	if ( getIndexOfTc(Data.tc_id) != -1 )
		return 0;
	
	if ( m_TcListSize == NII_MAX_NUMBER_OF_TCS)
		return 0;
	
	m_TcListSize++;
	m_TcList[m_TcListSize] = Data;
	
	return 1;
}


char cMosNii::removeTc(const ASAAC_PublicId tc_id)
{
	long Index = getIndexOfTc(tc_id);
	
	if ( Index == -1)
		return 0;
	
	memmove(&(m_TcList[Index]), &(m_TcList[Index+1]), ( (m_TcListSize-1) - Index) * sizeof(TransferConnection));
	
	m_TcListSize--;
	
	return 1;
}


void cMosNii::removeAllTc()
{
	m_TcListSize = 0;
}


long cMosNii::countTcs()
{
	return m_TcListSize;
}


long cMosNii::countTcs(const ASAAC_NetworkDescriptor network_id)
{
	long Result = 0;
	
	for (long Index = 0; Index < m_TcListSize; Index++)
	{
		if ( (m_TcList[Index].network_id.port == network_id.port) &&
			 (m_TcList[Index].network_id.network == network_id.network) )
			Result++;
	}
	
	return Result;
}


char cMosNii::getTc(const ASAAC_PublicId tc_id, TransferConnection *Data)
{
	long Index = getIndexOfTc( tc_id );
	
	if ( Index == -1 )
		return 0;
		
	*Data = m_TcList[Index];
	
	return 1;
}


char cMosNii::setTc(const ASAAC_PublicId tc_id, const TransferConnection Data)
{
	if ( Data.tc_id != tc_id )
		return 0;

	long Index = getIndexOfTc(tc_id);
	
	if ( Index == -1)
		return 0;

	m_TcList[Index] = Data;
	
	return 1;
}


char cMosNii::allocateBuffer(ASAAC_PublicId *buffer_id)
{
	for ( *buffer_id = 0; *buffer_id < NII_MAX_NUMBER_OF_TCS; *buffer_id++ )
	{
		if (m_TcBufferArrayAllocated[ *buffer_id ] == 0)
		{
			m_TcBufferArrayAllocated[ *buffer_id ] = 1;
			return 1;
		}
	}
	
	return 0;
}


void cMosNii::releaseBuffer(ASAAC_PublicId buffer_id)
{
	if ( buffer_id < 0 )
		return;

	if ( buffer_id >= NII_MAX_NUMBER_OF_TCS )
		return;
	
	m_TcBufferArrayAllocated[ buffer_id ] = 0;
}


TcPacketData *cMosNii::getBuffer(ASAAC_PublicId buffer_id)
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

void cMosNii::handleStreamingTc(const ASAAC_NetworkDescriptor network_id, ASAAC_PublicId *tc_id)
{
#ifdef _DEBUG_
	cerr << "handle streaming tc: not completely implemented !!!" << endl;
#endif
	
	*tc_id = NII_UNUSED_ID;
	
	Network Nw;	
	if (getNw( network_id, &Nw ) == 0)
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


void cMosNii::handleMessageTc(const ASAAC_NetworkDescriptor network_id, ASAAC_PublicId *tc_id)
{
#ifdef _DEBUG_
	cout << "handle message tc" << endl;
#endif
	
	*tc_id = NII_UNUSED_ID;
	
	Network Nw;	
	if (getNw( network_id, &Nw ) == 0)
		return;
	
	ASAAC_NetworkDescriptor NetworkId;
	ASAAC_PublicId BufferId;
	
	ASAAC_NiiReturnStatus Result = receiveFromNetwork( Nw.fd, &NetworkId, &BufferId );
	
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
}


void* cMosNii::ServiceThread(void* Data)
{
#ifdef _DEBUG_
	cout << "ServiceThread: Entry" << endl;
#endif

	cMosNii *Nii = cMosNii::getInstance();

	for (;;)
	{
		fd_set rfds;
		int max_fd = Nii->m_ServiceFileDescriptor;
	
		FD_ZERO( &rfds );
		FD_SET( Nii->m_ServiceFileDescriptor, &rfds );

#ifdef _DEBUG_
		cout << "ServiceThread: Collect file descriptors for select()" << endl;
#endif
		
		for (long Index = 0; Index < Nii->m_NwListSize; Index++)
		{
			if ( Nii->m_NwList[Index].tc_id_with_data == NII_UNUSED_ID )
			{
				FD_SET( Nii->m_NwList[Index].fd, &rfds );
				
				if ( max_fd < Nii->m_NwList[Index].fd )
					max_fd = Nii->m_NwList[Index].fd;
			}
		}	
		
#ifdef _DEBUG_
		cout << "ServiceThread: execute select()" << endl;
#endif

		struct timeval tv = IntervalToTimeval( TimeIntervalInfinity );
		int retval = select( max_fd+1, &rfds, 0, 0, &tv );
	
#ifdef _DEBUG_
		cout << "ServiceThread: return from select()" << endl;
#endif

		if (retval == -1)
		{
			cerr << "cMosNii::listen thread() ERROR on select()" << endl;
		}

		if ( FD_ISSET(Nii->m_ServiceFileDescriptor, &rfds) )
		{

#ifdef _DEBUG_
			cout << "ServiceThread: ServiceFileDescriptor triggered this event" << endl;
#endif

			ASAAC_NetworkDescriptor NetworkId;
			ASAAC_PublicId BufferId;
			
			Nii->receiveFromNetwork( Nii->m_ServiceFileDescriptor, &NetworkId, &BufferId);
			Nii->releaseBuffer( BufferId );
		}
		else
		{
#ifdef _DEBUG_
			cout << "ServiceThread: A TC triggered this event" << endl;
#endif

			for (long Index = 0; Index < Nii->m_NwListSize; Index++)
			{
				if ( FD_ISSET(Nii->m_NwList[Index].fd, &rfds) == 0 )
					continue;
				
				ASAAC_PublicId TcId = NII_UNUSED_ID;
				
				if (Nii->m_NwList[Index].is_streaming == 1)
					Nii->handleStreamingTc( Nii->m_NwList[Index].network_id, &TcId );
				else Nii->handleMessageTc( Nii->m_NwList[Index].network_id, &TcId );
				
				if (TcId == NII_UNUSED_ID)
					continue;
				
				TransferConnection Tc;
				if (Nii->getTc(TcId, &Tc) == 0)
					continue;
				
				if (Tc.trigger_callback == ASAAC_BOOL_TRUE)
					ASAAC_MOS_callbackHandler( ASAAC_COMMS_EV_BUFFER_RECEIVED, Tc.callback_id, &Tc.event_info_data );

				Nii->m_NewDataTcId = TcId;
				pthread_cond_broadcast(&Nii->m_NewDataCondition);
			}
		}
	}
	
	return NULL;
}


ASAAC_NiiReturnStatus cMosNii::hasData( const ASAAC_PublicId tc_id )
{
	TransferConnection Tc;
	
	if ( getTc( tc_id, &Tc ) == 0)
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;
	
	if ( Tc.buffer_id == NII_UNUSED_ID )
		return ASAAC_MOS_NII_BUFFER_EMPTY;
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


ASAAC_NiiReturnStatus cMosNii::hasData( const ASAAC_NetworkDescriptor network_id, ASAAC_PublicId *tc_id )
{
	Network Nw;
	
	if ( getNw( network_id, &Nw ) )
		return ASAAC_MOS_NII_INVALID_NETWORK;

	if (Nw.tc_id_with_data == NII_UNUSED_ID)
		return ASAAC_MOS_NII_BUFFER_EMPTY;
	
	*tc_id = Nw.tc_id_with_data;
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


ASAAC_NiiReturnStatus cMosNii::waitForData( const ASAAC_Time time_out, ASAAC_PublicId *tc_id )
{
#ifdef _DEBUG_
	cout << "waitForData: wait for new_data condition" << endl;
#endif

	struct timespec TimespecTimeout = TimeToTimespec( time_out );
	int iErrorCode = pthread_cond_timedwait( &m_NewDataCondition, &m_NewDataMutex, &TimespecTimeout );
	
	if ( iErrorCode == ETIMEDOUT )
		return ASAAC_MOS_NII_BUFFER_EMPTY;

	if ( iErrorCode == -1 )
		return ASAAC_MOS_NII_CALL_FAILED;
	
	if (m_NewDataTcId == -1)
		return ASAAC_MOS_NII_CALL_FAILED;
	
	*tc_id = m_NewDataTcId;
	
	return ASAAC_MOS_NII_CALL_COMPLETE;
}


ASAAC_NiiReturnStatus cMosNii::receiveData( 		
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
		cerr << "ERROR: not a message TC " << endl;
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	if (Tc.direction != ASAAC_TRANSFER_DIRECTION_RECEIVE)
	{
#ifdef _DEBUG_
		cerr << "ERROR: not a receiving TC " << endl;
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	
	Network Nw;
	
	if ( getNw( Tc.network_id, &Nw ) == 0 )
		return ASAAC_MOS_NII_CALL_FAILED;
	
	TcPacketData *Buffer = getBuffer(Tc.buffer_id);
	
	if (Buffer == NULL)
		return ASAAC_MOS_NII_CALL_FAILED;
		
	memcpy(receive_data, Buffer->packet.data, Buffer->data_length);
	*data_length = Buffer->data_length;
	
	Nw.tc_id_with_data = NII_UNUSED_ID;
	releaseBuffer(Tc.buffer_id);
	Tc.buffer_id = NII_UNUSED_ID;
	
	setNw( Nw.network_id, Nw );
	setTc( Tc.tc_id, Tc );
	
	configureServices();	
	
#ifdef _DEBUG_
		cout << "RECEIVE " << data_length << " BYTE ON TC " << tc_id << endl;
#endif

	return ASAAC_MOS_NII_CALL_COMPLETE;
}


void cMosNii::configureServices()
{	
	if (m_IsListening == 0)
		return;
	
#ifdef _DEBUG_
	cout << "stopServices: entry" << endl;
#endif

	char * Data = "TestData";
	unsigned long Length = strlen(Data);
	
	in_addr NetworkAddr;
	inet_aton("127.0.0.1", &NetworkAddr);

	struct sockaddr_in Addr;
	Addr.sin_family = AF_INET;
	Addr.sin_addr = NetworkAddr;
	Addr.sin_port = 0xffff;

#ifdef _DEBUG_
	cout << "stopServices: send data via ServiceFileDescriptor" << endl;
#endif
	
	int result = sendto(m_ServiceFileDescriptor, &Data, Length, 0, (struct sockaddr *) &Addr, sizeof(Addr));

	if (result == -1)
	{
#ifdef _DEBUG_
		perror("stopServices: sendto(): ");
#endif
	}
	
#ifdef _DEBUG_
	cout << "stopServices: exit" << endl;
#endif
}

