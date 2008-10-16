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
	
	char buf_out[MSG_SIZE] = "hello world!";

	cout << "SendHelloWorldApplication sends message... " 
		<< ((ASAAC_APOS_sendMessage(1, &t, buf_out, 13) == ASAAC_TM_SUCCESS)?"SUCCESS":"ERROR") << endl;

	return 0;
}
