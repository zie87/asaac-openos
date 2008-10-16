#ifndef ENTRYPOINT_HH_
#define ENTRYPOINT_HH_

#include "OpenOSIncludes.hh"

typedef void*(*EntryPointAddr)( void* );

typedef struct {
	ASAAC_CharacterSequence		Name;
	EntryPointAddr				Address;	
} EntryPoint;

#endif /*ENTRYPOINT_HH_*/
