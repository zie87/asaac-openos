#include "OpenOS.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Communication/CommunicationManager.hh"
#include "Wrapper/SMOSWrapper.hh"

#include "Types.hh"
#include "ParameterParser.hh"


ASAAC_ENTITY


#define UNDEFINED_PATH CharSeq("<undefined>").asaac_str()

static EntityConfiguration entityConfiguration;


void initializeConfiguration();		
void parseParameter( char argc, char** argv );
void printHeader(EntityConfiguration conf);


ASAAC_THREAD(MainThread)
{
	for (unsigned long p = 0; p < entityConfiguration.Processes.Count; p++)
	{
		entityConfiguration.Processes.List[p].Process.global_pid = 
			ProcessManager::getInstance()->getProcessId( entityConfiguration.Processes.List[p].Alias ); 
		
		ASAAC_SMOS_createProcess( &(entityConfiguration.Processes.List[p].Process) );
		
		for (unsigned long t = 0; t < entityConfiguration.Processes.List[p].Threads.Count; t++)
		{
			entityConfiguration.Processes.List[p].Threads.List[t].global_pid = 
				entityConfiguration.Processes.List[p].Process.global_pid;
				 
			ASAAC_SMOS_createThread( &entityConfiguration.Processes.List[p].Threads.List[t] );
		}

        if (entityConfiguration.Processes.List[p].Alias == PROC_PCS)
        {
            CommunicationManager::getInstance()->configurePCS();            
        }
	}
    
    for (unsigned long p = 0; p < entityConfiguration.Processes.Count; p++)
    {
        ASAAC_SMOS_runProcess( entityConfiguration.Processes.List[p].Process.global_pid );
    }
	
	ASAAC_APOS_suspendSelf();
	
    return 0;
}


int main( char argc, char** argv )
{
	// 1st step: reading parameter
	try
	{
		initializeConfiguration();		
		parseParameter( argc, argv );
		printHeader(entityConfiguration);		
	}
    catch ( ASAAC_Exception& e )
    {
    	e.addPath("syntax error in parameter list", LOCATION);
    	e.logMessage();
    	return 1;
    }
    catch (...)
    {
    	OSException("critical error in parameter list", LOCATION).logMessage();
    	return 2;
    }
	
	// 2nd step: start execution	
    try
    {    	
        // Initialize OS
        OpenOS::getInstance()->initializeEntity(entityConfiguration.CpuId, entityConfiguration.Flush);

		// Create main thread
		createThread("MainThread", ProcessManager::getInstance()->getCurrentProcess()->getId(), 1);
	    
	    // Enter main cycle of Master-Process
	    ProcessManager::getInstance()->getCurrentProcess()->run();   

		// Deinitialize OS	    
        OpenOS::getInstance()->deinitialize();
    }
    catch ( ASAAC_Exception &e )
    {
        cerr << "Caught exception in main loop of OpenOS Entity: " << e.getMessage() << endl;
    }
    catch (...)
    {
    	cerr << "Caught critical exception in main loop of OpenOS Entity." << endl;
    }
    
    // 3rd step: shut down
    cout << "OpenOS Entity shuts down now..." << endl;

    return 0;
}


void initializeConfiguration()
{
	entityConfiguration.Flush = false;
	entityConfiguration.CpuId = OS_UNUSED_ID;
	
	entityConfiguration.Processes.Count = 0;
		
	for (unsigned short p = 0; p < OS_MAX_NUMBER_OF_PROCESSES; p++)
	{
		entityConfiguration.Processes.List[p].Alias = PROC_Undefined;
		entityConfiguration.Processes.List[p].Process.global_pid = OS_UNUSED_ID;
		entityConfiguration.Processes.List[p].Process.programme_file_name = CharSeq("").asaac_str();
		entityConfiguration.Processes.List[p].Process.programme_file_Size = 0;
		entityConfiguration.Processes.List[p].Process.access_type = ASAAC_LOCAL_ACCESS;
		entityConfiguration.Processes.List[p].Process.cpu_id = 0;
		
		for (unsigned short s = 0; s < ASAAC_MAX_NUMBER_OF_APOS_SERVICES; s++)
			entityConfiguration.Processes.List[p].Process.apos_services[s] = ASAAC_BOOL_TRUE;
			
		entityConfiguration.Processes.List[p].Process.timeout = TimeInterval(1, Seconds).asaac_Interval();
		
		entityConfiguration.Processes.List[p].Threads.Count = 0;
		
		for (unsigned short t = 0; t < OS_MAX_NUMBER_OF_PROCESSES; t++)
		{
			entityConfiguration.Processes.List[p].Threads.List[t].global_pid = OS_UNUSED_ID;
			entityConfiguration.Processes.List[p].Threads.List[t].thread_id = OS_UNUSED_ID;
			entityConfiguration.Processes.List[p].Threads.List[t].entry_point = CharSeq("").asaac_str();
			entityConfiguration.Processes.List[p].Threads.List[t].cpu_id = 0;
			entityConfiguration.Processes.List[p].Threads.List[t].stack_size = 65536;
			entityConfiguration.Processes.List[p].Threads.List[t].security_rating.classification_level = ASAAC_UNCLASSIFIED;
			entityConfiguration.Processes.List[p].Threads.List[t].security_rating.security_category = ASAAC_LEVEL_1;
		}
	}
		
}	

	
void parseParameter( char argc, char** argv )
{
	CharacterSequence Parameter;
	
	for (int Index = 1; Index < argc; Index++)
		Parameter << argv[Index] << "  ";

    Parameter = "-f -cpu_id:0 -process(global_pid:$GSM programme_file_name:GSM-Application/Debug/GSM-Application cpu_id:0 thread(thread_id:1 entry_point:MainThread) thread(thread_id:2 entry_point:ErrorHandlerThread) thread(thread_id:3 entry_point:LoggingHandlerThread)) -process( global_pid:$PCS  programme_file_name:PCS-Application/Debug/PCS-Application  cpu_id:0 thread(thread_id:1 entry_point:MainThread))"; 
    //Parameter = "-f -cpu_id:0 -process(global_pid:$GSM programme_file_name:GSM-Application/Debug/GSM-Application cpu_id:0 thread(thread_id:1 entry_point:MainThread) thread(thread_id:2 entry_point:ErrorHandlerThread) thread(thread_id:3 entry_point:LoggingHandlerThread)) -process( global_pid:$PCS  programme_file_name:PCS/Debug/PCS  cpu_id:0 thread(thread_id:1 entry_point:MainThread) thread(thread_id:2 entry_point:VcListenThread) thread(thread_id:3 entry_point:TcListenThread) thread(thread_id:4 entry_point:RateLimiterThread))"; 
    //Parameter = "-f -cpu_id:0 -process(global_pid:$GSM programme_file_name:GSM-Application/Debug/GSM-Application cpu_id:0 thread(thread_id:1 entry_point:MainThread) thread(thread_id:2 entry_point:ErrorHandlerThread) thread(thread_id:3 entry_point:LoggingHandlerThread))"; 

	ParameterParser Parser;
	Parser.setConfiguration(entityConfiguration);
	
	Parser.parse( Parameter );
	
	entityConfiguration = Parser.getConfiguration();
}


void printHeader(EntityConfiguration conf)
{
	char * pattern = 
		"                                                                     \n"
		"                 ##                                                  \n"
		"                ####           ESG                                   \n"
		"              ########         Elektroniksystem- und Logistik-GmbH:  \n"
        "             ##########        ------------------------------------  \n"
        "           ######  ######                                            \n"
        "          ######    ######                                           \n"
        "          ####        ####            OpenOS ASAAC Layer:            \n"
		"  #       ###   #  #   ###       #    -------------------            \n"
		"  ##      #   ###  ###   #      ##    Version: 2.0                   \n"
		"  ####       ####  ####       ####    Date:    %s\n"
		"  ######   ######  ######   ######                                   \n"
		"  ##############    ##############                                   \n"
  		"    ###########      ###########                                     \n"
    	"      #######   ####   #######        Starting OpenOS Entity:        \n"
     	"       ####    ######    ####         -----------------------        \n"
       	"         #   ##########   #           Flush:     %s\n"
        "           ##############             CpuId:     %d\n"
		"          ######    ######            Processes: %d\n"
		"          #####      #####                                           \n"
		"          ###          ###                                           \n"
		"          #              #                                           \n"
		"                                                                     \n";
				
	printf(pattern, 
		__DATE__, 
		(conf.Flush == true)?"yes":"no", 
		conf.CpuId, 
		conf.Processes.Count); 	
}

void printProcess()
{
	char * pattern =
		"  -------------------------------------------------------------------\n"
		"  Starting process:       %s\n"
		"  -------------------------------------------------------------------\n"
		"  - programme_file_name:  %s\n"
		"  - cpu_id:               %s\n";
		
	printf(pattern,
		 "global_pid",
		 "programme_file_name",
		 "cpu_id");	
}

void printThread()
{
	char * pattern =
		"  -------------------------------------------------------------------\n"
		"  - thread_id:            %s\n"
		"  -------------------------------------------------------------------\n"
		"    - entry_point:        %s\n";
		
	printf(pattern,
		"thread_id",
		"entry_point");	
}

