#include "FileManager.hh"

#include "Exceptions/Exceptions.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "Managers/AllocatorManager.hh"
#include "Managers/PrivateIdManager.hh"
#include "IPC/BlockingScope.hh"

#include "OpenOSObject.hh"


//***********************************************************************************************
//* Administrative functions                                                                    *
//***********************************************************************************************

FileManager::FileManager()
{
    m_IsInitialized = false;
    
    initialize();
}


size_t FileManager::predictSize()
{
	size_t CumulativeSize = 0;
	
	// m_InfoDataList
	CumulativeSize +=  FileInfoList::predictSize(OS_MAX_NUMBER_OF_LOCAL_FILES);
	
	CumulativeSize += Semaphore::predictSize();
	
	return CumulativeSize;
}


void FileManager::initialize()
{
    if (m_IsInitialized)
        throw DoubleInitializationException(LOCATION);

    try
    {
        m_IsInitialized = true;

        m_Allocator.initialize( predictSize() );
                
        m_FileInfoList.initialize( &m_Allocator, true, OS_MAX_NUMBER_OF_LOCAL_FILES );
        
        m_Semaphore.initialize( &m_Allocator, true );
        
        readFileInfoListFromEnvironment();                
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error initializing FileManager", LOCATION);
        
        deinitialize();
        
        throw;
    }
    
}


void FileManager::deinitialize( const bool do_throw )
{
    if (m_IsInitialized == false)
        return;
    
    m_IsInitialized = false;

    try
    {
        writeFileInfoListToEnvironment();

        m_Semaphore.deinitialize();
        
        m_FileInfoList.deinitialize();
        
        m_Allocator.deinitialize();
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error while deinitializing FileManager", LOCATION);

        e.raiseError( do_throw );
    }    
}


FileManager::~FileManager()
{
	deinitialize();
}


FileManager* FileManager::getInstance()
{
	static FileManager StaticFileManager;
	
	return &StaticFileManager;
}


//***********************************************************************************************
//* Public APOS function                                                                        *
//***********************************************************************************************

void FileManager::executeFile( const ASAAC_CharacterSequence name, const ProcessAlias alias )
{
	try
	{
    	CharacterSequence ErrorString;

    	// For APOS processes drop all privileges. set uid and gid == nobody
	    if ( alias == PROC_APOS )
	    {
	    	// It is important here first to set the group id and then the user id
	    	
	        if (setgid( 65534 ) == -1)
	        	OSException( (ErrorString << "setgid: " << strerror(errno)).c_str(), LOCATION ).raiseError();

	        ErrorString.erase();
	        
	        if (setuid( 65534 ) == -1)
	        	OSException( (ErrorString << "setuid: " << strerror(errno)).c_str(), LOCATION ).raiseError();
	    } 
	    
	    deinitialize( true );
	    
	    //deallocate objects, because after execve all objects will be lost
	    AllocatorManager::getInstance()->deallocateAllObjects();

	    //Now load and execute the file
	    execve( CharSeq(name).c_str(), 0, environ);
	
	    //if the last call returned, an error occured
	    OSException e( (ErrorString << "execve: " << strerror(errno)).c_str(), LOCATION );

		//reallocate all objects, because execve failed.
	    AllocatorManager::getInstance()->reallocateAllObjects();
	    	    
	    try
	    {
	    	initialize();
	    }
	    catch ( ASAAC_Exception &e2 )
	    {
	    	//TODO: add e2 messages to e
	    	e.addPath("Reinitialization of FileManager failed.", LOCATION);
	    }
	    
	    throw e;
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error executing file", LOCATION);
		
		throw;
	}
}


void FileManager::createDirectory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access)
{
    try 
    {
	    CharSeq Name = name;
        
        //TODO: check if dir exists, then throw a recource exception

	    if (mkdir(Name.c_str(), AccessRightsToMode( access )) == -1)
             throw OSException( strerror(errno), LOCATION );             
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error creating directory", LOCATION);
        
        throw;
    }
}


void FileManager::deleteDirectory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
{
    try 
    {
        CharSeq Name = name;
        
        if (remove(Name.c_str()) == -1)
             throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error deleting directory", LOCATION);
        
        throw;
    }
}


void FileManager::createFile(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size)
{
    int Handle = -1;
    CharSeq Name = name;
    
    try 
    {
        
        //TODO: check if file exists, then throw a recource exception

        Handle = creat(Name.c_str(), AccessRightsToMode( access ));
        
        if (Handle == -1)
             throw OSException( strerror(errno), LOCATION );
             
        if (ftruncate( Handle, file_size ) == -1)
             throw OSException( strerror(errno), LOCATION );
        
        if (close(Handle) == -1)
             throw OSException( strerror(errno), LOCATION );                  
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error creating file", LOCATION);
        
        if (Handle != -1)
			oal_remove(Name.c_str());   
			
		throw;        
    }
}


void FileManager::createSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size)
{
    int Handle   = -1;
    CharSeq Name = name;
    
    try 
    {
        int Flags = O_RDWR | O_CREAT;
        int Mode = AccessRightsToMode(access);    
        
#ifdef _ELINOS_4_1_
        Path = "/dev/vmfileshm/" + name;
        Flags = 0;
        Handle = oal_open( Path.c_str(), Flags );
#else
        //TODO: check if file exists, then throw a recource exception
        
        Handle = oal_shm_open( Name.c_str(), Flags, Mode );
#endif                          

        if (Handle == -1)
             throw OSException( strerror(errno), LOCATION );
             
#ifdef _ELINOS_4_1_
#else
        if (ftruncate( Handle, file_size ) == -1)
             throw OSException( strerror(errno), LOCATION );
#endif
        
        if (close(Handle) == -1)
             throw OSException( strerror(errno), LOCATION );                  
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createSharedMemory", LOCATION);
        
        if (Handle != -1)
			oal_remove(Name.c_str());   

		throw;        
    }
}


void FileManager::createMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long queue_size, const unsigned long message_size)
{
    int Handle = -1;
    CharSeq Path = name;
    
    try 
    {
        int Flags = O_CREAT;
        int Mode = AccessRightsToMode(access);
            
        oal_mq_attr Attributes;
        Attributes.mq_flags   = 0;
        Attributes.mq_maxmsg  = queue_size;
        Attributes.mq_msgsize = message_size;
        Attributes.mq_curmsgs = 0;
        
        //TODO: check if file exists, then throw a recource exception

		mq_unlink( Path.c_str() );
		
        Handle = oal_mq_open( Path.c_str(), Flags, Mode, &Attributes);
        
        if (Handle == -1)
             throw OSException( strerror(errno), LOCATION );
             
        if (close(Handle) == -1)
             throw OSException( strerror(errno), LOCATION );                  
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createMessageQueue", LOCATION);
        
        if (Handle != -1)
        	NO_EXCEPTION( deleteMessageQueue( Path.asaac_str(), ASAAC_IMMEDIATELY, TimeIntervalInstant ) );
        
        throw;
    }
}


void FileManager::deleteFile(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
{
    CharacterSequence Name;
    
    try
    {
        int Result = oal_remove(Name.c_str());
    
        if (Result != 0)
            throw OSException( strerror(errno), LOCATION );
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath( "Error deleting file", LOCATION);

		throw;        
    }
}    

        
void FileManager::deleteSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
{
    CharacterSequence Name = name;
    
    try
    {
#ifdef _ELINOS_4_1_
        //On elinos/pikeos platform shared memory is static
        //and cannot be removed
        int Result = 0;
#else
        int Result = oal_shm_unlink(Name.c_str());
#endif                              
    
        if (Result != 0)
            throw OSException( strerror(errno), LOCATION );
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath( "Error deleting shared memory", LOCATION);
		
		throw;
    }
}


void FileManager::deleteMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
{
    CharacterSequence Name = name;
    
    try
    {
        int Result = oal_mq_unlink(Name.c_str());
    
        if (Result != 0)
            throw OSException( strerror(errno), LOCATION );
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath( "Error deleting message queue", LOCATION);
		
		throw;
    }
}


void FileManager::openFile( const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle ) 
{
    CharacterSequence Name = name;
    int PosixHandle = -1;

    try
    {
    	file_handle = getAsaacHandleByName( name, REGULAR_FILE, use_option, true );  
    	
    	if (file_handle == OS_UNUSED_ID)
    	{
	        int Flags = UseOptionToFlags(use_option);
	    
	        PosixHandle = oal_open( Name.c_str(), Flags );
	        
	        if ( PosixHandle == -1 ) 
	            throw OSException( strerror(errno), LOCATION );
	        
	        setDefaultFlagsToPosixHandle( PosixHandle );
	                
	        file_handle = generateAsaacHandle();
	
	        storeFileData( file_handle, use_option, name, REGULAR_FILE, PosixHandle );
    	}
    }
    catch (ASAAC_Exception &e)
    {
        if (PosixHandle != -1)
            oal_close(PosixHandle);
        
        e.addPath( "Error opening file", LOCATION);
        
		throw;
    }
}


void FileManager::openSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle)
{
    CharacterSequence Name = name;
    int PosixHandle = -1;

    try
    {
    	file_handle = getAsaacHandleByName( name, SHARED_MEMORY_OBJECT, use_option, true );  
    	
    	if (file_handle == OS_UNUSED_ID)
    	{
	        int Flags = UseOptionToFlags(use_option);
	        int Mode = S_IREAD | S_IWRITE;
	    
#ifdef _ELINOS_4_1_
	        Name << "/dev/vmfileshm/" << name;
	        PosixHandle = oal_open( Name.c_str(), Flags, Mode );
#else
	        PosixHandle = oal_shm_open( Name.c_str(), Flags, Mode );
#endif
	        
	        if ( PosixHandle == -1 ) 
	            throw OSException( strerror(errno), LOCATION );
	        
	        setDefaultFlagsToPosixHandle( PosixHandle );
	                
	        file_handle = generateAsaacHandle();
	                
	        storeFileData( file_handle, use_option, name, SHARED_MEMORY_OBJECT, PosixHandle );
    	}                
    }
    catch (ASAAC_Exception &e)
    {
        if (PosixHandle != -1)
            oal_close(PosixHandle);
        
        e.addPath( "Error opeing shared memory", LOCATION);

		throw;
    }
}


void FileManager::openMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle)
{
    CharacterSequence Name = name;
    int PosixHandle = -1;

    try
    {
    	file_handle = getAsaacHandleByName( name, MESSAGE_QUEUE, use_option, true );  
    	
    	if (file_handle == OS_UNUSED_ID)
    	{
	        int Flags = UseOptionToFlags(use_option);
	    
	        PosixHandle = oal_mq_open( Name.c_str(), Flags );
	        
	        if ( PosixHandle == -1 ) 
	            throw OSException( strerror(errno), LOCATION );
	        
	        setDefaultFlagsToPosixHandle( PosixHandle );
	                
	        file_handle = generateAsaacHandle();
	
	        storeFileData( file_handle, use_option, name, MESSAGE_QUEUE, PosixHandle );
    	}
    }
    catch (ASAAC_Exception &e)
    {
        if (PosixHandle != -1)
            oal_close(PosixHandle);
        
        e.addPath( "Error opening message queue", LOCATION);

		throw;
    }
}


void FileManager::closeFile(const ASAAC_PrivateId file_handle)
{
    //In LAS_PROCESS_INIT state do not close any file handle to transfer them to OSProcess state
    if (OpenOS::getInstance()->getActivityState() == LAS_PROCESS_INIT)
        return; 
    
    try
    {
		FileInfoData Data = getFileDataByAsaacHandle(file_handle);

        if (oal_close( Data.PosixHandle ) < 0)
            throw OSException( strerror(errno), LOCATION );

        releaseFileData( file_handle );
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error closing file", LOCATION);
        
        throw;
    }
}


void FileManager::closeAllFiles()
{
    //In LAS_PROCESS_INIT state do not close any file handle to transfer them to OSProcess state
    if (OpenOS::getInstance()->getActivityState() == LAS_PROCESS_INIT)
    	return; 

    try
    {   
        while (m_FileInfoList.getCount() > 0)
        {
            ASAAC_PrivateId AsaacHandle = m_FileInfoList.idOf(0);
            
            closeFile( AsaacHandle );
        }
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error closing all files", LOCATION);
        
        throw;
    }
}


void FileManager::lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval timeout)
{
	throw OSException("Error locking file - not yet implemented.", LOCATION);
}


void FileManager::unlockFile(const ASAAC_PrivateId filehandle)
{
    throw OSException("Error unlocking file - not yet implemented.", LOCATION);
}


void FileManager::getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights &access, ASAAC_LockStatus &lock_status)
{
    try 
    {
		FileInfoData Data = getFileDataByAsaacHandle(filehandle);
		
		access = UseOptionToAccessRights( Data.UseOption );		
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error retrieving file attributes", LOCATION);

		throw;
    }
}


void FileManager::seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long &new_pos)
{
    try 
    {
		FileInfoData Data = getFileDataByAsaacHandle(filehandle);

        if (( Data.Type != REGULAR_FILE ) && ( Data.Type != SHARED_MEMORY_OBJECT ))
        	throw OSException("FileType is not provided.", LOCATION); 

		int whence;

    	switch ( seek_mode )
    	{
	    	case ASAAC_START_OF_FILE:    whence = SEEK_SET; break;
		    case ASAAC_CURRENT_POSITION: whence = SEEK_CUR; break;
		    case ASAAC_END_OF_FILE:      whence = SEEK_END; break;
		    default: throw OSException("SeekMode is out of range", LOCATION);
    	}
		
		ssize_t result = oal_lseek( Data.PosixHandle, set_pos, whence ); 

        if ( result == -1)
             throw OSException( strerror(errno), LOCATION );
             
        new_pos = result;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error seeking file", LOCATION);

		throw;
    }
}


void FileManager::readFile(const ASAAC_PrivateId filehandle, ASAAC_Address buffer_address, const long read_count, long &count_read, const ASAAC_TimeInterval timeout)
{
    try 
    {
    	BlockingScope TimeoutScope();
		FileInfoData Data = getFileDataByAsaacHandle(filehandle);
		
		TimeStamp Timeout( timeout );
		
		ssize_t result;
		timespec TimeSpecTimeout = Timeout.timespec_Time();
		
		switch ( Data.Type )
		{
        	case REGULAR_FILE:
        	case SHARED_MEMORY_OBJECT: 
        	{
		        result = oal_read(Data.PosixHandle, buffer_address, read_count);
        	}
		    break;
		        
        	case MESSAGE_QUEUE: 
        	{
				unsigned Prio;		
				
				do 
				{
					if (Timeout.isInfinity())
						result = oal_mq_receive( Data.PosixHandle, (char*)buffer_address, read_count, &Prio );
					else result = oal_mq_timedreceive( Data.PosixHandle, (char*)buffer_address, read_count, &Prio, &TimeSpecTimeout );

					if ( result <= 0 )
					{
						if ( errno == ETIMEDOUT ) 
							throw TimeoutException( LOCATION );
						
						if ( errno != EINTR ) 
							throw OSException( strerror(errno), LOCATION );
					}
				} 
				while (( result <= 0 ) && ( errno == EINTR ));
        	}				
			break;
				        	
        	default: throw OSException("FileType is not provided", LOCATION);
		}
		
        if ( result == -1)
             throw OSException( strerror(errno), LOCATION );
             
        count_read = result;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error reading file", LOCATION);

		throw;
    }
}


void FileManager::writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long &count_written, const ASAAC_TimeInterval timeout)
{
    try 
    {
    	BlockingScope TimeoutScope();

		FileInfoData Data = getFileDataByAsaacHandle( file_handle );
		
		TimeStamp Timeout( timeout );
		
		ssize_t result;
		timespec TimeSpecTimeout = Timeout.timespec_Time();

		switch ( Data.Type )
		{
        	case REGULAR_FILE:
        	case SHARED_MEMORY_OBJECT: 
        	{
		        result = oal_write(Data.PosixHandle, buffer_address, write_count);
		        count_written = result;
        	}
        	break;
        	
        	case MESSAGE_QUEUE: 
        	{
				do 
				{
					if (Timeout.isInfinity())
						result = oal_mq_send( Data.PosixHandle, (const char*)buffer_address, write_count, 1 );
					else result = oal_mq_timedsend( Data.PosixHandle, (const char*)buffer_address, write_count, 1, &TimeSpecTimeout );
					
					if ( result < 0 )
					{
						if ( errno == ETIMEDOUT ) 
							throw TimeoutException( LOCATION );
						
						if ( errno != EINTR ) 
							throw OSException( strerror(errno), LOCATION );
					}
				} 
				while (( result < 0 ) && ( errno == EINTR ));
				
		        count_written = write_count;
        	}
        	break;
        	
        	default: throw OSException("FileType is not provided", LOCATION);
		}
        
        if ( result == -1)
             throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("Error writing file", LOCATION);

		throw;
    }
}


void FileManager::mapFile(const ASAAC_PrivateId file_handle, const unsigned long size, const unsigned long offset, ASAAC_Address &address)
{
	try
	{
		FileInfoData Data = getFileDataByAsaacHandle(file_handle);
		
		if ((Data.Type != REGULAR_FILE) && (Data.Type != SHARED_MEMORY_OBJECT))
			throw OSException("FileType is not provided", LOCATION); 
		
		int PosixHandle = Data.PosixHandle;
		int Permissions = 0;
		
		switch ( Data.UseOption.use_access )
		{
			case ASAAC_READ:      Permissions = PROT_READ; break;
            case ASAAC_WRITE:     Permissions = PROT_WRITE; break;
            case ASAAC_READWRITE: Permissions = PROT_READ | PROT_WRITE; break;
            default: Permissions = PROT_READ; break; 
		}
		
		int Mode = MAP_SHARED;
		
		address = 0;
		address = oal_mmap( address, size, Permissions, Mode, PosixHandle, offset );
		
		if (address == MAP_FAILED)
			throw OSException( strerror(errno), LOCATION );
	}
	catch ( ASAAC_Exception &e)
	{
		e.addPath("Error mapping file", LOCATION);

		throw;
	}	
}


void FileManager::unmapFile(const ASAAC_Address address, const unsigned long size)
{
	try
	{
		if (oal_munmap(address, size) == -1)	
			throw OSException( strerror(errno), LOCATION );
	}
	catch ( ASAAC_Exception &e)
	{
		e.addPath("Error unmapping file", LOCATION);

		throw;
	}	
}


void FileManager::getFileBuffer(const unsigned long buffer_size, ASAAC_Address &buffer_address, const ASAAC_TimeInterval timeout)
{
    throw OSException("Error retrieving file buffer - not yet implemented.", LOCATION);
}


void FileManager::releaseFileBuffer(const ASAAC_Address buffer_address)
{
    throw OSException("Error releasing file buffer - not yet implemented.", LOCATION);
}


//***********************************************************************************************
//* Protected helper functions                                                                  *
//***********************************************************************************************

void   FileManager::setDefaultFlagsToPosixHandle( int handle ) const
{
    try
    {
        int iFileFlags = oal_fcntl( handle, F_GETFD );
    
        if ( iFileFlags == -1 ) 
            throw OSException( strerror(errno), LOCATION );
        
        iFileFlags &= !FD_CLOEXEC;
        iFileFlags = oal_fcntl( handle, F_SETFD, iFileFlags );
        
        if ( iFileFlags == -1 ) 
            throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e)
    {
        e.addPath("Error setting standard file flags to file handle", LOCATION);
        
        throw;
    }
}


mode_t FileManager::AccessRightsToMode( ASAAC_AccessRights AccessRights ) const
{
    mode_t Result = 0;
    
    switch (AccessRights)
    {
        case ASAAC_R:   Result = S_IRUSR | S_IRGRP | S_IROTH; break;
        case ASAAC_W:   Result = S_IWUSR | S_IWGRP | S_IWOTH; break;
        case ASAAC_D:   Result = S_IXUSR | S_IXGRP | S_IXOTH; break;
        case ASAAC_RW:  Result = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; break;
        case ASAAC_WD:  Result = S_IWUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH; break;
        case ASAAC_RWD: Result = S_IRWXU | S_IRWXG | S_IRWXO; break;
        case ASAAC_F:   Result = S_IRWXU | S_IRWXG | S_IRWXO; break;
        default: Result = 0;
    };
    
    return Result;
}


ASAAC_AccessRights FileManager::ModeToAccessRights( mode_t Mode ) const
{
    
}


int FileManager::UseOptionToFlags( ASAAC_UseOption UseOption ) const
{
    long Result = 0;
    
    switch (UseOption.use_access)
    {
        case ASAAC_READ:      Result = O_RDONLY; break;
        case ASAAC_WRITE:     Result = O_WRONLY; break;
        case ASAAC_READWRITE: Result = O_RDWR; break;
        default: Result = 0;
    };

    if ( UseOption.use_concur == ASAAC_EXCLUSIVE )
        Result |= O_EXCL;
    
    return Result;
}


ASAAC_UseOption FileManager::FlagsToUseOption( int Flags ) const
{
	ASAAC_UseOption Result;
	
	Result.use_access = ASAAC_READ;
	
	if ( (Flags & O_WRONLY) == O_WRONLY )
		Result.use_access = ASAAC_WRITE;
	
	if ( (Flags & O_RDWR) == O_RDWR )
		Result.use_access = ASAAC_READWRITE;

	if ( (Flags & O_EXCL) == O_EXCL )
		Result.use_concur = ASAAC_EXCLUSIVE;
	else Result.use_concur = ASAAC_SHARE;
	
	return Result;
}


ASAAC_UseOption FileManager::AccessRightsToUseOption( ASAAC_AccessRights AccessRights ) const
{
    ASAAC_UseOption Result;
    
    switch (AccessRights)
    {
        case ASAAC_R:   Result.use_access = ASAAC_READ; break;
        case ASAAC_W:   Result.use_access = ASAAC_WRITE; break;
        case ASAAC_D:   Result.use_access = ASAAC_READWRITE; break;
        case ASAAC_RW:  Result.use_access = ASAAC_READWRITE; break;
        case ASAAC_WD:  Result.use_access = ASAAC_READWRITE; break;
        case ASAAC_RWD: Result.use_access = ASAAC_READWRITE; break;
        case ASAAC_F:   Result.use_access = ASAAC_READWRITE; break;
        default: Result.use_access = ASAAC_READ;
    };
    
    Result.use_concur = ASAAC_SHARE;
    
    return Result;
}


ASAAC_AccessRights FileManager::UseOptionToAccessRights( ASAAC_UseOption UseOption ) const
{
    ASAAC_AccessRights Result;
    
    if ( UseOption.use_concur == ASAAC_SHARE )
    {
        switch (UseOption.use_access)
        {
            case ASAAC_READ:      Result = ASAAC_R; break;
            case ASAAC_WRITE:     Result = ASAAC_W; break;
            case ASAAC_READWRITE: Result = ASAAC_RW; break;
            default: Result = ASAAC_R;
        };
    }

    if ( UseOption.use_concur == ASAAC_EXCLUSIVE )
    {
        switch (UseOption.use_access)
        {
            case ASAAC_READ:      Result = ASAAC_R; break;
            case ASAAC_WRITE:     Result = ASAAC_WD; break;
            case ASAAC_READWRITE: Result = ASAAC_RWD; break;
            default: Result = ASAAC_R;
        };
    }

    return Result;
}


//***********************************************************************************************
//* Public administrative functions                                                             *
//***********************************************************************************************

void FileManager::reopenFiles()
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

    FileInfoData FileInfoArray[ OS_MAX_NUMBER_OF_LOCAL_FILES ];
    
    long Index = 0;
    
    while (m_FileInfoList.getCount() > 0)
    {
        FileInfoArray[Index] = m_FileInfoList[0];        
        closeFile( m_FileInfoList.idOf(0) );
    }
    
    for (unsigned long Index = 0; Index < OS_MAX_NUMBER_OF_LOCAL_FILES; Index++)
    {
        ASAAC_CharacterSequence Name = FileInfoArray[Index].Name;
        ASAAC_UseOption UseOption = FileInfoArray[Index].UseOption;
        ASAAC_PublicId DummyHandle;
        
       switch (FileInfoArray[Index].Type)
       {
            case REGULAR_FILE:          openFile(Name, UseOption, DummyHandle ); break;
            case SHARED_MEMORY_OBJECT:  openSharedMemory(Name, UseOption, DummyHandle ); break;
            case MESSAGE_QUEUE:         openMessageQueue(Name, UseOption, DummyHandle ); break;
            default: break;             
       }
    }
}


//***********************************************************************************************
//* FileInfo functions                                                                          *
//***********************************************************************************************

ASAAC_PrivateId FileManager::generateAsaacHandle()
{
	return PrivateIdManager::getInstance()->getNextId();
}


void FileManager::storeFileData( const ASAAC_PrivateId asaac_handle, const ASAAC_UseOption use_option, ASAAC_CharacterSequence name, FileType type, const int posix_handle )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	try
	{
		FileInfoData Data;
	    Data.PosixHandle = posix_handle;
	    Data.UseOption = use_option;
	    Data.Name = name;
	    Data.Type = type;
	    
	    Data.Derived = false;
	    
	    m_FileInfoList.add( asaac_handle, Data );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("FileData could not be stored", LOCATION);
		
		throw;
	}
}


long FileManager::indexOf( const int posix_handle )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	ProtectedScope Access( "Determine the index of a posix_handle", m_Semaphore );

	long Index;
	
	for ( Index = 0; Index < (long)m_FileInfoList.getCount(); Index++)
		if (m_FileInfoList[Index].PosixHandle == posix_handle)
			return Index;
						
    return -1;
}


long FileManager::indexOf( const ASAAC_CharacterSequence name, FileType type, ASAAC_UseOption use_option )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	ProtectedScope Access( "Determine the index of a filename", m_Semaphore );

	long Index;
	
	for ( Index = 0; Index < (long)m_FileInfoList.getCount(); Index++)
		if ((CharSeq(m_FileInfoList[Index].Name) == CharSeq(name)) &&
		    (m_FileInfoList[Index].Type == type) &&
		    (m_FileInfoList[Index].UseOption.use_access == use_option.use_access) &&
		    (m_FileInfoList[Index].UseOption.use_concur == use_option.use_concur))
			return Index;

	return -1;			
}


FileManager::FileInfoData FileManager::getFileDataByAsaacHandle( const ASAAC_PrivateId asaac_handle )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	ProtectedScope Access( "retrieve a file handle by an asaac handle", m_Semaphore );

	CharacterSequence ErrorString;
	
    long Index = m_FileInfoList.indexOf( asaac_handle );
    
    if (Index == -1)
    	throw OSException( (ErrorString << "data for given asaac_handle (" << asaac_handle << ") is not available").c_str() , LOCATION);
    
	FileInfoData Data = m_FileInfoList[ Index ]; 
	
    return Data;
}


ASAAC_PrivateId	FileManager::getAsaacHandleByName( const ASAAC_CharacterSequence name, FileType type, ASAAC_UseOption use_option, bool derived )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	long Index = indexOf(name, type, use_option);
	
	if (Index == -1)
		return OS_UNUSED_ID;
    else return m_FileInfoList.idOf(Index);	    	
}


void FileManager::releaseFileData( const ASAAC_PrivateId asaac_handle )
{
	if ( m_IsInitialized == false ) 
		throw UninitializedObjectException( LOCATION );

	m_FileInfoList.remove( asaac_handle );
}


//***********************************************************************************************
//* Backup and restore functions                                                                *
//***********************************************************************************************

ASAAC_CharacterSequence FileManager::getFileInfoString( FileInfoData Data )
{
	CharacterSequence Sequence;
	
	Sequence << Data.Name << ":";
	Sequence << (unsigned long)Data.Type << ":";
	Sequence << (unsigned long)Data.UseOption.use_access << ":";
	Sequence << (unsigned long)Data.UseOption.use_concur << ":";
	Sequence << (long)Data.PosixHandle;
	
	return Sequence.asaac_str();
}


FileManager::FileInfoData FileManager::getFileInfoData( ASAAC_CharacterSequence Sequence )
{
	FileInfoData Data;
	
	CharacterSequence Seq = Sequence;
	
	long Colon1 = Seq.find(":", false, 0);
	long Colon2 = Seq.find(":", false, Colon1+1);
	long Colon3 = Seq.find(":", false, Colon2+1);
	long Colon4 = Seq.find(":", false, Colon3+1);
	
	Data.Name                 =                               Seq.asaac_str(0, Colon1);
	Data.Type                 =                    (FileType) Seq.c_uint(Colon1+1, Colon2-Colon1-1);
	Data.UseOption.use_access =       (ASAAC_UseAccessRights) Seq.c_uint(Colon2+1, Colon3-Colon2-1);
	Data.UseOption.use_concur = (ASAAC_UseConcurrencePattern) Seq.c_uint(Colon3+1, Colon4-Colon3-1);
	Data.PosixHandle 		  =                               Seq.c_int(Colon4+1, Seq.size()-Colon4-1);
	
	Data.Derived = true;
	
	return Data;
}


void FileManager::writeFileInfoListToEnvironment()
{
	try
	{
		CharacterSequence HandleList;
		CharacterSequence Handle;
		CharacterSequence HandleData;
		
		for ( unsigned long Index = 0; Index < m_FileInfoList.getCount(); Index++ )
		{
			ASAAC_PrivateId AsaacHandle = m_FileInfoList.idOf(Index);
			
			HandleList << AsaacHandle << ",";
	
			Handle.erase();
			Handle << OS_ENV_ASAACHANDLE << AsaacHandle;
			HandleData = getFileInfoString( m_FileInfoList[Index] );
			
			setenv( Handle.c_str(), HandleData.c_str(), 1 ); 
		}
		
		HandleList.erase(HandleList.size()-1);
	
		setenv( OS_ENV_ASAACHANDLELIST, HandleList.c_str(), 1 );
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error writing FileInfo to environment", LOCATION);
		
		throw;
	} 
	
}


void FileManager::readFileInfoListFromEnvironment()
{
	try
	{
		CharacterSequence HandleList = getenv( OS_ENV_ASAACHANDLELIST );
		CharacterSequence Handle;
		CharacterSequence HandleData;
		
		long NewIndex = 0;
		long Index = 0;
		unsigned long Len;
		
		while ( Index < (long)HandleList.size() )
		{
			NewIndex = HandleList.find(",", false, Index);
			
			if (NewIndex == -1)
				Len = HandleList.size() - Index;
			else Len = NewIndex - Index;
			
			ASAAC_PrivateId AsaacHandle = HandleList.asaac_id( Index, Len );
			
			Handle.erase();
			Handle << OS_ENV_ASAACHANDLE << AsaacHandle;
			HandleData = getenv( Handle.c_str() );
			
			FileInfoData Data = getFileInfoData( HandleData.asaac_str() );
			
			m_FileInfoList.add( AsaacHandle, Data );
			
			Index = Index + Len + 1;
		}
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath( "Error reading FileInfo from environment", LOCATION );
		
		throw;
	}
}

