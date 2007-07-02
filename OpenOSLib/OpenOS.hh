#ifndef OPENOS_HH_
#define OPENOS_HH_

#include "ASAAC.h"


#define ASAAC_ENTITY 		void registerThreads() {}
#define ASAAC_APPLICATION 	int main( int argc, char** argv ){	return applicationMain(); } void registerThreads() {}
#define ASAAC_THREAD(x) 	void *x( void *Data ); bool registered##x = registerThread(#x, x); void * x (void* Data)


// The Makro 'ASAAC_THREAD(x)' easily installs ASAAC-Thread EntryPoints
// by using the function OpenOS_registerThread(char *, EntryPointAddr)
//
// Hint: 	It can only be used in C++ environments
//
// Example: x shall be probably the function 'MainThread':
//
//   ASAAC_THREAD(MainThread)
//   {
//       //Put here the code of your thread
//   }
//
// The Makro will produce the following:
//
//   void *MainThread( void *Data );  <---- Predefined function
//   ASAAC_ReturnStatus OpenOS_registeredMainThread = OpenOS_registerThread("MainTheread", MainThread); <---- Registering the function as a thread entry point
//
//   void *MainThread (void *Data)  <---- Implementation of the function
//   {
//       //Put here the code of your thread
//   }


typedef void*(*EntryPointAddr)( void* );

extern int applicationMain();
extern void registerThreads();
extern bool registerThread(char * name, EntryPointAddr address);


#endif /*OPENOS_HH_*/
