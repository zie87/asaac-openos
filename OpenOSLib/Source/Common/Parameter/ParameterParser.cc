#include "ParameterParser.hh"

#include "OpenOS.hh"


const unsigned long Unlimited = 0xffffffff;


ParameterParser::ParameterParser()
{
}


ParameterParser::~ParameterParser()
{
}


void ParameterParser::parse( ASAAC_CharacterSequence Sequence)
{
	setString( Sequence );
	
	unsigned long Index = 0;
	
	parseSample(Index, createSample(space), Unlimited);
	
	if (parseString(Index, "-f", 1) == 1)
		m_Configuration.Flush = true;
		
	if (parseSample(Index, createSample(space), Unlimited) < 1)
		throw OSException(LOCATION);
		
    if (parseString(Index, "-cpu_id=", 1) == 1)
    {
        m_Configuration.CpuId = parsePublicId( Index );
    } 
    
    if (parseSample(Index, createSample(space), Unlimited) < 1)
        throw OSException(LOCATION);
	
	for (unsigned short p = 0; p < OS_MAX_NUMBER_OF_PROCESSES; p++)
	{
		if (parseString(Index, "-process(", 1) == 1)
		{
			m_Configuration.ProcessConfiguration.Count = p+1;
			
			parseSample(Index, createSample(space), Unlimited);
			
			if (parseString(Index, "global_pid:", 1) == 1)
			{
				if (parseString(Index, "$", 1) == 1)
				{
					m_Configuration.ProcessConfiguration.List[p].Alias = PROC_UNDEFINED;
					m_Configuration.ProcessConfiguration.List[p].Description.global_pid = OS_UNUSED_ID;
					
					if (parseString(Index, "APOS", 1) == 1)
						m_Configuration.ProcessConfiguration.List[p].Alias = PROC_APOS;
					if (parseString(Index, "SMOS", 1) == 1)
						m_Configuration.ProcessConfiguration.List[p].Alias = PROC_SMOS;
					if (parseString(Index, "GSM", 1) == 1)
						m_Configuration.ProcessConfiguration.List[p].Alias = PROC_GSM;
					if (parseString(Index, "PCS", 1) == 1)
						m_Configuration.ProcessConfiguration.List[p].Alias = PROC_PCS;
					if (parseString(Index, "OLI", 1) == 1)
						m_Configuration.ProcessConfiguration.List[p].Alias = PROC_OLI;
					if (parseString(Index, "SM", 1) == 1)
						m_Configuration.ProcessConfiguration.List[p].Alias = PROC_SM;
						
					if (m_Configuration.ProcessConfiguration.List[p].Alias == PROC_UNDEFINED)
						throw OSException(LOCATION);
				}
				else 
				{
					m_Configuration.ProcessConfiguration.List[p].Description.global_pid = parsePublicId(Index);
					m_Configuration.ProcessConfiguration.List[p].Alias = PROC_UNDEFINED;
				}
			}
			else throw OSException(LOCATION);

			if (parseSample(Index, createSample(space), Unlimited) < 1)
				throw OSException(LOCATION);
	
			if (parseString(Index, "programme_file_name:", 1) == 1)
			{
				m_Configuration.ProcessConfiguration.List[p].Description.programme_file_name = parsePath(Index);
			}
			else throw OSException(LOCATION);

			if (parseSample(Index, createSample(space), Unlimited) < 1)
				throw OSException(LOCATION);
	
			if (parseString(Index, "cpu_id:", 1) == 1)
			{
				m_Configuration.ProcessConfiguration.List[p].Description.cpu_id = parsePublicId(Index);
			}
	
			for (unsigned long t = 0; t < OS_MAX_NUMBER_OF_THREADS; t++)
			{
				parseSample(Index, createSample(space), Unlimited);

				if (parseString(Index, "thread(", 1) == 1)
				{
					m_Configuration.ProcessConfiguration.List[p].ThreadConfiguration.Count = t+1;							

					parseSample(Index, createSample(space), Unlimited);

					if (parseString(Index, "thread_id:", Unlimited) == 1)
					{
						m_Configuration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.thread_id = parsePublicId(Index);							
					}
					else throw OSException(LOCATION);

					if (parseSample(Index, createSample(space), Unlimited) < 1)
						throw OSException(LOCATION);

					if (parseString(Index, "entry_point:", Unlimited) == 1)
					{
						m_Configuration.ProcessConfiguration.List[p].ThreadConfiguration.List[t].Description.entry_point = parseName(Index);							
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
            throw OSException(LOCATION);
	}
        
    m_Configuration.ConfigurationFile = parsePath(Index);

    parseSample(Index, createSample(space), Unlimited);
	
	if (Index != getString().size)
		throw OSException(LOCATION);
}


void ParameterParser::setConfiguration( EntityConfiguration Configuration )
{
	m_Configuration = Configuration;
}


EntityConfiguration ParameterParser::getConfiguration( ) 
{
	return m_Configuration;
}

