#ifndef OPENOSAPPLICATION_HH_
#define OPENOSAPPLICATION_HH_

typedef void*(*EntryPointAddr)( void* );

extern "C" int applicationMain( void );
extern "C" void registerThreads();
extern "C" char registerThread(char * name, EntryPointAddr address);

#endif /*OPENOSAPPLICATION_HH_*/
