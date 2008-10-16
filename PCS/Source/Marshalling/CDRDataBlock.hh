#ifndef CDRDATABLOCK_HH_
#define CDRDATABLOCK_HH_

#include "PcsHIncludes.hh"

class CDRDataBlock
{
public:
	CDRDataBlock( ASAAC_Address Data, unsigned long Size );
	
	unsigned long getCurrentIndex();
	
	void rewind();
	bool seek( unsigned long Index );

	char		getNextChar();
	bool		setNextChar( char Value );
	
	short		getNextShort();
	bool		setNextShort( short Value );
	
	long		getNextLong();
	bool		setNextLong( long Value );
	
	long long	getNextLongLong();
	bool		setNextLongLong( long long Value );
	
	float		getNextFloat();
	bool		setNextFloat( float Value );
	
	double		getNextDouble();
	bool		setNextDouble( double Value );
	
	long double	getNextLongDouble();
	bool		setNextLongDouble( long double Value );
	
	unsigned long 	getNextEnum();
	bool			setNextEnum( unsigned long Value );
	
	virtual ~CDRDataBlock();
	
private:

	bool		jumpToAlign( size_t Alignment );

	unsigned long			m_CurrentIndex;
	
	bool					m_LittleEndian;
	
	char* 					m_Data;
	unsigned long			m_Size;

};

#endif /*CDRDATABLOCK_HH_*/
