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
}


// Destroys all configurations and open transfer channels
cMosNii::~cMosNii()
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
		m_NwData[i].valid = 0;
}


/////////////////////////////////////////////////////////////////////////////
///////////////////////COMMUNICATION SERVICE INTERFACE///////////////////////
/////////////////////////////////////////////////////////////////////////////

// Configure a local communication interface 
ASAAC_NiiReturnStatus cMosNii::configureInterface(
		ASAAC_PublicId interface_id,
		const ASAAC_NetworkDescriptor& network_id,
		const ASAAC_InterfaceConfigurationData& configuration_data)
{
	unsigned long nw_idx;
	NwData* nw = 0;

	if (interface_id != NII_IF_ETHERNET)
		return ASAAC_MOS_NII_INVALID_INTERFACE;

	if (getIndex(nw_idx, network_id) == false) //network not configured yet
	{
		if (getEmptyNw(nw_idx) == false) //no slots free for new network entry
			return ASAAC_MOS_NII_STORAGE_FAULT;

		nw = &m_NwData[nw_idx];

		nw->valid = 1;
		nw->id = network_id;
		nw->interface = interface_id;
		nw->config_data = configuration_data;
		nw->open_tcs = 0;

		return ASAAC_MOS_NII_CALL_COMPLETE;
	}
	else //network already configured
	{
		nw = &m_NwData[nw_idx];

		if (nw->interface == interface_id)
			return ASAAC_MOS_NII_ALREADY_CONFIGURED;

		if (nw->open_tcs > 0)
			return ASAAC_MOS_NII_OPEN_TCS;

		nw->interface = interface_id;
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
	unsigned long nw_idx;
	unsigned long tc_idx;
	NwData* nw = 0;
	TcData* tc = 0;
	struct sockaddr_in addr;
#ifdef UDP_MULTICAST         
	struct ip_mreq multi;
	int option_on = 1;
#endif

	if (getIndex(nw_idx, network_id) == false) //network not configured yet
	{
		return ASAAC_MOS_NII_INVALID_NETWORK;
	}
	else //network ready to establish transfer connections
	{
		if (getIndex(tc_idx, tc_id) == false) //TC not established yet
		{
			if (getEmptyTc(tc_idx) == false)
				return ASAAC_MOS_NII_STORAGE_FAULT;

#ifdef _DEBUG_
			cerr << "cMosNii::configureTransfer(" << tc_idx << ") setup TC" << endl;
#endif
			tc = &m_TcData[tc_idx];

			nw = &m_NwData[nw_idx];

			tc->valid = 1;
			tc->id = tc_id;

			switch (message_streaming)
			{
			case ASAAC_TRANSFER_TYPE_MESSAGE:
				tc->fd = socket(AF_INET, SOCK_DGRAM, 0);
				if (tc->fd == -1)
				{
					perror("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) : ");
					return ASAAC_MOS_NII_CALL_FAILED;
				}
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") create datagram socket" << endl;
#endif   
				break;

			case ASAAC_TRANSFER_TYPE_STREAMING:
				tc->fd = socket(AF_INET, SOCK_STREAM, 0);

				if (tc->fd == -1)
				{
					perror("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) : ");
					return ASAAC_MOS_NII_CALL_FAILED;
				}

				tc->config_data.data_length
						= configuration_data.conf_data.tcpip.buffer_length;
				tc->config_data.stream_buffer
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

			tc->nw = nw;
			tc->direction = send_receive;
			tc->type = message_streaming;

			tc->callback_id = trigger_callback ? callback_id : 0;

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;

			switch (tc->direction)
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
				addr.sin_port = htons(nw->id.port);
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

			if (bind(tc->fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			{
#ifdef _DEBUG_
				perror("bind() on socket : ");
#endif
				return ASAAC_MOS_NII_CALL_FAILED;
			}

			nw->open_tcs += 1;

			if(tc->type == ASAAC_TRANSFER_TYPE_MESSAGE)
			{
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") set nonblocking IO " << endl;
#endif
				int val = fcntl(tc->fd, F_GETFL, 0);
				fcntl(tc->fd, F_SETFL, val | O_NONBLOCK); //COMMENT.SMS> add this flag for asynchronous signalling | O_ASYNC
				//COMMENT.SMS> fcntl(tc->fd, F_SETOWN, getpid()); //set owner for asynchronous signalling
			}
			else
			{
#ifdef _DEBUG_
				cerr << "cMosNii::configureTransfer(" << tc_idx << ") create thread for streaming IO " << endl;
#endif
				if((pthread_create(&(tc->th), NULL, streamTcThread, (void *) tc)) !=0)
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
	unsigned long tc_idx;
	TcData* tc = 0;

	if (getIndex(tc_idx, tc_id) == false) //TC not established yet
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

	unsigned long tc_idx;
	TcData* tc = 0;

	if (getIndex(tc_idx, tc_id) == false) //TC not established yet
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

		do
		{
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

			if(clock_gettime(CLOCK_REALTIME, &tp))
			{
				perror("GET REALTIME CLOCK :");
				return ASAAC_MOS_NII_CALL_FAILED;
			}

		}
		while(tp.tv_sec > time_out.sec && tp.tv_nsec > time_out.nsec); //loop until time is over

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
				<< " >  NII_RECEIVE_BUFFER_SIZE"<< endl;
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	m_CurrentBuffer = (m_CurrentBuffer + 1) % NII_MAX_NUMBER_OF_RECEIVEBUFFERS;

	receive_data = m_ReceiveBuffer[m_CurrentBuffer];

	fd_set rfds;
	struct timeval tv;
	int retval;
	struct timespec tp;

	if (clock_gettime(CLOCK_REALTIME, &tp))
	{
		perror("GET REALTIME CLOCK");
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	tv.tv_sec = time_out.sec - tp.tv_sec;
	if (tp.tv_nsec > time_out.nsec)
	{
		if (tv.tv_sec > 0)
		{
			tv.tv_sec -= 1;
			time_out.nsec += 1000000000;
		}
		else
		{
			time_out.nsec = tp.tv_nsec + 1000;
		}
	}
	tv.tv_usec = (time_out.nsec - tp.tv_nsec) / 1000; //convert to microseconds

#ifdef _DEBUG_
	cout << "cMosNii::receiveNetwork() has " << tv.tv_sec << " sec " << tv.tv_usec << " usec time to wait for socket events" << endl;
#endif
	int max_fd = 0;

	FD_ZERO(&rfds);

#ifdef _DEBUG_
	cout << "cMosNii::receiveNetwork() adding all TCs on network [" << network_id.network << "," << network_id.port << "] to selection " << endl;
#endif

	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
	{
		if ( (m_TcData[i].valid && m_TcData[i].nw->id.network == network_id.network) &&
			 (m_TcData[i].nw->id.port == network_id.port) )
		{
			//TC is valid and the Network is the same as required, then push file descriptor in set for select call
			FD_SET(m_TcData[i].fd, &rfds)
			
;			if (max_fd < m_TcData[i].fd)
				max_fd = m_TcData[i].fd;
#ifdef _DEBUG_
			cout << "cMosNii::receiveNetwork() adding TC " << m_TcData[i].id << " to selection " << endl;
#endif
		}
		else
		{

#ifdef _DEBUG_
			if(m_TcData[i].valid)
			{
				cout << "cMosNii::receiveNetwork() ignoring TC " << m_TcData[i].id << " on network  [" << m_TcData[i].nw->id.network << "," << m_TcData[i].nw->id.port << "]" << endl;
			}
#endif          
		}
	}

#ifdef _DEBUG_
	cout << "cMosNii::receiveNetwork() finished adding TCs to selection " << endl;
#endif

	retval = select(max_fd+1, &rfds, 0, 0, &tv);
	//Don't rely on the value of tv now! 

	if (retval == -1)
	{
		cerr << "cMosNii::receiveNetwork() ERROR on select()" << endl;
		return ASAAC_MOS_NII_CALL_FAILED;
	}
	else if (retval)
	{
#ifdef _DEBUG_
		cerr << "cMosNii::receiveNetwork() Data is available now." << endl;
#endif

		for(unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
		{
			//FD_ISSET(fd, &rfds) will be true

			if(FD_ISSET(m_TcData[i].fd, &rfds))
			{
				if(!m_TcData[i].valid)
				{
					cerr << "cMosNii::receiveNetwork() data is available but TC is not valid" << endl;
					return ASAAC_MOS_NII_CALL_FAILED;
				}
				tc_id = m_TcData[i].id;
				break;
			}
		}
	}
	else
	{
#ifdef _DEBUG_
		cout << "cMosNii::receiveNetwork() No data within timeout." << endl;
#endif
		return ASAAC_MOS_NII_CALL_FAILED;
	}

	return receiveTransfer(tc_id, receive_data, data_length_available, data_length, time_out);
}

// Release local resources previously allocated to handle the transmission or reception of information over a TC
ASAAC_NiiReturnStatus cMosNii::destroyTransfer(
		ASAAC_PublicId tc_id,
		const ASAAC_NetworkDescriptor& network_id)
{
	unsigned long tc_idx;
	TcData* tc = 0;

	if (getIndex(tc_idx, tc_id) == false) //TC not established yet
	{
		return ASAAC_MOS_NII_TC_NOT_CONFIGURED;
	}
	else
	{
		tc = &m_TcData[tc_idx];

		tc->valid = 0;

		close(tc->fd);

		tc->nw->open_tcs -= 1;

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
bool cMosNii::getIndex(unsigned long& index, const ASAAC_NetworkDescriptor& network_id)
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
	{
		if (m_NwData[i].valid && memcmp(&m_NwData[i].id, &network_id,
				sizeof(ASAAC_NetworkDescriptor)) == 0)
		{
			index = i;
			return true;
		};
	}
	return false;
}


// Returns TRUE, if the transfer connection is available and associated with a valid network interface
bool cMosNii::getIndex(unsigned long& index, ASAAC_PublicId tc_id)
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
	{
		if (m_TcData[i].valid && m_TcData[i].id == tc_id)
		{
			index = i;
			return true;
		};
	}
	return false;
}


// Returns TRUE, if an invalid, empty slot for a new connection was found and index is given as argument
bool cMosNii::getEmptyTc(unsigned long& index)
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_TC_CONNECTIONS; ++i)
	{
		if (m_TcData[i].valid == 0)
		{
			index = i;
			return true;
		};
	}
	return false;
}


// Returns TRUE, if an invalid, empty slot for a new network socket was found and index is given as argument
bool cMosNii::getEmptyNw(unsigned long& index)
{
	for (unsigned long i = 0; i < NII_MAX_NUMBER_OF_NETWORKS; ++i)
	{
		if (m_NwData[i].valid == 0)
		{
			index = i;
			return true;
		};
	}
	return false;
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

