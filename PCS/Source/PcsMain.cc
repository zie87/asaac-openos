#include "OpenOS.hh"
#include "PcsCIncludes.hh"

//#include "Common/Aid.h"
#include "LogicalInterfaces/asaac_pcs.hh"

#include "PcsCIncludes.hh"
#include "PcsObject.hh"

#include <iostream>


using namespace std;
using namespace ASAAC::PCS;


ASAAC_APPLICATION


ASAAC::PCS::Server 	pcsServer; 


ASAAC_THREAD( VcListenThread )
{
#ifdef _DEBUG_
	cout << "PCS: This is VcListenThread" << endl;
#endif

	PCS::getInstance()->loopVcListener();

    return 0;
};

ASAAC_THREAD( RateLimiterThread )
{
#ifdef _DEBUG_
	cout << "PCS: This is RateLimiterThread" << endl;
#endif

	PCS::getInstance()->loopRateLimiter();
	
	return 0;
}


void initializePcs()
{
	PCS::getInstance()->initialize();
}


ASAAC_ReturnStatus handleConfigureInterface(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleConfigureInterface" << endl;
#endif

    //do something with the paramter
     
    return ASAAC_ERROR;
}

ASAAC_ReturnStatus handleCreateTransferConnection(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleCreateTransferConnection" << endl;
#endif
	
	Parameter._u.reply_create_connection.result = PCS::getInstance()->createTransferConnection(Parameter._u.request_create_connection.tc_description );
     
    return ASAAC_SUCCESS;
 
}

ASAAC_ReturnStatus handleGetTransferConnectionDescription(ASAAC_PCS_MessageParameter &Parameter)
{
 #ifdef _DEBUG_
	cout << "PCS: handleGetTransferConnectionDescription" << endl;
#endif
    
    ASAAC_TcDescription tc_description;
      
    Parameter._u.reply_tc_description.result = PCS::getInstance()->getTransferConnectionDescription(Parameter._u.request_tc_description.tc_id,tc_description);
      
    Parameter._u.reply_tc_description.tc_description = tc_description;
     
     return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleDestroyTransferConnection(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleDestroyTransferConnection" << endl;
#endif
	
	Parameter._u.reply_destroy_connection.result = PCS::getInstance()->destroyTransferConnection(Parameter._u.request_destroy_connection.tc_id, Parameter._u.request_destroy_connection.network_descriptor );	
	
	 return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleGetNetworkPortStatus(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleGetNetworkPortStatus" << endl;
#endif
      //do something with the paramter
    return ASAAC_ERROR;
}

ASAAC_ReturnStatus handleAttachTransferConnectionToVirtualChannel(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleAttachTransferConnectionToVirtualChannel" << endl;
#endif
	
	Parameter._u.reply_attach_channel.result = PCS::getInstance()->attachTransferConnectionToVirtualChannel(Parameter._u.request_attach_channel.vc_description, Parameter._u.request_attach_channel.tc_id, Parameter._u.request_attach_channel.is_data_representation );
	
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleDetachTransferConnectionFromVirtualChannel(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleDetachTransferConnectionFromVirtualChannel" << endl;
#endif

	Parameter._u.reply_detach_channel.result = PCS::getInstance()->detachTransferConnectionFromVirtualChannel(Parameter._u.request_detach_channel.vc_id, Parameter._u.request_detach_channel.tc_id);
    
    return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleGetPMData(ASAAC_PCS_MessageParameter &Parameter)
{		
		ASAAC_PublicId vc_id;
		
		Parameter._u.reply_pm_data.result = PCS::getInstance()->getPMData(Parameter._u.request_pm_data.max_msg_length, Parameter._u.request_pm_data.timeout, Parameter._u.request_pm_data.sm_send_vc_id, vc_id);
		
		Parameter._u.reply_pm_data.vc_id = vc_id;
		
		return ASAAC_SUCCESS;
};


ASAAC_ReturnStatus handleReturnPMData(ASAAC_PCS_MessageParameter &Parameter)
{
		Parameter._u.reply_return_pm_data.result = PCS::getInstance()->returnPMData(Parameter._u.request_return_pm_data.vc_id,Parameter._u.request_return_pm_data.sm_receive_vc_id, Parameter._u.request_return_pm_data.sm_return_status);
		
		return ASAAC_SUCCESS;
};


void initializePcsServer()
{
   	pcsServer.setRequestVc( OS_PCS_SERVER_VC_REQUEST );
 	pcsServer.setReplyVc( OS_PCS_SERVER_VC_REPLY );
  	pcsServer.setTimeOut( TimeIntervalInfinity );
 
  	pcsServer.registerHandler( ASAAC_PCS_ConfigureInterface, handleConfigureInterface);
  	pcsServer.registerHandler( ASAAC_PCS_GetTransferConnectionDescription, handleGetTransferConnectionDescription);
   	pcsServer.registerHandler( ASAAC_PCS_GetNetworkPortStatus, handleGetNetworkPortStatus);
  	
  	pcsServer.registerHandler( ASAAC_PCS_CreateTransferConnection, handleCreateTransferConnection);
  	pcsServer.registerHandler( ASAAC_PCS_DestroyTransferConnection, handleDestroyTransferConnection);
   	pcsServer.registerHandler( ASAAC_PCS_AttachTransferConnectionToVirtualChannel, handleAttachTransferConnectionToVirtualChannel);
  	pcsServer.registerHandler( ASAAC_PCS_DetachTransferConnectionFromVirtualChannel, handleDetachTransferConnectionFromVirtualChannel);
	
  	pcsServer.registerHandler( ASAAC_PCS_GetPMData, handleGetPMData);
  	pcsServer.registerHandler( ASAAC_PCS_ReturnPMData, handleReturnPMData);
}

void startPcsThreads()
{
	ASAAC_APOS_startThread(2);
	ASAAC_APOS_startThread(3);
}


void stopPcsThreads()
{
	ASAAC_APOS_stopThread(2);
	ASAAC_APOS_stopThread(3);
}


using namespace std;

ASAAC_THREAD( MainThread )
{
#ifdef _DEBUG_
	cout << "PCS: Enter MainThread" << endl;
#endif
	
	try	
	{	
#ifdef _DEBUG_       
		cout << "PCS: initializePcsServer()" << endl;
#endif

		initializePcs();
		
#ifdef _DEBUG_       
		cout << "PCS: initializePcsServer()" << endl;
#endif

		initializePcsServer();
		
 		
#ifdef _DEBUG_       
		cout << "PCS: startPcsThreads()" << endl;
#endif

		startPcsThreads();
     	
#ifdef _DEBUG_       
		cout << "PCS: Enter Main Loop " << endl;
#endif
		
		for(;;)
		{
			pcsServer.handleOneRequest();
		}
	}
	catch ( PcsException &e )
	{
		cerr << e.getFullMessage() << endl;
	}
	
#ifdef _DEBUG_       
	cout << "PCS: Exit" << endl;
#endif
	
	return NULL;
}


