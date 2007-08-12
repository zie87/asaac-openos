#include "TcListener.hh"

#include "PcsHIncludes.hh"

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

ASAAC_TimedReturnStatus TcListener::listen(const ASAAC_PublicId& tc_id, const ASAAC_Address buffer, const unsigned long max_length, const ASAAC_TimeInterval& Timeout )
{
    ASAAC_CharAddress receive_data = (ASAAC_CharAddress)buffer;
    ASAAC_Length data_length = max_length;
    
#ifdef _DEBUG_
    cout << "TcListener::listen() on tc " << tc_id << endl;
#endif
    
    ASAAC_Time AbsTime = TimeStamp(Timeout).asaac_Time();

    ASAAC_NiiReturnStatus ret = ASAAC_MOS_receiveTransfer(tc_id, &receive_data, PCS_MAX_SIZE_OF_NWMESSAGE, &data_length, &AbsTime);
    
#ifdef _DEBUG_
    cout << "TcListener::listen() ASAAC_MOS_receiveTransfer() returned " << endl;
    if(ret == ASAAC_MOS_NII_CALL_COMPLETE)
    {
    	cout << "TcListener::listen() received data from TC: " << tc_id << " of length " << data_length << endl;
    }
#endif

    if(ret != ASAAC_MOS_NII_CALL_COMPLETE)
        return ASAAC_TM_TIMEOUT;
    
    if(m_Consumer->processTcMessage(tc_id, (ASAAC_Address)receive_data, data_length) != ASAAC_SUCCESS)
    {
     	cerr << "TcListener::listen() could not processTcMessage for TC " << tc_id << endl;
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
};
