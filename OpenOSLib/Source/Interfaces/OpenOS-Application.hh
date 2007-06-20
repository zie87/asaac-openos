#ifndef OPENOSAPPLICATION_HH_
#define OPENOSAPPLICATION_HH_

#include "ASAAC.h"



#ifdef __cplusplus
	#define ASAAC_ENTITY 		void registerThreads() {}
	#define ASAAC_APPLICATION 	int main( int argc, char** argv ){	return applicationMain(); } void registerThreads() {}
	#define ASAAC_THREAD(x) 	void *x( void *Data ); ASAAC_ReturnStatus OpenOS_registered##x = OpenOS_registerThread(#x, x); void * x (void* Data)
#else
	#define ASAAC_ENTITY 		
	#define ASAAC_APPLICATION 	int main( int argc, char** argv ){	return applicationMain(); } 
	#define ASAAC_THREAD(x) 	OpenOS_registerThread(#x, x)
#endif



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

 
extern 
#ifdef __cplusplus
"C"
#endif
int applicationMain();

extern 
#ifdef __cplusplus
"C"
#endif
void registerThreads();
 
extern 
#ifdef __cplusplus
"C"
#endif
ASAAC_ReturnStatus OpenOS_registerThread(char * name, EntryPointAddr address);


#endif /*OPENOSAPPLICATION_HH_*/
