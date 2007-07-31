#include "Aid.h"

#include "OpenOSObject.hh"

// Returns an ASAAC network id based on an IP4 address format X.X.X.X
ASAAC_PublicId IpAddressToNetworkId(const char* ip_addr)
{
    return oal_inet_addr(ip_addr);
}


ASAAC_Bool BoolNot(ASAAC_Bool b)
{
	if (b == ASAAC_BOOL_TRUE)
		return ASAAC_BOOL_FALSE;
	else return ASAAC_BOOL_TRUE;
}
