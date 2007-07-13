#ifndef OPENOSAPPLICATION_HH_
#define OPENOSAPPLICATION_HH_

typedef void*(*EntryPointAddr)( void* );

extern "C" int applicationMain( void );
extern "C" void registerThreads();
extern "C" void registerThread(char * name, EntryPointAddr address);
extern "C" char bufferThread(char * name, EntryPointAddr address);
extern "C" void registerBufferedThreads();

#endif /*OPENOSAPPLICATION_HH_*/
