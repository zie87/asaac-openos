#include <iostream>

#include "OpenOS.hh"
#include "OpenOSObject.hh"

#include "ProcessManagement/ProcessManager.hh"
#include "Communication/CommunicationManager.hh"
#include "Wrapper/SMOSWrapper.hh"

#include "Common/Parameter/ParameterTypes.hh"
#include "Common/Parameter/ParameterParser.hh"
#include "Common/Parameter/ConfigurationParser.hh"


ASAAC_ENTITY


static EntityConfiguration entityConfiguration;

void initializeConfiguration();		
void parseParameter( char argc, char** argv );
void parseConfiguration();
void printHeader(EntityConfiguration &conf);


ASAAC_THREAD(MainThread)
{
	for (unsigned long p = 0; p < entityConfiguration.ProcessConfiguration.Count; p++)
	{
		entityConfiguration.ProcessConfiguration.List[p].Description.global_pid = 
			Process::getId( entityConfiguration.ProcessConfiguration.List[p].Alias ); 
		
		ASAAC_SMOS_createProcess( &(entityConfiguration.ProcessConfiguration.List[p].Description) );
		
		for (unsigned long t = 0; t < entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.Count; t++)
		{
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.global_pid = 
				entityConfiguration.ProcessConfiguration.List[p].Description.global_pid;
				 
			ASAAC_SMOS_createThread( &entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description );
		}

        if (entityConfiguration.ProcessConfiguration.List[p].Alias == PROC_PCS)
        {
            CommunicationManager::getInstance()->configurePCS();            
        }
	}
    
    for (unsigned long p = 0; p < entityConfiguration.ProcessConfiguration.Count; p++)
    {
        ASAAC_SMOS_runProcess( entityConfiguration.ProcessConfiguration.List[p].Description.global_pid );
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
	}
    catch ( ASAAC_Exception& e )
    {
    	e.printMessage();
    	
        cout << "OpenOS Entity shuts down now..." << endl;
        return 1;
    }
	
	// 2nd step: start execution	
    try
    {    	
        // Initialize OS
        OpenOS::getInstance()->initialize( entityConfiguration.Flush, LAS_ENTITY, entityConfiguration.CpuId );

        // Register local threads
        registerThreads();
        
        // Determine entry points
        parseConfiguration();

		// Print configuration
		printHeader(entityConfiguration);		

		// Create main thread
		createThread("MainThread", ProcessManager::getInstance()->getCurrentProcess()->getId(), 1);
	    
	    // Enter main cycle of Master-Process
	    ProcessManager::getInstance()->getCurrentProcess()->run();   

		// Deinitialize OS	    
        OpenOS::getInstance()->deinitialize();
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath( "Caught exception in main loop of OpenOS Entity", LOCATION );
    	e.printMessage();
    	
        cout << "OpenOS Entity shuts down now..." << endl;
        return 2;
    }
    
    // 3rd step: shut down
    cout << "OpenOS Entity shuts down now..." << endl;
    return 0;
}


void initializeConfiguration()
{
	entityConfiguration.Flush = false;
	entityConfiguration.CpuId = OS_UNUSED_ID;
	
	entityConfiguration.ProcessConfiguration.Count = 0;
		
	for (unsigned short p = 0; p < OS_MAX_NUMBER_OF_PRECONFIGURED_PROCESSES; p++)
	{
		entityConfiguration.ProcessConfiguration.List[p].Alias = PROC_UNDEFINED;
		entityConfiguration.ProcessConfiguration.List[p].Description.global_pid = OS_UNUSED_ID;
		entityConfiguration.ProcessConfiguration.List[p].Description.programme_file_name.size = 0;
		entityConfiguration.ProcessConfiguration.List[p].Description.programme_file_Size = 0;
		entityConfiguration.ProcessConfiguration.List[p].Description.access_type = ASAAC_LOCAL_ACCESS;
		entityConfiguration.ProcessConfiguration.List[p].Description.cpu_id = 0;
		
		for (unsigned short s = 0; s < ASAAC_MAX_NUMBER_OF_APOS_SERVICES; s++)
			entityConfiguration.ProcessConfiguration.List[p].Description.apos_services[s] = ASAAC_BOOL_TRUE;
			
		entityConfiguration.ProcessConfiguration.List[p].Description.timeout = TimeInterval(1, Seconds).asaac_Interval();
		
		entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.Count = 0;
		
		for (short t = OS_MAX_NUMBER_OF_PRECONFIGURED_THREADS-1; t >= 0; t--)
		{
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.global_pid = OS_UNUSED_ID;
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.thread_id = OS_UNUSED_ID;
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.entry_point.size = 0;
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.cpu_id = 0;
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.stack_size = 0x000fffff;
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.security_rating.classification_level = ASAAC_UNCLASSIFIED;
			entityConfiguration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.security_rating.security_category = ASAAC_LEVEL_1;
		}
	}
		
}	

	
void parseParameter( char argc, char** argv )
{
	CharacterSequence Parameter;
	
	for (int Index = 1; Index < argc; Index++)
		Parameter << argv[Index] << "  ";
        
	ParameterParser Parser;
	Parser.setConfiguration( entityConfiguration );
	
	Parser.parse( Parameter.asaac_str() );
	
	entityConfiguration = Parser.getConfiguration();
}


void parseConfiguration()
{
    ConfigurationParser Parser;
    Parser.setConfiguration( entityConfiguration.ProcessConfiguration );
    
    Parser.parse( entityConfiguration.ConfigurationFile );
    
    entityConfiguration.ProcessConfiguration = Parser.getConfiguration(); 
}


void printHeader(EntityConfiguration &conf)
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
		"  ##      #   ###  ###   #      ##    Version: Beta 2                \n"
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
		conf.ProcessConfiguration.Count); 	
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

