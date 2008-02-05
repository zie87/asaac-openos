#ifndef OPENOS_H_
#define OPENOS_H_


#define ASAAC_ENTITY 		
#define ASAAC_APPLICATION 	int main( int argc, char** argv ){	return applicationMain(); } 
#define ASAAC_THREAD(x) 	registerThread(#x, x)

// In plain C environments an ASAAC application developer has to use the function 
// registerThread(char *, EntryPointAddr) by himself.
// Instead he can use the makro ASAAC_THREAD(x) where x is the name of the function.
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

#ifdef __cplusplus
extern "C" int applicationMain();
extern "C" void registerThreads();
extern "C" void registerThread(char * name, EntryPointAddr address);
#else
extern int applicationMain();
extern void registerThreads();
extern void registerThread(char * name, EntryPointAddr address);
#endif

#endif /*OPENOS_H_*/
