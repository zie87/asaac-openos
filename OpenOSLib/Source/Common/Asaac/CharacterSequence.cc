#include "CharacterSequence.hh"

#include "TimeStamp.hh"
#include "TimeInterval.hh"

#include "Exceptions/Exceptions.hh"

#define min(a,b) (a)<(b)?(a):(b)

CharacterSequence::CharacterSequence()
{
	erase();
}

CharacterSequence::CharacterSequence(const CharacterSequence &data)
{
	erase();
	this->assign(data);
}

CharacterSequence::CharacterSequence(const ASAAC_CharacterSequence &data)
{
	erase();
	this->assign(data);
}

CharacterSequence::CharacterSequence(const char * data)
{
	erase();
	this->assign(data);
}

CharacterSequence::CharacterSequence(const string &data)
{
	erase();
	this->assign(data);
}

CharacterSequence::CharacterSequence(unsigned long len, char ch)
{
	erase();
	this->assign(len, ch);
}

CharacterSequence::CharacterSequence(long number)
{
	erase();
	this->assign(number);
}

CharacterSequence::CharacterSequence(ASAAC_PublicId number)
{
	erase();
	this->assign(number);
}

CharacterSequence::CharacterSequence(ASAAC_Time time)
{
	erase();
	this->assign(time);
}

CharacterSequence::CharacterSequence(ASAAC_TimeInterval interval)
{
	erase();
	this->assign(interval);
}

CharacterSequence::~CharacterSequence()
{
}

CharacterSequence & CharacterSequence::append( const ASAAC_CharacterSequence &data )
{
	unsigned long max_size = min( data.size, ASAAC_OS_MAX_STRING_SIZE - m_Size );
	memcpy( m_Data + m_Size, data.data, max_size );
	
	m_Size += max_size;
	m_Data[m_Size] = 0;
	
	checkIntegrity();
	 
	return *this;
}

CharacterSequence & CharacterSequence::append( const CharacterSequence &data )
{
	return this->append( data.asaac_str() );
}

CharacterSequence & CharacterSequence::append( const char *data )
{
	return this->append( CharacterSequence(data) );
}

CharacterSequence & CharacterSequence::append( const string &data )
{
	return this->append( CharacterSequence(data) );
}

CharacterSequence & CharacterSequence::append( unsigned long len, char ch )
{
	return this->append( CharacterSequence(len, ch) );
}

CharacterSequence & CharacterSequence::append( long number )
{
	return this->append(CharacterSequence(number));
}

CharacterSequence & CharacterSequence::append( ASAAC_PublicId number )
{
	return this->append(CharacterSequence(number));
}

CharacterSequence & CharacterSequence::append( ASAAC_Time &time)
{
	return this->append(CharacterSequence(time));
}

CharacterSequence & CharacterSequence::append( ASAAC_TimeInterval &interval)
{
	return this->append(CharacterSequence(interval));
}

CharacterSequence & CharacterSequence::appendLineBreak()
{
	return this->append(LineBreak());
}

CharacterSequence & CharacterSequence::assign( const ASAAC_CharacterSequence &data, unsigned long begin_pos, unsigned long len )
{	
	checkAsaacString( data );
	
	if (begin_pos < data.size)
	{	
		if (len + begin_pos > data.size)
			len = data.size - begin_pos;	
	
		memcpy( m_Data, data.data + begin_pos, len );
	
		m_Size = len;
		m_Data[m_Size] = 0;
	}
	else
	{
		erase();
	}

	checkIntegrity();
	
	return *this;
}

CharacterSequence & CharacterSequence::assign( const CharacterSequence &data, unsigned long begin_pos, unsigned long len )
{
	return this->assign( data.asaac_str(), begin_pos, len);
}

CharacterSequence & CharacterSequence::assign( const char *data, unsigned long begin_pos, unsigned long len )
{
	if (!checkCharString(data))
		return *this;	

	ASAAC_CharacterSequence Seq;
	
	Seq.size = min( strlen(data), ASAAC_OS_MAX_STRING_SIZE );
	memcpy( Seq.data, data, Seq.size );

	checkIntegrity();
		
	return this->assign( Seq, begin_pos, len );
}

CharacterSequence & CharacterSequence::assign( const string &data, unsigned long begin_pos, unsigned long len )
{
	return this->assign( data.c_str() );	
}

CharacterSequence & CharacterSequence::assign( unsigned long len, char ch )
{
	if (len > ASAAC_OS_MAX_STRING_SIZE)
		len = ASAAC_OS_MAX_STRING_SIZE;

	ASAAC_CharacterSequence Seq;
	
	for (unsigned long i=0; i<len; i++)
		Seq.data[i] = ch;
		
	Seq.size = len;

	checkIntegrity();
		
	return this->assign( Seq );
}

CharacterSequence & CharacterSequence::assign( long number )
{
    snprintf( m_Data, sizeof(m_Data), "%li", number );

    m_Size = strlen( m_Data );
	m_Data[m_Size] = 0;

	checkIntegrity();
    
	return *this;
}

CharacterSequence & CharacterSequence::assign( ASAAC_PublicId number )
{
    snprintf( m_Data, sizeof(m_Data), "%lu", number );

    m_Size = strlen( m_Data );
	m_Data[m_Size] = 0;

	checkIntegrity();
    
	return *this;
}

CharacterSequence & CharacterSequence::assign( ASAAC_Time time)
{	
	tm t = TimeStamp(time).tm_Time();
	*this << (t.tm_mon + 1) << "/" << t.tm_mday << "/" <<  (1900 + t.tm_year) << " " <<  t.tm_hour << ":" <<  t.tm_min << ":" <<  t.tm_sec << "." << div(time.nsec, (long)100000).quot;
	
	return *this;
}

CharacterSequence & CharacterSequence::assign( ASAAC_TimeInterval interval)
{
	TimeInterval Interval = interval;

	//TODO: find out how to print interval
	return *this;
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, ASAAC_CharacterSequence data )
{
	return this->insert( pos, data, 0, data.size );	
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, CharacterSequence data )
{
	return this->insert( pos, data.asaac_str() );
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, char *data )
{
	return this->insert( pos, CharacterSequence(data) );
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, string data )
{
	return this->insert( pos, CharacterSequence(data) );
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, unsigned long len, char ch )
{
	return this->insert(pos, CharacterSequence(len, ch) );
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, long number )
{
	return this->insert( pos, CharacterSequence(number) );
}

CharacterSequence & CharacterSequence::insert( unsigned long pos, ASAAC_PublicId number )
{
	return this->insert( pos, CharacterSequence(number) );
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, ASAAC_CharacterSequence data, unsigned long source_pos, unsigned long len)
{
	checkAsaacString( data );

	if (dest_pos < ASAAC_OS_MAX_STRING_SIZE )
	{
		if (source_pos < data.size)
		{
			len = min( len, ASAAC_OS_MAX_STRING_SIZE - source_pos );
			
			if (len + dest_pos < ASAAC_OS_MAX_STRING_SIZE)
			{
				unsigned long move_size = min(len, ASAAC_OS_MAX_STRING_SIZE - dest_pos - len); 	
				memmove( m_Data + dest_pos+len, m_Data + dest_pos, move_size );
			}
			
			unsigned long cpy_size = min( len, ASAAC_OS_MAX_STRING_SIZE - dest_pos );  	
			memcpy( m_Data + dest_pos, data.data, cpy_size );
			
			m_Size = min( m_Size+len, ASAAC_OS_MAX_STRING_SIZE );
			m_Data[ m_Size ] = 0;
		}
	}

	checkIntegrity();
	
	return *this;	
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, CharacterSequence data, unsigned long source_pos, unsigned long len)
{
	return this->insert( dest_pos, data.asaac_str(), source_pos, len);
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, char *data, unsigned long source_pos, unsigned long len)
{
	return this->insert( dest_pos, CharacterSequence(data), source_pos, len );
}

CharacterSequence & CharacterSequence::insert( unsigned long dest_pos, string data, unsigned long source_pos, unsigned long len)
{
	return this->insert( dest_pos, CharacterSequence(data), source_pos, len );
}

long CharacterSequence::compare(const ASAAC_CharacterSequence data) const
{
	return strcmp( m_Data, CharacterSequence(data).c_str() );
}

long CharacterSequence::compare(const CharacterSequence &data) const
{
	return this->compare( data.asaac_str() );
}

long CharacterSequence::compare(const char *data) const
{
	return this->compare( CharacterSequence(data) );
}

long CharacterSequence::compare(const string &data) const
{
	return this->compare( CharacterSequence(data) );
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
	return contains( CharacterSequence(data) );
}

unsigned long CharacterSequence::contains(const char data, bool cs) const
{
	return contains( CharacterSequence(1, data) );
}

unsigned long CharacterSequence::contains(const string &data, bool cs) const
{
	return contains( CharacterSequence(data) );
}

unsigned long CharacterSequence::contains(const ASAAC_CharacterSequence data, bool cs) const
{
	return contains( CharacterSequence(data) );
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
	return find( CharacterSequence(data), cs, begin_pos );
}

long CharacterSequence::find(const char data, bool cs, unsigned long begin_pos ) const
{
	return find( CharacterSequence(1, data), cs, begin_pos );
}

long CharacterSequence::find(const string &data, bool cs, unsigned long begin_pos ) const
{
	return find( CharacterSequence(data), cs, begin_pos );
}

long CharacterSequence::find(const ASAAC_CharacterSequence data, bool cs, unsigned long begin_pos ) const
{
	return find( CharacterSequence(data), cs, begin_pos );
}

void CharacterSequence::convertTo( ASAAC_CharacterSequence &data, unsigned long begin_pos, unsigned long len ) const
{
	unsigned long cpy_size = min( m_Size, len );
	
	if ( cpy_size > 0 )	
		memcpy( data.data, m_Data + begin_pos, cpy_size );
		
	data.size = cpy_size;
}

void CharacterSequence::convertTo( CharacterSequence &data, unsigned long begin_pos, unsigned long len ) const
{
	data.assign( *this, begin_pos, len); 
}

void CharacterSequence::convertTo( char *data, unsigned long begin_pos, unsigned long len ) const
{
	unsigned long cpy_size = min( m_Size, len );
	
	if ( len > 0 )
	{	
		memcpy( data, m_Data + begin_pos, cpy_size );
		data[cpy_size] = 0;
	}
}

void CharacterSequence::convertTo( string &data, unsigned long begin_pos, unsigned long len ) const
{
	data = m_Data[0];
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
	m_Size = 0;
	m_Data[ m_Size ] = 0;
	
	return *this;
}

CharacterSequence & CharacterSequence::erase(unsigned long pos)
{
	return this->erase(pos, pos);
}
	
CharacterSequence & CharacterSequence::erase(unsigned long start, unsigned long end)
{
	if (start < m_Size)
	{
		if (end >= start)
		{
			end = min( end, m_Size ) + 1;
			unsigned long move_size = end-start;
			 
			memmove( m_Data + start, m_Data + end, move_size );
			
			m_Size -= move_size;
		}
	}
	
	checkIntegrity();	
	
	return *this;
}	

const char *CharacterSequence::c_str( unsigned long begin_pos, unsigned long len ) const
{
	return m_Data;
}

const ASAAC_CharacterSequence CharacterSequence::asaac_str( unsigned long begin_pos, unsigned long len ) const
{
	ASAAC_CharacterSequence Result;
	this->convertTo(Result, begin_pos, len);
	
	return Result;
}

const string CharacterSequence::cpp_str( unsigned long begin_pos, unsigned long len ) const
{
	string Result;
	this->convertTo(Result, begin_pos, len);
	
	return Result;
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
	return m_Size;
}

unsigned long CharacterSequence::length() const
{
	return m_Size;
}

const char *CharacterSequence::data() const
{
	return m_Data;
}

CharacterSequence & CharacterSequence::operator=(const CharacterSequence &data)
{
	return this->assign(data);
}

CharacterSequence & CharacterSequence::operator=(const char *data)
{
	return this->assign(data);
}

CharacterSequence & CharacterSequence::operator=(const string &data)
{
	return this->assign(data);
}

CharacterSequence & CharacterSequence::operator=(const ASAAC_CharacterSequence &data)
{
	return this->assign(data);
}

CharacterSequence & CharacterSequence::operator=(long number)
{
	return this->assign(number);
}

CharacterSequence & CharacterSequence::operator=(unsigned long number)
{
	return this->assign(number);
}

CharacterSequence & CharacterSequence::operator<<(const CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(const char *data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(const string &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(const ASAAC_CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(long number)
{
	return this->append(number);
}

CharacterSequence & CharacterSequence::operator<<(ASAAC_Time &time)
{
	return this->append(time);
}

CharacterSequence & CharacterSequence::operator<<(ASAAC_TimeInterval &interval)
{
	return this->append(interval);
}

CharacterSequence & CharacterSequence::operator+=(const CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator+=(const char *data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator+=(const string &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator+=(const ASAAC_CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator+=(long number)
{
	return this->append(number);
}

CharacterSequence & CharacterSequence::operator+=(ASAAC_PublicId number)
{
	return this->append(number);
}

CharacterSequence  CharacterSequence::operator+(const CharacterSequence &data)
{
	CharacterSequence cs(*this);
	return cs.append(data);
}

CharacterSequence  CharacterSequence::operator+(const char *data)
{
	CharacterSequence cs(*this);
	return cs.append(data);
}

CharacterSequence  CharacterSequence::operator+(const string &data)
{
	CharacterSequence cs(*this);
	return cs.append(data);
}

CharacterSequence  CharacterSequence::operator+(const ASAAC_CharacterSequence &data)
{
	CharacterSequence cs(*this);
	return cs.append(data);
}

CharacterSequence  CharacterSequence::operator+(long number)
{
	CharacterSequence cs(*this);
	return cs.append(number);
}

CharacterSequence  CharacterSequence::operator+(ASAAC_PublicId number)
{
	CharacterSequence cs(*this);
	return cs.append(number);
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
	if (Index >= m_Size)
		OSException("Index is out of range", LOCATION);
	
	return m_Data[Index];
}

CharacterSequence CharacterSequence::LineBreak()
{
	CharacterSequence Result;
	
	Result.m_Size = 1;
	Result.m_Data[0] = 10;
	Result.m_Data[1] = 0;
	
	return Result;
}

ostream & operator<<(ostream &stream, CharacterSequence seq)
{
	stream << seq.m_Data;
	
	return stream;
}

istream & operator>>(istream &stream, CharacterSequence &seq)
{
	stream >> seq.m_Data;
	
	return stream;
}

void CharacterSequence::checkIntegrity()
{
	if (m_Size > ASAAC_OS_MAX_STRING_SIZE)
		throw FatalException("Data structure is corrupted", LOCATION);

	if (m_Data[ m_Size ] != 0)
		throw FatalException("Data structure is corrupted", LOCATION);
}

bool CharacterSequence::checkAsaacString(const ASAAC_CharacterSequence data)
{
	if (data.size > ASAAC_OS_MAX_STRING_SIZE)
		throw FatalException("Data structure is corrupted", LOCATION);

	return true;	
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

