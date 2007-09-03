#ifndef PCSCONSTANTS_HH_
#define PCSCONSTANTS_HH_


#define PCS_CALLBACKID_TC                                       1

#define PCS_MAX_NUMBER_OF_VCS                                   8
#define PCS_MAX_NUMBER_OF_TCS                                   4
#define PCS_MAX_NUMBER_OF_BUFFER                                32

#define PCS_MAX_SIZE_OF_TCMESSAGE 							   (32*1024) 
#define PCS_MAX_SIZE_OF_NWMESSAGE 							   (PCS_MAX_SIZE_OF_TCMESSAGE + 8) 
//including header information (tc number and vc number)

#define PCS_MAX_SIZE_OF_MESSAGEQUEUE                            16
#define PCS_MAX_SIZE_OF_PMMESSAGEQUEUE                          128

#define PCS_MAX_NUMBER_OF_NETWORKS                              16
#define PCS_MAX_NUMBER_OF_MAPPINGS                              16

#define PCS_DEFAULT_RATE_LIMIT									TimeInterval(5, MilliSeconds).asaac_Interval()

#endif /*PCSCONSTANTS_HH_*/
