#include "FileManager.hh"

#include "Exceptions/Exceptions.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "Managers/AllocatorManager.hh"

#include "OpenOS.hh"


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
        
        readFileInfoListFromEnvironment();                
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error initializing FileManager", LOCATION);
        
        deinitialize();
        
        throw;
    }
    
}


void FileManager::deinitialize()
{
    if (m_IsInitialized == false)
        return;
    
    try
    {
        writeFileInfoListToEnvironment();

        m_FileInfoList.deinitialize();
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("Error while deinitializing FileManager", LOCATION);
        e.raiseError();
    }
    
    m_IsInitialized = false;
}


FileManager::~FileManager()
{
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
    // For APOS processes drop all privileges. set uid and gid == nobody
    if ( alias == PROC_APOS )
    {
        setuid( 65534 );
        setgid( 65534 );
    } 
    
    deinitialize();
    
    //deallocate objects, because after execve all objects will be lost
    AllocatorManager::getInstance()->deallocateAllObjects();
    
    //Now load and execute the file
    execve( CharSeq(name).c_str(), 0, environ);

	//reallocate all objects, because execve failed.
    AllocatorManager::getInstance()->reallocateAllObjects();
    
    //if the last call returned, an error occured
    OSException e( strerror(errno), LOCATION );
    
    try
    {
    	initialize();
    }
    catch ( ... )
    {
    	e.addPath("Reinitialization of FileManager failed.", LOCATION);
    }
    
    throw e;
}


ASAAC_ResourceReturnStatus FileManager::createDirectory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access)
{
    try 
    {
	    CharSeq Name = name;
        
        //TODO: check if dir exists, then throw a recource exception

	    if (mkdir(Name.c_str(), AccessRightsToMode( access )) == -1)
             throw OSException( strerror(errno), LOCATION );             
    }
    catch ( ResourceException &e )
    {
        return ASAAC_RS_RESOURCE;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createDirectory", LOCATION);
        e.raiseError();
        
        return ASAAC_RS_ERROR;
    }
    
	return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::deleteDirectory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
{
    try 
    {
        CharSeq Name = name;
        
        if (remove(Name.c_str()) == -1)
             throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::deleteDirectory", LOCATION);
        e.raiseError();
        
        return ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_ResourceReturnStatus FileManager::createFile(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size)
{
    int Handle = -1;
    
    try 
    {
        CharSeq Name = name;
        
        //TODO: check if file exists, then throw a recource exception

        Handle = creat(Name.c_str(), AccessRightsToMode( access ));
        
        if (Handle == -1)
             throw OSException( strerror(errno), LOCATION );
             
        if (ftruncate( Handle, file_size ) == -1)
             throw OSException( strerror(errno), LOCATION );
        
        if (close(Handle) == -1)
             throw OSException( strerror(errno), LOCATION );                  
    }
    catch ( ResourceException &e )
    {
        return ASAAC_RS_RESOURCE;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createFile", LOCATION);
        e.raiseError();
        
        if (Handle != -1)
            deleteFile( name, ASAAC_IMMEDIATELY, TimeIntervalInstant );
        
        return ASAAC_RS_ERROR;
    }
    
    return ASAAC_RS_SUCCESS;
}


ASAAC_ResourceReturnStatus FileManager::createSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size)
{
    int Handle   = -1;
    CharSeq Path = name;
    
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
        
        Handle = oal_shm_open( Path.c_str(), Flags, Mode );
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
    catch ( ResourceException &e )
    {
        return ASAAC_RS_RESOURCE;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createSharedMemory", LOCATION);
        e.raiseError();
        
        if (Handle != -1)
            deleteSharedMemory( Path.asaac_str(), ASAAC_IMMEDIATELY, TimeIntervalInstant );
        
        return ASAAC_RS_ERROR;
    }
    
    return ASAAC_RS_SUCCESS;
}


ASAAC_ResourceReturnStatus FileManager::createMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long queue_size, const unsigned long message_size)
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
    catch ( ResourceException &e )
    {
        return ASAAC_RS_RESOURCE;
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createMessageQueue", LOCATION);
        e.raiseError();
        
        if (Handle != -1)
            deleteMessageQueue( Path.asaac_str(), ASAAC_IMMEDIATELY, TimeIntervalInstant );
        
        return ASAAC_RS_ERROR;
    }
    
    return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::deleteFile(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
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
        e.addPath( "APOS::deleteFile", LOCATION);
        e.raiseError();
        
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
}    

        
ASAAC_TimedReturnStatus FileManager::deleteSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
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
        e.addPath( "APOS::deleteSharedMemory", LOCATION);
        e.raiseError();
        
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::deleteMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
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
        e.addPath( "APOS::deleteMessageQueue", LOCATION);
        e.raiseError();
        
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus FileManager::openFile( const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle ) 
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
        
        e.addPath( "APOS::openFile", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
        
    return ASAAC_SUCCESS;   
}


ASAAC_ReturnStatus FileManager::openSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle)
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
        
        e.addPath( "APOS::openSharedMemory", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
        
    return ASAAC_SUCCESS;   
}


ASAAC_ReturnStatus FileManager::openMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle)
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
        
        e.addPath( "APOS::openMessageQueue", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
        
    return ASAAC_SUCCESS;   
}


ASAAC_ReturnStatus FileManager::closeFile(const ASAAC_PrivateId file_handle)
{
    //In LAS_PROCESS_INIT state do not close any file handle to transfer them to OSProcess state
    if (OpenOS::getInstance()->getActivityState() == LAS_PROCESS_INIT)
        return ASAAC_SUCCESS; 
    
    try
    {
		FileInfoData Data = getFileDataByAsaacHandle(file_handle);

        if (oal_close( Data.PosixHandle ) < 0)
            throw OSException( strerror(errno), LOCATION );

        releaseFileData( file_handle );
    }
    catch (ASAAC_Exception &e)
    {
        e.addPath("APOS::closeFile", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
    return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus FileManager::closeAllFiles()
{
    //In LAS_PROCESS_INIT state do not close any file handle to transfer them to OSProcess state
    if (OpenOS::getInstance()->getActivityState() == LAS_PROCESS_INIT)
        return ASAAC_SUCCESS; 

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
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
    return ASAAC_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval timeout)
{
	OSException("APOS::lockFile - not yet implemented.", LOCATION).raiseError();
	return ASAAC_TM_ERROR;
}


ASAAC_ReturnStatus FileManager::unlockFile(const ASAAC_PrivateId filehandle)
{
    OSException("APOS::unlockFile - not yet implemented.", LOCATION).raiseError();
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus FileManager::getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights &access, ASAAC_LockStatus &lock_status)
{
    OSException("APOS::getFileAttributes - not yet implemented.", LOCATION).raiseError();
	return ASAAC_ERROR;
}


ASAAC_ReturnStatus FileManager::seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long &new_pos)
{
    OSException("APOS::seekFile - not yet implemented.", LOCATION).raiseError();
	return ASAAC_ERROR;
}


ASAAC_TimedReturnStatus FileManager::readFile(const ASAAC_PrivateId filehandle, ASAAC_Address buffer_address, const long read_count, long &count_read, const ASAAC_TimeInterval timeout)
{
    try 
    {
		FileInfoData Data = getFileDataByAsaacHandle(filehandle);
		
        count_read = oal_read(Data.PosixHandle, buffer_address, read_count);
        
        if ( count_read == -1)
             throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::readFile", LOCATION);
        e.raiseError();
        
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long &count_written, const ASAAC_TimeInterval timeout)
{
    OSException("APOS::writeFile - not yet implemented.", LOCATION).raiseError();
	return ASAAC_TM_ERROR;
}


ASAAC_TimedReturnStatus FileManager::mapFile(const ASAAC_PrivateId file_handle, const unsigned long size, const unsigned long offset, ASAAC_Address &address)
{
	try
	{
		FileInfoData Data = getFileDataByAsaacHandle(file_handle);
		
		if ((Data.Type != REGULAR_FILE) && (Data.Type != SHARED_MEMORY_OBJECT))
			throw OSException("FileType for this handle is not supported", LOCATION); 
		
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
		e.addPath("APOS::mapFile", LOCATION);
		e.raiseError();
		
		return ASAAC_TM_ERROR;
	}	
	
	return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus FileManager::unmapFile(const ASAAC_Address address, const unsigned long size)
{
	try
	{
		if (oal_munmap(address, size) == -1)	
			throw OSException( strerror(errno), LOCATION );
	}
	catch ( ASAAC_Exception &e)
	{
		e.addPath("APOS::unmapFile", LOCATION);
		e.raiseError();
		
		return ASAAC_ERROR;
	}	
	
	return ASAAC_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::getFileBuffer(const unsigned long buffer_size, ASAAC_Address &buffer_address, const ASAAC_TimeInterval timeout)
{
    OSException("APOS::getFileBuffer - not yet implemented.", LOCATION).raiseError();
	return ASAAC_TM_ERROR;
}


ASAAC_ReturnStatus FileManager::releaseFileBuffer(const ASAAC_Address buffer_address)
{
    OSException("APOS::releaseFileBuffer - not yet implemented.", LOCATION).raiseError();
	return ASAAC_ERROR;
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
        case ASAAC_R:   Result = S_IRUSR; break;
        case ASAAC_W:   Result = S_IWUSR; break;
        case ASAAC_D:   Result = S_IXUSR; break;
        case ASAAC_RW:  Result = S_IRUSR | S_IWUSR; break;
        case ASAAC_WD:  Result = S_IWUSR | S_IXUSR; break;
        case ASAAC_RWD: Result = S_IRWXU; break;
        case ASAAC_F:   Result = S_IRWXU; break;
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
    CharacterSequence Counter = getenv(OS_ENV_ASAACHANDLE_COUNTER);
    
    if ( Counter.empty() )
        Counter = (ASAAC_PublicId)0;
    
    Counter = (ASAAC_PublicId)(Counter.asaac_id() + 1);

    setenv(OS_ENV_ASAACHANDLE_COUNTER, Counter.c_str(), 1);
    
    return Counter.asaac_id();
}


void FileManager::storeFileData( const ASAAC_PrivateId asaac_handle, const ASAAC_UseOption use_option, ASAAC_CharacterSequence name, FileType type, const int posix_handle )
{
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
	long Index;
	
	for ( Index = 0; Index < (long)m_FileInfoList.getCount(); Index++)
		if (m_FileInfoList[Index].PosixHandle == posix_handle)
			return Index;
						
    return -1;
}


long FileManager::indexOf( const ASAAC_CharacterSequence name, FileType type, ASAAC_UseOption use_option )
{
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
	CharacterSequence ErrorString;
	
    long Index = m_FileInfoList.indexOf( asaac_handle );
    
    if (Index == -1)
    	throw OSException( (ErrorString << "data for given asaac_handle (" << asaac_handle << ") is not available").c_str() , LOCATION);
    	
    return m_FileInfoList[ Index ];	    
}


ASAAC_PrivateId	FileManager::getAsaacHandleByName( const ASAAC_CharacterSequence name, FileType type, ASAAC_UseOption use_option, bool derived )
{
	long Index = indexOf(name, type, use_option);
	
	if (Index == -1)
		return OS_UNUSED_ID;
    else return m_FileInfoList.idOf(Index);	    	
}


void FileManager::releaseFileData( const ASAAC_PrivateId asaac_handle )
{
	m_FileInfoList.remove( asaac_handle );
}


//***********************************************************************************************
//* Backup and restore functions                                                                *
//***********************************************************************************************

ASAAC_CharacterSequence FileManager::getFileInfoString( FileInfoData Data )
{
	CharacterSequence Sequence;
	
	Sequence << Data.Name << ":";
	Sequence << (unsigned int)Data.Type << ":";
	Sequence << (unsigned int)Data.UseOption.use_access << ":";
	Sequence << (unsigned int)Data.UseOption.use_concur << ":";
	Sequence << Data.PosixHandle;
	
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
		
		e.raiseError();
		
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
		
		e.raiseError();
		
		throw;
	}
}

