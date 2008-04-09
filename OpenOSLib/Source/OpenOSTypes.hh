#ifndef OPENOSTYPES_HH_
#define OPENOSTYPES_HH_

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

typedef unsigned long RevisionId;
typedef unsigned long SessionId;

class OpenOS;

#endif /*OPENOSTYPES_HH_*/
