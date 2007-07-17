#include "TcSender.hh"
#include <iostream>

using namespace std;

TcSender::TcSender()
{
}


void TcSender::initialize()
{

}


void TcSender::deinitialize()
{
	
}


ASAAC_ReturnStatus TcSender::processTcMessage( ASAAC_PublicId TcId, ASAAC_Address Data, unsigned long Length )
{
#ifdef _DEBUG_       
	cout << "TcSender::processTcMessage(" << TcId<< "," << Data << "," << Length << ")" << endl;
#endif	

    ASAAC_Time instantan = {0,0};    
#ifdef _DEBUG_
    cout << "TcSender::processTcMessage() calling ASAAC_MOS_sendTransfer()" << endl;
#endif    
    ASAAC_NiiReturnStatus ret = ASAAC_MOS_sendTransfer(TcId, (ASAAC_CharAddress) Data, Length, &instantan);
#ifdef _DEBUG_  
    cout << "ASAAC_MOS_sendTransfer() returned " << cMosNii::spell(ret) << endl;
#endif
    if(  ret == ASAAC_MOS_NII_CALL_COMPLETE  || ret == ASAAC_MOS_NII_CALL_OK)
    {
        return ASAAC_SUCCESS;
    }
    else
    {
        return  ASAAC_ERROR;  
    }
}
