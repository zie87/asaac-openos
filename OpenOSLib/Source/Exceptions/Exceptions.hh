#ifndef EXCEPTIONS_HH_
#define EXCEPTIONS_HH_



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

#define NO_EXCEPTION(x) try{ x; } catch(ASAAC_Exception &e){ /* do nothing */ }
#define LOCATION __PRETTY_FUNCTION__,__LINE__

#endif /*EXCEPTIONS_HH_*/
