#include <iostream>

#include "OpenOS.hh"
#include "ASAAC.h"

using namespace std;

const unsigned int MSG_SIZE  = 13;

ASAAC_APPLICATION

ASAAC_THREAD(MainThread)
{
	ASAAC_TimeInterval t;
	t.sec = 5;
	t.nsec = 0;
	
	char buf_in[MSG_SIZE] = "xxxxxxxxxxxx";
	unsigned long size;

	cout << "ReceiveHelloWorldApplication receives message... " 
		<< ((ASAAC_APOS_receiveMessage(1, &t, MSG_SIZE, buf_in, &size)==ASAAC_TM_SUCCESS)?"SUCCESS":"ERROR") << endl;
	cout << "  size    : " << size << endl;
	cout << "  message : " << buf_in << endl;

	return 0;
}
