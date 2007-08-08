#ifndef PCSCONSTANTS_HH_
#define PCSCONSTANTS_HH_


#define PCS_MAX_NUMBER_OF_VCS                                   8
#define PCS_MAX_NUMBER_OF_TCS                                   4

#define PCS_MAX_SIZE_OF_TCMESSAGE 							   (8*1024) 
#define PCS_MAX_SIZE_OF_NWMESSAGE 							   (PCS_MAX_SIZE_OF_TCMESSAGE + 8) 
//including header information

#define PCS_MAX_SIZE_OF_MESSAGEQUEUE                            16
#define PCS_MAX_SIZE_OF_PMMESSAGEQUEUE                          128

#define PCS_MAX_NUMBER_OF_NETWORKS                              16
#define PCS_MAX_NUMBER_OF_MAPPINGS                              16


#endif /*PCSCONSTANTS_HH_*/
