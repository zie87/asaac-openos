#ifndef OPENOS_H_
#define OPENOS_H_

#include "ASAAC.h"


#define ASAAC_ENTITY 		
#define ASAAC_APPLICATION 	int main( int argc, char** argv ){	return applicationMain(); } 
#define ASAAC_THREAD(x) 	registerThread(#x, x)

// In plain C environments an ASAAC application developer has to use the function 
// OpenOS_registerThread(char *, EntryPointAddr) by himself.
// Instead he can use the makro registerThread(x) where x is the name of the function.
// This is possible by implementing a function registerThreads().
// This function will be the first called function of the application.
//
// Example: x shall be probably the function 'MainThread':
//
//   void registerThreads()
//   {
//       ASAAC_THREAD(MainThread);
//       //register further threads here
//   }


typedef void*(*EntryPointAddr)( void* );

extern int applicationMain();
extern void registerThreads();
extern ASAAC_ReturnStatus registerThread(char * name, EntryPointAddr address);


#endif /*OPENOS_H_*/
