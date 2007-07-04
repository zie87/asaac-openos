#include "ParameterParser.hh"

#include "OpenOSObject.hh"


const unsigned long Unlimited = 0xffffffff;


ParameterParser::ParameterParser()
{
}


ParameterParser::~ParameterParser()
{
}


void ParameterParser::parse( ASAAC_CharacterSequence Sequence)
{
	try
	{
		CharacterSequence ErrorString;
		
		setString( Sequence );
		
		unsigned long Index = 0;
		
		parseSample(Index, createSample(space), Unlimited);
		
		if (parseString(Index, "-f", 1) == 1)
		{
			m_Configuration.Flush = true;
			
			if (parseSample(Index, createSample(space), Unlimited) < 1)
				throw OSException( (ErrorString << "Space was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
		}
			
	    if (parseString(Index, "-cpu_id=", 1) == 1)
	        m_Configuration.CpuId = parsePublicId( Index );
		else throw OSException( (ErrorString << "'-f' or '-cpu_id=' was expected. (Position: " << Index << ", Sequence: '" << m_Sequence.c_str(Index, 10) << "...')").c_str(), LOCATION); 
	    
	    if (parseSample(Index, createSample(space), Unlimited) < 1)
			throw OSException( (ErrorString << "Space was expected: '" << m_Sequence.c_str(Index, 10) << "'...").c_str(), LOCATION);
		        
	    m_Configuration.ConfigurationFile = parsePath(Index);
	
	    parseSample(Index, createSample(space), Unlimited);
	
		if (Index != getString().size)
			throw OSException( (ErrorString << "Unexpected character: " << m_Sequence.c_str(Index, 10) << "...").c_str(), LOCATION);
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Syntax error in parameter list", LOCATION);
		
		throw;
	}
}


void ParameterParser::setConfiguration( EntityConfiguration Configuration )
{
	m_Configuration = Configuration;
}


EntityConfiguration ParameterParser::getConfiguration( ) 
{
	return m_Configuration;
}

