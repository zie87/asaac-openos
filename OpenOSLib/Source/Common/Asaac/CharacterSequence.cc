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

CharacterSequence::CharacterSequence(const unsigned long len, const char ch)
{
	erase();
	this->assign(len, ch);
}

CharacterSequence::CharacterSequence(const ASAAC_PublicId number, const NumberRepresentation representation )
{
	erase();
	this->assign(number, representation);
}

CharacterSequence::CharacterSequence(const ASAAC_Time time)
{
	erase();
	this->assign(time);
}

CharacterSequence::CharacterSequence(const ASAAC_TimeInterval interval)
{
	erase();
	this->assign(interval);
}

CharacterSequence::CharacterSequence(const ASAAC_ReturnStatus status)
{
    erase();
    this->assign(status);
}

CharacterSequence::CharacterSequence(const ASAAC_TimedReturnStatus status)
{
    erase();
    this->assign(status);
}

CharacterSequence::CharacterSequence(const ASAAC_ResourceReturnStatus status)
{
    erase();
    this->assign(status);
}

CharacterSequence::CharacterSequence(const ASAAC_ProcessDescription &process_desc )
{
    erase();
    this->assign(process_desc);
}

CharacterSequence::CharacterSequence(const ASAAC_ThreadDescription &thread_desc )
{
    erase();
    this->assign(thread_desc);
}

CharacterSequence::CharacterSequence(const ASAAC_ThreadSchedulingInfo &thread_scheduling_info )
{
    erase();
    this->assign(thread_scheduling_info);
}

CharacterSequence::CharacterSequence(const ASAAC_ThreadStatus &thread_status )
{
    erase();
    this->assign(thread_status);
}

CharacterSequence::CharacterSequence(const ASAAC_VcDescription &vc_desc )
{
    erase();
    this->assign(vc_desc);
}

CharacterSequence::CharacterSequence(const ASAAC_VcMappingDescription &vc_mapping )
{
    erase();
    this->assign(vc_mapping);
}

CharacterSequence::CharacterSequence(const ASAAC_VcToTcMappingDescription &vc_to_tc_mapping )
{
    erase();
    this->assign(vc_to_tc_mapping);
}

CharacterSequence::CharacterSequence(const ASAAC_SecurityRating &security_rating)
{
    erase();
    this->assign(security_rating);
}

CharacterSequence::CharacterSequence(const ASAAC_SecurityInfo &security_info)
{
    erase();
    this->assign(security_info);
}

CharacterSequence::CharacterSequence(const ASAAC_VirtualChannelType &vc_type)
{
    erase();
    this->assign(vc_type);
}

CharacterSequence::CharacterSequence(const ASAAC_Bool &boolean)
{
    erase();
    this->assign(boolean);
}

CharacterSequence::CharacterSequence(const ASAAC_ClassificationLevel &classification_level)
{
    erase();
    this->assign(classification_level);
}

CharacterSequence::CharacterSequence(const ASAAC_Category &security_category)
{
    erase();
    this->assign(security_category);
}

CharacterSequence::CharacterSequence(const ASAAC_InterfaceData &if_config)
{
    erase();
    this->assign(if_config);
}

CharacterSequence::CharacterSequence(const ASAAC_TcDescription &tc_desc)
{
    erase();
    this->assign(tc_desc);
}

CharacterSequence::CharacterSequence(const ASAAC_NetworkDescriptor &network_desc)
{
    erase();
    this->assign(network_desc);
}

CharacterSequence::CharacterSequence(const ASAAC_NetworkPortStatus &network_status)
{
    erase();
    this->assign(network_status);
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

CharacterSequence & CharacterSequence::append( const unsigned long len, const char ch )
{
	return this->append( CharacterSequence(len, ch) );
}

CharacterSequence & CharacterSequence::append( const ASAAC_PublicId number, const NumberRepresentation representation )
{
	return this->append(CharacterSequence(number, representation));
}

CharacterSequence & CharacterSequence::append( const ASAAC_Time &time)
{
	return this->append(CharacterSequence(time));
}

CharacterSequence & CharacterSequence::append( const ASAAC_TimeInterval &interval)
{
	return this->append(CharacterSequence(interval));
}

CharacterSequence & CharacterSequence::appendLineBreak()
{
	return this->append(LineBreak());
}

CharacterSequence & CharacterSequence::assign( const ASAAC_CharacterSequence& data, const unsigned long begin_pos, const unsigned long len )
{	
	checkAsaacString( data );
	
	if (begin_pos < data.size)
	{	
		unsigned long Len = len;
		if (Len > (Unlimited-begin_pos))
			Len = data.size - begin_pos;
		else
		{
			if (Len + begin_pos > data.size)
				Len = data.size - begin_pos;
		}
	
		memcpy( m_Data, data.data + begin_pos, Len );
	
		m_Size = Len;
		m_Data[m_Size] = 0;
	}
	else
	{
		erase();
	}

	checkIntegrity();
	
	return *this;
}

CharacterSequence & CharacterSequence::assign( const CharacterSequence &data, const unsigned long begin_pos, const unsigned long len )
{
	return this->assign( data.asaac_str(), begin_pos, len);
}

CharacterSequence & CharacterSequence::assign( const char *data, const unsigned long begin_pos, const unsigned long len )
{
	if (checkCharString(data) == false)
		return *this;	

	ASAAC_CharacterSequence Seq;
	
	Seq.size = min( strlen(data), ASAAC_OS_MAX_STRING_SIZE );
	memcpy( Seq.data, data, Seq.size );

	checkIntegrity();
	
	return this->assign( Seq, begin_pos, len );
}

CharacterSequence & CharacterSequence::assign( const unsigned long len, const char ch )
{
    unsigned long Len = len;
	
	if (Len > ASAAC_OS_MAX_STRING_SIZE)
		Len = ASAAC_OS_MAX_STRING_SIZE;

	ASAAC_CharacterSequence Seq;
	
	for (unsigned long i=0; i<Len; i++)
		Seq.data[i] = ch;
		
	Seq.size = Len;

	checkIntegrity();
		
	return this->assign( Seq );
}

CharacterSequence & CharacterSequence::assign( const ASAAC_PublicId number, const NumberRepresentation representation )
{
	switch (representation)
	{
		case binary: throw OSException("Representation type 'binary' is not supported", LOCATION); break;
		case octal: snprintf( m_Data, sizeof(m_Data), "%#lo", number ); break;
		case unsigned_decimal: snprintf( m_Data, sizeof(m_Data), "%lu", number ); break;
		case signed_decimal: snprintf( m_Data, sizeof(m_Data), "%li", number ); break;
		case hexadecimal: snprintf( m_Data, sizeof(m_Data), "%#lx", number ); break;
		default: throw OSException("Representation type is invalid", LOCATION);
	}

    m_Size = strlen( m_Data );
	m_Data[m_Size] = 0;

	checkIntegrity();
    
	return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_Time time)
{
    erase();
    
	tm t = TimeStamp(time).tm_Time();
	*this << (long)(t.tm_mon + 1) << "/" << (long)t.tm_mday << "/" <<  (long)(1900 + t.tm_year) << " " <<  (long)t.tm_hour << ":" <<  (long)t.tm_min << ":" <<  (long)t.tm_sec << "." << (long)div(time.nsec, (long)100000).quot;
	
	return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_TimeInterval interval)
{
	TimeInterval Interval = interval;

	//TODO: find out how to print interval
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ReturnStatus status)
{
    erase();

    switch (status)
    {
        case ASAAC_SUCCESS: *this << "ASAAC_SUCCESS"; break; 
        case ASAAC_ERROR: *this << "ASAAC_ERROR"; break;
        default: *this << "NULL";
    }
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_TimedReturnStatus status)
{
    erase();

    switch (status)
    {
        case ASAAC_TM_SUCCESS: *this << "ASAAC_TM_SUCCESS"; break; 
        case ASAAC_TM_ERROR: *this << "ASAAC_TM_ERROR"; break;
        case ASAAC_TM_TIMEOUT: *this << "ASAAC_TM_TIMEOUT"; break;
        default: *this << "NULL";
    }
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ResourceReturnStatus status)
{
    erase();

    switch (status)
    {
        case ASAAC_RS_SUCCESS: *this << "ASAAC_RS_SUCCESS"; break; 
        case ASAAC_RS_ERROR: *this << "ASAAC_RS_ERROR"; break;
        case ASAAC_RS_RESOURCE: *this << "ASAAC_RS_RESOURCE"; break;
        default: *this << "NULL";
    }
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ProcessDescription &process_desc )
{
    erase();

    *this << "{";
    *this << "programme_file_name: " << CharSeq(process_desc.programme_file_name); 
    *this << "}";
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ThreadDescription &thread_desc )
{
    erase();

    *this << "{"; 
    *this << "global_pid: " << CharSeq(thread_desc.global_pid) << ", ";
    *this << "thread_id: " << CharSeq(thread_desc.thread_id) << ", ";
    *this << "entry_point: " << CharSeq(thread_desc.entry_point); 
    *this << "}";
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ThreadSchedulingInfo &thread_scheduling_info )
{
    erase();

    *this << "{";
    *this << "global_pid: " << CharSeq(thread_scheduling_info.global_pid) << ", "; 
    *this << "thread_id: " << CharSeq(thread_scheduling_info.thread_id);
    *this << "}";
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ThreadStatus &thread_status )
{
    erase();

    switch (thread_status)
    {
        case ASAAC_DORMANT: *this << "ASAAC_DORMANT"; break; 
        case ASAAC_READY: *this << "ASAAC_DORMANT"; break;
        case ASAAC_WAITING: *this << "ASAAC_DORMANT"; break;
        case ASAAC_RUNNING: *this << "ASAAC_DORMANT"; break;
        default: *this << "NULL";
    }
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_VcDescription &vc_desc )
{
    erase();

    *this << "{";
    *this << "global_vc_id: " << CharSeq(vc_desc.global_vc_id) << ", ";
    *this << "max_msg_length: " << CharSeq(vc_desc.max_msg_length) << ", ";
    *this << "max_number_of_buffers: " << CharSeq(vc_desc.max_number_of_buffers) << ", ";
    *this << "max_number_of_threads_attached: " << CharSeq(vc_desc.max_number_of_threads_attached) << ", ";
    *this << "max_number_of_TCs_attached: " << CharSeq(vc_desc.max_number_of_TCs_attached) << ", ";
    *this << "security_rating: " << CharSeq(vc_desc.security_rating) << ", ";
    *this << "security_info: " << CharSeq(vc_desc.security_info) << ", ";
    *this << "vc_type: " << CharSeq(vc_desc.vc_type) << ", ";
    *this << "cpu_id: " << CharSeq(vc_desc.cpu_id) << ", ";
    *this << "is_typed_message: " << CharSeq(vc_desc.is_typed_message);
    *this << "}";
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_VcMappingDescription &vc_mapping )
{
    erase();

    *this << "{";
    *this << "global_pid: " << CharSeq(vc_mapping.global_pid) << ", ";
    *this << "local_vc_id: " << CharSeq(vc_mapping.local_vc_id) << ", ";
    *this << "global_vc_id: " << CharSeq(vc_mapping.global_vc_id) << ", ";
    *this << "local_thread_id: " << CharSeq(vc_mapping.local_thread_id) << ", ";
    *this << "buffer_size: " << CharSeq(vc_mapping.buffer_size) << ", ";
    *this << "number_of_message_buffers: " << CharSeq(vc_mapping.number_of_message_buffers) << ", ";
    *this << "is_reading: " << CharSeq(vc_mapping.is_reading) << ", ";
    *this << "is_lifo_queue: " << CharSeq(vc_mapping.is_lifo_queue) << ", ";
    *this << "is_refusing_queue: " << CharSeq(vc_mapping.is_refusing_queue) << ", ";
    *this << "Priority: " << CharSeq(vc_mapping.Priority);
    *this << "}";
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_VcToTcMappingDescription &vc_to_tc_mapping )
{
    erase();

    *this << "{"; 
    *this << "global_vc_id: " << CharSeq(vc_to_tc_mapping.global_vc_id) << ", ";
    *this << "tc_id: " << CharSeq(vc_to_tc_mapping.tc_id) << ", ";
    *this << "is_data_representation: " << CharSeq(vc_to_tc_mapping.is_data_representation);
    *this << "}";
  
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_SecurityRating &security_rating )
{
    erase();

    *this << "{";
    *this << "classification_level: " << CharSeq(security_rating.classification_level) << ", "; 
    *this << "security_category: " << CharSeq(security_rating.security_category);
    *this << "}";
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_SecurityInfo &security_info )
{
    erase();

    switch (security_info)
    {
        case ASAAC_Marked: *this << "ASAAC_Marked"; break;
        case ASAAC_Unmarked: *this << "ASAAC_Unmarked"; break;
        default: *this << "NULL";
    }
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_VirtualChannelType &vc_type )
{
    erase();

    switch (vc_type)
    {
        case ASAAC_Application_Header_VC: *this << "ASAAC_Application_Header_VC"; break;
        case ASAAC_Application_Raw_VC: *this << "ASAAC_Application_Raw_VC"; break;
        case ASAAC_OLI_VC: *this << "ASAAC_OLI_VC"; break;
        default: *this << "NULL";
    }
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_Bool &boolean )
{
    erase();

    switch (boolean)
    {
        case ASAAC_BOOL_FALSE: *this << "ASAAC_BOOL_FALSE"; break;
        case ASAAC_BOOL_TRUE: *this << "ASAAC_BOOL_TRUE"; break;
        default: *this << "NULL";
    }
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_ClassificationLevel &classification_level)
{
    erase();

    switch (classification_level)
    {
        case ASAAC_UNCLASSIFIED: *this << "ASAAC_UNCLASSIFIED"; break;
        case ASAAC_CONFIDENTIAL: *this << "ASAAC_CONFIDENTIAL"; break;
        case ASAAC_SECRET: *this << "ASAAC_SECRET"; break;
        case ASAAC_TOP_SECRET: *this << "ASAAC_TOP_SECRET"; break;
        default: *this << "NULL";
    }
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_Category &security_category)
{
    erase();

    switch (security_category)
    {
        case ASAAC_LEVEL_1: *this << "ASAAC_LEVEL_1"; break;
        case ASAAC_LEVEL_2: *this << "ASAAC_LEVEL_2"; break;
        case ASAAC_LEVEL_3: *this << "ASAAC_LEVEL_3"; break;
        default: *this << "NULL";
    }
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_InterfaceData &if_config )
{
    erase();

    *this << "{";
    *this << "if_id :" << CharSeq(if_config.if_id) << ", ";
    *this << "nw_id :" << CharSeq(if_config.nw_id) << ", ";
    *this << "cpu_id :" << CharSeq(if_config.cpu_id) << ", ";
    *this << "conf_data_type :" << CharSeq(if_config.conf_data_type) << ", ";
    *this << "conf_data_size :" << CharSeq(if_config.conf_data_size) << ", ";
    //*this << "conf_data :" << CharSeq(if_config.conf_data);
    *this << "}";
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_TcDescription &tc_desc )
{
    erase();

    *this << "{";
    *this << "tc_id :" << CharSeq(tc_desc.tc_id) << ", ";
    *this << "network_descr :" << CharSeq(tc_desc.network_descr) << ", ";
    *this << "is_receiver :" << CharSeq(tc_desc.is_receiver) << ", ";
    *this << "is_msg_transfer :" << CharSeq(tc_desc.is_msg_transfer) << ", ";
    *this << "is_fragmented :" << CharSeq(tc_desc.is_fragmented) << ", ";
    *this << "security_rating :" << CharSeq(tc_desc.security_rating) << ", ";
    *this << "cpu_id :" << CharSeq(tc_desc.cpu_id) << ", ";
    *this << "conf_data_type :" << CharSeq(tc_desc.conf_data_type) << ", ";
    *this << "conf_data_size :" << CharSeq(tc_desc.conf_data_size) << ", ";
    //*this << "conf_data :" << CharSeq(tc_desc.conf_data);
    *this << "}";
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_NetworkDescriptor &network_desc )
{
    erase();

    *this << "{";
    *this << "network :" << CharSeq(network_desc.network) << ", ";
    *this << "port :" << CharSeq(network_desc.port);
    *this << "}";
    
    return *this;
}

CharacterSequence & CharacterSequence::assign( const ASAAC_NetworkPortStatus &network_status )
{
    erase();

    *this << "{";
    *this << "final_status :" << CharSeq(network_status.final_status) << ", ";
    *this << "status_data_length :" << CharSeq(network_status.status_data_length) << ", ";
    //*this << "detailed_status_data[256] :" << CharSeq(network_status.detailed_status_data);
    *this << "}";
    
    return *this;
}

CharacterSequence & CharacterSequence::insert( const unsigned long pos, const unsigned long len, const char ch )
{
	return this->insert(pos, CharacterSequence(len, ch) );
}

CharacterSequence & CharacterSequence::insert( const unsigned long pos, const ASAAC_PublicId number, const NumberRepresentation representation )
{
	return this->insert( pos, CharacterSequence(number) );
}

CharacterSequence & CharacterSequence::insert( const unsigned long dest_pos, const ASAAC_CharacterSequence &data, const unsigned long source_pos, const unsigned long len)
{
	checkAsaacString( data );

	if (dest_pos < ASAAC_OS_MAX_STRING_SIZE )
	{
		if (source_pos < data.size)
		{
			unsigned long Len = min( len, ASAAC_OS_MAX_STRING_SIZE - source_pos );
			
			if (Len + dest_pos < ASAAC_OS_MAX_STRING_SIZE)
			{
				unsigned long move_size = min(Len, ASAAC_OS_MAX_STRING_SIZE - dest_pos - Len); 	
				memmove( m_Data + dest_pos+Len, m_Data + dest_pos, move_size );
			}
			
			unsigned long cpy_size = min( Len, ASAAC_OS_MAX_STRING_SIZE - dest_pos );  	
			memcpy( m_Data + dest_pos, data.data, cpy_size );
			
			m_Size = min( m_Size+Len, ASAAC_OS_MAX_STRING_SIZE );
			m_Data[ m_Size ] = 0;
		}
	}

	checkIntegrity();
	
	return *this;	
}

CharacterSequence & CharacterSequence::insert( const unsigned long dest_pos, const CharacterSequence &data, const unsigned long source_pos, const unsigned long len)
{
	return this->insert( dest_pos, data.asaac_str(), source_pos, len);
}

CharacterSequence & CharacterSequence::insert( const unsigned long dest_pos, const char *data, const unsigned long source_pos, const unsigned long len)
{
	return this->insert( dest_pos, CharacterSequence(data), source_pos, len );
}

long CharacterSequence::compare(const ASAAC_CharacterSequence &data) const
{
	return this->compare( CharacterSequence(data) );
}

long CharacterSequence::compare(const CharacterSequence &data) const
{
	return strcmp( m_Data, data.m_Data );
}

long CharacterSequence::compare(const char *data) const
{
	return this->compare( CharacterSequence(data) );
}

unsigned long CharacterSequence::contains(const CharacterSequence &data, const bool cs) const
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

unsigned long CharacterSequence::contains(const char *data, const bool cs) const
{
	return contains( CharacterSequence(data) );
}

unsigned long CharacterSequence::contains(const char data, const bool cs) const
{
	return contains( CharacterSequence(1, data) );
}

unsigned long CharacterSequence::contains(const ASAAC_CharacterSequence &data, const bool cs) const
{
	return contains( CharacterSequence(data) );
}

long CharacterSequence::find( const CharacterSequence &data, bool cs, const unsigned long begin_pos ) const
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

long CharacterSequence::find(const char *data, bool cs, const unsigned long begin_pos ) const
{
	return find( CharacterSequence(data), cs, begin_pos );
}

long CharacterSequence::find(const char data, bool cs, const unsigned long begin_pos ) const
{
	return find( CharacterSequence(1, data), cs, begin_pos );
}

long CharacterSequence::find(const ASAAC_CharacterSequence &data, const bool cs, const unsigned long begin_pos ) const
{
	return find( CharacterSequence(data), cs, begin_pos );
}

void CharacterSequence::convertTo( ASAAC_CharacterSequence &data, const unsigned long begin_pos, const unsigned long len ) const
{
	unsigned long cpy_size = min( m_Size, len );
	
	if ( cpy_size > 0 )	
		memcpy( data.data, m_Data + begin_pos, cpy_size );
		
	data.size = cpy_size;
}

void CharacterSequence::convertTo( CharacterSequence &data, const unsigned long begin_pos, const unsigned long len ) const
{
	data.assign( *this, begin_pos, len); 
}

void CharacterSequence::convertTo( char *data, const unsigned long begin_pos, const unsigned long len ) const
{
	unsigned long cpy_size = min( m_Size, len );
	
	if ( len > 0 )
	{	
		memcpy( data, m_Data + begin_pos, cpy_size );
		data[cpy_size] = 0;
	}
}

void CharacterSequence::convertTo( long &number, const unsigned long begin_pos, const unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	number = Data.c_int();
}

void CharacterSequence::convertTo( ASAAC_PublicId &number, const unsigned long begin_pos, const unsigned long len ) const
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

CharacterSequence & CharacterSequence::erase(const unsigned long pos)
{
	return this->erase(pos, pos);
}
	
CharacterSequence & CharacterSequence::erase(const unsigned long start, const unsigned long end)
{
	if (start < m_Size)
	{
		if (end >= start)
		{
			unsigned long End = end;
			
			End = min( End, m_Size ) + 1;
			unsigned long move_size = End-start;
			 
			memmove( m_Data + start, m_Data + End, move_size );
			
			m_Size -= move_size;
			m_Data[ m_Size ] = 0;
		}
	}
	
	checkIntegrity();	
	
	return *this;
}	

static char Buffer[ASAAC_OS_MAX_STRING_SIZE + 1];

const char *CharacterSequence::c_str( const unsigned long begin_pos, const unsigned long len ) const
{
	if ((begin_pos == 0) && (len >= m_Size))
		return m_Data;
		
	convertTo( Buffer, begin_pos, len );
	
	return Buffer;
}

ASAAC_CharacterSequence CharacterSequence::asaac_str( const unsigned long begin_pos, const unsigned long len ) const
{
	ASAAC_CharacterSequence Result;
	this->convertTo(Result, begin_pos, len);
	
	return Result;
}

int CharacterSequence::c_int( const unsigned long begin_pos, const unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtol(Data.c_str(), &e, 10);
}

long CharacterSequence::c_long( const unsigned long begin_pos, const unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtol(Data.c_str(), &e, 10);
}

unsigned int CharacterSequence::c_uint( const unsigned long begin_pos, const unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtoul(Data.c_str(), &e, 10);
}

unsigned long CharacterSequence::c_ulong( const unsigned long begin_pos, const unsigned long len ) const
{
	CharacterSequence Data;
	this->convertTo(Data, begin_pos, len);

	char *e;
	return strtoul(Data.c_str(), &e, 10);
}

ASAAC_PublicId CharacterSequence::asaac_id( const unsigned long begin_pos, const unsigned long len ) const
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

CharacterSequence & CharacterSequence::operator=(const char data)
{
	return this->assign(1,data);
}

CharacterSequence & CharacterSequence::operator=(const ASAAC_CharacterSequence &data)
{
	return this->assign(data);
}

CharacterSequence & CharacterSequence::operator=(const long number)
{
	return this->assign(number);
}

CharacterSequence & CharacterSequence::operator=(const ASAAC_PublicId number)
{
	return this->assign(number);
}

CharacterSequence & CharacterSequence::operator=(const ASAAC_Time &time)
{
	return this->assign(time);
}

CharacterSequence & CharacterSequence::operator=(const ASAAC_TimeInterval &interval)
{
	return this->assign(interval);
}

CharacterSequence & CharacterSequence::operator<<(const CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(const char *data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(const char data)
{
	return this->append(1,data);
}

CharacterSequence & CharacterSequence::operator<<(const ASAAC_CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator<<(const long number)
{
	return this->append(number);
}

CharacterSequence & CharacterSequence::operator<<(const ASAAC_PublicId number)
{
	return this->append(number);
}

CharacterSequence & CharacterSequence::operator<<(const ASAAC_Time &time)
{
	return this->append(time);
}

CharacterSequence & CharacterSequence::operator<<(const ASAAC_TimeInterval &interval)
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

CharacterSequence & CharacterSequence::operator+=(const char data)
{
	return this->append(1,data);
}

CharacterSequence & CharacterSequence::operator+=(const ASAAC_CharacterSequence &data)
{
	return this->append(data);
}

CharacterSequence & CharacterSequence::operator+=(const long number)
{
	return this->append(number);
}

CharacterSequence & CharacterSequence::operator+=(const ASAAC_PublicId number)
{
	return this->append(number);
}

CharacterSequence & CharacterSequence::operator+=(const ASAAC_Time &time)
{
	return this->append(time);
}

CharacterSequence & CharacterSequence::operator+=(const ASAAC_TimeInterval &interval)
{
	return this->append(interval);
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

CharacterSequence  CharacterSequence::operator+(const char data)
{
	CharacterSequence cs(*this);
	return cs.append(1,data);
}

CharacterSequence  CharacterSequence::operator+(const ASAAC_CharacterSequence &data)
{
	CharacterSequence cs(*this);
	return cs.append(data);
}

CharacterSequence  CharacterSequence::operator+(const long number)
{
	CharacterSequence cs(*this);
	return cs.append(number);
}

CharacterSequence  CharacterSequence::operator+(const ASAAC_PublicId number)
{
	CharacterSequence cs(*this);
	return cs.append(number);
}

CharacterSequence CharacterSequence::operator+(const ASAAC_Time &time)
{
	CharacterSequence cs(*this);
	return cs.append(time);
}

CharacterSequence CharacterSequence::operator+(const ASAAC_TimeInterval &interval)
{
	CharacterSequence cs(*this);
	return cs.append(interval);
}

bool CharacterSequence::operator==(const CharacterSequence &data) const
{
	return (this->compare(data) == 0);
}

bool CharacterSequence::operator==(const char *data) const
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

bool CharacterSequence::operator!=(const ASAAC_CharacterSequence &data) const
{
	return (this->compare(data) != 0);
}

char CharacterSequence::operator[](const unsigned long Index) const
{
	CharacterSequence error_string;
	
	
	if (Index >= m_Size)
		throw OSException((error_string<< "Index is out of range (" << Index << ")").c_str(), LOCATION);
	
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

bool CharacterSequence::checkAsaacString(const ASAAC_CharacterSequence &data)
{
	if (data.size > ASAAC_OS_MAX_STRING_SIZE)
		throw FatalException("Data structure is corrupted", LOCATION);

	return true;	
}

bool CharacterSequence::checkCharString( const char * data )
{
	if (data != NULL) return true;
	return false;
}

bool CharacterSequence::compareChar(const char c1, const char c2, bool cs)
{
	char C1 = c1;
	char C2 = c2;
	
	if ( cs = false )
	{
		if ((C1 > 0x40) && (C1 < 0x5b))
			C1 += 0x20;

		if ((C2 > 0x40) && (C2 < 0x5b))
			C2 += 0x20;			
	}
	
	return (C1 == C2);
}

