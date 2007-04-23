#ifndef PARAMETERPARSER_HH_
#define PARAMETERPARSER_HH_

#include "Types.hh"
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


typedef enum {
  alnum,
  alpha,
  blank,
  cntrl,
  digit,
  graph,
  lower,
  print,
  punct,
  space,
  upper,
  xdigit
} RegExSampleType;


class ParameterParser
{
	public:
		ParameterParser();
		~ParameterParser();
	
		void parse( CharacterSequence &sequence );

		void setConfiguration( EntityConfiguration configuration );
		EntityConfiguration getConfiguration( ); 
	
	private:
		CharacterSequence 	m_Sequence;
		
		EntityConfiguration m_Configuration;
		
		unsigned long 	parseString(unsigned long &Index, char *String, unsigned long Max);
		unsigned long 	parseSample(unsigned long &Index, char *Sample, unsigned long Max, bool Inverse = false);
		
		char *createSample( char Begin, char End );
		char *createSample( RegExSampleType Type );
		char *createSample( char *Sample1, char *Sample2 );
		
		ASAAC_PublicId parsePublicId(unsigned long &Index);
		ASAAC_CharacterSequence parsePath(unsigned long &Index);
		ASAAC_CharacterSequence parseName(unsigned long &Index);
};


#endif /*PARAMETERPARSER_HH_*/
