#ifndef TYPES_HH_
#define TYPES_HH_

#include "OpenOSIncludes.hh"


typedef struct {
	ASAAC_ThreadDescription	List[OS_MAX_NUMBER_OF_THREADS];
	unsigned long				Count;		
} ThreadConfigurationList;


typedef struct {
	ProcessAlias				Alias;
	ASAAC_ProcessDescription 	Process;
	ThreadConfigurationList 	Threads;
} ProcessConfiguration;


typedef struct {
	ProcessConfiguration 		List[OS_MAX_NUMBER_OF_PROCESSES];
	unsigned long				Count;		
} ProcessConfigurationList;


typedef struct {
	bool 						Flush;
	ASAAC_PublicId 				CpuId;
	ProcessConfigurationList	Processes;
} EntityConfiguration;


#endif /*TYPES_HH_*/
