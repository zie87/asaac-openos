#include "ConfigurationParser.hh"

#include "OpenOS.hh"
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
    FileName = FileNameGenerator::getAsaacPath(FileName);
    
    ASAAC_UseOption UseOption;
    UseOption.use_access = ASAAC_READ;
    UseOption.use_concur = ASAAC_SHARE;

    ASAAC_PrivateId Handle;
    FileManager::getInstance()->openFile( FileName, UseOption, Handle );
    
    long Size = 1024;
    long Read = 0;
    char Buffer[Size+1];
    FileManager::getInstance()->readFile( Handle, Buffer, Size, Read, TimeIntervalInfinity );
    
    Buffer[Read] = 0;
    
    CharacterSequence Sequence = Buffer;
    setString( Sequence );
}


void ConfigurationParser::parse( ASAAC_CharacterSequence FileName )
{
    loadFile( FileName );
    
    unsigned long Index = 0;

    parseSample(Index, createSample(space), Unlimited);
    
    for (unsigned short p = 0; p < OS_MAX_NUMBER_OF_PROCESSES; p++)
    {
        if (parseString(Index, "process", 1) == 1)
        {
            m_Configuration.Count = p+1;
            
            parseSample(Index, createSample(space), Unlimited);

            if (parseString(Index, "(", 1) < 1)
                throw OSException(LOCATION);
                
            parseSample(Index, createSample(space), Unlimited);
            
            if (parseString(Index, "global_pid:", 1) == 1)
            {
                if (parseString(Index, "$", 1) == 1)
                {
                    m_Configuration.List[p].Alias = PROC_Undefined;
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
                        
                    if (m_Configuration.List[p].Alias == PROC_Undefined)
                        throw OSException(LOCATION);
                }
                else 
                {
                    m_Configuration.List[p].Description.global_pid = parsePublicId(Index);
                    m_Configuration.List[p].Alias = PROC_Undefined;
                }
            }
            else throw OSException(LOCATION);

            if (parseSample(Index, createSample(space), Unlimited) < 1)
                throw OSException(LOCATION);
    
            if (parseString(Index, "programme_file_name:", 1) == 1)
            {
                m_Configuration.List[p].Description.programme_file_name = parsePath(Index);
            }
            else throw OSException(LOCATION);

            if (parseSample(Index, createSample(space), Unlimited) < 1)
                throw OSException(LOCATION);
    
            if (parseString(Index, "cpu_id:", 1) == 1)
            {
                m_Configuration.List[p].Description.cpu_id = parsePublicId(Index);
            }
    
            for (unsigned long t = 0; t < OS_MAX_NUMBER_OF_THREADS; t++)
            {
                parseSample(Index, createSample(space), Unlimited);

                if (parseString(Index, "thread", 1) == 1)
                {
                    m_Configuration.List[p].ThreadConfiguration.Count = t+1;                          

                    parseSample(Index, createSample(space), Unlimited);

                    if (parseString(Index, "(", 1) < 1)
                        throw OSException(LOCATION);

                    parseSample(Index, createSample(space), Unlimited);

                    if (parseString(Index, "thread_id:", Unlimited) == 1)
                    {
                        m_Configuration.List[p].ThreadConfiguration.List[t].Description.thread_id = parsePublicId(Index);                         
                    }
                    else throw OSException(LOCATION);

                    if (parseSample(Index, createSample(space), Unlimited) < 1)
                        throw OSException(LOCATION);

                    if (parseString(Index, "entry_point:", Unlimited) == 1)
                    {
                        m_Configuration.List[p].ThreadConfiguration.List[t].Description.entry_point = parseName(Index);                           
                    }
                    else throw OSException(LOCATION);

                    parseSample(Index, createSample(space), Unlimited);

                    if (parseString(Index, ")", 1) != 1)
                        throw OSException(LOCATION);
                }
                else break; 
            }
            
            parseSample(Index, createSample(space), Unlimited);
                
            if (parseString(Index, ")", 1) != 1)
                throw OSException(LOCATION);
        }
        else break;

        if (parseSample(Index, createSample(space), Unlimited) < 1)
            break;
    }
        
    parseSample(Index, createSample(space), Unlimited);

    if (Index != getString().length())
        throw OSException("Unexpected Characters", LOCATION);
}


void ConfigurationParser::setConfiguration( ProcessList Configuration )
{
    m_Configuration = Configuration;
}


ProcessList ConfigurationParser::getConfiguration( )
{
    return m_Configuration;
} 
