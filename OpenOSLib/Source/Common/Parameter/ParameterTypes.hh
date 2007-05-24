#ifndef TYPES_HH_
#define TYPES_HH_

#include "OpenOSIncludes.hh"

typedef struct {
    ASAAC_ThreadDescription    Description;    
} ThreadDescription;

typedef struct {
	ThreadDescription	        List[OS_MAX_NUMBER_OF_THREADS];
	unsigned long				Count;		
} ThreadList;


typedef struct {
	ProcessAlias				Alias;
	ASAAC_ProcessDescription 	Description;
	ThreadList     	            ThreadConfiguration;
} ProcessDescription;


typedef struct {
	ProcessDescription  		List[OS_MAX_NUMBER_OF_PROCESSES];
	unsigned long				Count;		
} ProcessList;


typedef struct {
	bool 						Flush;
	ASAAC_PublicId 				CpuId;
    ASAAC_CharacterSequence     ConfigurationFile;    
	ProcessList       	        ProcessConfiguration;
} EntityConfiguration;


#endif /*TYPES_HH_*/
