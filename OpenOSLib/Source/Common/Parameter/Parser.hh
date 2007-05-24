#ifndef PARSER_HH_
#define PARSER_HH_

#include "OpenOSIncludes.hh"


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


class Parser
{
public:
	Parser();
	virtual ~Parser();

    void setString(CharacterSequence String);
    CharacterSequence getString();

    unsigned long   parseString(unsigned long &Index, char *String, unsigned long Max);
    unsigned long   parseSample(unsigned long &Index, char *Sample, unsigned long Max, bool Inverse = false);
    
    char *createSample( char Begin, char End );
    char *createSample( RegExSampleType Type );
    char *createSample( char *Sample1, char *Sample2 );
                    
    ASAAC_PublicId parsePublicId(unsigned long &Index);
    ASAAC_CharacterSequence parsePath(unsigned long &Index);
    ASAAC_CharacterSequence parseName(unsigned long &Index);
    
private:
    CharacterSequence   m_Sequence;    
};

#endif /*PARSER_HH_*/
