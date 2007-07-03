#include "PcsClass.hh"

#include "Common/Aid.h"

#include "OpenOS.hh"
#include "OpenOSObject.hh"

#include "LogicalInterfaces/asaac_pcs.hh"

ASAAC_APPLICATION

#include <iostream>
using namespace std;

#include <assert.h>

using namespace ASAAC::PCS;

PCS 				pcs;
PCSConfiguration    pcsConfig;
ASAAC::PCS::Server 	pcsServer;
 


ASAAC_THREAD( TcListenThread )
{
#ifdef _DEBUG_
	cout << "PCS: This is TcListenThread" << endl;
#endif

	pcs.tcListener();
    
    return 0;    
};

ASAAC_THREAD( VcListenThread )
{
#ifdef _DEBUG_
	cout << "PCS: This is VcListenThread" << endl;
#endif

	pcs.vcListener();

    return 0;
};

ASAAC_THREAD( RateLimiterThread )
{
#ifdef _DEBUG_
	cout << " PCS: This is RateLimiterThread" << endl;
#endif

	pcs.rateLimiter();
	
	return 0;
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
	
	Parameter._u.reply_create_connection.result = pcs.createTransferConnection(Parameter._u.request_create_connection.tc_description );
     
    return ASAAC_SUCCESS;
 
}

ASAAC_ReturnStatus handleGetTransferConnectionDescription(ASAAC_PCS_MessageParameter &Parameter)
{
 #ifdef _DEBUG_
	cout << "PCS: handleGetTransferConnectionDescription" << endl;
#endif
    
    ASAAC_TcDescription tc_description;
      
    Parameter._u.reply_tc_description.result = pcs.getTransferConnectionDescription(Parameter._u.request_tc_description.tc_id,tc_description);
      
    Parameter._u.reply_tc_description.tc_description = tc_description;
     
     return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleDestroyTransferConnection(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleDestroyTransferConnection" << endl;
#endif
	
	Parameter._u.reply_destroy_connection.result = pcs.destroyTransferConnection(Parameter._u.request_destroy_connection.tc_id, Parameter._u.request_destroy_connection.network_descriptor );	
	
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
	
	Parameter._u.reply_attach_channel.result = pcs.attachTransferConnectionToVirtualChannel(Parameter._u.request_attach_channel.vc_description, Parameter._u.request_attach_channel.tc_id, Parameter._u.request_attach_channel.is_data_representation );
	
	return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleDetachTransferConnectionFromVirtualChannel(ASAAC_PCS_MessageParameter &Parameter)
{
#ifdef _DEBUG_
	cout << "PCS: handleDetachTransferConnectionFromVirtualChannel" << endl;
#endif

	Parameter._u.reply_detach_channel.result = pcs.detachTransferConnectionFromVirtualChannel(Parameter._u.request_detach_channel.vc_id, Parameter._u.request_detach_channel.tc_id);
    
    return ASAAC_SUCCESS;
}

ASAAC_ReturnStatus handleGetPMData(ASAAC_PCS_MessageParameter &Parameter)
{		
		ASAAC_PublicId vc_id;
		
		Parameter._u.reply_pm_data.result = pcs.getPMData(Parameter._u.request_pm_data.max_msg_length, Parameter._u.request_pm_data.timeout, Parameter._u.request_pm_data.sm_send_vc_id, vc_id);
		
		Parameter._u.reply_pm_data.vc_id = vc_id;
		
		return ASAAC_SUCCESS;
};


ASAAC_ReturnStatus handleReturnPMData(ASAAC_PCS_MessageParameter &Parameter)
{
		Parameter._u.reply_return_pm_data.result = pcs.returnPMData(Parameter._u.request_return_pm_data.vc_id,Parameter._u.request_return_pm_data.sm_receive_vc_id, Parameter._u.request_return_pm_data.sm_return_status);
		
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
	ASAAC_APOS_startThread(4);
}


void stopPcsThreads()
{
	ASAAC_APOS_stopThread(2);
	ASAAC_APOS_stopThread(3);
	ASAAC_APOS_stopThread(4);
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
	catch ( PCSException &E )
	{
		cout << E.getFullMessage() << endl;
	}
	
#ifdef _DEBUG_       
	cout << "PCS: Exit" << endl;
#endif
	return 0;
}








//JUNK YARD






		/*
		char Buffer[1024];

		const string VcDescriptionFormat = "5l S(2e) 2e l o 64o";
		
		MarshallingProcessor Processor;

		unsigned long ActualSize;

		cout << "__________ Translating Description to CDR ___________" << endl;

		Processor.writeToCDR( &Description, sizeof( Description ), Buffer, 1024, VcDescriptionFormat, ActualSize );

		cout << endl << endl << "__________" << endl;

		return 0;

		cout << "sizeof U(1:l | 2:S(o s s)) = " << Processor.getSize("U(1:l | 2:S(o s s))") << endl;

		
		cout << "Size of VcDescription: " << Processor.getSize( "S(5l S(2e) 2e l o 64o)" ) << endl;
		
		VcDescription TestDesc;
		
		cout << "Size until target " << (unsigned long)&(TestDesc.cpu_id) - 
										(unsigned long)&(TestDesc) << endl;
		
		cout << "sizeof(SecurityRating) = " << sizeof( SecurityRating ) << endl;
		
		cout << "sizeof(VcDescription) = " << sizeof(VcDescription) << endl;
		
		cout << "alignment of data rep = " << __alignof__( TestDesc.data_representation_format ) << endl;
		cout << "sizeof of data rep = " << sizeof( TestDesc.data_representation_format ) << endl;

		
		
		return 0;
		*/
		
		
		
		
		
		
		
		
/*  
	const unsigned long MSG_SIZE  = 200;
	
	ASAAC_PublicId REC_VC = 7;
	ASAAC_PublicId SND_VC = 8;
	
	ASAAC_PublicId A2B_GVC = 11;
	ASAAC_PublicId B2A_GVC = 22;
	
	ASAAC_PublicId TC_ONE = 111;
	ASAAC_PublicId TC_TWO = 222;
	
	#define PORT_ONE  6601
	#define PORT_TWO  6602
	
	#define HOST_A_IP "192.168.2.251"
	#define HOST_B_IP "192.168.2.250"
	#define LOCAL_NETWORK "127.0.0.1"
	
	const ASAAC_PublicId HOST_A_PID = 5;
	const ASAAC_PublicId HOST_B_PID = 6;

 	
 	ASAAC_VcToTcMappingDescription TcMapping; //depends on host configuration
	TcMapping.is_data_representation = ASAAC_BOOL_FALSE;


    ASAAC_PublicId snd_port = 0;
    ASAAC_PublicId snd_network= 0;
    ASAAC_PublicId rec_port = 0;
    ASAAC_PublicId rec_network = IpAddressToNetworkId(LOCAL_NETWORK);
   
    ASAAC_PublicId snd_tc = 0;
    ASAAC_PublicId rec_tc = 0;
	
	
    if(pid == HOST_A_PID)
    {
    	snd_network = IpAddressToNetworkId(HOST_B_IP);
    	snd_port = PORT_ONE;
    	rec_port = PORT_TWO;
    	snd_tc = TC_ONE;
    	rec_tc = TC_TWO;
    	TcMapping.global_vc_id = A2B_GVC;
		TcMapping.tc_id        = snd_tc;
		
		pcsConfig.addLocalVc( REC_VC, A2B_GVC );
	

 #ifdef _DEBUG_   
    	cout << "PCS (" << HOST_A_PID << ") IS RUNNING ON " << HOST_A_IP << endl;
    	cout << "SEND PORT " << snd_port << " RECEIVE PORT " << rec_port << endl;
    	cout << "SEND TC " << snd_tc << " RECEIVE TC " << rec_tc << endl;
 #endif	
    }
    else if(pid == HOST_B_PID)
    {
    	snd_network = IpAddressToNetworkId(HOST_A_IP);
    	snd_port = PORT_TWO;
    	rec_port = PORT_ONE;
    	snd_tc = TC_TWO;
    	rec_tc = TC_ONE;
    	TcMapping.global_vc_id = A2B_GVC;
		TcMapping.tc_id        = rec_tc;
		
		pcsConfig.addLocalVc( SND_VC, A2B_GVC );
		
 #ifdef _DEBUG_   
    	cout << "PCS (" << HOST_B_PID << ") IS RUNNING ON " << HOST_B_IP << endl;
    	cout << "SEND PORT " << snd_port << " RECEIVE PORT " << rec_port << endl;
    	cout << "SEND TC " << snd_tc << " RECEIVE TC " << rec_tc << endl;
 #endif	
    }
    else
    {
    	cerr << "PCS IS NOT RUNNING ON A VALID HOST COMPUTER" << endl;
    	return 0;	
    }


	ASAAC_VcDescription RecVcDesc;
	RecVcDesc.vc_type      = ASAAC_Application_Raw_VC;
	RecVcDesc.security_info = ASAAC_Unmarked;
	RecVcDesc.global_vc_id = A2B_GVC;
		
	pcsConfig.addVcDescription( RecVcDesc );
	
	ASAAC_VcDescription SndVcDesc;
	SndVcDesc.vc_type      = ASAAC_Application_Raw_VC;
	SndVcDesc.security_info = ASAAC_Unmarked;
	SndVcDesc.global_vc_id = B2A_GVC;
		
	pcsConfig.addVcDescription( SndVcDesc );

	pcsConfig.addTcMapping( TcMapping );
	

	
	ASAAC_NetworkDescriptor* snd_nw = 0;
	ASAAC_NetworkDescriptor* rec_nw = 0;
	
	pcsConfig.addNetwork(snd_network,snd_port, snd_nw);
	
	pcsConfig.addNetwork(rec_network,rec_port, rec_nw);
	
	ASAAC_TcDescription RecTcDesc;
	
	RecTcDesc.tc_id = rec_tc;
	RecTcDesc.network_descr = *rec_nw;
	RecTcDesc.is_receiver = ASAAC_BOOL_TRUE;
	RecTcDesc.is_msg_transfer = ASAAC_BOOL_TRUE;

	pcsConfig.addTcDescription( RecTcDesc );

	ASAAC_TcDescription SndTcDesc;
	
	SndTcDesc.tc_id = snd_tc;
	SndTcDesc.network_descr = *snd_nw;
	SndTcDesc.is_receiver = ASAAC_BOOL_FALSE;
	SndTcDesc.is_msg_transfer = ASAAC_BOOL_TRUE;

	pcsConfig.addTcDescription( SndTcDesc );
	
	ASAAC_TimeInterval sndTcRate = {0,500000000}; //one message every 500 millisec maximum is nominal
	
	pcsConfig.addTcRateLimit(snd_tc, sndTcRate);*/
		
