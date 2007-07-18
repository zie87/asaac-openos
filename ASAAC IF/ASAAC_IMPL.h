#ifndef _ASAAC_IMPL_H_
#define _ASAAC_IMPL_H_
#include <time.h>

#define _ASAAC_Bool_defined
typedef enum {
    ASAAC_BOOL_FALSE,
    ASAAC_BOOL_TRUE
} ASAAC_Bool;

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
typedef struct {
	unsigned long  filler ;
} ASAAC_InterfaceType;

#define _ASAAC_LoadInstructions_defined
typedef struct {
	unsigned long  filler ;
} ASAAC_LoadInstructions;

#define _ASAAC_SchedulingInfo_defined
typedef struct {
	int   priority ;
	ASAAC_Bool is_vc_attached ;
	int   MAX_NR_OF_TRIGGER_VC ;
	int   act_nr_of_trigger_vc ;
	ASAAC_Bool is_periodic ;
	struct timespec  start_time ;
	struct timespec  period ;
} ASAAC_SchedulingInfo;


typedef enum {
    NETWORK_TYPE_TCP ,
    NETWORK_TYPE_UDP
} NetworkType;

typedef struct {
    long* buffer_length;
    void* stream_buffer;
} TcpIpConfiguration;


typedef struct { ;
} UdpConfiguration;


typedef union {
	TcpIpConfiguration  tcpip ;
	UdpConfiguration  udp ;
} TcNetworkSpecific ;


#define _ASAAC_TC_ConfigurationData_defined
typedef struct  {
	unsigned long     local_vc_id ;  // local vc for GSM instance specifies connection to Network Agent
	NetworkType       network_type ;
	TcNetworkSpecific conf_data ;
} ASAAC_TC_ConfigurationData;


#define ASAAC_OS_MAX_STRING_SIZE 1200U

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


//This type has been redifined here to avoid 
//implicit conversion from TimeInterval to Time or vice versa.
#define _ASAAC_TimeInterval_defined
typedef struct {
	long sec;
	long nsec;
} ASAAC_TimeInterval;

//SMS.31.10.06> Sequence is the inproper type: char* defined instead
//#define _ASAAC_DataRepresentation_defined
//typedef ASAAC_CharacterSequence ASAAC_DataRepresentation;

typedef enum {
  ASAAC_SCHEDULING_DISCIPLINE_FIFO,
  ASAAC_SCHEDULING_DISCIPLINE_RR
} ASAAC_SchedulingDiscipline;

#define _ASAAC_SchedulingInfo_defined 1
typedef struct
{
	unsigned long 		 	   priority;
	ASAAC_SchedulingDiscipline discipline;
} ASAAC_SchedulingInfo;


#endif //_ASAAC_IMPL_H_
