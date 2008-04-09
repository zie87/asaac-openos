#ifndef CONFIGURATIONPARSER_HH_
#define CONFIGURATIONPARSER_HH_

#include "Parser.hh"
#include "ParameterTypes.hh"
#include "OpenOSIncludes.hh"

class ConfigurationParser : public Parser
{
public:
	ConfigurationParser();
	virtual ~ConfigurationParser();

    void loadFile( ASAAC_CharacterSequence FileName );

    void parse( ASAAC_CharacterSequence FileName );

    void setConfiguration( ProcessList Configuration );
    ProcessList getConfiguration( ); 

private:
    ProcessList m_Configuration;
};

#endif /*CONFIGURATIONPARSER_HH_*/
