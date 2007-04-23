#ifndef CHARACTERSEQUENCE_H_
#define CHARACTERSEQUENCE_H_

#include "OpenOSIncludes.hh"

using namespace std;


class CharacterSequence
{
public:
	static const unsigned long Unlimited = 0xffffffff;
	
	CharacterSequence();
	CharacterSequence(const CharacterSequence &data);
	CharacterSequence(const ASAAC_CharacterSequence &data);
	CharacterSequence(const char * data);
	CharacterSequence(const string &data);
	CharacterSequence(unsigned long len, char ch);
	CharacterSequence(int number);
	CharacterSequence(unsigned int number);
	CharacterSequence(ASAAC_PublicId number);
	CharacterSequence(ASAAC_Time time);
	CharacterSequence(ASAAC_TimeInterval interval);
	virtual ~CharacterSequence();
	
	CharacterSequence & append( const ASAAC_CharacterSequence &data );
	CharacterSequence & append( const CharacterSequence &data );
	CharacterSequence & append( const char *data );
	CharacterSequence & append( const string &data );
	CharacterSequence & append( unsigned long len, char ch );
	CharacterSequence & append( int number );
	CharacterSequence & append( unsigned int number );
	CharacterSequence & append( ASAAC_PublicId number );
	CharacterSequence & append( ASAAC_Time &time);
	CharacterSequence & append( ASAAC_TimeInterval &interval);
	CharacterSequence & appendLineBreak();
		
	CharacterSequence & assign( const ASAAC_CharacterSequence &data, unsigned long begin_pos = 0, unsigned long len = Unlimited );
	CharacterSequence & assign( const CharacterSequence &data, unsigned long begin_pos = 0, unsigned long len = Unlimited );
	CharacterSequence & assign( const char *data, unsigned long begin_pos = 0, unsigned long len = Unlimited );
	CharacterSequence & assign( const string &data, unsigned long begin_pos = 0, unsigned long len = Unlimited );
	CharacterSequence & assign( unsigned long len, char ch );
	CharacterSequence & assign( int number );
	CharacterSequence & assign( unsigned int number );
	CharacterSequence & assign( ASAAC_PublicId number );
	
	CharacterSequence & insert( unsigned long pos, ASAAC_CharacterSequence data );
	CharacterSequence & insert( unsigned long pos, CharacterSequence data );
	CharacterSequence & insert( unsigned long pos, char *data );
	CharacterSequence & insert( unsigned long pos, string data );
	CharacterSequence & insert( unsigned long pos, unsigned long len, char ch );
	CharacterSequence & insert( unsigned long pos, int number );
	CharacterSequence & insert( unsigned long pos, unsigned int number );
	CharacterSequence & insert( unsigned long pos, ASAAC_PublicId number );
	CharacterSequence & insert( unsigned long dest_pos, ASAAC_CharacterSequence data, unsigned long source_pos, unsigned long len);
	CharacterSequence & insert( unsigned long dest_pos, CharacterSequence data, unsigned long source_pos, unsigned long len);
	CharacterSequence & insert( unsigned long dest_pos, char *data, unsigned long source_pos, unsigned long len);
	CharacterSequence & insert( unsigned long dest_pos, string data, unsigned long source_pos, unsigned long len);
	
	int compare(const CharacterSequence &data) const;
	int compare(const char *data) const;
	int compare(const string &data) const;
	int compare(const ASAAC_CharacterSequence data) const;
	
	unsigned long contains(const CharacterSequence &data, bool cs = true) const;
	unsigned long contains(const char *data, bool cs = true) const;
	unsigned long contains(const char data, bool cs = true) const;
	unsigned long contains(const string &data, bool cs = true) const;
	unsigned long contains(const ASAAC_CharacterSequence data, bool cs = true) const;
	
	long find(const CharacterSequence &data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const char *data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const char data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const string &data, bool cs = true, unsigned long begin_pos = 0) const;
	long find(const ASAAC_CharacterSequence data, bool cs = true, unsigned long begin_pos = 0) const;
	
	void convertTo( ASAAC_CharacterSequence &data, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	void convertTo( CharacterSequence &data, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	void convertTo( char *data, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	void convertTo( string &data, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	void convertTo( int &number, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	void convertTo( unsigned int &number, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	void convertTo( ASAAC_PublicId &number, unsigned long begin_pos = 0, unsigned long len = Unlimited ) const;
	
	CharacterSequence & erase();	
	CharacterSequence & erase(unsigned long pos);	
	CharacterSequence & erase(unsigned long start, unsigned long end);
	
	const char *					c_str(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	const ASAAC_CharacterSequence 	asaac_str(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	const string 					cpp_str(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	int 							c_int(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	unsigned int 					c_uint(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	unsigned long 					c_ulong(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	ASAAC_PublicId 					asaac_id(unsigned long begin_pos = 0, unsigned long len = Unlimited) const;
	
	bool empty() const;
	bool filled() const;
	unsigned long size() const;
	unsigned long length() const;
	const char *data() const;
	
	CharacterSequence & operator=(const CharacterSequence &data);
	CharacterSequence & operator=(const char *data);
	CharacterSequence & operator=(const string &data);
	CharacterSequence & operator=(const ASAAC_CharacterSequence &data);
	CharacterSequence & operator=(int number);
	CharacterSequence & operator=(unsigned int number);
	
	CharacterSequence & operator<<(const CharacterSequence &data);
	CharacterSequence & operator<<(const char *data);
	CharacterSequence & operator<<(const string &data);
	CharacterSequence & operator<<(const ASAAC_CharacterSequence &data);
	CharacterSequence & operator<<(int number);
	CharacterSequence & operator<<(ASAAC_Time &time);
	CharacterSequence & operator<<(ASAAC_TimeInterval &interval);
	
	CharacterSequence & operator+=(const CharacterSequence &data);
	CharacterSequence & operator+=(const char *data);
	CharacterSequence & operator+=(const string &data);
	CharacterSequence & operator+=(const ASAAC_CharacterSequence &data);
	CharacterSequence & operator+=(int number);
	CharacterSequence & operator+=(ASAAC_PublicId number);

	CharacterSequence operator+(const CharacterSequence &data);
	CharacterSequence operator+(const char *data);
	CharacterSequence operator+(const string &data);
	CharacterSequence operator+(const ASAAC_CharacterSequence &data);
	CharacterSequence operator+(int number);
	CharacterSequence operator+(ASAAC_PublicId number);

	bool operator==(const CharacterSequence &data) const;
	bool operator==(const char *data) const;
	bool operator==(const string &data) const;
	bool operator==(const ASAAC_CharacterSequence &data) const;
	
	bool operator!=(const CharacterSequence &data) const;
	bool operator!=(const char *data) const;
	bool operator!=(const string &data) const;
	bool operator!=(const ASAAC_CharacterSequence &data) const;
	
	char operator[](const unsigned long Index) const;
	
private: 
	static bool checkCharString(const char * data);
	static bool compareChar(char c1, char c2, bool cs);

	string m_Data;
};

typedef CharacterSequence CharSeq;

#endif /*CHARACTERSEQUENCE_H_*/
