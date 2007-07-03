#ifndef PCSCONSTANTS_HH_
#define PCSCONSTANTS_HH_

#include "NIIConstants.h"


#define PCS_MAX_NUMBER_OF_VCS                                   8
#define PCS_MAX_NUMBER_OF_TCS                                   4

#define PCS_MAX_SIZE_OF_TCMESSAGE 							    NII_MAX_SIZE_OF_TCMESSAGE 
#define PCS_MAX_SIZE_OF_NWMESSAGE 							   (PCS_MAX_SIZE_OF_TCMESSAGE + 8) 
//including header information

#define PCS_MAX_SIZE_OF_MESSAGEQUEUE                            16
#define PCS_MAX_SIZE_OF_PMMESSAGEQUEUE                          128

#define PCS_MAX_NUMBER_OF_NETWORKS                              NII_MAX_NUMBER_OF_NETWORKS
#define PCS_MAX_NUMBER_OF_MAPPINGS                              16


#endif /*PCSCONSTANTS_HH_*/
