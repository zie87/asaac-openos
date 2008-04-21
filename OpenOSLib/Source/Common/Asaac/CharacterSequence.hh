#ifndef CHARACTERSEQUENCE_H_
#define CHARACTERSEQUENCE_H_

#include "OpenOSIncludes.hh"

using namespace std;

typedef enum {
	 binary,
	 octal,
	 unsigned_decimal,
	 signed_decimal,
	 hexadecimal
} NumberRepresentation;

class CharacterSequence
{
public:
	static const unsigned long Unlimited = 0xffffffff;
		
	CharacterSequence();
	CharacterSequence(const CharacterSequence &data);
	CharacterSequence(const ASAAC_CharacterSequence &data);
	CharacterSequence(const char * data);
	CharacterSequence(const unsigned long len, const char ch);
	CharacterSequence(const ASAAC_PublicId number, const NumberRepresentation representation = unsigned_decimal );
	CharacterSequence(const ASAAC_Time time);
	CharacterSequence(const ASAAC_TimeInterval interval);
    CharacterSequence(const ASAAC_ProcessDescription &process_desc );
    CharacterSequence(const ASAAC_ThreadDescription &thread_desc );
    CharacterSequence(const ASAAC_ThreadSchedulingInfo &thread_scheduling_info );
    CharacterSequence(const ASAAC_ThreadStatus &thread_status );
	virtual ~CharacterSequence();
	
	CharacterSequence & append( const ASAAC_CharacterSequence &data );
	CharacterSequence & append( const CharacterSequence &data );
	CharacterSequence & append( const char *data );
	CharacterSequence & append( const unsigned long len, const char ch );
	CharacterSequence & append( const ASAAC_PublicId number, const NumberRepresentation representation = unsigned_decimal );
	CharacterSequence & append( const ASAAC_Time &time);
	CharacterSequence & append( const ASAAC_TimeInterval &interval);
	CharacterSequence & appendLineBreak();
		
	CharacterSequence & assign( const ASAAC_CharacterSequence &data, const unsigned long begin_pos = 0, const unsigned long len = Unlimited );
	CharacterSequence & assign( const CharacterSequence &data, const unsigned long begin_pos = 0, const unsigned long len = Unlimited );
	CharacterSequence & assign( const char *data, const unsigned long begin_pos = 0, const unsigned long len = Unlimited );
	CharacterSequence & assign( const unsigned long len, const char ch );
	CharacterSequence & assign( const ASAAC_PublicId number, const NumberRepresentation representation = unsigned_decimal );
	CharacterSequence & assign( const ASAAC_Time time);
	CharacterSequence & assign( const ASAAC_TimeInterval interval);
    CharacterSequence & assign( const ASAAC_ProcessDescription &process_desc );
    CharacterSequence & assign( const ASAAC_ThreadDescription &thread_desc );
    CharacterSequence & assign( const ASAAC_ThreadSchedulingInfo &thread_scheduling_info );
    CharacterSequence & assign( const ASAAC_ThreadStatus &thread_status );
	
	CharacterSequence & insert( const unsigned long pos, const unsigned long len, const char ch );
	CharacterSequence & insert( const unsigned long pos, const ASAAC_PublicId number, const NumberRepresentation representation = unsigned_decimal );
	CharacterSequence & insert( const unsigned long dest_pos, const ASAAC_CharacterSequence &data, const unsigned long source_pos = 0, const unsigned long len = Unlimited );
	CharacterSequence & insert( const unsigned long dest_pos, const CharacterSequence &data, const unsigned long source_pos = 0, const unsigned long len = Unlimited );
	CharacterSequence & insert( const unsigned long dest_pos, const char *data, const unsigned long source_pos = 0, const unsigned long len = Unlimited );
	
	long compare(const CharacterSequence &data) const;
	long compare(const char *data) const;
	long compare(const ASAAC_CharacterSequence &data) const;
	
	unsigned long contains(const CharacterSequence &data, bool cs = true) const;
	unsigned long contains(const char *data, bool cs = true) const;
	unsigned long contains(const char data, bool cs = true) const;
	unsigned long contains(const ASAAC_CharacterSequence &data, bool cs = true) const;
	
	long find(const CharacterSequence &data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const char *data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const char data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const ASAAC_CharacterSequence &data, bool cs = true, unsigned long begin_pos = 0) const;
	
	void convertTo( ASAAC_CharacterSequence &data, const unsigned long begin_pos = 0, const unsigned long len = Unlimited ) const;
	void convertTo( CharacterSequence &data, const unsigned long begin_pos = 0, const unsigned long len = Unlimited ) const;
	void convertTo( char *data, const unsigned long begin_pos = 0, const unsigned long len = Unlimited ) const;
	void convertTo( long &number, const unsigned long begin_pos = 0, const unsigned long len = Unlimited ) const;
	void convertTo( ASAAC_PublicId &number, const unsigned long begin_pos = 0, const unsigned long len = Unlimited ) const;
	
	CharacterSequence & erase();	
	CharacterSequence & erase(const unsigned long pos);	
	CharacterSequence & erase(const unsigned long start, const unsigned long end);
	
	const char *					c_str(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	ASAAC_CharacterSequence 		asaac_str(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	int 							c_int(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	long 							c_long(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	unsigned int 					c_uint(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	unsigned long 					c_ulong(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	ASAAC_PublicId 					asaac_id(const unsigned long begin_pos = 0, const unsigned long len = Unlimited) const;
	
	bool empty() const;
	bool filled() const;
	unsigned long size() const;
	unsigned long length() const;
	const char *data() const;
	
	CharacterSequence & operator=(const CharacterSequence &data);
	CharacterSequence & operator=(const char *data);
	CharacterSequence & operator=(const char data);
	CharacterSequence & operator=(const ASAAC_CharacterSequence &data);
	CharacterSequence & operator=(const long number);
	CharacterSequence & operator=(const ASAAC_PublicId number);
	CharacterSequence & operator=(const ASAAC_Time &time);
	CharacterSequence & operator=(const ASAAC_TimeInterval &interval);
	
	CharacterSequence & operator<<(const CharacterSequence &data);
	CharacterSequence & operator<<(const char *data);
	CharacterSequence & operator<<(const char data);
	CharacterSequence & operator<<(const ASAAC_CharacterSequence &data);
	CharacterSequence & operator<<(const long number);
	CharacterSequence & operator<<(const ASAAC_PublicId number);
	CharacterSequence & operator<<(const ASAAC_Time &time);
	CharacterSequence & operator<<(const ASAAC_TimeInterval &interval);
	
	CharacterSequence & operator+=(const CharacterSequence &data);
	CharacterSequence & operator+=(const char *data);
	CharacterSequence & operator+=(const char data);
	CharacterSequence & operator+=(const ASAAC_CharacterSequence &data);
	CharacterSequence & operator+=(const long number);
	CharacterSequence & operator+=(const ASAAC_PublicId number);
	CharacterSequence & operator+=(const ASAAC_Time &time);
	CharacterSequence & operator+=(const ASAAC_TimeInterval &interval);

	CharacterSequence operator+(const CharacterSequence &data);
	CharacterSequence operator+(const char *data);
	CharacterSequence operator+(const char data);
	CharacterSequence operator+(const ASAAC_CharacterSequence &data);
	CharacterSequence operator+(const long number);
	CharacterSequence operator+(const ASAAC_PublicId number);
	CharacterSequence operator+(const ASAAC_Time &time);
	CharacterSequence operator+(const ASAAC_TimeInterval &interval);

	bool operator==(const CharacterSequence &data) const;
	bool operator==(const char *data) const;
	bool operator==(const ASAAC_CharacterSequence &data) const;
	
	bool operator!=(const CharacterSequence &data) const;
	bool operator!=(const char *data) const;
	bool operator!=(const ASAAC_CharacterSequence &data) const;
	
	char operator[](const unsigned long Index) const;
	
	static CharacterSequence LineBreak();

	friend ostream & operator<<(ostream &stream, CharacterSequence seq);
	friend istream & operator>>(istream &stream, CharacterSequence &seq);	
	
private: 
	void checkIntegrity();

	static bool checkAsaacString(const ASAAC_CharacterSequence &data);
	static bool checkCharString(const char * data);
	static bool compareChar(const char c1, char c2, const bool cs);

	char 			m_Data[ASAAC_OS_MAX_STRING_SIZE + 1];
	unsigned long 	m_Size;
};

typedef CharacterSequence CharSeq;

#endif /*CHARACTERSEQUENCE_H_*/
