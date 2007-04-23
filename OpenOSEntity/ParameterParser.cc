#include "ParameterParser.hh"

#include "OpenOS.hh"

const unsigned long Unlimited = 0xffffffff;

ParameterParser::ParameterParser()
{
}


ParameterParser::~ParameterParser()
{
}


void ParameterParser::parse(CharacterSequence &sequence)
{
	m_Sequence = sequence;
	
	unsigned long Index = 0;
	
	parseSample(Index, createSample(space), Unlimited);
	
	if (parseString(Index, "-f", 1) == 1)
		m_Configuration.Flush = true;
		
	if (parseSample(Index, createSample(space), Unlimited) < 1)
		throw OSException(LOCATION);
		
	if (parseString(Index, "-cpu_id:", 1) == 1)
	{
		m_Configuration.CpuId = parsePublicId( Index );
	} 
	
	if (parseSample(Index, createSample(space), Unlimited) < 1)
		throw OSException(LOCATION);
	
	for (unsigned short p = 0; p < OS_MAX_NUMBER_OF_PROCESSES; p++)
	{
		if (parseString(Index, "-process(", 1) == 1)
		{
			m_Configuration.Processes.Count = p+1;
			
			parseSample(Index, createSample(space), Unlimited);
			
			if (parseString(Index, "global_pid:", 1) == 1)
			{
				if (parseString(Index, "$", 1) == 1)
				{
					m_Configuration.Processes.List[p].Alias = PROC_Undefined;
					m_Configuration.Processes.List[p].Process.global_pid = OS_UNUSED_ID;
					
					if (parseString(Index, "APOS", 1) == 1)
						m_Configuration.Processes.List[p].Alias = PROC_APOS;
					if (parseString(Index, "SMOS", 1) == 1)
						m_Configuration.Processes.List[p].Alias = PROC_SMOS;
					if (parseString(Index, "GSM", 1) == 1)
						m_Configuration.Processes.List[p].Alias = PROC_GSM;
					if (parseString(Index, "PCS", 1) == 1)
						m_Configuration.Processes.List[p].Alias = PROC_PCS;
					if (parseString(Index, "OLI", 1) == 1)
						m_Configuration.Processes.List[p].Alias = PROC_OLI;
					if (parseString(Index, "SM", 1) == 1)
						m_Configuration.Processes.List[p].Alias = PROC_SM;
						
					if (m_Configuration.Processes.List[p].Alias == PROC_Undefined)
						throw OSException(LOCATION);
				}
				else 
				{
					m_Configuration.Processes.List[p].Process.global_pid = parsePublicId(Index);
					m_Configuration.Processes.List[p].Alias = PROC_Undefined;
				}
			}
			else throw OSException(LOCATION);

			if (parseSample(Index, createSample(space), Unlimited) < 1)
				throw OSException(LOCATION);
	
			if (parseString(Index, "programme_file_name:", 1) == 1)
			{
				m_Configuration.Processes.List[p].Process.programme_file_name = parsePath(Index);
			}
			else throw OSException(LOCATION);

			if (parseSample(Index, createSample(space), Unlimited) < 1)
				throw OSException(LOCATION);
	
			if (parseString(Index, "cpu_id:", 1) == 1)
			{
				m_Configuration.Processes.List[p].Process.cpu_id = parsePublicId(Index);
			}
	
			for (unsigned long t = 0; t < OS_MAX_NUMBER_OF_THREADS; t++)
			{
				parseSample(Index, createSample(space), Unlimited);

				if (parseString(Index, "thread(", 1) == 1)
				{
					m_Configuration.Processes.List[p].Threads.Count = t+1;							

					parseSample(Index, createSample(space), Unlimited);

					if (parseString(Index, "thread_id:", Unlimited) == 1)
					{
						m_Configuration.Processes.List[p].Threads.List[t].thread_id = parsePublicId(Index);							
					}
					else throw OSException(LOCATION);

					if (parseSample(Index, createSample(space), Unlimited) < 1)
						throw OSException(LOCATION);

					if (parseString(Index, "entry_point:", Unlimited) == 1)
					{
						m_Configuration.Processes.List[p].Threads.List[t].entry_point = parseName(Index);							
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

		parseSample(Index, createSample(space), Unlimited);	
	}

	parseSample(Index, createSample(space), Unlimited);
	
	if (Index != m_Sequence.length())
		throw OSException(LOCATION);
}


void ParameterParser::setConfiguration( EntityConfiguration configuration )
{
	m_Configuration = configuration;
}


EntityConfiguration ParameterParser::getConfiguration( ) 
{
	return m_Configuration;
}


unsigned long ParameterParser::parseString(unsigned long &Index, char *String, unsigned long Max)
{
	unsigned long Length = strlen(String);
	
	unsigned long i = Index;
	unsigned long c = 0;
	
	bool Flag = true;
	
	for (; c < Max; c++)
	{
		for (unsigned long j = 0; j < Length; j++, i++)
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


unsigned long ParameterParser::parseSample(unsigned long &Index, char *Sample, unsigned long Max, bool Inverse)
{
	unsigned long Length = strlen(Sample);
	
	unsigned long i = Index;
	unsigned long c = 0;

	bool Flag = false;
	
	for (; c < Max; c++, i++)
	{
		Index = i;

		Flag = false;
		
		for (unsigned long j = 0; j < Length; j++)
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


char *ParameterParser::createSample(char Begin, char End)
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


char *ParameterParser::createSample(RegExSampleType Type)
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


char *ParameterParser::createSample(char *Sample1, char *Sample2)
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


ASAAC_PublicId ParameterParser::parsePublicId(unsigned long &Index)
{
	unsigned long StartIndex = Index;
	unsigned long Len = parseSample(Index, createSample(xdigit), 10);
	return m_Sequence.asaac_id( StartIndex, Len );
}


ASAAC_CharacterSequence ParameterParser::parsePath(unsigned long &Index)
{
	static ASAAC_CharacterSequence Result;
	unsigned long StartIndex = Index;
	Result = m_Sequence.asaac_str( StartIndex, parseSample(Index, createSample(graph), sizeof(Result.data)) );
	return Result;
}


ASAAC_CharacterSequence ParameterParser::parseName(unsigned long &Index)
{
	static ASAAC_CharacterSequence Result;
	unsigned long StartIndex = Index;
	Result = m_Sequence.asaac_str( StartIndex, parseSample(Index, createSample(alnum), sizeof(Result.data)) );
	return Result;
}

