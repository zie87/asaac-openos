#ifndef ASAACCONSTANTS_HH_
#define ASAACCONSTANTS_HH_

#define OS_MAGIC_NUMBER						 	 0x0A5AAC05

#define OS_UNLIMITED                     		 0xffffffff

#define OS_UNUSED_ID                     		 0xffffffff
#define OS_MAX_ID                       		(OS_UNUSED_ID-1)

#define OS_PROCESSID_MAX				 		 OS_MAX_ID
#define OS_PROCESSID_MIN				 		 0

#define OS_PROCESSID_SMOS_MAX            		 OS_PROCESSID_MAX
#define OS_PROCESSID_SMOS            			(OS_PROCESSID_MAX-1000)
#define OS_PROCESSID_APOS_MAX           		(OS_PROCESSID_MAX-1001)
#define OS_PROCESSID_APOS                		 OS_PROCESSID_MIN

#define OS_PROCESSID_MASTER             		(OS_PROCESSID_SMOS_MAX-0)
#define OS_PROCESSID_GSM                		(OS_PROCESSID_SMOS_MAX-1)
#define OS_PROCESSID_OLI                		(OS_PROCESSID_SMOS_MAX-2)
#define OS_PROCESSID_PCS                		(OS_PROCESSID_SMOS_MAX-3)
#define OS_PROCESSID_SM                 		(OS_PROCESSID_SMOS_MAX-4)

#define OS_OLI_GLOBAL_VC_REQUEST 				(OS_MAX_ID-0)
#define OS_OLI_GLOBAL_VC_REPLY 		 			(OS_MAX_ID-1)
#define OS_PCS_GLOBAL_VC_REQUEST 				(OS_MAX_ID-2)
#define OS_PCS_GLOBAL_VC_REPLY 		    		(OS_MAX_ID-3)
#define OS_SM_GLOBAL_VC_REQUEST					(OS_MAX_ID-4)
#define OS_SM_GLOBAL_VC_REPLY					(OS_MAX_ID-5)

#define OS_OLI_CLIENT_VC_REQUEST 				(OS_MAX_ID-0)
#define OS_OLI_CLIENT_VC_REPLY 		 			(OS_MAX_ID-1)
#define OS_OLI_SERVER_VC_REQUEST 				(OS_MAX_ID-2)
#define OS_OLI_SERVER_VC_REPLY 		    		(OS_MAX_ID-3)

#define OS_PCS_CLIENT_VC_REQUEST 				(OS_MAX_ID-4)
#define OS_PCS_CLIENT_VC_REPLY 		    		(OS_MAX_ID-5)
#define OS_PCS_SERVER_VC_REQUEST 				(OS_MAX_ID-6)
#define OS_PCS_SERVER_VC_REPLY 		    		(OS_MAX_ID-7)

#define OS_SM_CLIENT_VC_SEND 		 			(OS_MAX_ID-8)
#define OS_SM_CLIENT_VC_RECEIVE 				(OS_MAX_ID-9)
#define OS_SM_SERVER_VC_SEND 		    		(OS_MAX_ID-10)
#define OS_SM_SERVER_VC_RECEIVE 				(OS_MAX_ID-11)


/* Object Pool Sizes */
#define OS_MAX_NUMBER_OF_SEMAPHORES		 		 64
#define OS_MAX_NUMBER_OF_SPINLOCKS		 		 1
#define OS_MAX_NUMBER_OF_EVENTS			 		 32
#define OS_MAX_NUMBER_OF_EVENTTABLES	 		 2
#define OS_MAX_NUMBER_OF_TRIGGERS		 		 10

#define OS_SCHEDULING_INFO_SIZE			 		 sizeof(sched_param)

#define OS_SIZE_OF_SIMPLE_COMMANDBUFFER	 		 4096
#define OS_MAX_NUMBER_OF_COMMAND_HANDLERS		 32

/* Values for Threads, Processes and Vcs */
#define OS_POLICY								 SCHED_RR
#define OS_MAX_NUMBER_OF_SIGNALS 				 64
#define OS_MAX_NUMBER_OF_PRECONFIGURED_PROCESSES 4
#define OS_MAX_NUMBER_OF_PRECONFIGURED_THREADS	 4
#define OS_MAX_NUMBER_OF_PROCESSES		 		 32
#define OS_MAX_NUMBER_OF_THREADS		 		 16
#define OS_MAX_NUMBER_OF_GLOBALVCS		 		 32
#define OS_MAX_NUMBER_OF_LOCALVCS		 		 32
#define OS_MAX_NUMBER_OF_PROTECTEDSCOPES 		 16
#define OS_MAX_NUMBER_OF_CPU			 		 16
#define OS_MAX_NUMBER_OF_EXCEPTIONS		 		 16
#define OS_MAX_NUMBER_OF_ALLOCATOR		 		 4096

#define OS_MAX_NUMBER_OF_TC_BUFFERS		     	 10

#define OS_SIZE_OF_ERROR_QUEUE		     		 8

#define OS_MAX_NUMBER_OF_VC_THREADS_ATTACHED	 32

#define OS_MAX_NUMBER_OF_LOCAL_FILES			 32
#define OS_MAX_NUMBER_OF_GLOBAL_FILES		    (OS_MAX_NUMBER_OF_PROCESSES * OS_MAX_NUMBER_OF_LOCAL_FILES)

/* WARNING LIMITS */
#define OS_SIZE_OF_STACK_LIMIT					 0x000fffff

/* SIGNALS */
#define OS_SIGNAL_KILL				 		     SIGRTMIN
#define OS_SIGNAL_SUSPEND					    (SIGRTMIN+1)
#define OS_SIGNAL_RESUME			    		(SIGRTMIN+2)

#define OS_ERROR_QUEUE					 		 CharacterSequence("/errorq").asaac_str()
#define OS_LOGGING_QUEUE				 		 CharacterSequence("/loggingq").asaac_str()
#define OS_TEMP							 		 CharacterSequence("Temp/").asaac_str()

/* TIMEOUTS */
#define OS_SIMPLE_COMMAND_TIMEOUT   	 		 TimeInterval( 1000, MilliSeconds ).asaac_Interval() 
#define OS_COMPLEX_COMMAND_TIMEOUT  	 		 TimeInterval(    2, Seconds ).asaac_Interval() 

#define OS_TIME_INFINITY_SECONDS				 2147483647
#define OS_TIME_INFINITY_NANOSECONDS			 999999999

/* ENVIRONMENT */
#define OS_ENV_PRIVATEID_COUNTER				 "OS_PRIVATEID_COUNTER"

#define OS_ENV_ASAACHANDLE						 "OS_ASAACHANDLE_"
#define OS_ENV_ASAACHANDLELIST					 "OS_ASAACHANDLELIST"

#define OS_ENV_ID_CPU				 			 "OS_ID_CPU"
#define OS_ENV_ID_PROCESS				 		 "OS_ID_PROCESS"

#endif /*ASAACCONSTANTS_HH_*/
