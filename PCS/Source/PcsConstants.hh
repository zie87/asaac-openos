#ifndef PCSCONSTANTS_HH_
#define PCSCONSTANTS_HH_

#include "OpenOS.hh"

const unsigned long PCS_NUMBER_OF_VCS = 8;

//COMMENT.SBS> CAREFULLY SET: NUMBER_OF_TCS X MAXIMUM_MESSAGE_LENGTH X PCS_MAXIMUM_QUEUED_MESSAGES
/// This will be the queueing buffer size in the TcRateLimiter component of PCS

const unsigned long PCS_NUMBER_OF_TCS = 4;
const unsigned long PCS_MAXIMUM_MESSAGE_LENGTH = TC_MAX_MESSAGE_SIZE  + 8; //including header information
const unsigned long PCS_MAXIMUM_QUEUED_MESSAGES = 16;

const unsigned long PCS_MAXIMUM_QUEUED_PM_MESSAGES = 128;


const unsigned long PCS_NUMBER_OF_NETWORKS = 4;

const unsigned long PCS_NUMBER_OF_MAPPINGS = 16;




#endif /*PCSCONSTANTS_HH_*/
