#include "ConfigurationParser.hh"

#include "OpenOSObject.hh"
#include "Managers/FileManager.hh"
#include "Managers/FileNameGenerator.hh"

const unsigned long Unlimited = 0xffffffff;


ConfigurationParser::ConfigurationParser()
{
}


ConfigurationParser::~ConfigurationParser()
{
}


void ConfigurationParser::loadFile( ASAAC_CharacterSequence FileName )
{	
	try
	{
	    FileName = FileNameGenerator::getAsaacPath(FileName);
	    
	    ASAAC_UseOption UseOption;
	    UseOption.use_access = ASAAC_READ;
	    UseOption.use_concur = ASAAC_SHARE;
	
	    ASAAC_PrivateId Handle;
	    FileManager::getInstance()->openFile( FileName, UseOption, Handle );
	    
	    long Read = 0;
	    ASAAC_CharacterSequence Buffer;
	    FileManager::getInstance()->readFile( Handle, Buffer.data, ASAAC_OS_MAX_STRING_SIZE, Read, TimeIntervalInfinity );
	    
	    Buffer.size = Read;
	    
	    setString( Buffer );
	}
	catch ( ASAAC_Exception &e )
	{
		CharacterSequence ErrorString;
		
		e.addPath( (ErrorString << "Error loading file: " << CharSeq(FileName)).c_str(), LOCATION);
		
		throw;
	}
}


void ConfigurationParser::parse( ASAAC_CharacterSequence FileName )
{
    loadFile( FileName );

	CharacterSequence ErrorString;
    
    try
    {
	    unsigned long Index = 0;
	
	    parseSample(Index, createSample(space), Unlimited);
	    
	    for (unsigned short p = 0; p < OS_MAX_NUMBER_OF_PRECONFIGURED_PROCESSES; p++)
	    {
	        if (parseString(Index, "process", 1) == 1)
	        {
	            m_Configuration.Count = p+1;
	            
	            parseSample(Index, createSample(space), Unlimited);
	
	            if (parseString(Index, "(", 1) < 1)
					throw OSException( (ErrorString << "'(' was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	                
	            parseSample(Index, createSample(space), Unlimited);
	            
	            if (parseString(Index, "global_pid:", 1) == 1)
	            {
	                if (parseString(Index, "$", 1) == 1)
	                {
	                    m_Configuration.List[p].Alias = PROC_UNDEFINED;
	                    m_Configuration.List[p].Description.global_pid = OS_UNUSED_ID;
	                    
	                    if (parseString(Index, "APOS", 1) == 1)
	                        m_Configuration.List[p].Alias = PROC_APOS;
	                    if (parseString(Index, "SMOS", 1) == 1)
	                        m_Configuration.List[p].Alias = PROC_SMOS;
	                    if (parseString(Index, "GSM", 1) == 1)
	                        m_Configuration.List[p].Alias = PROC_GSM;
	                    if (parseString(Index, "PCS", 1) == 1)
	                        m_Configuration.List[p].Alias = PROC_PCS;
	                    if (parseString(Index, "OLI", 1) == 1)
	                        m_Configuration.List[p].Alias = PROC_OLI;
	                    if (parseString(Index, "SM", 1) == 1)
	                        m_Configuration.List[p].Alias = PROC_SM;
	                        
	                    if (m_Configuration.List[p].Alias == PROC_UNDEFINED)
	                        throw OSException( (ErrorString << "'APOS', 'SMOS', 'GSM', 'PCS', 'OLI' or 'SM'  was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	                }
	                else 
	                {
	                    m_Configuration.List[p].Description.global_pid = parsePublicId(Index);
	                    m_Configuration.List[p].Alias = PROC_UNDEFINED;
	                }
	            }
	            else throw OSException( (ErrorString << "'$'  was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	
	            if (parseSample(Index, createSample(space), Unlimited) < 1)
	                throw OSException( (ErrorString << "Space was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	    
	            if (parseString(Index, "programme_file_name:", 1) == 1)
	            {
	                m_Configuration.List[p].Description.programme_file_name = parsePath(Index);
	            }
	            else throw OSException( (ErrorString << "'programme_file_name:'  was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	
	            if (parseSample(Index, createSample(space), Unlimited) < 1)
	                throw OSException( (ErrorString << "Space was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	    
	            if (parseString(Index, "cpu_id:", 1) == 1)
	            {
	                m_Configuration.List[p].Description.cpu_id = parsePublicId(Index);
	            }
	    
	            for (unsigned long t = 0; t < OS_MAX_NUMBER_OF_PRECONFIGURED_THREADS; t++)
	            {
	                parseSample(Index, createSample(space), Unlimited);
	
	                if (parseString(Index, "thread", 1) == 1)
	                {
	                    m_Configuration.List[p].ThreadConfiguration.Count = t+1;                          
	
	                    parseSample(Index, createSample(space), Unlimited);
	
	                    if (parseString(Index, "(", 1) < 1)
	                        throw OSException( (ErrorString << "'(' was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	
	                    parseSample(Index, createSample(space), Unlimited);
	
	                    if (parseString(Index, "thread_id:", Unlimited) == 1)
	                    {
	                        m_Configuration.List[p].ThreadConfiguration.List[t].Description.thread_id = parsePublicId(Index);                         
	                    }
	                    else throw OSException( (ErrorString << "'thread_id:' was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	
	                    if (parseSample(Index, createSample(space), Unlimited) < 1)
	                        throw OSException( (ErrorString << "Space was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	
	                    if (parseString(Index, "entry_point:", Unlimited) == 1)
	                    {
	                        m_Configuration.List[p].ThreadConfiguration.List[t].Description.entry_point = parseName(Index);                           
	                    }
	                    else throw OSException( (ErrorString << "'entry_point:' was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	
	                    parseSample(Index, createSample(space), Unlimited);
	
	                    if (parseString(Index, ")", 1) != 1)
	                        throw OSException( (ErrorString << "')' was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	                }
	                else break; 
	            }
	            
	            parseSample(Index, createSample(space), Unlimited);
	                
	            if (parseString(Index, ")", 1) != 1)
	                throw OSException( (ErrorString << "')' was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
	        }
	        else break;
	
	        if (parseSample(Index, createSample(space), Unlimited) < 1)
	            break;
	    }
	        
	    parseSample(Index, createSample(space), Unlimited);
	
	    if (Index != getString().size)
			throw OSException( (ErrorString << "Unexpected character: '" << m_Sequence.c_str(Index, 10) << "...'").c_str(), LOCATION);
    }
    catch ( ASAAC_Exception &e )
    {
    	e.addPath( "Syntax error in configuration file", LOCATION );
    	
    	throw;
    }
}


void ConfigurationParser::setConfiguration( ProcessList Configuration )
{
    m_Configuration = Configuration;
}


ProcessList ConfigurationParser::getConfiguration( )
{
    return m_Configuration;
} 
