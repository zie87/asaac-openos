#include "CharacterSequence.hh"

#include "TimeStamp.hh"
#include "TimeInterval.hh"

CharacterSequence::CharacterSequence()
{
}

CharacterSequence::CharacterSequence(const CharacterSequence &data)
{
	this->assign(data);
}

CharacterSequence::CharacterSequence(const ASAAC_CharacterSequence &data)
{
	this->assign(data);
}

CharacterSequence::CharacterSequence(const char * data)
{
	this->assign(data);
}

CharacterSequence::CharacterSequence(const string &data)
{
	this->assign(data);
}

CharacterSequence::CharacterSequence(unsigned long len, char ch)
{
	this->assign(len, ch);
}

CharacterSequence::CharacterSequence(long number)
{
	char buffer[16];
    snprintf(buffer,sizeof(buffer),"%d",number);
	this->assign(buffer);
}

CharacterSequence::CharacterSequence(ASAAC_PublicId number)
{
	char buffer[16];
    snprintf(buffer,sizeof(buffer),"%lu",number);
	this->assign(buffer);
}

CharacterSequence::CharacterSequence(ASAAC_Time time)
{
	erase();
	
	tm t = TimeStamp(time).tm_Time();
	
	*this << (t.tm_mon + 1) << "/" << t.tm_mday << "/" <<  (1900 + t.tm_year) << " " <<  t.tm_hour << ":" <<  t.tm_min << ":" <<  t.tm_sec << "." << div(time.nsec, (long)100000).quot;
}

CharacterSequence::CharacterSequence(ASAAC_TimeInterval longerval)
{
}

CharacterSequence::~CharacterSequence()
{
}

CharacterSequence & CharacterSequence::append( const ASAAC_CharacterSequence &data )
{
	m_Data.append( data.data, data.size );
	return *this;
}

CharacterSequence & CharacterSequence::append( const CharacterSequence &data )
{
	m_Data.append( data.cpp_str() );
	return *this;
}

CharacterSequence & CharacterSequence::append( const char *data )
{
	if (checkCharString(data))
		m_Data.append( data );
	return *this;
}

CharacterSequence & CharacterSequence::append( const string &data )
{
	m_Data.append( data );
	return *this;
}

CharacterSequence & CharacterSequence::append( unsigned long len, char ch )
{
	m_Data.append(len, ch);
	return *this;
}

CharacterSequence & CharacterSequence::append( long number )
{
	this->append(CharacterSequence(number));
	return *this;
}

CharacterSequence & CharacterSequence::append( ASAAC_PublicId number )
{
	this->append(CharacterSequence(number));
	return *this;
}

CharacterSequence & CharacterSequence::append( ASAAC_Time &time)
{
	this->append(CharacterSequence(time));
	return *this;
}

CharacterSequence & CharacterSequence::append( ASAAC_TimeInterval &interval)
{
	this->append(CharacterSequence(interval));
	return *this;
}

CharacterSequence & CharacterSequence::appendLineBreak()
{
	char lb[2];
	lb[0] = 10;
	lb[1] = 0;
	m_Data.append(lb);
	return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_CharacterSequence &data, unsigned long begin_pos, unsigned long len )
{
	if ((data.size == 0) && (begin_pos == 0))
		m_Data.erase();
			
	if (begin_pos >= data.size)
		return *this;
		
	if ((begin_pos + len) > data.size)
		len = data.size - begin_pos; 
		
	const char *Addr = data.data + begin_pos;
	m_Data.assign( Addr, len );
	
	return *this;
}

CharacterSequence & CharacterSequence::assign( const CharacterSequence &data, unsigned long begin_pos, unsigned long len )
{
	const char * SourceData = data.m_Data.c_str();
	return assign(SourceData, begin_pos, len);
}

CharacterSequence & CharacterSequence::assign( const char *data, unsigned long begin_pos, unsigned long len )
{
	if (checkCharString(data))
	{
		unsigned long Size = strlen(data);

		if ((Size == 0) && (begin_pos == 0))
			m_Data.erase();
			
		if (begin_pos >= Size)
			return *this;
			
		if ((begin_pos + len) > Size)
			len = Size - begin_pos;
			 
		const char *Addr = data + begin_pos;
		m_Data.assign( Addr, len );
	}
	return *this;	
}

CharacterSequence & CharacterSequence::assign( const string &data, unsigned long begin_pos, unsigned long len )
{
	return assign(data.c_str());	
}

CharacterSequence & CharacterSequence::assign( unsigned long len, char ch )
{
	m_Data.assign(len, ch);
	return *this;
}

CharacterSequence & CharacterSequence::assign( long number )
{
	this->assign(CharacterSequence(number));
	return *this;
}

CharacterSequence & CharacterSequence::assign( ASAAC_PublicId number )
{
	this->assign(CharacterSequence(number));
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, ASAAC_CharacterSequence data )
{
	m_Data.insert( pos, data.data, 1, data.size );
	return *this;	
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, CharacterSequence data )
{
	m_Data.insert( pos, data.cpp_str() );
	return *this;	
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, char *data )
{
	if (data != 0)
		m_Data.insert( pos, data );
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, string data )
{
	m_Data.insert( pos, data );
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, unsigned long len, char ch )
{
	m_Data.insert(pos, len, ch);
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, long number )
{
	this->insert(pos, CharacterSequence(number));
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, ASAAC_PublicId number )
{
	this->insert(pos, CharacterSequence(number));
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, ASAAC_CharacterSequence data, unsigned long source_pos, unsigned long len)
{
	m_Data.insert( dest_pos, data.data, source_pos, len );
	return *this;	
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, CharacterSequence data, unsigned long source_pos, unsigned long len)
{
	m_Data.insert( dest_pos, data.cpp_str(), source_pos, len);
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, char *data, unsigned long source_pos, unsigned long len)
{
	if (checkCharString(data))
		m_Data.insert( dest_pos, data, source_pos, len);
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, string data, unsigned long source_pos, unsigned long len)
{
	m_Data.insert( dest_pos, data, source_pos, len);
	return *this;
}

long CharacterSequence::compare(const CharacterSequence &data) const
{
	return m_Data.compare(data.m_Data);
}

long CharacterSequence::compare(const char *data) const
{
	if (checkCharString(data))
		return m_Data.compare(data);
	else if (size() == 0)
		return 0;
	return -1;
}

long CharacterSequence::compare(const string &data) const
{
	return m_Data.compare(data);
}

long CharacterSequence::compare(const ASAAC_CharacterSequence data) const
{
	return m_Data.compare(CharacterSequence(data).cpp_str());
}

unsigned long CharacterSequence::contains(const CharacterSequence &data, bool cs) const
{
	unsigned long Index1, Index2, Result;
	
	Index2 = 0;
	Result = 0;
	
	if (data.length() > 0)
	{	
		for (Index1 = 0; Index1 < length(); Index1++)
		{
			if (compareChar( (*this)[Index1], data[Index2], cs) == false)
			{
				Index2 = 0;
			}
			else 
			{
				Index2++;
				if (Index2 == data.length())
					Result++;
			}
		}
	}
	
	return Result;
}

unsigned long CharacterSequence::contains(const char *data, bool cs) const
{
	return contains(CharSeq(data));
}

unsigned long CharacterSequence::contains(const char data, bool cs) const
{
	return contains(CharSeq(1, data));
}

unsigned long CharacterSequence::contains(const string &data, bool cs) const
{
	return contains(CharSeq(data));
}

unsigned long CharacterSequence::contains(const ASAAC_CharacterSequence data, bool cs) const
{
	return contains(CharSeq(data));
}

long CharacterSequence::find( const CharacterSequence &data, bool cs, unsigned long begin_pos ) const
{
	unsigned long Index1, Index2;
	
	Index2 = 0;
	
	if ( (data.length() > 0) && (begin_pos >= 0) ) 
	{	
		for (Index1 = begin_pos; Index1 < length(); Index1++)
		{
			if (compareChar( (*this)[Index1], data[Index2], cs) == false)
			{
				Index2 = 0;
			}
			else 
			{
				Index2++;
				if (Index2 == data.length())
					return (Index1 - Index2 + 1);
			}
		}
	}
	
	return -1;
}

long CharacterSequence::find(const char *data, bool cs, unsigned long begin_pos ) const
{
	return find(CharSeq(data), cs, begin_pos );
}

long CharacterSequence::find(const char data, bool cs, unsigned long begin_pos ) const
{
	return find(CharSeq(1, data), cs, begin_pos );
}

long CharacterSequence::find(const string &data, bool cs, unsigned long begin_pos ) const
{
	return find(CharSeq(data), cs, begin_pos );
}

long CharacterSequence::find(const ASAAC_CharacterSequence data, bool cs, unsigned long begin_pos ) const
{
	return find(CharSeq(data), cs, begin_pos );
}

void CharacterSequence::convertTo( ASAAC_CharacterSequence &data, unsigned long begin_pos, unsigned long len ) const
{
	const char *source = m_Data.c_str();

	source += begin_pos;

	if (begin_pos >= size())
		data.size = 0;
	else
	{	
		if ((begin_pos + len) > size())
			len = size() - begin_pos;
	
		if (len > sizeof(data.data))
			len = sizeof(data.data);
	
		memcpy(data.data, source, len);
		
		data.size = len;
	}
}

void CharacterSequence::convertTo( CharacterSequence &data, unsigned long begin_pos, unsigned long len ) const
{
	data.assign( *this, begin_pos, len); 
}

void CharacterSequence::convertTo( char *data, unsigned long begin_pos, unsigned long len ) const
{
	const char *source = m_Data.c_str();
	
	source += begin_pos;
	
	if ((begin_pos + len) > size())
		len = size() - begin_pos;
	
	if (data != 0)
		memcpy(data, source, len);
	
	data[len] = 0;
}

void CharacterSequence::convertTo( string &data, unsigned long begin_pos, unsigned long len ) const
{
	static CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);
	
	data = Data.m_Data;
}
	
void CharacterSequence::convertTo( long &number, unsigned long begin_pos, unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	number = Data.c_int();
}

void CharacterSequence::convertTo( ASAAC_PublicId &number, unsigned long begin_pos, unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	number = Data.asaac_id();
}	
	
CharacterSequence & CharacterSequence::erase()
{
	m_Data.erase();
	return *this;
}

CharacterSequence & CharacterSequence::erase(unsigned long pos)
{
	m_Data.erase(pos, 1);
	return *this;
}
	
CharacterSequence & CharacterSequence::erase(unsigned long start, unsigned long end)
{
	m_Data.erase(start, end);
	return *this;
}	

const char *CharacterSequence::c_str( unsigned long begin_pos, unsigned long len ) const
{
	static string str;	
	convertTo(str, begin_pos, len);
	return str.c_str();
}

const ASAAC_CharacterSequence CharacterSequence::asaac_str( unsigned long begin_pos, unsigned long len ) const
{
	static ASAAC_CharacterSequence result;
	CharacterSequence(m_Data).convertTo(result, begin_pos, len);
	return result;
}

const string CharacterSequence::cpp_str( unsigned long begin_pos, unsigned long len ) const
{
	static CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);
	
	return Data.m_Data;
}

long CharacterSequence::c_int( unsigned long begin_pos, unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtol(Data.c_str(), &e, 10);
}

unsigned long CharacterSequence::c_uint( unsigned long begin_pos, unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtoul(Data.c_str(), &e, 10);
}

unsigned long CharacterSequence::c_ulong( unsigned long begin_pos, unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtoul(Data.c_str(), &e, 10);
}

ASAAC_PublicId CharacterSequence::asaac_id( unsigned long begin_pos, unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtoul(Data.c_str(), &e, 10);
}

bool CharacterSequence::empty() const
{
	return (length() == 0);
}

bool CharacterSequence::filled() const
{
	return (length() > 0);
}

unsigned long CharacterSequence::size() const
{
	return m_Data.length();
}

unsigned long CharacterSequence::length() const
{
	return m_Data.length();
}

const char *CharacterSequence::data() const
{
	ASAAC_CharacterSequence result;
	convertTo(result);
	return result.data;
}

CharacterSequence & CharacterSequence::operator=(const CharacterSequence &data)
{
	this->assign(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator=(const char *data)
{
	this->assign(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator=(const string &data)
{
	this->assign(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator=(const ASAAC_CharacterSequence &data)
{
	this->assign(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator=(long number)
{
	this->assign(number);
	return *this;
}

CharacterSequence & CharacterSequence::operator=(unsigned long number)
{
	this->assign(number);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(const CharacterSequence &data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(const char *data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(const string &data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(const ASAAC_CharacterSequence &data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(long number)
{
	this->append(number);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(ASAAC_Time &time)
{
	this->append(time);
	return *this;
}

CharacterSequence & CharacterSequence::operator<<(ASAAC_TimeInterval &interval)
{
	this->append(interval);
	return *this;
}

CharacterSequence & CharacterSequence::operator+=(const CharacterSequence &data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator+=(const char *data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator+=(const string &data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator+=(const ASAAC_CharacterSequence &data)
{
	this->append(data);
	return *this;
}

CharacterSequence & CharacterSequence::operator+=(long number)
{
	this->append(number);
	return *this;
}

CharacterSequence & CharacterSequence::operator+=(ASAAC_PublicId number)
{
	this->append(number);
	return *this;
}

CharacterSequence  CharacterSequence::operator+(const CharacterSequence &data)
{
	CharacterSequence cs(*this);
	cs.append(data);
	return cs;
}

CharacterSequence  CharacterSequence::operator+(const char *data)
{
	CharacterSequence cs(*this);
	cs.append(data);
	return cs;
}

CharacterSequence  CharacterSequence::operator+(const string &data)
{
	CharacterSequence cs(*this);
	cs.append(data);
	return cs;
}

CharacterSequence  CharacterSequence::operator+(const ASAAC_CharacterSequence &data)
{
	CharacterSequence cs(*this);
	cs.append(data);
	return cs;
}

CharacterSequence  CharacterSequence::operator+(long number)
{
	CharacterSequence cs(*this);
	cs.append(number);
	return cs;
}

CharacterSequence  CharacterSequence::operator+(ASAAC_PublicId number)
{
	CharacterSequence cs(*this);
	cs.append(number);
	return cs;
}

bool CharacterSequence::operator==(const CharacterSequence &data) const
{
	return (this->compare(data) == 0);
}

bool CharacterSequence::operator==(const char *data) const
{
	return (this->compare(data) == 0);
}

bool CharacterSequence::operator==(const string &data) const
{
	return (this->compare(data) == 0);
}

bool CharacterSequence::operator==(const ASAAC_CharacterSequence &data) const
{
	return (this->compare(data) == 0);
}

bool CharacterSequence::operator!=(const CharacterSequence &data) const
{
	return (this->compare(data) != 0);
}

bool CharacterSequence::operator!=(const char *data) const
{
	return (this->compare(data) != 0);
}

bool CharacterSequence::operator!=(const string &data) const
{
	return (this->compare(data) != 0);
}

bool CharacterSequence::operator!=(const ASAAC_CharacterSequence &data) const
{
	return (this->compare(data) != 0);
}

char CharacterSequence::operator[](const unsigned long Index) const
{
	return m_Data[Index];
}

bool CharacterSequence::checkCharString( const char * data )
{
	if (data != 0) return true;
	return false;
}

bool CharacterSequence::compareChar(char c1, char c2, bool cs)
{
	if ( cs = false )
	{
		if ((c1 > 0x40) && (c1 < 0x5b))
			c1 += 0x20;

		if ((c2 > 0x40) && (c2 < 0x5b))
			c2 += 0x20;			
	}
	
	return (c1 == c2);
}

