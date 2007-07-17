#include "TcListener.hh"

TcListener::TcListener()
{
    
};


void TcListener::initialize()
{

}


void TcListener::deinitialize()
{
	
}

    
ASAAC_ReturnStatus TcListener::addListeningConsumer(TcMessageConsumer& Consumer )
{
    m_Consumer = &Consumer;
    return ASAAC_SUCCESS;
};

ASAAC_TimedReturnStatus TcListener::listen(const ASAAC_NetworkDescriptor& network_id, const ASAAC_TimeInterval& Timeout )
{
    ASAAC_CharAddress receive_data;
    ASAAC_Length data_length;
    ASAAC_PublicId tc_id;
#ifdef _DEBUG_
    cout << "TcListener::listen() on network [" << network_id.network << "," << network_id.port << "]" << endl;
#endif    
    ASAAC_Time AbsTime = TimeStamp(Timeout).asaac_Time();
    
    ASAAC_NiiReturnStatus ret = ASAAC_MOS_receiveNetwork(&network_id, &receive_data, PCS_MAX_SIZE_OF_NWMESSAGE, &data_length, &tc_id, &AbsTime);
#ifdef _DEBUG_
    cout << "TcListener::listen() ASAAC_MOS_receiveNetwork() returned " << cMosNii::spell(ret) << endl;
    if(ret == ASAAC_MOS_NII_CALL_COMPLETE)
    {
    	cout << "TcListener::listen() received data : " << " from TC " << tc_id << " of length " << data_length << endl;
    }
#endif

    if(ret != ASAAC_MOS_NII_CALL_COMPLETE)
        return ASAAC_TM_TIMEOUT;;
    
    ASAAC_PublicId tc_header = ntohl(* ((long*)receive_data));
	
	if(tc_header != tc_id)
	{
     	cerr << "TcListener::listen() wrong TC header " << tc_header << " while receiving data for TC " << tc_id << endl;
		//return ASAAC_TM_ERROR;
	}
 
    if(m_Consumer->processTcMessage(tc_header,(ASAAC_Address) (receive_data + 4),data_length-4) != ASAAC_SUCCESS)
    {
     	cerr << "TcListener::listen() could not processTcMessage for TC " << tc_id << endl;
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
};
