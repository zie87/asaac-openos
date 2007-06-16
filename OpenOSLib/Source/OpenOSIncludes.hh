#ifndef OPENOSINCLUDES_HH_
#define OPENOSINCLUDES_HH_

#include "OAL/oal.h"
#include "ASAAC.h"
#include "OpenOSConstants.hh"

#include "Common/Asaac/CharacterSequence.hh"
#include "Common/Asaac/TimeInterval.hh"
#include "Common/Asaac/TimeStamp.hh"
#include "Common/Aid.h"

typedef void* Address;

typedef enum {
	OS_Undefined,
	OS_Entity,
	OS_Starter,
	OS_Process,
	OS_Remote
} OpenOSContext;

typedef enum {
	PROC_Undefined,
	PROC_APOS,
	PROC_SMOS,
	PROC_GSM,
	PROC_PCS,
	PROC_OLI,
	PROC_SM
} ProcessAlias;

typedef unsigned long SessionId;

class OpenOS;

#endif /*OPENOSINCLUDES_HH_*/
