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
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
		m_TcData[i].valid = 0;

	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
		m_NwData[i].valid = 0;
	
	m_NiiLocalPort = 55500;
	m_CurrentBuffer = 0;

	pthread_condattr_t CondAttr;
	pthread_condattr_init( &CondAttr );
	pthread_condattr_setpshared( &CondAttr, PTHREAD_PROCESS_PRIVATE );

	
	pthread_mutexattr_t MutexAttr;
	pthread_mutexattr_init( &MutexAttr );    
	pthread_mutexattr_setpshared( &MutexAttr, PTHREAD_PROCESS_PRIVATE );
	
	pthread_cond_init( &m_ListeningThreadCondition, &CondAttr );
	pthread_mutex_init( &m_ListeningThreadMutex, &MutexAttr ); 	
	pthread_cond_init( &m_NewDataCondition, &CondAttr );
	pthread_mutex_init( &m_NewDataMutex, &MutexAttr ); 
	
	m_NewDataTcIndex = -1;
	m_IsListening = 1;
	m_LoopBackFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	pthread_create( &m_ListeningThread, NULL, cMosNii::listenThread, NULL );
}


// Destroys all configurations and open transfer channels
cMosNii::~cMosNii()
{
	pthread_cancel( m_ListeningThread );
	close(m_LoopBackFileDescriptor);

	pthread_cond_destroy( &m_ListeningThreadCondition );
	pthread_mutex_destroy( &m_ListeningThreadMutex );
	pthread_cond_destroy( &m_NewDataCondition );
	pthread_mutex_destroy( &m_NewDataMutex );
	
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
		m_NwData[i].valid = 0;
}


/////////////////////////////////////////////////////////////////////////////
///////////////////////COMMUNICATION SERVICE INTERFACE///////////////////////
/////////////////////////////////////////////////////////////////////////////

// Configure a local communication interface 
ASAAC_NiiReturnStatus cMosNii::configureInterface(
		const ASAAC_NetworkDescriptor& network_id,
		const ASAAC_InterfaceConfigurationData& configuration_data)
{
	NwData* nw = 0;
	long nw_idx = getNwIndex( network_id );
	
	if (nw_idx == -1)//network not configured yet
	{
		nw_idx = getEmptyNw();
		
		if (nw_idx == -1) //no slots free for new network entry
			return ASAAC_MOS_NII_STORAGE_FAULT;

		nw = &m_NwData[nw_idx];

		nw->valid = 1;
		nw->id = network_id;
		nw->config_data = configuration_data;
		nw->open_tcs = 0;

		return ASAAC_MOS_NII_CALL_COMPLETE;
	}
	else //network already configured
	{
		nw = &m_NwData[nw_idx];

		if (nw->open_tcs > 0)
			return ASAAC_MOS_NII_OPEN_TCS;

		nw->config_data = configuration_data;

		return ASAAC_MOS_NII_CALL_COMPLETE;
	}

}


// Configure the local resource to handle the transmission or reception of information over a Transfer Channel (TC)
ASAAC_NiiReturnStatus cMosNii::configureTransfer(ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor& network_id, 
		ASAAC_TransferDirection send_receive,
		ASAAC_TransferType message_streaming,
		ASAAC_TC_ConfigurationData configuration_data, 
		ASAAC_Bool trigger_callback,
		ASAAC_PublicId callback_id)
{
	NwData nw;
	TcData tc;
	struct sockaddr_in addr;
#ifdef UDP_MULTICAST         
	struct ip_mreq multi;
	int option_on = 1;
#endif

	long nw_idx = getNwIndex(network_id);
	
	if (nw_idx == -1)//network not configured yet
	{
		return ASAAC_MOS_NII_INVALID_NETWORK;
	}
	else //network ready to establish transfer connections
	{
		long tc_idx = getTcIndex( tc_id );
		
		if (tc_idx == -1) //TC not established yet
		{
			tc_idx = getEmptyTc();
			
			if (tc_idx == -1)
				return ASAAC_MOS_NII_STORAGE_FAULT;

#ifdef _DEBUG_
			cerr << "cMosNii::configureTransfer(" << tc_idx << ") setup TC" << endl;
#endif
			tc = m_TcData[tc_idx];
			nw = m_NwData[nw_idx];

			tc.hasData = 0;
			tc.id = tc_id;
			tc.nw = &m_NwData[nw_idx];
			tc.direction = send_receive;
			tc.type = message_streaming;
			tc.trigger_callback = trigger_callback;
			tc.callback_id = callback_id;

			switch (message_streaming)
			{
			case ASAAC_TRANSFER_TYPE_MESSAGE:
				tc.fd = socket(AF_INET, SOCK_DGRAM, 0);
				if (tc.fd == -1)
				{
					perror("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) : ");
					return ASAAC_MOS_NII_CALL_FAILED;
				}
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") create datagram socket" << endl;
#endif   
				break;

			case ASAAC_TRANSFER_TYPE_STREAMING:
				tc.fd = socket(AF_INET, SOCK_STREAM, 0);

				if (tc.fd == -1)
				{
					perror("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) : ");
					return ASAAC_MOS_NII_CALL_FAILED;
				}

				tc.config_data.data_length
						= configuration_data.conf_data.tcpip.buffer_length;
				tc.config_data.stream_buffer
						= (char*) configuration_data.conf_data.tcpip.stream_buffer;

#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") create stream socket" << endl;
#endif   
				break;

			default:
				cerr <<"cMosNii::configureTransfer() unknown TransferType :"
						<< message_streaming << endl;
				return ASAAC_MOS_NII_CALL_FAILED;
				break;
			}

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;

			switch (tc.direction)
			{
			case ASAAC_TRANSFER_DIRECTION_SEND:
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") SEND -> local port " << m_NiiLocalPort << endl;
#endif
				addr.sin_port = htons(m_NiiLocalPort++);

				break;
			case ASAAC_TRANSFER_DIRECTION_RECEIVE:
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") RECEIVE -> local port " << nw->id.port << endl;
#endif
				addr.sin_port = htons(nw.id.port);
#ifdef UDP_MULTICAST         
				if(tc->type == ASAAC_TRANSFER_TYPE_MESSAGE)
				{
					setsockopt(tc->fd,SOL_SOCKET,SO_REUSEADDR , &option_on, sizeof(int));
					multi.imr_multiaddr.s_addr = nw->id.network;
					multi.imr_interface.s_addr = INADDR_ANY;
					setsockopt(tc->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP , &multi, sizeof(ip_mreq));
				}
#else
				cerr << "CAUTION: no special treatment of socket for Multicast UDP Messages" << endl;
#endif
				break;
			}

			if (bind(tc.fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			{
#ifdef _DEBUG_
				perror("bind() on socket : ");
#endif
				return ASAAC_MOS_NII_CALL_FAILED;
			}

			nw.open_tcs += 1;

			if(tc.type == ASAAC_TRANSFER_TYPE_MESSAGE)
			{
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") set nonblocking IO " << endl;
#endif
				int val = fcntl(tc.fd, F_GETFL, 0);
				fcntl(tc.fd, F_SETFL, val | O_NONBLOCK); //COMMENT.SMS> add this flag for asynchronous signalling | O_ASYNC
				//COMMENT.SMS> fcntl(tc->fd, F_SETOWN, getpid()); //set owner for asynchronous signalling
			}
			else
			{
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") create thread for streaming IO " << endl;
#endif
				if((pthread_create(&(tc.th), NULL, streamTcThread, (void *) &m_TcData[tc_idx])) !=0)
				{
#ifdef _DEBUG_
					perror("pthread_create() : ");
#endif
					return ASAAC_MOS_NII_CALL_FAILED;
				}
			}

#ifdef _DEBUG_
			cerr << "cMosNii::configureTransfer() : done" << endl;
#endif
		
			// Now the TC can be stored
			stopListening();			
			m_TcData[tc_idx] = tc;
			m_TcData[tc_idx].valid = 1;
			m_NwData[nw_idx] = nw;
			m_NwData[nw_idx].valid = 1;
			startListening();			

			return ASAAC_MOS_NII_CALL_COMPLETE;
		}
		else //TC already configured
		{
			return ASAAC_MOS_NII_ALREADY_CONFIGURED;
		}
	}

#ifdef _DEBUG_
	perror("configureTransfer() : ");
#endif

	return ASAAC_MOS_NII_CALL_FAILED;
}


// Send a block of data on the given TC
ASAAC_NiiReturnStatus cMosNii::sendTransfer(
		ASAAC_PublicId tc_id,
		const char* transmit_data, 
		unsigned long data_length, 
		ASAAC_Time time_out)
{
	TcData* tc = 0;

	long tc_idx = getTcIndex(tc_id);
	
	if ( tc_idx == -1 ) //TC not established yet
	{
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;
	}
	else
	{
		tc = &m_TcData[tc_idx];

		if (tc->type != ASAAC_TRANSFER_TYPE_MESSAGE)
		{
#ifdef _DEBUG_
			cerr << "ERROR: no message TC " << endl;
#endif
			return ASAAC_MOS_NII_CALL_FAILED;
		}

		if (tc->direction != ASAAC_TRANSFER_DIRECTION_SEND)
		{
#ifdef _DEBUG_
			cerr << "ERROR: no send TC " << endl;
#endif
			return ASAAC_MOS_NII_CALL_FAILED;
		}

		int bytes = 0;
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = tc->nw->id.network;
		addr.sin_port = htons(tc->nw->id.port);

		struct timespec tp;

		do
		{
			bytes = sendto(tc->fd, transmit_data, data_length, 0,
					(struct sockaddr *) &addr, sizeof(addr));

			if( bytes < 0)
			{
				if(errno != EWOULDBLOCK)
				{
#ifdef _DEBUG_
					perror("sendto() : ");
					if(errno == EAFNOSUPPORT)
					{
						cerr << "ADDRESS FAMILY NOT SUPPORTED" << endl;
					}
#endif
					return ASAAC_MOS_NII_CALL_FAILED;
				}
			}
			else
			{
				if (tc->trigger_callback == ASAAC_BOOL_TRUE)
					ASAAC_MOS_callbackHandler( ASAAC_COMMS_EV_BUFFER_SEND, tc->callback_id, NULL );
				
				return ASAAC_MOS_NII_CALL_OK;
			}

			if(clock_gettime(CLOCK_REALTIME, &tp))
			{
				perror("GET REALTIME CLOCK :");
				return ASAAC_MOS_NII_CALL_FAILED;
			}

		}
		while( lower(TimespecToTime(tp), time_out) ); //loop until time is over

	}//TC installed

	return ASAAC_MOS_NII_CALL_FAILED;
}


// Receive a block of data on the given TC
ASAAC_NiiReturnStatus cMosNii::receiveTransfer(
		ASAAC_PublicId tc_id, 
		ASAAC_CharAddress& receive_data,
		unsigned long data_length_available, 
		unsigned long& data_length, 
		ASAAC_Time time_out)
{
	if (data_length_available > NII_MAX_SIZE_OF_RECEIVEBUFFER)
	{
		cerr << "ASAAC_MOS_receiveTransfer() "<< data_length_available
			 << " >  NII_RECEIVE_BUFFER_SIZE"<< endl;
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	m_CurrentBuffer = (m_CurrentBuffer + 1) % NII_MAX_NUMBER_OF_RECEIVEBUFFERS;

	receive_data = m_ReceiveBuffer[m_CurrentBuffer];

	TcData* tc = 0;

	long tc_idx = getTcIndex(tc_id);
	
	if (tc_idx == -1) //TC not established yet
	{
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;
	}
	else
	{
		tc = &m_TcData[tc_idx];

		if (tc->type != ASAAC_TRANSFER_TYPE_MESSAGE)
		{
#ifdef _DEBUG_
			cerr << "ERROR: no message TC " << endl;
#endif
			return ASAAC_MOS_NII_CALL_FAILED;
		}

		if (tc->direction != ASAAC_TRANSFER_DIRECTION_RECEIVE)
		{
#ifdef _DEBUG_
			cerr << "ERROR: no receive TC " << endl;
#endif
			return ASAAC_MOS_NII_CALL_FAILED;
		}

		int bytes = 0;

		struct sockaddr_in addr;
		socklen_t length = (socklen_t) sizeof(addr);

		struct timespec tp;

		waitForData(time_out, tc_id);
		
		bytes = recvfrom(tc->fd, receive_data, data_length_available, 0, (struct sockaddr *) &addr, &length);

		if( bytes < 0)
		{
			if(errno != EWOULDBLOCK)
			{
				perror("RECEIVE FROM SOCKET :");
				return ASAAC_MOS_NII_CALL_FAILED;
			}
		}
		else
		{
			data_length = bytes;
#ifdef _DEBUG_
			cout << "RECEIVE " << bytes << " BYTE ON TC " << tc_id << endl;
#endif

			return ASAAC_MOS_NII_CALL_COMPLETE;
		}
	}//TC installed

	return ASAAC_MOS_NII_CALL_FAILED;
}


ASAAC_NiiReturnStatus cMosNii::receiveNetwork(
		const ASAAC_NetworkDescriptor& network_id,
		ASAAC_CharAddress& receive_data, 
		unsigned long data_length_available, 
		unsigned long& data_length,
		ASAAC_PublicId& tc_id, 
		ASAAC_Time time_out)
{
	if (data_length_available > NII_MAX_SIZE_OF_RECEIVEBUFFER)
	{
		cerr << "ASAAC_MOS_receiveTransfer() "<< data_length_available
				<< " >  NII_MAX_SIZE_OF_RECEIVEBUFFER"<< endl;
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	m_CurrentBuffer = (m_CurrentBuffer + 1) % NII_MAX_NUMBER_OF_RECEIVEBUFFERS;

	receive_data = m_ReceiveBuffer[m_CurrentBuffer];

	waitForDataOnNw(time_out, network_id);

	return receiveTransfer(tc_id, receive_data, data_length_available, data_length, time_out);
}

// Release local resources previously allocated to handle the transmission or reception of information over a TC
ASAAC_NiiReturnStatus cMosNii::destroyTransfer(
		ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor& network_id)
{
	TcData* tc = NULL;

	long tc_idx = getTcIndex(tc_id);
	
	if (tc_idx == -1) //TC not established yet
	{
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;
	}
	else
	{
		stopListening();
		
		tc = &m_TcData[tc_idx];

		tc->valid = 0;
		tc->nw->open_tcs -= 1;
		close(tc->fd);

		startListening();
		
		return ASAAC_MOS_NII_CALL_COMPLETE;
	}

#ifdef _DEBUG_
	cerr << "ERROR: destroyTransfer()" << endl;
#endif
	return ASAAC_MOS_NII_CALL_FAILED;
}




///////////////////////////////////////////////////////////////////////////
///////////////////////PRIVATE AND PROTECTED METHODS///////////////////////
///////////////////////////////////////////////////////////////////////////


// Returns TRUE, if network descriptor is available and associated with a valid interface and socket
long cMosNii::getNwIndex(const ASAAC_NetworkDescriptor& network_id)
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
	{
		if (m_NwData[i].valid && memcmp(&m_NwData[i].id, &network_id,
				sizeof(ASAAC_NetworkDescriptor)) == 0)
		{
			return i;
		};
	}
	
	return -1;
}


// Returns TRUE, if the transfer connection is available and associated with a valid network interface
long cMosNii::getTcIndex(const ASAAC_PublicId tc_id)
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
	{
		if (m_TcData[i].valid && m_TcData[i].id == tc_id)
		{
			return i;
		};
	}
	
	return -1;
}


// Returns TRUE, if an invalid, empty slot for a new connection was found and index is given as argument
long cMosNii::getEmptyTc()
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
	{
		if (m_TcData[i].valid == 0)
		{
			return i;
		};
	}
	
	return -1;
}


// Returns TRUE, if an invalid, empty slot for a new network socket was found and index is given as argument
long cMosNii::getEmptyNw()
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
	{
		if (m_NwData[i].valid == 0)
		{
			return i;
		};
	}
	
	return -1;
}


// Returns a character string representation of the return status code
char* cMosNii::spell(ASAAC_NiiReturnStatus status)
{
	switch (status)
	{
	case ASAAC_MOS_NII_CALL_FAILED:
		return "ASAAC_MOS_NII_CALL_FAILED";
	case ASAAC_MOS_NII_CALL_COMPLETE:
		return "ASAAC_MOS_NII_CALL_COMPLETE";
	case ASAAC_MOS_NII_CALL_OK:
		return "ASAAC_MOS_NII_CALL_OK";
	case ASAAC_MOS_NII_INVALID_INTERFACE:
		return "ASAAC_MOS_NII_INVALID_INTERFACE";
	case ASAAC_MOS_NII_INVALID_CONFIG:
		return "ASAAC_MOS_NII_INVALID_CONFIG";
	case ASAAC_MOS_NII_INVALID_NETWORK:
		return "ASAAC_MOS_NII_INVALID_NETWORK";
	case ASAAC_MOS_NII_INVALID_TC:
		return "ASAAC_MOS_NII_INVALID_TC";
	case ASAAC_MOS_NII_INVALID_MESSAGE_SIZE:
		return "ASAAC_MOS_NII_INVALID_MESSAGE_SIZE";
	case ASAAC_MOS_NII_OPEN_TCS:
		return "ASAAC_MOS_NII_OPEN_TCS";
	case ASAAC_MOS_NII_ALREADY_CONFIGURED:
		return "ASAAC_MOS_NII_ALREADY_CONFIGURED";
	case ASAAC_MOS_NII_TC_NOT_CONFIGURED:
		return "ASAAC_MOS_NII_TC_NOT_CONFIGURED";
	case ASAAC_MOS_NII_STORAGE_FAULT:
		return "ASAAC_MOS_NII_STORAGE_FAULT";
	case ASAAC_MOS_NII_BUFFER_EMPTY:
		return "ASAAC_MOS_NII_BUFFER_EMPTY";
	case ASAAC_MOS_NII_BUFFER_NOT_READY:
		return "ASAAC_MOS_NII_BUFFER_NOT_READY";
	case ASAAC_MOS_NII_STATUS_ERROR:
		return "ASAAC_MOS_NII_STATUS_ERROR";
	case ASAAC_MOS_NII_STATUS_INIT:
		return "ASAAC_MOS_NII_STATUS_INIT";
	case ASAAC_MOS_NII_STATUS_OK:
		return "ASAAC_MOS_NII_STATUS_OK";

	default:
		return "MOS NII ERROR: undefined return status";
	}
}

// This function is called as a new thread, which writes/reads data to/from a stream-based TC
void* cMosNii::streamTcThread(void* pTcData)
{
#ifdef _DEBUG_
	cerr << "streamTcThread: starting" << endl;
#endif

	void* nothing = (void*) 0;

	if (pTcData == 0)
	{
#ifdef _DEBUG_
		cerr << "streamTcThread: wrong argument. Must be of type TcData*" << endl;
#endif
		pthread_exit(nothing);
	}

	TcData* tc = static_cast<TcData*>(pTcData );

	if (tc->type == ASAAC_TRANSFER_TYPE_MESSAGE)
	{
#ifdef _DEBUG_
		cerr << "streamTcThread: wrong type of TC. Must be of type NII_TC_STREAM" << endl;
#endif
		pthread_exit(nothing);
	}

	if (tc->direction == ASAAC_TRANSFER_DIRECTION_SEND)
	{
		struct sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = tc->nw->id.network;
		server.sin_port = htons(tc->nw->id.port);
		int written_bytes = 0; //actually written bytes
		unsigned long os = 0; //offset in buffer

		while (connect(tc->fd, (const sockaddr*) &server,
				sizeof(struct sockaddr_in)) < 0)
		{
#ifdef _DEBUG_
			cerr << "streamTcThread: ERROR while connecting to server" << endl;
			cerr << "ERRNO: " << errno << endl;
#endif
			sleep(1);
		}

#ifdef _DEBUG_
		cerr << "streamTcThread: SUCCESSFULLY connecting to server" << endl;
#endif
		while (tc->valid == 1)
		{
			if (*tc->config_data.data_length > 0)
			{
#ifdef _DEBUG_
				cerr << "streamTcThread: try to write " << *tc->config_data.data_length << " byte to socket " << endl;
#endif
				written_bytes = write(tc->fd, tc->config_data.stream_buffer+os,
						*tc->config_data.data_length);
				if (written_bytes < 0)
				{
#ifdef _DEBUG_
					cerr << "streamTcThread: FAILED to write" << *tc->config_data.data_length << " byte to socket " << endl;
					cerr << "ERRNO: " << errno << endl;
#endif
				}//failed write to socket
				else
				{
#ifdef _DEBUG_
					cerr << "streamTcThread: wrote " << written_bytes << " byte to socket " << endl;
#endif
					if (written_bytes < (long) *tc->config_data.data_length)
					{
						os += written_bytes;
						*tc->config_data.data_length -= written_bytes;
					}
					else
					{
						os = 0;
						*tc->config_data.data_length = 0;
					}
#ifdef _DEBUG_
					cerr << "streamTcThread: offset " << os << " byte on buffer and " << *tc->config_data.data_length << " byte left to write" << endl;
#endif
				}//successful written to socket
			}//data available
		}//tc valid
	}//sender
	else if (tc->direction == ASAAC_TRANSFER_DIRECTION_RECEIVE)
	{
		struct sockaddr_in client;
		int clnt_len;

		listen(tc->fd, 5);

		while (tc->valid == 1)
		{
#ifdef _DEBUG_
			cerr << "streamTcThread: accepting clients connection" << endl;
#endif
			int client_socket = accept(tc->fd, (sockaddr*) &client,
					(socklen_t*) &clnt_len);
			int read_bytes = -1;

			if (client_socket < 0)
			{
#ifdef _DEBUG_
				cerr << "streamTcThread: ERROR while accepting client" << endl;
				cerr << "ERRNO: " << errno << endl;
#endif
				pthread_exit(nothing);
			}
			else
			{
#ifdef _DEBUG_
				cerr << "streamTcThread: SUCCESSFULLY accepting client" << endl;
#endif
				while (read_bytes != 0)
				{
					if (*tc->config_data.data_length == 0)
					{
#ifdef _DEBUG_
						cerr << "streamTcThread: try to read from socket " << endl;
#endif
						read_bytes = read(client_socket,
								tc->config_data.stream_buffer,
								NII_MAX_SIZE_OF_STREAMBUFFER);
						if (read_bytes < 0)
						{
#ifdef _DEBUG_
							cerr << "streamTcThread: FAILED to read from socket " << endl;
							cerr << "ERRNO: " << errno << endl;
#endif
						}//failed to read from socket
						else if (read_bytes > 0)
						{
#ifdef _DEBUG_
							cerr << "streamTcThread: read " << read_bytes << " byte from socket " << endl;
#endif
							*tc->config_data.data_length = read_bytes;
						}//successful read from socket
						else
						{
							cerr << "ERRNO: "<< errno<< endl;
						}
					}//buffer empty
				}//stilll bytes to read
				close(client_socket);
			}//accepted client to server
		}//TC is valid
	}//receiver

#ifdef _DEBUG_
	cerr << "streamTcThread: exit" << endl;
#endif

	pthread_exit(nothing);
}


void* cMosNii::listenThread(void* Data)
{
	cMosNii *Nii = cMosNii::getInstance();

	for (;;)
	{
		fd_set rfds;
		int max_fd = Nii->m_LoopBackFileDescriptor;
	
		FD_ZERO(&rfds);
		FD_SET(Nii->m_LoopBackFileDescriptor, &rfds);
		
		for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
		{
			if (( Nii->m_TcData[i].valid ) && 
				( Nii->m_TcData[i].direction == ASAAC_TRANSFER_DIRECTION_RECEIVE ) && 
				( Nii->m_TcData[i].hasData == 0 ))
			{
				//TC is valid and the Network is the same as required, then push file descriptor in set for select call
				FD_SET(Nii->m_TcData[i].fd, &rfds);
				
				if (max_fd < Nii->m_TcData[i].fd)
					max_fd = Nii->m_TcData[i].fd;
			}
		}	
		
		struct timeval tv = {1000,0}; //TODO: Shall be infinity
		int retval = select(max_fd+1, &rfds, 0, 0, &tv);
	
		if (retval == -1)
		{
			cerr << "cMosNii::listen thread() ERROR on select()" << endl;
		}

		if ( FD_ISSET(Nii->m_LoopBackFileDescriptor, &rfds) )
		{
			CallingThreadData Data;
			unsigned long Length = sizeof(CallingThreadData);
			
			in_addr NetworkAddr;
			inet_aton("127.0.0.1", &NetworkAddr);

			struct sockaddr_in Addr;
			Addr.sin_family = AF_INET;
			Addr.sin_addr = NetworkAddr;
			Addr.sin_port = 0xffff;
			
			size_t AddrLength = sizeof(struct sockaddr_in);
			
			recvfrom(Nii->m_LoopBackFileDescriptor, &Data, Length, 0, (struct sockaddr *) &Addr, &AddrLength); 
			
			pthread_cond_broadcast(&Data.CallingThreadCondition);			
			
			Nii->m_IsListening = 0;
			pthread_cond_wait( &Nii->m_ListeningThreadCondition, &Nii->m_ListeningThreadMutex );
			Nii->m_IsListening = 1;
		}
		else
		{
			for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
			{
				if (( Nii->m_TcData[i].valid == 1 ) && 
					( Nii->m_TcData[i].direction == ASAAC_TRANSFER_DIRECTION_RECEIVE ))
				{
					if ( FD_ISSET(Nii->m_TcData[i].fd, &rfds) )
					{
						Nii->m_TcData[i].hasData = 1;
						
						if (Nii->m_TcData[i].trigger_callback == ASAAC_BOOL_TRUE)
							ASAAC_MOS_callbackHandler( ASAAC_COMMS_EV_BUFFER_RECEIVED, Nii->m_TcData[i].callback_id, NULL );
						
						m_NewDataTcIndex = i;
						
						pthread_cond_broadcast(&Nii->m_NewDataCondition);			
					}
				}
			}
		}
	}
	
	return NULL;
}


ASAAC_TimedReturnStatus cMosNii::waitForData( const ASAAC_Time time_out, ASAAC_PublicId &tc_id )
{
	struct timespec TimespecTimeout = TimeToTimespec( time_out );
	int iErrorCode = pthread_cond_timedwait( &m_NewDataCondition, &m_NewDataMutex, &TimespecTimeout );
	
	if ( iErrorCode == ETIMEDOUT )
		return ASAAC_TM_TIMEOUT;

	if ( iErrorCode == -1 )
		return ASAAC_TM_ERROR;
	
	tc_id = m_NewDataTcIndex;
	
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus cMosNii::waitForDataOnTc( const ASAAC_Time time_out, const ASAAC_PublicId tc_id )
{
	if (getTcIndex(tc_id) == -1) //TC not established yet
		return ASAAC_TM_ERROR;

	ASAAC_PublicId ReceivedTcId;
	
	do
	{
		ASAAC_TimedReturnStatus Result = waitForData( time_out, ReceivedTcId );
		
		if ( Result != ASAAC_TM_SUCCESS )
			return Result;
	}	
	while ( ReceivedTcId != tc_id );
	
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus cMosNii::waitForDataOnNw( const ASAAC_Time time_out, const ASAAC_NetworkDescriptor& network_id )
{
	if ( getNwIndex(network_id) == -1 ) //TC not established yet
		return ASAAC_TM_ERROR;
	
	ASAAC_PublicId 			ReceivedTcId;
	long 		   			ReceivedTcIndex;
	ASAAC_NetworkDescriptor ReceivedNetworkId;
	
	do
	{
		ASAAC_TimedReturnStatus Result = waitForData( time_out, ReceivedTcId );
		
		if ( Result != ASAAC_TM_SUCCESS )
			return Result;
		
		ReceivedTcIndex = getTcIndex( ReceivedTcId );
		if (ReceivedTcIndex == -1) //TC not established yet
			return ASAAC_TM_ERROR;
		
		ReceivedNetworkId = m_TcData[ReceivedTcIndex].nw->id;
	}	
	while (( ReceivedNetworkId.network != network_id.network ) || ( ReceivedNetworkId.port != network_id.port ));
	
	return ASAAC_TM_SUCCESS;
}


void cMosNii::startListening()
{
	if (m_IsListening == 1)
		return;
	
	pthread_cond_broadcast(&m_ListeningThreadCondition);			
}


void cMosNii::stopListening()
{	
	if (m_IsListening == 0)
		return;
	
	CallingThreadData Data;
	unsigned long Length = sizeof(CallingThreadData);
	
	pthread_condattr_t CondAttr;
	pthread_condattr_init( &CondAttr );
	pthread_condattr_setpshared( &CondAttr, PTHREAD_PROCESS_PRIVATE );
	pthread_cond_init( &Data.CallingThreadCondition, &CondAttr );
	
	pthread_mutexattr_t MutexAttr;
	pthread_mutexattr_init( &MutexAttr );    
	pthread_mutexattr_setpshared( &MutexAttr, PTHREAD_PROCESS_PRIVATE );
	pthread_mutex_init( &Data.CallingThreadMutex, &MutexAttr ); 	
	
	in_addr NetworkAddr;
	inet_aton("127.0.0.1", &NetworkAddr);

	struct sockaddr_in Addr;
	Addr.sin_family = AF_INET;
	Addr.sin_addr = NetworkAddr;
	Addr.sin_port = 0xffff;
	
	sendto(m_LoopBackFileDescriptor, &Data, Length, 0, (struct sockaddr *) &Addr, sizeof(Addr));
	
	pthread_cond_wait( &Data.CallingThreadCondition, &Data.CallingThreadMutex );

	pthread_cond_destroy( &Data.CallingThreadCondition );
	pthread_mutex_destroy( &Data.CallingThreadMutex );
}


// Returns a network address based on an IP4 address format X.X.X.X
ASAAC_PublicId cMosNii::networkAddress(const char* ip_addr)
{
#ifdef _DEBUG_    
	cout << "networkAddress: " << ip_addr << endl;
#endif
	return inet_addr(ip_addr);
}

ASAAC_PublicId cMosNii::getLocalNetwork()
{
	return inet_addr("127.0.0.1");
}

// Returns a multicast address based on a network identifier between 3-255
ASAAC_PublicId cMosNii::networkGroup(char address)
{
	char ip_addr[16];
	sprintf(ip_addr, "224.0.0.%d", address);
#ifdef _DEBUG_ 
	cout << "networkGroup: " << ip_addr << endl;
#endif

	return inet_addr(ip_addr);
}

