#ifndef EXCEPTIONS_HH_
#define EXCEPTIONS_HH_


#define LOCATION __PRETTY_FUNCTION__,__LINE__

#include "ASAAC_Exception.hh"


//ASAAC Exception
#include "APOSClientException.hh"
#include "ApplicationException.hh"
#include "FatalException.hh"
#include "HWFailureException.hh"
#include "HWResourceException.hh"
#include "OSException.hh"
#include "ProcessorException.hh"
#include "ResourceException.hh"
#include "SMBPException.hh"
#include "SMOSException.hh"

//Specialized OS Exception
#include "DoubleInitializationException.hh"
#include "TimeoutException.hh"
#include "UninitializedObjectException.hh"

#endif /*EXCEPTIONS_HH_*/
