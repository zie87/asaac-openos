#ifndef ENTRYPOINT_HH_
#define ENTRYPOINT_HH_

#include "OpenOSIncludes.hh"

typedef void*(*EntryPointAddr)( void* );

struct EntryPoint {

	ASAAC_CharacterSequence		Name;
	EntryPointAddr				Address;
	
};

#endif /*ENTRYPOINT_HH_*/
