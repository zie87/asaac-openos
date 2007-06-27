#include "Parser.hh"


Parser::Parser()
{
}


Parser::~Parser()
{
}


void Parser::setString( ASAAC_CharacterSequence String )
{
    m_Sequence = String;
}


ASAAC_CharacterSequence Parser::getString()
{
    return m_Sequence.asaac_str();
}


unsigned long Parser::parseString(unsigned long &Index, char *String, unsigned long Max)
{
    unsigned long StringLength = strlen(String);
    unsigned long Length = m_Sequence.size();
    
    unsigned long i = Index;
    unsigned long c = 0;
    
    bool Flag = true;
    
    for (; (c < Max) && (i < Length); c++)
    {
        for (unsigned long j = 0; (j < StringLength) && (i < Length); j++, i++)
            if ( String[j] != m_Sequence[i] )
            {
                Flag = false;
                break;
            }
            
        if (Flag == false)
            break;
        
        Index = i;
    }
    
    return c;   
}


unsigned long Parser::parseSample(unsigned long &Index, char *Sample, unsigned long Max, bool Inverse)
{
    unsigned long SampleLength = strlen(Sample);
    unsigned long Length = m_Sequence.size();
    
    unsigned long i = Index;
    unsigned long c = 0;

    bool Flag = false;
    
    for (; c < Max; c++, i++)
    {
        Index = i;

		if (i == Length)
			break;

        Flag = false;
        
        for (unsigned long j = 0; j < SampleLength; j++)
            if (Sample[j] == m_Sequence[i])
            {
                Flag = true;
                break;
            }
            
        if (Flag == Inverse)
            break;
    }
    
    return c;
}


char *Parser::createSample(char Begin, char End)
{
    static char Sample[256];
    
    unsigned long i = 0;
    
    if (Begin <= End)
    {
        for (unsigned long j = Begin; j <= (unsigned long)End; j++, i++)
            Sample[i] = j;
    }
    else
    {
        for (unsigned long j = 0; j <= (unsigned long)End; j++, i++)
            Sample[i] = j;

        for (unsigned long j = Begin; j < 256; j++, i++)
            Sample[i] = j;
    }
    
    Sample[i] = 0;
    
    return Sample;
}


char *Parser::createSample(RegExSampleType Type)
{
    static char SampleEmpty[  1] = "";
    static char SampleAlnum[ 65] = "abcdefgehijklmnopqrstuvwxyzABCDEFGEHIJKLMNOPQRSTUVWXYZ0123456789";
    static char SampleAlpha[ 55] = "abcdefgehijklmnopqrstuvwxyzABCDEFGEHIJKLMNOPQRSTUVWXYZ";
    static char SampleBlank[  3] = {0x20, 0x09, 0};
    static char SampleCntrl[ 33] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 
                                    0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
                                    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x7F, 0};
    static char SampleDigit[ 11] = "0123456789";
    static char SampleGraph[ 99] = "abcdefgehijklmnopqrstuvwxyzABCDEFGEHIJKLMNOPQRSTUVWXYZ01234567890!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~.";
    static char SampleLower[ 28] = "abcdefgehijklmnopqrstuvwxyz";
    static char SamplePrint[100] = "abcdefgehijklmnopqrstuvwxyzABCDEFGEHIJKLMNOPQRSTUVWXYZ01234567890!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~. ";
    static char SamplePunct[ 35] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~.";
    static char SampleSpace[  7] = {0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, 0};
    static char SampleUpper[ 28] = "ABCDEFGEHIJKLMNOPQRSTUVWXYZ";
    static char SampleXDigit[23] = "0123456789ABCDEFabcdef";
    
    switch (Type)
    {
        case alnum: return SampleAlnum;
        case alpha: return SampleAlpha;
        case blank: return SampleBlank;
        case cntrl: return SampleCntrl;
        case digit: return SampleDigit;
        case graph: return SampleGraph;
        case lower: return SampleLower;
        case print: return SamplePrint;
        case punct: return SamplePunct;
        case space: return SampleSpace;
        case upper: return SampleUpper;
        case xdigit: return SampleXDigit;
        default: return SampleEmpty;
    }
}


char *Parser::createSample(char *Sample1, char *Sample2)
{
    static char Sample[256];
    
    unsigned long Length  = strlen(Sample1);
    unsigned long Length2 = strlen(Sample2);

    memcpy(Sample, Sample1, Length);

    bool Flag = false;
    
    for (unsigned long j = 0; j < Length2; j++)
    {
        Flag = false;
        
        for (unsigned long i = 0; i < Length; i++)
            if (Sample[i] == Sample2[j])
            {
                Flag = true;
                break;
            }
            
        if (Flag == false) 
        {
            Sample[Length] = Sample2[j];
            Length++;
        }
    }
    
    Sample[Length] = 0;
    
    return Sample;
}


ASAAC_PublicId Parser::parsePublicId(unsigned long &Index)
{
    unsigned long StartIndex = Index;
    unsigned long Len = parseSample(Index, createSample(xdigit), 10);
    return m_Sequence.asaac_id( StartIndex, Len );
}


ASAAC_CharacterSequence Parser::parsePath(unsigned long &Index)
{
    static ASAAC_CharacterSequence Result;
    unsigned long StartIndex = Index;
    Result = m_Sequence.asaac_str( StartIndex, parseSample(Index, createSample(graph), sizeof(Result.data)) );
    return Result;
}


ASAAC_CharacterSequence Parser::parseName(unsigned long &Index)
{
    static ASAAC_CharacterSequence Result;
    unsigned long StartIndex = Index;
    Result = m_Sequence.asaac_str( StartIndex, parseSample(Index, createSample(alnum), sizeof(Result.data)) );
    return Result;
}

