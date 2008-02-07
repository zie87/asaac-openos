#include "ASAAC_IMPL.h"
/*
 */

#ifndef ASAAC_H
#define ASAAC_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** typedefs **/
#if !defined(_ASAAC_Address_defined)
#define _ASAAC_Address_defined 1
typedef long ASAAC_Address;
#endif
#if !defined(_ASAAC_Bool_defined)
#define _ASAAC_Bool_defined 1
typedef enum {
  ASAAC_BOOL_FALSE,
  ASAAC_BOOL_TRUE
} ASAAC_Bool;
#endif
#ifndef ASAAC_OS_MAX_STRING_SIZE
#define ASAAC_OS_MAX_STRING_SIZE 256U
#endif /* !ASAAC_OS_MAX_STRING_SIZE */

#if !defined(_ASAAC_CharacterSequence_defined)
#define _ASAAC_CharacterSequence_defined 1
typedef struct ASAAC_CharacterSequence_type ASAAC_CharacterSequence;
struct ASAAC_CharacterSequence_type {
unsigned long size;
char data[256];
};

#endif
#if !defined(_ASAAC_Length_defined)
#define _ASAAC_Length_defined 1
typedef unsigned long ASAAC_Length;
#endif
#if !defined(_ASAAC_PrivateId_defined)
#define _ASAAC_PrivateId_defined 1
typedef unsigned long ASAAC_PrivateId;
#endif
#if !defined(_ASAAC_PublicId_defined)
#define _ASAAC_PublicId_defined 1
typedef unsigned long ASAAC_PublicId;
#endif
#ifndef ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE
#define ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE 256U
#endif /* !ASAAC_OS_MAX_PUBLIC_ID_SET_SIZE */

#if !defined(_ASAAC_PublicIdSet_defined)
#define _ASAAC_PublicIdSet_defined 1
typedef struct ASAAC_PublicIdSet_type ASAAC_PublicIdSet;
struct ASAAC_PublicIdSet_type {
ASAAC_PublicId vc_id[256];
};

#endif
#if !defined(_ASAAC_Time_defined)
#define _ASAAC_Time_defined 1
typedef struct ASAAC_Time_type ASAAC_Time;
struct ASAAC_Time_type {
long sec;
long nsec;
};

#endif
#if !defined(_ASAAC_TimeInterval_defined)
#define _ASAAC_TimeInterval_defined 1
typedef ASAAC_Time ASAAC_TimeInterval;
#endif
#if !defined(_ASAAC_NetworkDescriptor_defined)
#define _ASAAC_NetworkDescriptor_defined 1
typedef struct ASAAC_NetworkDescriptor_type ASAAC_NetworkDescriptor;
struct ASAAC_NetworkDescriptor_type {
ASAAC_PublicId network;
ASAAC_PublicId port;
};

#endif
#if !defined(_ASAAC_AccessType_defined)
#define _ASAAC_AccessType_defined 1
typedef enum {
  ASAAC_OLI_ACCESS,
  ASAAC_LOCAL_ACCESS
} ASAAC_AccessType;
#endif
#if !defined(_ASAAC_OliChannel_defined)
#define _ASAAC_OliChannel_defined 1
typedef struct ASAAC_OliChannel_type ASAAC_OliChannel;
struct ASAAC_OliChannel_type {
ASAAC_PublicId vc_sending;
ASAAC_PublicId vc_receiving;
unsigned long fragment_size;
};

#endif
#if !defined(_ASAAC_AccessInfo_defined)
#define _ASAAC_AccessInfo_defined 1
typedef struct ASAAC_AccessInfo_type ASAAC_AccessInfo;
struct ASAAC_AccessInfo_type {
ASAAC_AccessType _d;
union {
ASAAC_OliChannel oli_channel;
unsigned long no_parameter;
} _u;
};
#endif
#if !defined(_ASAAC_AccessRights_defined)
#define _ASAAC_AccessRights_defined 1
typedef enum {
  ASAAC_R,
  ASAAC_W,
  ASAAC_D,
  ASAAC_RW,
  ASAAC_WD,
  ASAAC_RWD,
  ASAAC_F
} ASAAC_AccessRights;
#endif
#ifndef ASAAC_MAX_NUMBER_OF_ACTION_PARAM
#define ASAAC_MAX_NUMBER_OF_ACTION_PARAM 256U
#endif /* !ASAAC_MAX_NUMBER_OF_ACTION_PARAM */

#if !defined(_ASAAC_Action_defined)
#define _ASAAC_Action_defined 1
typedef struct ASAAC_Action_type ASAAC_Action;
struct ASAAC_Action_type {
unsigned long action_number;
long parameters[256];
};

#endif
#if !defined(_ASAAC_AlarmType_defined)
#define _ASAAC_AlarmType_defined 1
typedef enum {
  ASAAC_NO_ALARM,
  ASAAC_CYCLIC_ALARM,
  ASAAC_ONLY_ONCE_ALARM
} ASAAC_AlarmType;
#endif
#if !defined(_ASAAC_BitFinalResult_defined)
#define _ASAAC_BitFinalResult_defined 1
typedef enum {
  ASAAC_BIT_FINAL_RESULT_OK,
  ASAAC_BIT_FINAL_RESULT_FAIL
} ASAAC_BitFinalResult;
#endif
#if !defined(_ASAAC_BitType_defined)
#define _ASAAC_BitType_defined 1
typedef enum {
  ASAAC_IBIT,
  ASAAC_CBIT,
  ASAAC_PBIT
} ASAAC_BitType;
#endif
#ifndef ASAAC_MAX_CHAR_IN_CBIT_DETAILED_RESULT
#define ASAAC_MAX_CHAR_IN_CBIT_DETAILED_RESULT 256U
#endif /* !ASAAC_MAX_CHAR_IN_CBIT_DETAILED_RESULT */

#if !defined(_ASAAC_CbitDetailedResult_defined)
#define _ASAAC_CbitDetailedResult_defined 1
typedef struct ASAAC_CbitDetailedResult_type ASAAC_CbitDetailedResult;
struct ASAAC_CbitDetailedResult_type {
unsigned long no_bytes;
char component_bit_result[256];
};

#endif
#if !defined(_ASAAC_CbitResult_defined)
#define _ASAAC_CbitResult_defined 1
typedef struct ASAAC_CbitResult_type ASAAC_CbitResult;
struct ASAAC_CbitResult_type {
ASAAC_BitFinalResult cbit_final_result;
ASAAC_CbitDetailedResult cbit_detailed_result;
};

#endif
#ifndef ASAAC_MAX_CHAR_IN_IBIT_DETAILED_RESULT
#define ASAAC_MAX_CHAR_IN_IBIT_DETAILED_RESULT 256U
#endif /* !ASAAC_MAX_CHAR_IN_IBIT_DETAILED_RESULT */

#if !defined(_ASAAC_IbitDetailedResult_defined)
#define _ASAAC_IbitDetailedResult_defined 1
typedef struct ASAAC_IbitDetailedResult_type ASAAC_IbitDetailedResult;
struct ASAAC_IbitDetailedResult_type {
unsigned long no_bytes;
char component_bit_result[256];
};

#endif
#if !defined(_ASAAC_IbitResult_defined)
#define _ASAAC_IbitResult_defined 1
typedef struct ASAAC_IbitResult_type ASAAC_IbitResult;
struct ASAAC_IbitResult_type {
ASAAC_BitFinalResult ibit_final_result;
ASAAC_IbitDetailedResult ibit_detailed_result;
};

#endif
#ifndef ASAAC_MAX_CHAR_IN_PBIT_DETAILED_RESULT
#define ASAAC_MAX_CHAR_IN_PBIT_DETAILED_RESULT 256U
#endif /* !ASAAC_MAX_CHAR_IN_PBIT_DETAILED_RESULT */

#if !defined(_ASAAC_PbitDetailedResult_defined)
#define _ASAAC_PbitDetailedResult_defined 1
typedef struct ASAAC_PbitDetailedResult_type ASAAC_PbitDetailedResult;
struct ASAAC_PbitDetailedResult_type {
unsigned long no_bytes;
char component_bit_result[256];
};

#endif
#if !defined(_ASAAC_PbitResult_defined)
#define _ASAAC_PbitResult_defined 1
typedef struct ASAAC_PbitResult_type ASAAC_PbitResult;
struct ASAAC_PbitResult_type {
ASAAC_BitFinalResult pbit_final_result;
ASAAC_PbitDetailedResult pbit_detailed_result;
};

#endif
#if !defined(_ASAAC_BitResultAll_defined)
#define _ASAAC_BitResultAll_defined 1
typedef struct ASAAC_BitResultAll_type ASAAC_BitResultAll;
struct ASAAC_BitResultAll_type {
ASAAC_BitType _d;
union {
ASAAC_IbitResult ibit_result;
ASAAC_CbitResult cbit_result;
ASAAC_PbitResult pbit_result;
} _u;
};
#endif
#if !defined(_ASAAC_BitReturnStatus_defined)
#define _ASAAC_BitReturnStatus_defined 1
typedef enum {
  ASAAC_BIT_CALL_OK,
  ASAAC_BIT_CALL_FAILED
} ASAAC_BitReturnStatus;
#endif
#if !defined(_ASAAC_BitResult_defined)
#define _ASAAC_BitResult_defined 1
typedef struct ASAAC_BitResult_type ASAAC_BitResult;
struct ASAAC_BitResult_type {
ASAAC_BitType bit_type;
ASAAC_BitResultAll bit_result;
};

#endif
#if !defined(_ASAAC_BitTestStatus_defined)
#define _ASAAC_BitTestStatus_defined 1
typedef enum {
  ASAAC_BIT_PASSED,
  ASAAC_BIT_ONGOING,
  ASAAC_BIT_FAILED
} ASAAC_BitTestStatus;
#endif
#if !defined(_ASAAC_BreachType_defined)
#define _ASAAC_BreachType_defined 1
typedef enum {
  ASAAC_NON_AUTHORIZED_SERVICE,
  ASAAC_MULTI_LEVEL_SECURITY_ERROR,
  ASAAC_UNAUTHORIZED_COMMS
} ASAAC_BreachType;
#endif
#if !defined(_ASAAC_Category_defined)
#define _ASAAC_Category_defined 1
typedef enum {
  ASAAC_LEVEL_1,
  ASAAC_LEVEL_2,
  ASAAC_LEVEL_3
} ASAAC_Category;
#endif
#if !defined(_ASAAC_CbitModeType_defined)
#define _ASAAC_CbitModeType_defined 1
typedef enum {
  ASAAC_PARTITIONED,
  ASAAC_COMPLETE
} ASAAC_CbitModeType;
#endif
#if !defined(_ASAAC_CfmType_defined)
#define _ASAAC_CfmType_defined 1
typedef enum {
  ASAAC_PCM,
  ASAAC_NSM,
  ASAAC_MMM,
  ASAAC_DPM,
  ASAAC_SPM,
  ASAAC_GPM
} ASAAC_CfmType;
#endif
#if !defined(_ASAAC_CharAddress_defined)
#define _ASAAC_CharAddress_defined 1
typedef ASAAC_Address ASAAC_CharAddress;
#endif
#if !defined(_ASAAC_DataRepresentation_defined)
#define _ASAAC_DataRepresentation_defined 1
typedef ASAAC_Address ASAAC_DataRepresentation;
#endif
#if !defined(_ASAAC_DownloadChannelType_defined)
#define _ASAAC_DownloadChannelType_defined 1
typedef enum {
  ASAAC_OLI_THEN_MLI,
  ASAAC_MLI_ONLY
} ASAAC_DownloadChannelType;
#endif
#if !defined(_ASAAC_OliMliChannel_defined)
#define _ASAAC_OliMliChannel_defined 1
typedef struct ASAAC_OliMliChannel_type ASAAC_OliMliChannel;
struct ASAAC_OliMliChannel_type {
ASAAC_PublicId vc_sending;
ASAAC_PublicId vc_receiving;
ASAAC_PublicId tc_sending;
ASAAC_PublicId tc_receiving;
};

#endif
#if !defined(_ASAAC_MliChannel_defined)
#define _ASAAC_MliChannel_defined 1
typedef struct ASAAC_MliChannel_type ASAAC_MliChannel;
struct ASAAC_MliChannel_type {
ASAAC_PublicId tc_sending;
ASAAC_PublicId tc_receiving;
};

#endif
#if !defined(_ASAAC_DownloadChannel_defined)
#define _ASAAC_DownloadChannel_defined 1
typedef struct ASAAC_DownloadChannel_type ASAAC_DownloadChannel;
struct ASAAC_DownloadChannel_type {
ASAAC_DownloadChannelType _d;
union {
ASAAC_OliMliChannel oli_mli_channel;
ASAAC_MliChannel mli_channel;
} _u;
};
#endif
#if !defined(_ASAAC_DownloadType_defined)
#define _ASAAC_DownloadType_defined 1
typedef enum {
  ASAAC_RTGTABLE_DOWNLOAD,
  ASAAC_NETWORK_DOWNLOAD,
  ASAAC_IMAGE_DOWNLOAD,
  ASAAC_POWER_DOWNLOAD,
  ASAAC_TIME_DOWNLOAD
} ASAAC_DownloadType;
#endif
#if !defined(_ASAAC_LoadInstructions_defined)
#define _ASAAC_LoadInstructions_defined 1
typedef CORBA_any ASAAC_LoadInstructions;
#endif
#if !defined(_ASAAC_ConfigTableDescription_defined)
#define _ASAAC_ConfigTableDescription_defined 1
typedef struct ASAAC_ConfigTableDescription_type ASAAC_ConfigTableDescription;
struct ASAAC_ConfigTableDescription_type {
unsigned long table_size;
ASAAC_CharacterSequence table_name;
unsigned long fragment_size;
unsigned long number_occurences;
};

#endif
#if !defined(_ASAAC_NetworkConfigDescription_defined)
#define _ASAAC_NetworkConfigDescription_defined 1
typedef struct ASAAC_NetworkConfigDescription_type ASAAC_NetworkConfigDescription;
struct ASAAC_NetworkConfigDescription_type {
ASAAC_PublicId network_id;
unsigned long table_size;
ASAAC_CharacterSequence table_name;
unsigned long fragment_size;
unsigned long number_occurences;
};

#endif
#if !defined(_ASAAC_ImageDescription_defined)
#define _ASAAC_ImageDescription_defined 1
typedef struct ASAAC_ImageDescription_type ASAAC_ImageDescription;
struct ASAAC_ImageDescription_type {
ASAAC_PublicId pe_id;
unsigned long image_size;
ASAAC_CharacterSequence image_name;
unsigned long image_content;
unsigned long fragment_size;
unsigned long number_occurrences;
unsigned long load_instruction_size;
ASAAC_LoadInstructions load_instructions;
};

#endif
#if !defined(_ASAAC_DownloadDescription_defined)
#define _ASAAC_DownloadDescription_defined 1
typedef struct ASAAC_DownloadDescription_type ASAAC_DownloadDescription;
struct ASAAC_DownloadDescription_type {
ASAAC_DownloadType _d;
union {
ASAAC_ConfigTableDescription config_table_description;
ASAAC_NetworkConfigDescription network_config_description;
ASAAC_ImageDescription image_description;
} _u;
};
#endif
#if !defined(_ASAAC_CfmDescription_defined)
#define _ASAAC_CfmDescription_defined 1
typedef struct ASAAC_CfmDescription_type ASAAC_CfmDescription;
struct ASAAC_CfmDescription_type {
ASAAC_PublicId cfm_id;
ASAAC_CfmType cfm_type;
ASAAC_TimeInterval time_out;
ASAAC_DownloadChannelType download_channel_type;
ASAAC_DownloadChannel download_channel;
ASAAC_DownloadType download_type;
ASAAC_DownloadDescription download_description;
};

#endif
#if !defined(_ASAAC_PeType_defined)
#define _ASAAC_PeType_defined 1
typedef unsigned char ASAAC_PeType[32];
typedef unsigned char ASAAC_PeType_slice;
#endif
#if !defined(_ASAAC_PeResources_defined)
#define _ASAAC_PeResources_defined 1
typedef struct ASAAC_PeResources_type ASAAC_PeResources;
struct ASAAC_PeResources_type {
ASAAC_PublicId pe_id;
ASAAC_PeType type;
unsigned long performance;
unsigned long memory;
};

#endif
#if !defined(_ASAAC_TimerResources_defined)
#define _ASAAC_TimerResources_defined 1
typedef struct ASAAC_TimerResources_type ASAAC_TimerResources;
struct ASAAC_TimerResources_type {
unsigned long id;
unsigned long resolution;
};

#endif
#ifndef ASAAC_NETWORK_MAX_NO
#define ASAAC_NETWORK_MAX_NO 8U
#endif /* !ASAAC_NETWORK_MAX_NO */

#if !defined(_ASAAC_CfmResources_defined)
#define _ASAAC_CfmResources_defined 1
typedef struct ASAAC_CfmResources_type ASAAC_CfmResources;
struct ASAAC_CfmResources_type {
ASAAC_PeResources pe[8];
unsigned long global_memory;
ASAAC_TimerResources timer[8];
unsigned char network_interfaces[8];
};

#endif
#if !defined(_ASAAC_CfmInfo_defined)
#define _ASAAC_CfmInfo_defined 1
typedef struct ASAAC_CfmInfo_type ASAAC_CfmInfo;
struct ASAAC_CfmInfo_type {
unsigned char id[4];
unsigned char manufacturer_id[32];
unsigned char part_no[256];
unsigned char hw_version[256];
unsigned char serial_no[256];
unsigned char production_batch_date[32];
unsigned char cfm_type[32];
unsigned char msl_version[32];
unsigned char standard_mpi_version_compliance[8];
unsigned char standard_mos_version_compliance[8];
unsigned char standard_mli_version_compliance[8];
unsigned long num_network;
unsigned long num_pe;
ASAAC_CfmResources cfm_resources;
};

#endif
#if !defined(_ASAAC_CfmParameterReturnStatus_defined)
#define _ASAAC_CfmParameterReturnStatus_defined 1
typedef enum {
  ASAAC_CFM_INFO_CALL_OK,
  ASAAC_CFM_INFO_CALL_FAILED
} ASAAC_CfmParameterReturnStatus;
#endif
#if !defined(_ASAAC_CfmMliChannel_defined)
#define _ASAAC_CfmMliChannel_defined 1
typedef struct ASAAC_CfmMliChannel_type ASAAC_CfmMliChannel;
struct ASAAC_CfmMliChannel_type {
ASAAC_PublicId cfm_id;
ASAAC_CfmType cfm_type;
ASAAC_PublicId tc_sending;
ASAAC_PublicId tc_receiving;
ASAAC_TimeInterval time_out;
};

#endif
#if !defined(_ASAAC_CfmPartNo_defined)
#define _ASAAC_CfmPartNo_defined 1
typedef char *  ASAAC_CfmPartNo;
#endif
#if !defined(_ASAAC_OpeStatus_defined)
#define _ASAAC_OpeStatus_defined 1
typedef enum {
  ASAAC_OK,
  ASAAC_FAILED,
  ASAAC_NOT_AVAILABLE,
  ASAAC_IN_PROGRESS
} ASAAC_OpeStatus;
#endif
#if !defined(_ASAAC_PeStatus_defined)
#define _ASAAC_PeStatus_defined 1
typedef struct ASAAC_PeStatus_type ASAAC_PeStatus;
struct ASAAC_PeStatus_type {
ASAAC_PublicId pe_id;
ASAAC_OpeStatus pbit_status;
ASAAC_OpeStatus cbit_status;
ASAAC_OpeStatus ibit_status;
ASAAC_OpeStatus rtg_download_status;
ASAAC_OpeStatus msl_download_status;
ASAAC_OpeStatus os_download_status;
ASAAC_OpeStatus gsm_download_status;
ASAAC_OpeStatus rtbp_download_status;
};

#endif
#ifndef ASAAC_MAX_NUMBER_OF_PE
#define ASAAC_MAX_NUMBER_OF_PE 8U
#endif /* !ASAAC_MAX_NUMBER_OF_PE */

#if !defined(_ASAAC_CfmStatusPeGeneric_defined)
#define _ASAAC_CfmStatusPeGeneric_defined 1
typedef struct ASAAC_CfmStatusPeGeneric_type ASAAC_CfmStatusPeGeneric;
struct ASAAC_CfmStatusPeGeneric_type {
unsigned long number_of_pe;
ASAAC_PeStatus pe_status[8];
};

#endif
#if !defined(_ASAAC_CfmStat_defined)
#define _ASAAC_CfmStat_defined 1
typedef struct ASAAC_CfmStat_type ASAAC_CfmStat;
struct ASAAC_CfmStat_type {
ASAAC_CfmType _d;
union {
ASAAC_CfmStatusPeGeneric pe_status;
unsigned long no_parameter;
} _u;
};
#endif
#if !defined(_ASAAC_CfmStatusGeneric_defined)
#define _ASAAC_CfmStatusGeneric_defined 1
typedef struct ASAAC_CfmStatusGeneric_type ASAAC_CfmStatusGeneric;
struct ASAAC_CfmStatusGeneric_type {
ASAAC_OpeStatus cfm_consolidated_status;
ASAAC_OpeStatus pbit_status;
ASAAC_OpeStatus cbit_status;
ASAAC_OpeStatus ibit_status;
ASAAC_OpeStatus rtg_download_status;
ASAAC_OpeStatus msl_download_status;
};

#endif
#if !defined(_ASAAC_CfmStatus_defined)
#define _ASAAC_CfmStatus_defined 1
typedef struct ASAAC_CfmStatus_type ASAAC_CfmStatus;
struct ASAAC_CfmStatus_type {
ASAAC_CfmStatusGeneric status_generic;
ASAAC_CfmType cfm_type;
ASAAC_CfmStat cfm_status;
};

#endif
#if !defined(_ASAAC_CfmStatusReturnStatus_defined)
#define _ASAAC_CfmStatusReturnStatus_defined 1
typedef enum {
  ASAAC_CFM_STATUS_CALL_OK,
  ASAAC_CFM_STATUS_CALL_FAILED
} ASAAC_CfmStatusReturnStatus;
#endif
#if !defined(_ASAAC_CfmVersion_defined)
#define _ASAAC_CfmVersion_defined 1
typedef char *  ASAAC_CfmVersion;
#endif
#if !defined(_ASAAC_CfmSerialNo_defined)
#define _ASAAC_CfmSerialNo_defined 1
typedef char *  ASAAC_CfmSerialNo;
#endif
#if !defined(_ASAAC_ClassificationLevel_defined)
#define _ASAAC_ClassificationLevel_defined 1
typedef enum {
  ASAAC_UNCLASSIFIED,
  ASAAC_CONFIDENTIAL,
  ASAAC_SECRET,
  ASAAC_TOP_SECRET
} ASAAC_ClassificationLevel;
#endif
#if !defined(_ASAAC_ClockMode_defined)
#define _ASAAC_ClockMode_defined 1
typedef enum {
  ASAAC_MASTER_REFERENCE_CLOCK,
  ASAAC_REFERENCE_CLOCK,
  ASAAC_MODULE_CLOCK
} ASAAC_ClockMode;
#endif
#if !defined(_ASAAC_ClockInfo_defined)
#define _ASAAC_ClockInfo_defined 1
typedef struct ASAAC_ClockInfo_type ASAAC_ClockInfo;
struct ASAAC_ClockInfo_type {
ASAAC_ClockMode clock_mode;
ASAAC_PublicId clock_id;
ASAAC_PublicId tc_id_from_parent;
ASAAC_PublicId tc_id_to_parent;
ASAAC_TimeInterval sync_wave_period;
unsigned long max_of_missed_alt;
ASAAC_TimeInterval range_for_alt;
ASAAC_TimeInterval alt_res_bound;
ASAAC_TimeInterval max_alt_diff;
ASAAC_TimeInterval timeout;
};

#endif
#if !defined(_ASAAC_DeleteOption_defined)
#define _ASAAC_DeleteOption_defined 1
typedef enum {
  ASAAC_NORMAL,
  ASAAC_IMMEDIATELY
} ASAAC_DeleteOption;
#endif
#if !defined(_ASAAC_ErrorCode_defined)
#define _ASAAC_ErrorCode_defined 1
typedef unsigned long ASAAC_ErrorCode;
#endif
#if !defined(_ASAAC_ErrorType_defined)
#define _ASAAC_ErrorType_defined 1
typedef enum {
  ASAAC_APPLICATION_ERROR,
  ASAAC_APOS_CLIENT_ERROR,
  ASAAC_RESOURCE_ERROR,
  ASAAC_OS_ERROR,
  ASAAC_SMOS_ERROR,
  ASAAC_SMBP_ERROR,
  ASAAC_PROCESSOR_ERROR,
  ASAAC_HW_RESOURCE_ERROR,
  ASAAC_HW_FAILURE,
  ASAAC_FATAL_ERROR
} ASAAC_ErrorType;
#endif
#if !defined(_ASAAC_ErrorInfo_defined)
#define _ASAAC_ErrorInfo_defined 1
typedef struct ASAAC_ErrorInfo_type ASAAC_ErrorInfo;
struct ASAAC_ErrorInfo_type {
ASAAC_ErrorCode error_code;
ASAAC_ErrorType error_type;
ASAAC_PublicId cfm_id;
ASAAC_PublicId pe_id;
ASAAC_PublicId process_id;
ASAAC_PublicId thread_id;
ASAAC_PublicId tc_id;
ASAAC_PublicId vc_id;
ASAAC_NetworkDescriptor network;
ASAAC_Address location;
ASAAC_Time absolute_global_time;
ASAAC_Time absolute_local_time;
ASAAC_Time relative_local_time;
ASAAC_CharacterSequence error_message;
};

#endif
#if !defined(_ASAAC_EventStatus_defined)
#define _ASAAC_EventStatus_defined 1
typedef enum {
  ASAAC_EVENT_STATUS_SET,
  ASAAC_EVENT_STATUS_RESET
} ASAAC_EventStatus;
#endif
#if !defined(_ASAAC_EventType_defined)
#define _ASAAC_EventType_defined 1
typedef enum {
  ASAAC_COMMS_EV_ERROR,
  ASAAC_COMMS_EV_INFO,
  ASAAC_COMMS_EV_CONFIGURED_OK,
  ASAAC_COMMS_EV_BUFFER_SEND,
  ASAAC_COMMS_EV_BUFFER_RECEIVED,
  ASAAC_COMMS_TEST_RETURN,
  ASAAC_COMMS_TEST_TIMEOUT,
  ASAAC_TIMER_ALARM,
  ASAAC_CBIT_ERROR_DETECT,
  ASAAC_MMM_SD_EVENT,
  ASAAC_DEV0_EVENT0,
  ASAAC_KBD_PRESS
} ASAAC_EventType;
#endif
#if !defined(_ASAAC_FaultReport_defined)
#define _ASAAC_FaultReport_defined 1
typedef struct ASAAC_FaultReport_type ASAAC_FaultReport;
struct ASAAC_FaultReport_type {
ASAAC_ErrorType error_type;
ASAAC_ErrorCode error_code;
ASAAC_PublicId system_id;
ASAAC_PublicId ia_id;
ASAAC_PublicId cfm_id;
ASAAC_PublicId re_id;
ASAAC_PublicId smm_id;
ASAAC_PublicId config_id;
ASAAC_PublicId pe_id;
ASAAC_PublicId proc_id;
ASAAC_PublicId thread_id;
ASAAC_PublicId tc_id;
ASAAC_PublicId vc_id;
ASAAC_NetworkDescriptor network;
unsigned long ia_error_count;
ASAAC_Address fault_address;
ASAAC_Time local_time;
ASAAC_Time global_time;
ASAAC_CharacterSequence error_message;
};

#endif
#if !defined(_ASAAC_FederatedClockInfo_defined)
#define _ASAAC_FederatedClockInfo_defined 1
typedef struct ASAAC_FederatedClockInfo_type ASAAC_FederatedClockInfo;
struct ASAAC_FederatedClockInfo_type {
ASAAC_ClockMode clock_mode;
ASAAC_PublicId clock_id;
ASAAC_PublicId Tc_Id_To_Federated;
ASAAC_PublicId Tc_Id_From_Federated;
};

#endif
#if !defined(_ASAAC_FunctionId_defined)
#define _ASAAC_FunctionId_defined 1
typedef ASAAC_PublicId ASAAC_FunctionId;
#endif
#if !defined(_ASAAC_GsmConfigData_defined)
#define _ASAAC_GsmConfigData_defined 1
typedef CORBA_any ASAAC_GsmConfigData;
#endif
#if !defined(_ASAAC_RemoteServiceId_defined)
#define _ASAAC_RemoteServiceId_defined 1
typedef enum {
  ASAAC_PBIT_RESULT,
  ASAAC_CFM_STATUS,
  ASAAC_CFM_INFO,
  ASAAC_NETWORK_STATUS,
  ASAAC_POWER_STATUS,
  ASAAC_TEST_MESSAGE,
  ASAAC_IBIT_START,
  ASAAC_IBIT_RESULT
} ASAAC_RemoteServiceId;
#endif
#if !defined(_ASAAC_InputLocalParameters_defined)
#define _ASAAC_InputLocalParameters_defined 1
typedef struct ASAAC_InputLocalParameters_type ASAAC_InputLocalParameters;
struct ASAAC_InputLocalParameters_type {
ASAAC_RemoteServiceId _d;
union {
ASAAC_PublicId network_id;
unsigned long no_parameter;
} _u;
};
#endif
#ifndef ASAAC_INTERFACE_CONFIG_MAX_LEN
#define ASAAC_INTERFACE_CONFIG_MAX_LEN 1024U
#endif /* !ASAAC_INTERFACE_CONFIG_MAX_LEN */

#if !defined(_ASAAC_InterfaceDescription_defined)
#define _ASAAC_InterfaceDescription_defined 1
typedef unsigned char ASAAC_InterfaceDescription[1024];
typedef unsigned char ASAAC_InterfaceDescription_slice;
#endif
#if !defined(_ASAAC_InterfaceConfigurationData_defined)
#define _ASAAC_InterfaceConfigurationData_defined 1
typedef struct ASAAC_InterfaceConfigurationData_type ASAAC_InterfaceConfigurationData;
struct ASAAC_InterfaceConfigurationData_type {
unsigned long configuration_data_length;
ASAAC_InterfaceDescription configuration_data;
};

#endif
#if !defined(_ASAAC_InterfaceType_defined)
#define _ASAAC_InterfaceType_defined 1
typedef CORBA_any ASAAC_InterfaceType;
#endif
#if !defined(_ASAAC_InterfaceData_defined)
#define _ASAAC_InterfaceData_defined 1
typedef struct ASAAC_InterfaceData_type ASAAC_InterfaceData;
struct ASAAC_InterfaceData_type {
ASAAC_PublicId if_id;
ASAAC_NetworkDescriptor nw_id;
ASAAC_PublicId cpu_id;
ASAAC_InterfaceType conf_data_type;
unsigned long conf_data_size;
ASAAC_InterfaceConfigurationData conf_data;
};

#endif
#if !defined(_ASAAC_IOoperation_defined)
#define _ASAAC_IOoperation_defined 1
typedef enum {
  ASAAC_IO_no_op,
  ASAAC_IO_read,
  ASAAC_IO_write,
  ASAAC_IO_seek,
  ASAAC_IO_test
} ASAAC_IOoperation;
#endif
#if !defined(_ASAAC_LoadFileResult_defined)
#define _ASAAC_LoadFileResult_defined 1
typedef enum {
  ASAAC_RET_LOAD_ACK_LOAD_OK,
  ASAAC_RET_LOAD_ACK_FAILURE_ALREADY_LOADED,
  ASAAC_RET_LOAD_ACK_FAILURE_UNKNOWN_FORMAT,
  ASAAC_RET_LOAD_ACK_FAILURE_CHECKSUM_ERROR,
  ASAAC_RET_LOAD_ACK_FAILURE_INSUFFICIENT_RESOURCES,
  ASAAC_RET_LOAD_ACK_UNKNOWN_ERROR,
  ASAAC_RET_LOAD_INVALID_TC,
  ASAAC_RET_LOAD_INVALID_SERVICE,
  ASAAC_RET_LOAD_TIME_OUT
} ASAAC_LoadFileResult;
#endif
#if !defined(_ASAAC_LockStatus_defined)
#define _ASAAC_LockStatus_defined 1
typedef enum {
  ASAAC_LOCKED,
  ASAAC_UNLOCKED
} ASAAC_LockStatus;
#endif
#if !defined(_ASAAC_Log_defined)
#define _ASAAC_Log_defined 1
typedef char *  ASAAC_Log;
#endif
#if !defined(_ASAAC_LogMessageType_defined)
#define _ASAAC_LogMessageType_defined 1
typedef enum {
  ASAAC_LOG_MESSAGE_TYPE_ERROR,
  ASAAC_LOG_MESSAGE_TYPE_APPLICATION,
  ASAAC_LOG_MESSAGE_TYPE_GSM,
  ASAAC_LOG_MESSAGE_TYPE_MAINTENANCE
} ASAAC_LogMessageType;
#endif
#if !defined(_ASAAC_LogReturnStatus_defined)
#define _ASAAC_LogReturnStatus_defined 1
typedef enum {
  ASAAC_MOS_LOG_CALL_OK,
  ASAAC_MOS_LOG_CALL_FAILED,
  ASAAC_MOS_LOG_READ_INDEX_OUT_OF_RANGE
} ASAAC_LogReturnStatus;
#endif
#if !defined(_ASAAC_MemoryUsage_defined)
#define _ASAAC_MemoryUsage_defined 1
typedef enum {
  ASAAC_READ_ONLY,
  ASAAC_READ_WRITE
} ASAAC_MemoryUsage;
#endif
#if !defined(_ASAAC_MSLStatus_defined)
#define _ASAAC_MSLStatus_defined 1
typedef enum {
  ASAAC_MSL_OK,
  ASAAC_MSL_FAILED,
  ASAAC_MSL_INVALID_PARAMETER,
  ASAAC_MSL_FAILED_TO_CREATE_REGION,
  ASAAC_MSL_FAILED_TO_DELETE_REGION,
  ASAAC_MSL_FAILED_TO_ATTACH_REGION,
  ASAAC_MSL_FAILED_TO_DETACH_REGION,
  ASAAC_MSL_FAILED_TO_CREATE_VM,
  ASAAC_MSL_FAILED_TO_DELETE_VM,
  ASAAC_MSL_INVALID_LINEAR_ADDRESS,
  ASAAC_MSL_INVALID_REGION_ID,
  ASAAC_MSL_INVALID_VM_ID,
  ASAAC_MSL_FAILED_TO_ADD_SEP,
  ASAAC_MSL_FAILED_TO_CREATE_CONTEXT,
  ASAAC_MSL_FAILED_TO_DELETE_CONTEXT,
  ASAAC_MSL_FAILED_TO_SWITCH_CONTEXT,
  ASAAC_MSL_FAILED_TO_REGISTER_CALLBACK,
  ASAAC_MSL_FAILED_TO_DELETE_CALLBACK,
  ASAAC_MSL_INVALID_EVENT_ID,
  ASAAC_MSL_CALLBACK_INVALID_PARAMETER,
  ASAAC_MSL_CALLBACK_FAILED,
  ASAAC_MSL_FAULT_LOG_SUCCESS,
  ASAAC_MSL_TIMER_NO_ALARM,
  ASAAC_MSL_TIMER_INVALID_ALARM,
  ASAAC_MSL_TIMER_INVALID_ID,
  ASAAC_MSL_IO_FAILED,
  ASAAC_MSL_IO_BUSY,
  ASAAC_MSL_INVALID_CALL
} ASAAC_MSLStatus;
#endif
#if !defined(_ASAAC_NetworkInterface_defined)
#define _ASAAC_NetworkInterface_defined 1
typedef struct ASAAC_NetworkInterface_type ASAAC_NetworkInterface;
struct ASAAC_NetworkInterface_type {
ASAAC_NetworkDescriptor network_type;
unsigned long number_links;
};

#endif
#if !defined(_ASAAC_NetworkPortFinalStatus_defined)
#define _ASAAC_NetworkPortFinalStatus_defined 1
typedef enum {
  ASAAC_HEALTHY,
  ASAAC_FAULTY
} ASAAC_NetworkPortFinalStatus;
#endif
#if !defined(_ASAAC_NetworkPortState_defined)
#define _ASAAC_NetworkPortState_defined 1
typedef struct ASAAC_NetworkPortState_type ASAAC_NetworkPortState;
struct ASAAC_NetworkPortState_type {
ASAAC_PublicId port_id;
ASAAC_NetworkPortFinalStatus status;
};

#endif
#ifndef ASAAC_NW_PORT_STATUS_MAX_LEN
#define ASAAC_NW_PORT_STATUS_MAX_LEN 256U
#endif /* !ASAAC_NW_PORT_STATUS_MAX_LEN */

#if !defined(_ASAAC_NetworkPortStatus_defined)
#define _ASAAC_NetworkPortStatus_defined 1
typedef struct ASAAC_NetworkPortStatus_type ASAAC_NetworkPortStatus;
struct ASAAC_NetworkPortStatus_type {
ASAAC_NetworkPortFinalStatus final_status;
unsigned long status_data_length;
unsigned char detailed_status_data[256];
};

#endif
#if !defined(_ASAAC_NiiReturnStatus_defined)
#define _ASAAC_NiiReturnStatus_defined 1
typedef enum {
  ASAAC_MOS_NII_CALL_COMPLETE,
  ASAAC_MOS_NII_CALL_OK,
  ASAAC_MOS_NII_CALL_FAILED,
  ASAAC_MOS_NII_INVALID_INTERFACE,
  ASAAC_MOS_NII_INVALID_NETWORK,
  ASAAC_MOS_NII_INVALID_TC,
  ASAAC_MOS_NII_INVALID_CONFIG,
  ASAAC_MOS_NII_INVALID_PARAMETER,
  ASAAC_MOS_NII_ALREADY_CONFIGURED,
  ASAAC_MOS_NII_TC_NOT_CONFIGURED,
  ASAAC_MOS_NII_OPEN_TCS,
  ASAAC_MOS_NII_INVALID_MESSAGE_SIZE,
  ASAAC_MOS_NII_BUFFER_NOT_READY,
  ASAAC_MOS_NII_BUFFER_EMPTY,
  ASAAC_MOS_NII_STORAGE_FAULT,
  ASAAC_MOS_NII_STATUS_OK,
  ASAAC_MOS_NII_STATUS_ERROR,
  ASAAC_MOS_NII_STATUS_INIT
} ASAAC_NiiReturnStatus;
#endif
#ifndef ASAAC_MAX_NUMBER_OF_PORTS
#define ASAAC_MAX_NUMBER_OF_PORTS 64U
#endif /* !ASAAC_MAX_NUMBER_OF_PORTS */

#if !defined(_ASAAC_NetworkStatus_defined)
#define _ASAAC_NetworkStatus_defined 1
typedef struct ASAAC_NetworkStatus_type ASAAC_NetworkStatus;
struct ASAAC_NetworkStatus_type {
ASAAC_PublicId network_id;
ASAAC_NetworkPortStatus consolidated_status;
unsigned long nb_of_ports;
ASAAC_NetworkPortState port_status[64];
};

#endif
#if !defined(_ASAAC_Node_defined)
#define _ASAAC_Node_defined 1
typedef unsigned long ASAAC_Node;
#endif
#ifndef ASAAC_MAX_NUMBER_OF_NODES
#define ASAAC_MAX_NUMBER_OF_NODES 256U
#endif /* !ASAAC_MAX_NUMBER_OF_NODES */

#if !defined(_ASAAC_NodeList_defined)
#define _ASAAC_NodeList_defined 1
typedef struct ASAAC_NodeList_type ASAAC_NodeList;
struct ASAAC_NodeList_type {
unsigned long iterator;
unsigned long actual_size;
ASAAC_Node nodes[256];
};

#endif
#if !defined(_ASAAC_SchedulingInfo_defined)
#define _ASAAC_SchedulingInfo_defined 1
typedef CORBA_any ASAAC_SchedulingInfo;
#endif
#if !defined(_ASAAC_SwitchOp_defined)
#define _ASAAC_SwitchOp_defined 1
typedef enum {
  ASAAC_SWITCHOP_ON,
  ASAAC_SWITCHOP_OFF,
  ASAAC_SWITCHOP_LIMBO
} ASAAC_SwitchOp;
#endif
#if !defined(_ASAAC_SwitchStat_defined)
#define _ASAAC_SwitchStat_defined 1
typedef struct ASAAC_SwitchStat_type ASAAC_SwitchStat;
struct ASAAC_SwitchStat_type {
long millivolts;
long milliamps;
ASAAC_SwitchOp state;
};

#endif
#if !defined(_ASAAC_SwitchStatus_defined)
#define _ASAAC_SwitchStatus_defined 1
typedef struct ASAAC_SwitchStatus_type ASAAC_SwitchStatus;
struct ASAAC_SwitchStatus_type {
ASAAC_PublicId switch_id;
ASAAC_SwitchStat status;
};

#endif
#ifndef ASAAC_MAX_NUMBER_OF_POWER_SWITCHES
#define ASAAC_MAX_NUMBER_OF_POWER_SWITCHES 256U
#endif /* !ASAAC_MAX_NUMBER_OF_POWER_SWITCHES */

#if !defined(_ASAAC_PowerSwitch_defined)
#define _ASAAC_PowerSwitch_defined 1
typedef struct ASAAC_PowerSwitch_type ASAAC_PowerSwitch;
struct ASAAC_PowerSwitch_type {
unsigned long number_switches;
ASAAC_SwitchStatus switch_state[256];
};

#endif
#if !defined(_ASAAC_OctetSequence_defined)
#define _ASAAC_OctetSequence_defined 1
typedef unsigned char ASAAC_OctetSequence[256];
typedef unsigned char ASAAC_OctetSequence_slice;
#endif
#if !defined(_ASAAC_OutputRemoteParameters_defined)
#define _ASAAC_OutputRemoteParameters_defined 1
typedef struct ASAAC_OutputRemoteParameters_type ASAAC_OutputRemoteParameters;
struct ASAAC_OutputRemoteParameters_type {
ASAAC_RemoteServiceId _d;
union {
ASAAC_PbitResult powerup_bit_result;
ASAAC_CfmStatus module_status;
ASAAC_CfmInfo module_info;
ASAAC_NetworkStatus net_status;
ASAAC_PowerSwitch power_switch_status;
unsigned long no_parameter;
ASAAC_IbitResult interruptive_bit_result;
} _u;
};
#endif
#if !defined(_ASAAC_PeIdReturnStatus_defined)
#define _ASAAC_PeIdReturnStatus_defined 1
typedef enum {
  ASAAC_PE_ID_CALL_OK,
  ASAAC_PE_ID_CALL_FAILED
} ASAAC_PeIdReturnStatus;
#endif
#if !defined(_ASAAC_PeInfoReturnStatus_defined)
#define _ASAAC_PeInfoReturnStatus_defined 1
typedef enum {
  ASAAC_PE_INFO_CALL_OK,
  ASAAC_PE_INFO_CALL_FAILED
} ASAAC_PeInfoReturnStatus;
#endif
#if !defined(_ASAAC_PoolType_defined)
#define _ASAAC_PoolType_defined 1
typedef enum {
  ASAAC_CODE_RAM,
  ASAAC_DATA_RAM,
  ASAAC_STACK_RAM,
  ASAAC_DEV_RAM,
  ASAAC_BUFFER,
  ASAAC_TFC,
  ASAAC_STREAM_BUFFER
} ASAAC_PoolType;
#endif
#if !defined(_ASAAC_PowerSwitchReturnStatus_defined)
#define _ASAAC_PowerSwitchReturnStatus_defined 1
typedef enum {
  ASAAC_POWER_SWITCH_CALL_OK,
  ASAAC_POWER_SWITCH_CALL_FAILED
} ASAAC_PowerSwitchReturnStatus;
#endif
#if !defined(_ASAAC_IdentAposService_defined)
#define _ASAAC_IdentAposService_defined 1
typedef enum {
  ASAAC_SERVICE_sendMessageNonblocking,
  ASAAC_SERVICE_receiveMessageNonblocking,
  ASAAC_SERVICE_sendMessage,
  ASAAC_SERVICE_receiveMessage,
  ASAAC_SERVICE_lockBuffer,
  ASAAC_SERVICE_sendBuffer,
  ASAAC_SERVICE_receiveBuffer,
  ASAAC_SERVICE_unlockBuffer,
  ASAAC_SERVICE_waitOnMultiChannel,
  ASAAC_SERVICE_createSemaphore,
  ASAAC_SERVICE_deleteSemaphore,
  ASAAC_SERVICE_waitForSemaphore,
  ASAAC_SERVICE_postSemaphore,
  ASAAC_SERVICE_getSemaphoreStatus,
  ASAAC_SERVICE_getSemaphoreId,
  ASAAC_SERVICE_createEvent,
  ASAAC_SERVICE_deleteEvent,
  ASAAC_SERVICE_setEvent,
  ASAAC_SERVICE_resetEvent,
  ASAAC_SERVICE_waitForEvent,
  ASAAC_SERVICE_getEventStatus,
  ASAAC_SERVICE_getEventId,
  ASAAC_SERVICE_getAbsoluteGlobalTime,
  ASAAC_SERVICE_getAbsoluteLocalTime,
  ASAAC_SERVICE_getRelativeLocalTime,
  ASAAC_SERVICE_sleep,
  ASAAC_SERVICE_sleepUntil,
  ASAAC_SERVICE_getMyThreadId,
  ASAAC_SERVICE_terminateSelf,
  ASAAC_SERVICE_terminateErrorHandler,
  ASAAC_SERVICE_suspendSelf,
  ASAAC_SERVICE_startThread,
  ASAAC_SERVICE_stopThread,
  ASAAC_SERVICE_lockThreadPreemption,
  ASAAC_SERVICE_unlockThreadPreemption,
  ASAAC_SERVICE_getThreadStatus,
  ASAAC_SERVICE_createFile,
  ASAAC_SERVICE_deleteFile,
  ASAAC_SERVICE_openFile,
  ASAAC_SERVICE_closeFile,
  ASAAC_SERVICE_getFileAttributes,
  ASAAC_SERVICE_readFile,
  ASAAC_SERVICE_writeFile,
  ASAAC_SERVICE_createDirectory,
  ASAAC_SERVICE_deleteDirectory,
  ASAAC_SERVICE_seekFile,
  ASAAC_SERVICE_lockFile,
  ASAAC_SERVICE_unlockFile,
  ASAAC_SERVICE_getFileBuffer,
  ASAAC_SERVICE_releaseFileBuffer,
  ASAAC_SERVICE_setPowerSwitch,
  ASAAC_SERVICE_resetPowerSwitches,
  ASAAC_SERVICE_getPowerSwitch,
  ASAAC_SERVICE_logMessage,
  ASAAC_SERVICE_raiseApplicationError,
  ASAAC_SERVICE_getErrorInformation,
  ASAAC_number_of_APOS_services
} ASAAC_IdentAposService;
#endif
#ifndef ASAAC_MAX_NUMBER_OF_APOS_SERVICES
#define ASAAC_MAX_NUMBER_OF_APOS_SERVICES 54U
#endif /* !ASAAC_MAX_NUMBER_OF_APOS_SERVICES */

#if !defined(_ASAAC_ServiceAccessList_defined)
#define _ASAAC_ServiceAccessList_defined 1
typedef unsigned char ASAAC_ServiceAccessList[54];
typedef unsigned char ASAAC_ServiceAccessList_slice;
#endif
#if !defined(_ASAAC_ProcessDescription_defined)
#define _ASAAC_ProcessDescription_defined 1
typedef struct ASAAC_ProcessDescription_type ASAAC_ProcessDescription;
struct ASAAC_ProcessDescription_type {
ASAAC_PublicId global_pid;
ASAAC_CharacterSequence programme_file_name;
unsigned long programme_file_Size;
ASAAC_AccessType access_type;
ASAAC_AccessInfo access_info;
ASAAC_PublicId cpu_id;
ASAAC_ServiceAccessList apos_services;
ASAAC_TimeInterval timeout;
};

#endif
#if !defined(_ASAAC_QueuingDiscipline_defined)
#define _ASAAC_QueuingDiscipline_defined 1
typedef enum {
  ASAAC_QUEUING_DISCIPLINE_FIFO,
  ASAAC_QUEUING_DISCIPLINE_PRIORITY
} ASAAC_QueuingDiscipline;
#endif
#if !defined(_ASAAC_ReadFileResult_defined)
#define _ASAAC_ReadFileResult_defined 1
typedef enum {
  ASAAC_READ_FILE_ACK_OK,
  ASAAC_READ_FILE_ACK_FAILURE_NO_FILE,
  ASAAC_READ_FILE_ACK_FAILURE_NO_READ_ACCESS
} ASAAC_ReadFileResult;
#endif
#if !defined(_ASAAC_RegionID_defined)
#define _ASAAC_RegionID_defined 1
typedef unsigned long ASAAC_RegionID;
#endif
#if !defined(_ASAAC_ReturnStatus_defined)
#define _ASAAC_ReturnStatus_defined 1
typedef enum {
  ASAAC_SUCCESS,
  ASAAC_ERROR
} ASAAC_ReturnStatus;
#endif
#if !defined(_ASAAC_ResourceReturnStatus_defined)
#define _ASAAC_ResourceReturnStatus_defined 1
typedef enum {
  ASAAC_RS_SUCCESS,
  ASAAC_RS_ERROR,
  ASAAC_RS_RESOURCE
} ASAAC_ResourceReturnStatus;
#endif
#if !defined(_ASAAC_SecurityInfo_defined)
#define _ASAAC_SecurityInfo_defined 1
typedef enum {
  ASAAC_Marked,
  ASAAC_Unmarked
} ASAAC_SecurityInfo;
#endif
#if !defined(_ASAAC_SecurityRating_defined)
#define _ASAAC_SecurityRating_defined 1
typedef struct ASAAC_SecurityRating_type ASAAC_SecurityRating;
struct ASAAC_SecurityRating_type {
ASAAC_ClassificationLevel classification_level;
ASAAC_Category security_category;
};

#endif
#if !defined(_ASAAC_SeekMode_defined)
#define _ASAAC_SeekMode_defined 1
typedef enum {
  ASAAC_START_OF_FILE,
  ASAAC_CURRENT_POSITION,
  ASAAC_END_OF_FILE
} ASAAC_SeekMode;
#endif
#if !defined(_ASAAC_State_defined)
#define _ASAAC_State_defined 1
typedef ASAAC_PublicId ASAAC_State;
#endif
#ifndef ASAAC_TC_CONFIG_MAX_LEN
#define ASAAC_TC_CONFIG_MAX_LEN 256U
#endif /* !ASAAC_TC_CONFIG_MAX_LEN */

#if !defined(_ASAAC_TcConfigurationData_defined)
#define _ASAAC_TcConfigurationData_defined 1
typedef unsigned char ASAAC_TcConfigurationData[256];
typedef unsigned char ASAAC_TcConfigurationData_slice;
#endif
#if !defined(_ASAAC_TcDescription_defined)
#define _ASAAC_TcDescription_defined 1
typedef struct ASAAC_TcDescription_type ASAAC_TcDescription;
struct ASAAC_TcDescription_type {
ASAAC_PublicId tc_id;
ASAAC_NetworkDescriptor network_descr;
ASAAC_Bool is_receiver;
ASAAC_Bool is_msg_transfer;
ASAAC_Bool is_fragmented;
ASAAC_SecurityRating security_rating;
ASAAC_PublicId cpu_id;
ASAAC_InterfaceType conf_data_type;
unsigned long conf_data_size;
ASAAC_TcConfigurationData conf_data;
};

#endif
#if !defined(_ASAAC_TcConfigurationDMC_defined)
#define _ASAAC_TcConfigurationDMC_defined 1
typedef struct ASAAC_TcConfigurationDMC_type ASAAC_TcConfigurationDMC;
struct ASAAC_TcConfigurationDMC_type {
ASAAC_PublicId tc_id_routing;
ASAAC_PublicId fragment_id;
unsigned long start_address;
unsigned long length_1;
unsigned long increment_1;
unsigned long length_2;
unsigned long increment_2;
unsigned long length_3;
unsigned long increment_3;
};

#endif
#if !defined(_ASAAC_TC_ConfigurationData_defined)
#define _ASAAC_TC_ConfigurationData_defined 1
typedef ASAAC_Address ASAAC_TC_ConfigurationData;
#endif
#if !defined(_ASAAC_ThreadDescription_defined)
#define _ASAAC_ThreadDescription_defined 1
typedef struct ASAAC_ThreadDescription_type ASAAC_ThreadDescription;
struct ASAAC_ThreadDescription_type {
ASAAC_PublicId global_pid;
ASAAC_PublicId thread_id;
ASAAC_CharacterSequence entry_point;
ASAAC_PublicId cpu_id;
unsigned long stack_size;
ASAAC_SecurityRating security_rating;
};

#endif
#if !defined(_ASAAC_ThreadSchedulingInfo_defined)
#define _ASAAC_ThreadSchedulingInfo_defined 1
typedef struct ASAAC_ThreadSchedulingInfo_type ASAAC_ThreadSchedulingInfo;
struct ASAAC_ThreadSchedulingInfo_type {
ASAAC_PublicId global_pid;
ASAAC_PublicId thread_id;
ASAAC_SchedulingInfo scheduling_info;
};

#endif
#if !defined(_ASAAC_ThreadStatus_defined)
#define _ASAAC_ThreadStatus_defined 1
typedef enum {
  ASAAC_DORMANT,
  ASAAC_READY,
  ASAAC_WAITING,
  ASAAC_RUNNING
} ASAAC_ThreadStatus;
#endif
#if !defined(_ASAAC_TimedReturnStatus_defined)
#define _ASAAC_TimedReturnStatus_defined 1
typedef enum {
  ASAAC_TM_SUCCESS,
  ASAAC_TM_ERROR,
  ASAAC_TM_TIMEOUT
} ASAAC_TimedReturnStatus;
#endif
#if !defined(_ASAAC_TimerReturnStatus_defined)
#define _ASAAC_TimerReturnStatus_defined 1
typedef enum {
  ASAAC_MOS_TIMER_CALL_OK,
  ASAAC_MOS_TIMER_CALL_FAILED
} ASAAC_TimerReturnStatus;
#endif
#if !defined(_ASAAC_TransferDirection_defined)
#define _ASAAC_TransferDirection_defined 1
typedef enum {
  ASAAC_TRANSFER_DIRECTION_SEND,
  ASAAC_TRANSFER_DIRECTION_RECEIVE
} ASAAC_TransferDirection;
#endif
#if !defined(_ASAAC_TransferType_defined)
#define _ASAAC_TransferType_defined 1
typedef enum {
  ASAAC_TRANSFER_TYPE_MESSAGE,
  ASAAC_TRANSFER_TYPE_STREAMING
} ASAAC_TransferType;
#endif
#if !defined(_ASAAC_UseAccessRights_defined)
#define _ASAAC_UseAccessRights_defined 1
typedef enum {
  ASAAC_READ,
  ASAAC_WRITE,
  ASAAC_READWRITE
} ASAAC_UseAccessRights;
#endif
#if !defined(_ASAAC_UseConcurrencePattern_defined)
#define _ASAAC_UseConcurrencePattern_defined 1
typedef enum {
  ASAAC_SHARE,
  ASAAC_EXCLUSIVE
} ASAAC_UseConcurrencePattern;
#endif
#if !defined(_ASAAC_UseOption_defined)
#define _ASAAC_UseOption_defined 1
typedef struct ASAAC_UseOption_type ASAAC_UseOption;
struct ASAAC_UseOption_type {
ASAAC_UseAccessRights use_access;
ASAAC_UseConcurrencePattern use_concur;
};

#endif
#if !defined(_ASAAC_VirtualChannelType_defined)
#define _ASAAC_VirtualChannelType_defined 1
typedef enum {
  ASAAC_Application_Header_VC,
  ASAAC_Application_Raw_VC,
  ASAAC_OLI_VC
} ASAAC_VirtualChannelType;
#endif
#ifndef ASAAC_MAX_DATA_REPRESENTATION
#define ASAAC_MAX_DATA_REPRESENTATION 256U
#endif /* !ASAAC_MAX_DATA_REPRESENTATION */

#if !defined(_ASAAC_VcDescription_defined)
#define _ASAAC_VcDescription_defined 1
typedef struct ASAAC_VcDescription_type ASAAC_VcDescription;
struct ASAAC_VcDescription_type {
ASAAC_PublicId global_vc_id;
unsigned long max_msg_length;
unsigned long max_number_of_buffers;
unsigned long max_number_of_threads_attached;
unsigned long max_number_of_TCs_attached;
ASAAC_SecurityRating security_rating;
ASAAC_SecurityInfo security_info;
ASAAC_VirtualChannelType vc_type;
ASAAC_PublicId cpu_id;
ASAAC_Bool is_typed_message;
ASAAC_DataRepresentation data_representation_format;
};

#endif
#if !defined(_ASAAC_VcMappingDescription_defined)
#define _ASAAC_VcMappingDescription_defined 1
typedef struct ASAAC_VcMappingDescription_type ASAAC_VcMappingDescription;
struct ASAAC_VcMappingDescription_type {
ASAAC_PublicId global_pid;
ASAAC_PublicId local_vc_id;
ASAAC_PublicId global_vc_id;
ASAAC_PublicId local_thread_id;
unsigned long buffer_size;
unsigned long number_of_message_buffers;
ASAAC_Bool is_reading;
ASAAC_Bool is_lifo_queue;
ASAAC_Bool is_refusing_queue;
unsigned long Priority;
};

#endif
#if !defined(_ASAAC_VcToTcMappingDescription_defined)
#define _ASAAC_VcToTcMappingDescription_defined 1
typedef struct ASAAC_VcToTcMappingDescription_type ASAAC_VcToTcMappingDescription;
struct ASAAC_VcToTcMappingDescription_type {
ASAAC_PublicId global_vc_id;
ASAAC_PublicId tc_id;
ASAAC_Bool is_data_representation;
};

#endif
#if !defined(_ASAAC_OLI_VcHeader_defined)
#define _ASAAC_OLI_VcHeader_defined 1
typedef struct ASAAC_OLI_VcHeader_type ASAAC_OLI_VcHeader;
struct ASAAC_OLI_VcHeader_type {
ASAAC_PublicId vc_id;
};

#endif
#if !defined(_ASAAC_OLI_OliMessageId_defined)
#define _ASAAC_OLI_OliMessageId_defined 1
typedef enum {
  ASAAC_OLI_RequestFileRead,
  ASAAC_OLI_ReplyFileRead,
  ASAAC_OLI_RequestMliDownload,
  ASAAC_OLI_ReplyMliDownload
} ASAAC_OLI_OliMessageId;
#endif
#if !defined(_ASAAC_OLI_RequestFileReadPayload_defined)
#define _ASAAC_OLI_RequestFileReadPayload_defined 1
typedef struct ASAAC_OLI_RequestFileReadPayload_type ASAAC_OLI_RequestFileReadPayload;
struct ASAAC_OLI_RequestFileReadPayload_type {
unsigned long size;
unsigned long offset;
ASAAC_CharacterSequence filename;
};

#endif
#if !defined(_ASAAC_OLI_ReplyFileReadPayload_defined)
#define _ASAAC_OLI_ReplyFileReadPayload_defined 1
typedef struct ASAAC_OLI_ReplyFileReadPayload_type ASAAC_OLI_ReplyFileReadPayload;
struct ASAAC_OLI_ReplyFileReadPayload_type {
unsigned long size;
unsigned long checksum;
ASAAC_ReadFileResult result;
ASAAC_OctetSequence filedata;
};

#endif
#if !defined(_ASAAC_OLI_RequestRemoteMliDownload_defined)
#define _ASAAC_OLI_RequestRemoteMliDownload_defined 1
typedef struct ASAAC_OLI_RequestRemoteMliDownload_type ASAAC_OLI_RequestRemoteMliDownload;
struct ASAAC_OLI_RequestRemoteMliDownload_type {
ASAAC_CfmDescription cfm_description;
};

#endif
#if !defined(_ASAAC_OLI_ReplyRemoteMliDownload_defined)
#define _ASAAC_OLI_ReplyRemoteMliDownload_defined 1
typedef struct ASAAC_OLI_ReplyRemoteMliDownload_type ASAAC_OLI_ReplyRemoteMliDownload;
struct ASAAC_OLI_ReplyRemoteMliDownload_type {
unsigned long block_number;
ASAAC_LoadFileResult result;
};

#endif
#if !defined(_ASAAC_OLI_OliMessageParameter_defined)
#define _ASAAC_OLI_OliMessageParameter_defined 1
typedef struct ASAAC_OLI_OliMessageParameter_type ASAAC_OLI_OliMessageParameter;
struct ASAAC_OLI_OliMessageParameter_type {
ASAAC_OLI_OliMessageId _d;
union {
ASAAC_OLI_RequestFileReadPayload request_read_file;
ASAAC_OLI_ReplyFileReadPayload reply_read_file;
ASAAC_OLI_RequestRemoteMliDownload request_mli_download;
ASAAC_OLI_ReplyRemoteMliDownload reply_mli_download;
} _u;
};
#endif
#if !defined(_ASAAC_OLI_OliMessage_defined)
#define _ASAAC_OLI_OliMessage_defined 1
typedef struct ASAAC_OLI_OliMessage_type ASAAC_OLI_OliMessage;
struct ASAAC_OLI_OliMessage_type {
unsigned long transfer_id;
ASAAC_OLI_OliMessageId unique_message_id;
ASAAC_OLI_OliMessageParameter message_parameter;
};

#endif
#if !defined(_ASAAC_GLI_GliMessageId_defined)
#define _ASAAC_GLI_GliMessageId_defined 1
typedef enum {
  ASAAC_GLI_Load_Configuration,
  ASAAC_GLI_Configuration_Loaded,
  ASAAC_GLI_Stop_Configuration,
  ASAAC_GLI_Configuration_Stopped,
  ASAAC_GLI_Run_Configuration,
  ASAAC_GLI_Configuration_Running,
  ASAAC_GLI_Change_Configuration,
  ASAAC_GLI_Configuration_Changed,
  ASAAC_GLI_Request_New_Cfm,
  ASAAC_GLI_Cfm_Allocated,
  ASAAC_GLI_Deallocate_Cfm,
  ASAAC_GLI_Cfm_Deallocated,
  ASAAC_GLI_Fault_Report,
  ASAAC_GLI_Request_BIT_Result,
  ASAAC_GLI_Report_BIT_Result,
  ASAAC_GLI_Are_You_Alive,
  ASAAC_GLI_I_Am_Alive,
  ASAAC_GLI_Request_SC,
  ASAAC_GLI_SC_Response,
  ASAAC_GLI_DH_Send_M,
  ASAAC_GLI_DH_Send_X,
  ASAAC_GLI_DH_Send_XimodM,
  ASAAC_GLI_DH_Send_XjmodM,
  ASAAC_GLI_Request_Key,
  ASAAC_GLI_Send_Key
} ASAAC_GLI_GliMessageId;
#endif
#if !defined(_ASAAC_GLI_GliAliveParameter_defined)
#define _ASAAC_GLI_GliAliveParameter_defined 1
typedef struct ASAAC_GLI_GliAliveParameter_type ASAAC_GLI_GliAliveParameter;
struct ASAAC_GLI_GliAliveParameter_type {
ASAAC_PublicId function_id;
ASAAC_PublicId status_id;
};

#endif
#if !defined(_ASAAC_GLI_GliMessageParameter_defined)
#define _ASAAC_GLI_GliMessageParameter_defined 1
typedef struct ASAAC_GLI_GliMessageParameter_type ASAAC_GLI_GliMessageParameter;
struct ASAAC_GLI_GliMessageParameter_type {
ASAAC_GLI_GliMessageId _d;
union {
ASAAC_PublicId config_to_be_loaded;
ASAAC_PublicId config_loaded;
ASAAC_PublicId config_to_be_acquired;
ASAAC_PublicId config_acquired;
ASAAC_PublicId config_to_be_run;
ASAAC_PublicId config_run;
ASAAC_PublicId configuration_event;
ASAAC_PublicId new_configuration;
unsigned long no_parameter;
ASAAC_PublicId Allocated_Cfm_Id;
ASAAC_PublicId Deallocate_Cfm_Id;
ASAAC_PublicId Deallocated_Cfm_Id;
ASAAC_FaultReport the_fault;
ASAAC_BitType type;
ASAAC_BitResult result;
ASAAC_GLI_GliAliveParameter alive_param;
ASAAC_PublicId request_sc_tls_id;
ASAAC_Bool response;
unsigned long key;
ASAAC_PublicId request_key_tls_id;
unsigned long key_array[10];
} _u;
};
#endif
#if !defined(_ASAAC_GLI_GliMessage_defined)
#define _ASAAC_GLI_GliMessage_defined 1
typedef struct ASAAC_GLI_GliMessage_type ASAAC_GLI_GliMessage;
struct ASAAC_GLI_GliMessage_type {
ASAAC_GLI_GliMessageId unique_message_id;
ASAAC_GLI_GliMessageParameter message_parameter;
};

#endif
#if !defined(_ASAAC_SMLI_SmliMessageId_defined)
#define _ASAAC_SMLI_SmliMessageId_defined 1
typedef enum {
  ASAAC_SMLI_Request_Lc_Change,
  ASAAC_SMLI_Lc_Changed,
  ASAAC_SMLI_Signal_For_Lc_Change,
  ASAAC_SMLI_Ready_For_Lc_Change,
  ASAAC_SMLI_Security_Data_Written,
  ASAAC_SMLI_SM_Config_Complete,
  ASAAC_SMLI_Distant_Error_Event
} ASAAC_SMLI_SmliMessageId;
#endif
#if !defined(_ASAAC_SMLI_SmliMessageParameter_defined)
#define _ASAAC_SMLI_SmliMessageParameter_defined 1
typedef struct ASAAC_SMLI_SmliMessageParameter_type ASAAC_SMLI_SmliMessageParameter;
struct ASAAC_SMLI_SmliMessageParameter_type {
ASAAC_SMLI_SmliMessageId _d;
union {
ASAAC_PublicId request_lc_change_event_id;
ASAAC_PublicId lc_changed_logical_config_id;
ASAAC_PublicId signal_for_lc_change_logical_config_id;
ASAAC_PublicId ready_for_lc_change_event_id;
unsigned long no_parameter_1;
unsigned long no_parameter_2;
ASAAC_PublicId logical_config_id;
} _u;
};
#endif
#if !defined(_ASAAC_SMLI_SmliMessage_defined)
#define _ASAAC_SMLI_SmliMessage_defined 1
typedef struct ASAAC_SMLI_SmliMessage_type ASAAC_SMLI_SmliMessage;
struct ASAAC_SMLI_SmliMessage_type {
ASAAC_SMLI_SmliMessageId unique_message_id;
ASAAC_SMLI_SmliMessageParameter message_parameter;
};

#endif
#if !defined(_ASAAC_MLI_TcHeader_defined)
#define _ASAAC_MLI_TcHeader_defined 1
typedef struct ASAAC_MLI_TcHeader_type ASAAC_MLI_TcHeader;
struct ASAAC_MLI_TcHeader_type {
ASAAC_PublicId tc_id;
};

#endif
#ifndef ASAAC_MLI_VC_PAYLOAD_IDENTIFIER
#define ASAAC_MLI_VC_PAYLOAD_IDENTIFIER 0U
#endif /* !ASAAC_MLI_VC_PAYLOAD_IDENTIFIER */

#ifndef ASAAC_MLI_VC_SEGMENT_IDENTIFIER
#define ASAAC_MLI_VC_SEGMENT_IDENTIFIER 1583419873U
#endif /* !ASAAC_MLI_VC_SEGMENT_IDENTIFIER */

#if !defined(_ASAAC_MLI_OptionalHeaderElement_defined)
#define _ASAAC_MLI_OptionalHeaderElement_defined 1
typedef struct ASAAC_MLI_OptionalHeaderElement_type ASAAC_MLI_OptionalHeaderElement;
struct ASAAC_MLI_OptionalHeaderElement_type {
ASAAC_PublicId header_element_identifier;
unsigned long header_element_length;
};

#endif

/** stub prototypes **/
ASAAC_ReturnStatus ASAAC_APOS_sleep(const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_APOS_sleepUntil(const ASAAC_Time* absolute_local_time);
ASAAC_ReturnStatus ASAAC_APOS_getMyThreadId(ASAAC_PublicId* thread_id);
ASAAC_ReturnStatus ASAAC_APOS_startThread(const ASAAC_PublicId thread_id);
ASAAC_ReturnStatus ASAAC_APOS_suspendSelf();
ASAAC_ReturnStatus ASAAC_APOS_stopThread(const ASAAC_PublicId thread_id);
void ASAAC_APOS_terminateSelf();
ASAAC_ReturnStatus ASAAC_APOS_lockThreadPreemption(unsigned long* lock_level);
ASAAC_ReturnStatus ASAAC_APOS_unlockThreadPreemption(unsigned long* lock_level);
ASAAC_ReturnStatus ASAAC_APOS_getThreadStatus(const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status);
ASAAC_ReturnStatus ASAAC_APOS_getAbsoluteLocalTime(ASAAC_Time* absolute_local_time);
ASAAC_ReturnStatus ASAAC_APOS_getAbsoluteGlobalTime(ASAAC_Time* absolute_global_time);
ASAAC_ReturnStatus ASAAC_APOS_getRelativeLocalTime(ASAAC_Time* relative_local_time);
ASAAC_ResourceReturnStatus ASAAC_APOS_createSemaphore(const ASAAC_CharacterSequence* name, ASAAC_PrivateId* semaphore_id, const unsigned long init_value, const unsigned long max_value, const ASAAC_QueuingDiscipline queuing_discipline);
ASAAC_ReturnStatus ASAAC_APOS_deleteSemaphore(const ASAAC_PrivateId semaphore_id);
ASAAC_TimedReturnStatus ASAAC_APOS_waitForSemaphore(const ASAAC_PrivateId semaphore_id, const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_APOS_postSemaphore(const ASAAC_PrivateId semaphore_id);
ASAAC_ReturnStatus ASAAC_APOS_getSemaphoreStatus(const ASAAC_PrivateId semaphore_id, unsigned long* current_value, unsigned long* waiting_callers);
ASAAC_ReturnStatus ASAAC_APOS_getSemaphoreId(const ASAAC_CharacterSequence* name, ASAAC_PrivateId* semaphore_id);
ASAAC_ResourceReturnStatus ASAAC_APOS_createEvent(const ASAAC_CharacterSequence* name, ASAAC_PrivateId* event_id);
ASAAC_ReturnStatus ASAAC_APOS_deleteEvent(const ASAAC_PrivateId event_id);
ASAAC_ReturnStatus ASAAC_APOS_setEvent(const ASAAC_PrivateId event_id);
ASAAC_ReturnStatus ASAAC_APOS_resetEvent(const ASAAC_PrivateId event_id);
ASAAC_TimedReturnStatus ASAAC_APOS_waitForEvent(const ASAAC_PrivateId event_id, const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_APOS_getEventStatus(const ASAAC_PrivateId event_id, ASAAC_EventStatus* event_status, unsigned long* waiting_callers);
ASAAC_ReturnStatus ASAAC_APOS_getEventId(const ASAAC_CharacterSequence* name, ASAAC_PrivateId* event_id);
ASAAC_ReturnStatus ASAAC_APOS_logMessage(const ASAAC_CharacterSequence* log_message, const ASAAC_LogMessageType message_type);
ASAAC_ReturnStatus ASAAC_APOS_raiseApplicationError(const ASAAC_ErrorCode error_code, const ASAAC_CharacterSequence* error_message);
ASAAC_ReturnStatus ASAAC_APOS_getErrorInformation(ASAAC_PublicId* faulty_thread_id, ASAAC_ErrorType* error_type, ASAAC_ErrorCode* error_code, ASAAC_CharacterSequence* error_message);
void ASAAC_APOS_terminateErrorHandler(const ASAAC_ReturnStatus return_status);
ASAAC_ReturnStatus ASAAC_APOS_getDebugErrorInformation(ASAAC_ErrorType* error_type, ASAAC_ErrorCode* error_code, ASAAC_CharacterSequence* error_message);
ASAAC_ResourceReturnStatus ASAAC_APOS_sendMessageNonblocking(const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference, const unsigned long actual_size);
ASAAC_ResourceReturnStatus ASAAC_APOS_receiveMessageNonblocking(const ASAAC_PublicId local_vc_id, const unsigned long maximum_size, const ASAAC_Address message_buffer_reference, unsigned long* actual_size);
ASAAC_TimedReturnStatus ASAAC_APOS_sendMessage(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval* timeout, const ASAAC_Address message_buffer_reference, const unsigned long actual_size);
ASAAC_TimedReturnStatus ASAAC_APOS_receiveMessage(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval* timeout, const unsigned long maximum_size, const ASAAC_Address message_buffer_reference, unsigned long* actual_size);
ASAAC_TimedReturnStatus ASAAC_APOS_lockBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval* timeout, ASAAC_Address* message_buffer_reference, const unsigned long maximum_size);
ASAAC_ReturnStatus ASAAC_APOS_sendBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference, const unsigned long actual_size);
ASAAC_TimedReturnStatus ASAAC_APOS_receiveBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_TimeInterval* timeout, ASAAC_Address* message_buffer_reference, unsigned long* actual_size);
ASAAC_ReturnStatus ASAAC_APOS_unlockBuffer(const ASAAC_PublicId local_vc_id, const ASAAC_Address message_buffer_reference);
ASAAC_TimedReturnStatus ASAAC_APOS_waitOnMultiChannel(const ASAAC_PublicIdSet* vc_set_in, const unsigned long min_no_vc, ASAAC_PublicIdSet* vc_set_out, const ASAAC_TimeInterval* timeout);
ASAAC_ResourceReturnStatus ASAAC_APOS_createDirectory(const ASAAC_CharacterSequence* name, const ASAAC_AccessRights access);
ASAAC_TimedReturnStatus ASAAC_APOS_deleteDirectory(const ASAAC_CharacterSequence* name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval* timeout);
ASAAC_ResourceReturnStatus ASAAC_APOS_createFile(const ASAAC_CharacterSequence* name, const ASAAC_AccessRights access, const unsigned long file_size);
ASAAC_TimedReturnStatus ASAAC_APOS_deleteFile(const ASAAC_CharacterSequence* name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_APOS_openFile(const ASAAC_CharacterSequence* name, const ASAAC_UseOption* use_option, ASAAC_PrivateId* file_handle);
ASAAC_ReturnStatus ASAAC_APOS_closeFile(const ASAAC_PrivateId file_handle);
ASAAC_TimedReturnStatus ASAAC_APOS_lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_APOS_unlockFile(const ASAAC_PrivateId filehandle);
ASAAC_ReturnStatus ASAAC_APOS_getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights* access, ASAAC_LockStatus* lock_status);
ASAAC_ReturnStatus ASAAC_APOS_seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long* new_pos);
ASAAC_TimedReturnStatus ASAAC_APOS_readFile(const ASAAC_PrivateId filehandle, ASAAC_Address* buffer_address, const long read_count, long* count_read, const ASAAC_TimeInterval* timeout);
ASAAC_TimedReturnStatus ASAAC_APOS_writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long* count_written, const ASAAC_TimeInterval* timeout);
ASAAC_TimedReturnStatus ASAAC_APOS_getFileBuffer(const unsigned long buffer_size, ASAAC_Address* buffer_address, const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_APOS_releaseFileBuffer(const ASAAC_Address buffer_address);
ASAAC_ReturnStatus ASAAC_APOS_setPowerSwitch(const ASAAC_PublicId switch_id, const ASAAC_SwitchOp switch_op);
ASAAC_ReturnStatus ASAAC_APOS_resetPowerSwitches();
ASAAC_ReturnStatus ASAAC_APOS_getPowerSwitchStatus(ASAAC_PowerSwitch* power_switch);
ASAAC_TimerReturnStatus ASAAC_MOS_getAbsoluteLocalTime(ASAAC_Time* ac_system_time);
ASAAC_TimerReturnStatus ASAAC_MOS_getRelativeLocalTime(ASAAC_Time* cfm_time);
ASAAC_TimerReturnStatus ASAAC_MOS_getAbsoluteGlobalTime(ASAAC_Time* absolute_global_time);
ASAAC_TimerReturnStatus ASAAC_MOS_configureClock(const ASAAC_ClockInfo* clock_info);
ASAAC_TimerReturnStatus ASAAC_MOS_attachFederatedClock(const ASAAC_FederatedClockInfo* federated_clock_info);
ASAAC_TimerReturnStatus ASAAC_MOS_setupTimer(const ASAAC_PublicId timer_id, const ASAAC_Time* time_to_expire, const ASAAC_PublicId callback_id, const ASAAC_AlarmType alarm_type);
ASAAC_TimerReturnStatus ASAAC_MOS_startTimer(const ASAAC_PublicId timer_id);
ASAAC_TimerReturnStatus ASAAC_MOS_stopTimer(const ASAAC_PublicId timer_id);
ASAAC_TimerReturnStatus ASAAC_MOS_readTimer(const ASAAC_PublicId timer_id, ASAAC_Time* time_to_expire);
ASAAC_ResourceReturnStatus ASAAC_MOS_readLogDevice(const ASAAC_LogMessageType message_type, const unsigned long log_id, ASAAC_CharacterSequence* log_message);
ASAAC_ResourceReturnStatus ASAAC_MOS_writeLogDevice(const ASAAC_LogMessageType message_type, const unsigned long log_id, const ASAAC_CharacterSequence* log_message);
ASAAC_ReturnStatus ASAAC_MOS_erasePhysicalMemory();
ASAAC_MSLStatus ASAAC_MOS_registerCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id, const ASAAC_Address callback);
ASAAC_MSLStatus ASAAC_MOS_enableCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id);
ASAAC_MSLStatus ASAAC_MOS_disableCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id);
ASAAC_MSLStatus ASAAC_MOS_deleteCallback(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id);
void ASAAC_MOS_callbackHandler(const ASAAC_EventType event_type, const ASAAC_PublicId callback_id, const ASAAC_Address event_info_data);
ASAAC_BitReturnStatus ASAAC_MOS_getPbitResult(ASAAC_PbitResult* pbit_result);
ASAAC_BitReturnStatus ASAAC_MOS_getCbitResult(ASAAC_CbitResult* cbit_result);
ASAAC_BitReturnStatus ASAAC_MOS_startIbit();
ASAAC_BitReturnStatus ASAAC_MOS_getIbitResult(ASAAC_IbitResult* ibit_result);
ASAAC_ReturnStatus ASAAC_MOS_startCbit(const unsigned long test_code, const ASAAC_CbitModeType mode, ASAAC_BitTestStatus* bit_test_status);
ASAAC_CfmParameterReturnStatus ASAAC_MOS_getCfmInfo(ASAAC_CfmInfo* cfm_info);
ASAAC_CfmStatusReturnStatus ASAAC_MOS_getCfmStatus(ASAAC_CfmStatus* cfm_status);
ASAAC_PeIdReturnStatus ASAAC_MOS_getMyPeId(ASAAC_PublicId* my_pe_id);
ASAAC_PeInfoReturnStatus ASAAC_MOS_getPeInfo(ASAAC_PeResources* my_resources);
ASAAC_NiiReturnStatus ASAAC_MOS_configureInterface(const ASAAC_PublicId interface_id, const ASAAC_NetworkDescriptor* network_id, const ASAAC_InterfaceConfigurationData* configuration_data);
ASAAC_NiiReturnStatus ASAAC_MOS_configureTransfer(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_id, const ASAAC_TransferDirection send_receive, const ASAAC_TransferType message_streaming, const ASAAC_TC_ConfigurationData configuration_data, const ASAAC_Bool trigger_callback, const ASAAC_PublicId callback_id);
ASAAC_NiiReturnStatus ASAAC_MOS_sendTransfer(const ASAAC_PublicId tc_id, const ASAAC_CharAddress transmit_data, const ASAAC_Length data_length, const ASAAC_Time* time_out);
ASAAC_NiiReturnStatus ASAAC_MOS_receiveTransfer(const ASAAC_PublicId tc_id, ASAAC_CharAddress* receive_data, const ASAAC_Length data_length_available, ASAAC_Length* data_length, const ASAAC_Time* time_out);
ASAAC_NiiReturnStatus ASAAC_MOS_destroyTransfer(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_id);
ASAAC_NiiReturnStatus ASAAC_MOS_getNetworkPortStatus(const ASAAC_NetworkDescriptor* network_id, ASAAC_NetworkPortStatus* info_data);
ASAAC_NiiReturnStatus ASAAC_MOS_receiveNetwork(const ASAAC_NetworkDescriptor* network, ASAAC_CharAddress* receive_data, const ASAAC_Length data_length_available, ASAAC_Length* data_length, ASAAC_PublicId* tc_id, const ASAAC_Time* time_out);
ASAAC_ReturnStatus ASAAC_MOS_setPowerSwitch(const ASAAC_PublicId switch_id, const ASAAC_SwitchOp switch_op);
ASAAC_ReturnStatus ASAAC_MOS_resetPowerSwitches();
ASAAC_ReturnStatus ASAAC_MOS_getPowerSwitchStatus(ASAAC_PowerSwitch* power_switch);
ASAAC_MSLStatus ASAAC_MOS_SetTxData(unsigned long* key, const ASAAC_Address buffer, const unsigned long size);
ASAAC_MSLStatus ASAAC_MOS_SetRxData(unsigned long* key, const ASAAC_Address buffer, const unsigned long size);
ASAAC_MSLStatus ASAAC_MOS_StartTransfer(const unsigned long key, const unsigned long lba, const ASAAC_IOoperation op);
ASAAC_NiiReturnStatus ASAAC_MOS_configureFragmentedTransfer(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_id, const ASAAC_TransferDirection send_receive, const ASAAC_TransferType message_streaming, const ASAAC_TC_ConfigurationData configuration_data, const ASAAC_Bool trigger_callback, const ASAAC_PublicId callback_id);
ASAAC_NiiReturnStatus ASAAC_MOS_sendFragmentedTransfer(const ASAAC_PublicId tc_id, const ASAAC_CharAddress transmit_data, const ASAAC_Length data_length);
ASAAC_NiiReturnStatus ASAAC_MOS_receiveFragmentedTransfer(const ASAAC_PublicId tc_id, ASAAC_CharAddress* receive_data, ASAAC_Length* data_length_available, const ASAAC_Length data_length);
ASAAC_ReturnStatus ASAAC_SMBP_getRootNode(ASAAC_Node* root_node);
ASAAC_ReturnStatus ASAAC_SMBP_readNode(const ASAAC_Node parent_node, const ASAAC_PublicId item_id, ASAAC_Node* node_id);
ASAAC_ReturnStatus ASAAC_SMBP_getNodeId(const ASAAC_Node node_id, ASAAC_PublicId* identifier);
ASAAC_ReturnStatus ASAAC_SMBP_getAttributes(const ASAAC_Node node_id, const unsigned long buffer_size, const ASAAC_Address buffer);
ASAAC_ReturnStatus ASAAC_SMBP_getChildNodes(const ASAAC_Node parent_node, ASAAC_NodeList* node_list);
ASAAC_ReturnStatus ASAAC_SMBP_getLength(const ASAAC_NodeList* node_list, unsigned long* set_size);
ASAAC_ReturnStatus ASAAC_SMBP_item(const ASAAC_NodeList* node_list, ASAAC_Node* node_id);
ASAAC_TimedReturnStatus ASAAC_SMOS_createProcess(const ASAAC_ProcessDescription* process_desc);
ASAAC_ReturnStatus ASAAC_SMOS_createThread(const ASAAC_ThreadDescription* thread_desc);
ASAAC_ReturnStatus ASAAC_SMOS_runProcess(const ASAAC_PublicId process_id);
ASAAC_ReturnStatus ASAAC_SMOS_stopProcess(const ASAAC_PublicId process_id);
ASAAC_ReturnStatus ASAAC_SMOS_destroyProcess(const ASAAC_PublicId process_id);
ASAAC_ReturnStatus ASAAC_SMOS_setSchedulingParameters(const ASAAC_ThreadSchedulingInfo* thread_scheduling_info);
ASAAC_ReturnStatus ASAAC_SMOS_getThreadState(const ASAAC_PublicId process_id, const ASAAC_PublicId thread_id, ASAAC_ThreadStatus* thread_status);
ASAAC_TimedReturnStatus ASAAC_SMOS_getError(ASAAC_ErrorInfo* error_info, const ASAAC_TimeInterval* time_out);
ASAAC_TimedReturnStatus ASAAC_SMOS_activateErrorHandler(const ASAAC_PublicId process_id, const ASAAC_PublicId faulty_thread_id, const ASAAC_ErrorType error_type, const ASAAC_ErrorCode error_code, const ASAAC_CharacterSequence* error_message, ASAAC_ReturnStatus* error_handler_status, const ASAAC_TimeInterval* time_out);
ASAAC_ReturnStatus ASAAC_SMOS_createVirtualChannel(const ASAAC_VcDescription* vc_desc);
ASAAC_ReturnStatus ASAAC_SMOS_destroyVirtualChannel(const ASAAC_PublicId vc_id);
ASAAC_ReturnStatus ASAAC_SMOS_attachChannelToProcessOrThread(const ASAAC_VcMappingDescription* vc_mapping);
ASAAC_ReturnStatus ASAAC_SMOS_detachAllThreadsOfProcessFromVc(const ASAAC_PublicId vc_id, const ASAAC_PublicId process_id);
ASAAC_ReturnStatus ASAAC_SMOS_attachTransferConnectionToVirtualChannel(const ASAAC_VcToTcMappingDescription* vc_to_tc_mapping);
ASAAC_ReturnStatus ASAAC_SMOS_detachTransferConnectionFromVirtualChannel(const ASAAC_PublicId vc_id, const ASAAC_PublicId tc_id);
ASAAC_ReturnStatus ASAAC_SMOS_configureInterface(const ASAAC_InterfaceData* if_config);
ASAAC_ReturnStatus ASAAC_SMOS_createTransferConnection(const ASAAC_TcDescription* tc_desc);
ASAAC_ReturnStatus ASAAC_SMOS_destroyTransferConnection(const ASAAC_PublicId tc_id, const ASAAC_NetworkDescriptor* network_descr);
ASAAC_ReturnStatus ASAAC_SMOS_getNetworkPortStatus(const ASAAC_NetworkDescriptor* network_desc, ASAAC_NetworkPortStatus* network_status);
ASAAC_TimedReturnStatus ASAAC_SMOS_getPMData(ASAAC_PublicId* vc_id, ASAAC_Address* message_buffer_reference, const unsigned long max_msg_length, unsigned long* msg_length, const ASAAC_TimeInterval* timeout);
ASAAC_ReturnStatus ASAAC_SMOS_returnPMData(const ASAAC_PublicId vc_id, const ASAAC_Address message_buffer_reference, const unsigned long msg_length, const ASAAC_ReturnStatus sm_return_status);
ASAAC_TimedReturnStatus ASAAC_SMOS_getAuditData(ASAAC_BreachType* breach_type, ASAAC_CharacterSequence* audit_message, ASAAC_Time* rel_time, ASAAC_Time* abs_time, const ASAAC_TimeInterval* time_out);
ASAAC_ReturnStatus ASAAC_SMOS_erasePhysicalMemory();
ASAAC_ReturnStatus ASAAC_SMOS_getPbitResult(ASAAC_PbitResult* pbit_result);
ASAAC_ReturnStatus ASAAC_SMOS_startCbit(const unsigned long test_code, const ASAAC_CbitModeType mode, ASAAC_BitTestStatus* bit_test_status);
ASAAC_ReturnStatus ASAAC_SMOS_getCbitResult(ASAAC_CbitResult* cbit_result);
ASAAC_ReturnStatus ASAAC_SMOS_startIbit();
ASAAC_ReturnStatus ASAAC_SMOS_getIbitResult(ASAAC_IbitResult* ibit_result);
ASAAC_ReturnStatus ASAAC_SMOS_getMyCfmStatus(ASAAC_CfmStatus* cfm_status);
ASAAC_ReturnStatus ASAAC_SMOS_getMyCfmInfo(ASAAC_CfmInfo* cfm_info);
ASAAC_ReturnStatus ASAAC_SMOS_getMyPeId(ASAAC_PublicId* my_pe_id);
ASAAC_TimedReturnStatus ASAAC_SMOS_requestDownloadToCfm(const ASAAC_CfmDescription* remoteCfm);
ASAAC_TimedReturnStatus ASAAC_SMOS_getRemoteInfo(const ASAAC_RemoteServiceId serviceid, const ASAAC_CfmMliChannel* mliChannel, const ASAAC_InputLocalParameters* input_buffer, const unsigned long input_length, ASAAC_OutputRemoteParameters* output_buffer, const unsigned long output_max_length, unsigned long* output_actual_length);
ASAAC_ReturnStatus ASAAC_SMOS_configureClock(const ASAAC_ClockInfo* clock_info);
ASAAC_ReturnStatus ASAAC_SMOS_attachFederatedClock(const ASAAC_FederatedClockInfo* federated_clock_info);
ASAAC_ResourceReturnStatus ASAAC_SMOS_readLog(const ASAAC_LogMessageType message_type, const unsigned long log_id, ASAAC_CharacterSequence* log_message);
ASAAC_ResourceReturnStatus ASAAC_SMOS_writeLog(const ASAAC_LogMessageType message_type, const unsigned long log_id, const ASAAC_CharacterSequence* log_message);
ASAAC_TimedReturnStatus ASAAC_SMOS_getLogReport(ASAAC_CharacterSequence* log_message, ASAAC_LogMessageType* message_type, ASAAC_PublicId* process_id, const ASAAC_TimeInterval* timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
