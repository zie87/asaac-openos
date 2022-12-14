#ifndef OPENOS_HH_
#define OPENOS_HH_


#define ASAAC_ENTITY 		void registerThreads() { registerBufferedThreads(); }
#define ASAAC_APPLICATION 	int main( int argc, char** argv ){	return applicationMain(); } void registerThreads() { registerBufferedThreads(); }
#define ASAAC_THREAD(x) 	void *x( void *Data ); bool registered##x = bufferThread(#x, x); void * x (void* Data)


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

extern "C" int applicationMain();
extern "C" void registerThreads();
extern "C" char bufferThread(char * name, EntryPointAddr address);
extern "C" void registerBufferedThreads();


#endif /*OPENOS_HH_*/
