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
	LAS_UNDEFINED,
	LAS_ENTITY,
	LAS_PROCESS_INIT,
	LAS_PROCESS_RUNNING,
	LAS_REMOTE
} LocalActivityState;

typedef enum {
	PROC_UNDEFINED,
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
