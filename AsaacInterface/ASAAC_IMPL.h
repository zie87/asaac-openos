#ifndef _ASAAC_IMPL_H_
#define _ASAAC_IMPL_H_
#include <time.h>

#define _ASAAC_Bool_defined
typedef enum {
    ASAAC_BOOL_FALSE,
    ASAAC_BOOL_TRUE
} ASAAC_Bool;

//This type has been redifined here to avoid 
//implicit conversion from TimeInterval to Time or vice versa.
#define _ASAAC_Time_defined
typedef struct {
	long sec;
	long nsec;
} ASAAC_Time;

#define _ASAAC_TimeInterval_defined
typedef struct {
	long sec;
	long nsec;
} ASAAC_TimeInterval;

#define _ASAAC_Address_defined
typedef void* ASAAC_Address ;

#define _ASAAC_CharAddress_defined
typedef char* ASAAC_CharAddress ;

#define _ASAAC_GsmConfigData_defined
typedef struct {
	unsigned long  filler ;
} ASAAC_GsmConfigData;

#define _ASAAC_DataRepresentation_defined
typedef char* ASAAC_DataRepresentation;

#define _ASAAC_InterfaceType_defined
typedef enum {
	ASAAC_IT_IPEC,
	ASAAC_IT_IMC,
	ASAAC_IT_BMC
} ASAAC_InterfaceType;

#define _ASAAC_LoadInstructions_defined
typedef struct {
	unsigned long  filler ;
} ASAAC_LoadInstructions;

#define _ASAAC_SchedulingInfo_defined
typedef struct {
	long				priority ;
	ASAAC_Bool 			is_vc_attached ;
	unsigned long		MAX_NR_OF_TRIGGER_VC ;
	unsigned long		act_nr_of_trigger_vc ;
	ASAAC_Bool 			is_periodic ;
	ASAAC_Time  		start_time ;
	ASAAC_TimeInterval  period ;
} ASAAC_SchedulingInfo;


typedef enum {
    NETWORK_TYPE_TCP ,
    NETWORK_TYPE_UDP
} NetworkType;

typedef struct {
    long* buffer_length;
    void* stream_buffer;
} TcpIpConfiguration;


typedef struct { 
} UdpConfiguration;


typedef union {
	TcpIpConfiguration  tcpip ;
	UdpConfiguration  udp ;
} TcNetworkSpecific ;


// local vc for GSM instance specifies connection to Network Agent
#define _ASAAC_TC_ConfigurationData_defined
typedef struct  {
	unsigned long     local_vc_id ;  
	NetworkType       network_type ;
	TcNetworkSpecific conf_data ;
} ASAAC_TC_ConfigurationData;


#define ASAAC_OS_MAX_STRING_SIZE 2048U

// Character Sequence type copied from ASAAC.h
// it is required for the ASAAC_DataRepresenation
#if !defined(_ASAAC_CharacterSequence_defined)
#define _ASAAC_CharacterSequence_defined 1
typedef struct ASAAC_CharacterSequence_type ASAAC_CharacterSequence;
struct ASAAC_CharacterSequence_type {
unsigned long size;
char data[ASAAC_OS_MAX_STRING_SIZE];
};
#endif


//SMS.31.10.06> Sequence is the inproper type: char* defined instead
//#define _ASAAC_DataRepresentation_defined
//typedef ASAAC_CharacterSequence ASAAC_DataRepresentation;


#endif //_ASAAC_IMPL_H_
