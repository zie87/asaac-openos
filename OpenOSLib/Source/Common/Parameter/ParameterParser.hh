#ifndef PARAMETERPARSER_HH_
#define PARAMETERPARSER_HH_

#include "Parser.hh"
#include "ParameterTypes.hh"
#include "OpenOSIncludes.hh"


// Pattern for OpenOS-Entity parameter
//
//		 	[-f] 
//			[-cpu:[0-9]+] 
//			[-process
//				(
//					[pid:[0-9]+ | name:[:alnum::punct:]+]{1} 
//					path:[:alnum::punct:]+ 
//					[cpu:[0-9]+]{1} 
//					threads( [ [:alnum::punct:]*:[0-9] ]+ )
//				)
//			]+


class ParameterParser: public Parser
{
	public:
		ParameterParser();
		~ParameterParser();
	
		void parse( ASAAC_CharacterSequence Sequence );

		void setConfiguration( EntityConfiguration Configuration );
		EntityConfiguration getConfiguration( ); 
	
	private:
		EntityConfiguration m_Configuration;
};


#endif /*PARAMETERPARSER_HH_*/
