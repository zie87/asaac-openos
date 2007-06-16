#include "FileManager.hh"

#include "Exceptions/Exceptions.hh"
#include "ProcessManagement/ProcessManager.hh"

#include "OpenOS.hh"


FileManager::FileManager()
{
	m_SemaphoreAllocator.initialize( Semaphore::predictSize() );
	
	m_Semaphore.initialize( &m_SemaphoreAllocator );
	
	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_FILES; Index ++ )
	{
		m_Files[ Index ].Handle = 0;
		m_Files[ Index ].Flags  = 0;
		m_Files[ Index ].Name   = CharSeq("").asaac_str();
		m_Files[ Index ].Type   = FILETYPE_UNDEFINED;
	}
}


FileManager::~FileManager()
{
	m_Semaphore.deinitialize();
	
	m_SemaphoreAllocator.deinitialize();
}


FileManager* FileManager::getInstance()
{
	static FileManager StaticFileManager;
	
	return &StaticFileManager;
}


ASAAC_ReturnStatus FileManager::openFile( const ASAAC_CharacterSequence Name, FileType Type, long Flags, long& FileHandle ) 
{
    try
    {
        CharSeq ErrorString;
        
		ProtectedScope Access( "Opening a file", m_Semaphore );
	
		// find free spot in File Table
		FileInfoBlock* FreeBlock = findFileByHandle( 0 );
		
		if ( FreeBlock == 0 ) 
			throw ResourceException( "File Manager has no more free slots", LOCATION );
		
		switch( Type ) {
		  
			case REGULAR_FILE : 
				FileHandle = oal_open( CharSeq(Name).c_str(),
								Flags,
								S_IREAD | S_IWRITE );
			  	break;
			  
			case SHARED_MEMORY_OBJECT :
#ifdef _ELINOS_4_1_
			  	FileHandle = oal_open( (CharSeq("/dev/vmfileshm/") + CharSeq(Name)).c_str(), Flags);
	
#else
			  	FileHandle = oal_shm_open( CharSeq(Name).c_str(),
						     	Flags,
						     	S_IREAD | S_IWRITE );
#endif					     	
			  	break;
			  
			case MESSAGE_QUEUE:
			  
			  	FileHandle = oal_mq_open( CharSeq(Name).c_str(),
						    	Flags );
			  	break;
			   
			default: FileHandle = -1; 
		}
		
		if ( FileHandle == -1 ) 
			throw OSException( strerror(errno), LOCATION );
		
		// Set file to "!FD_CLOEXEC" so the handle is inherited in exec() calls
		try
		{
			long iFileFlags = oal_fcntl( FileHandle, F_GETFD );
		
			if ( iFileFlags == -1 ) 
				throw OSException( "Error getting FileFlags", LOCATION );
			
			iFileFlags &= !FD_CLOEXEC; //TODO: not nice to have "!" here
			iFileFlags = oal_fcntl( FileHandle, F_SETFD, iFileFlags );
			
			if ( iFileFlags == -1 ) 
				throw OSException( "Error setting FileFlags", LOCATION );
		}
		catch (ASAAC_Exception &e)
		{
			e.addPath("Error setting file to ""!FD_CLOEXEC"" so the handle is inherited in exec() calls", LOCATION);
			throw;
		}
		
		// Enter new file data longo File table
		FreeBlock->Name   = Name;
		FreeBlock->Flags  = Flags;
		FreeBlock->Handle = FileHandle;
		FreeBlock->Type	  = Type;
    }
    catch (ASAAC_Exception &e)
    {
        CharSeq ErrorString;
        
        e.addPath((ErrorString << "Error opening file: '" << Name << "'").c_str(), LOCATION);
        
        throw;
    }
    	
	return ASAAC_SUCCESS;	
}


ASAAC_ReturnStatus FileManager::closeFile( const ASAAC_CharacterSequence& Name, long Flags )
{
	try
	{
		FileInfoBlock* File = findFileByName( Name, Flags );
		
		if ( File == 0 ) 
			throw ResourceException( "File not found", LOCATION );
		
		ASAAC_ReturnStatus Status = ASAAC_SUCCESS;
		
		while (File != 0)
		{
			if (closeFile(File->Handle) == ASAAC_ERROR)
				Status = ASAAC_ERROR;
					
			File = findFileByName( Name, Flags );
		}
		
		if (Status == ASAAC_ERROR)
			throw OSException("At least one handle could not be closed", LOCATION);
	}
	catch (ASAAC_Exception &e)
	{
		CharSeq ErrorString;
		e.addPath( (ErrorString << "Error closing file: " << Name).c_str(), LOCATION);
		e.raiseError();
        
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus FileManager::closeFile( long Handle )
{
	//In OSStarter state do not close any file handle to transfer them to OSProcess state
	if (OpenOS::getInstance()->getContext() == OS_Starter)
		return ASAAC_SUCCESS; 
	
	try
	{
		ProtectedScope Access( "Closing a file", m_Semaphore );
		FileInfoBlock* File = findFileByHandle( Handle );
		
		CharSeq ErrorString;
		
		if ( File == 0 ) 
			throw ResourceException( (ErrorString << "File handle not found: " << Handle).c_str(), LOCATION );
			
		if (oal_close( File->Handle ) < 0)
		    throw OSException( strerror(errno), LOCATION );
        
		File->Handle    = 0;
		File->Flags     = 0;
		File->Name      = CharSeq("").asaac_str();
		File->Type      = FILETYPE_UNDEFINED;
	}
	catch (ASAAC_Exception &e)
	{
		e.addPath("Error closing file", LOCATION);
		e.raiseError();
		return ASAAC_ERROR;
	}
	
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus FileManager::closeAllFiles()
{
	//In OSStarter state do not close any file handle to transfer them to OSProcess state
	if (OpenOS::getInstance()->getContext() == OS_Starter)
		return ASAAC_SUCCESS; 

	try
	{	
		ProtectedScope Access( "Closing all files", m_Semaphore );
		
		for ( long Index = 0; Index < OS_MAX_NUMBER_OF_FILES; Index ++ )
		{
			if ( m_Files[ Index ].Handle != 0 )
			{
		        if (oal_close( m_Files[ Index ].Handle ) < 0)
		            throw OSException( strerror(errno), LOCATION );
				
				m_Files[ Index ].Handle    = 0;
				m_Files[ Index ].Flags     = 0;
				m_Files[ Index ].Name      = CharSeq("").asaac_str();
		    	m_Files[ Index ].Type      = FILETYPE_UNDEFINED;
			}
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


ASAAC_ReturnStatus FileManager::removeFile( const ASAAC_CharacterSequence& Name)
{
	FileInfoBlock* File = findFileByName( Name );
	
	if ( File != 0 ) 
		return removeFile(File->Handle);
	
	return ASAAC_ERROR;
}
 
    
ASAAC_ReturnStatus FileManager::removeFile( long FileHandle )
{
    CharacterSequence fileName;
    FileType fileType;
    
    try
    {
		FileInfoBlock* File = findFileByHandle( FileHandle );
		
		if ( File != 0 ) 
        {
			fileName = File->Name;
			fileType = File->Type;
			
			closeFile( fileName.asaac_str() );
		
			ProtectedScope Access( "Removing a file", m_Semaphore );
		
			long result;
			switch( fileType ) {		
				case REGULAR_FILE : result = oal_remove(fileName.c_str());
									break;
									
				case SHARED_MEMORY_OBJECT :
#ifdef _ELINOS_4_1_
									//On elinos/pikeos platform shared memory is static
									//and cannot be removed
									result = 0;
#else
									result = oal_shm_unlink(fileName.c_str());
#endif								
									break;
									
				case MESSAGE_QUEUE:
									result = oal_mq_unlink(fileName.c_str());
									break; 
				default: 
									result = -1; 
									break;
			}
		
			if (result != 0)
	            throw OSException( strerror(errno), LOCATION );
		}
    }
    catch (ASAAC_Exception &e)
    {
        CharacterSequence ErrorString;
        
        e.addPath( (ErrorString << "Error removing file: " << fileName).c_str(), LOCATION);
        
        e.raiseError();
        
        return ASAAC_ERROR;
    }
	
	return ASAAC_SUCCESS;
}    
        

long FileManager::getFileHandle( const ASAAC_CharacterSequence& Name, FileType Type, long Flags, long Size )
{
    long FileHandle = -1;

    try
    {
        CharacterSequence ErrorString;        
        
		FileInfoBlock* File = findFileByName( Name, Flags );
		
		if ( File == 0 )
		{
			if ( openFile( Name, Type, Flags, FileHandle ) == ASAAC_ERROR )
				 throw OSException("File cannot be opened", LOCATION);
        }
        else FileHandle = File->Handle;
        
#ifdef _ELINOS_4_1_
        //on elinos/pikeos platform shared memory is static
#else       
        if (Type == SHARED_MEMORY_OBJECT)
        {    
	        struct stat FileStat;
	
	        if ( fstat( FileHandle, &FileStat ) == -1 )
                throw OSException( (ErrorString << "stat: " << strerror(errno)).c_str(), LOCATION );
	        
            if (FileStat.st_size < Size)
            {   
		        if ( oal_ftruncate(FileHandle, Size) == -1 )
                    throw OSException( (ErrorString << "ftruncate: " << strerror(errno)).c_str(), LOCATION );
            }
        }
#endif              
    }
    catch (ASAAC_Exception &e)
    {
        CharacterSequence ErrorString;
        
        e.addPath( (ErrorString << "Error requesting file handle: " << Name).c_str(), LOCATION);
        
        throw;
    }
	
	return FileHandle;
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
    try 
    {
        CharSeq Name = name;
        
        //TODO: check if file exists, then throw a recource exception

        long Handle = creat(Name.c_str(), AccessRightsToMode( access ));
        
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
        
        //TODO: delete file if already created
        
        return ASAAC_RS_ERROR;
    }
    
    return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus FileManager::deleteFile(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout)
{
    try 
    {
        CharSeq Name = name;
        
        if (remove(Name.c_str()) == -1)
             throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::deleteFile", LOCATION);
        e.raiseError();
        
        return ASAAC_TM_ERROR;
    }
    
    return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus FileManager::openFile(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle)
{
    try 
    {
        CharSeq Name = name;
        
        long handle = open(Name.c_str(), UseOptionToFlags(use_option));
        
        if ( handle == -1)
             throw OSException( strerror(errno), LOCATION );
        
        file_handle = handle;
    }
    catch ( ASAAC_Exception &e )
    {
        CharacterSequence ErrorString;
        
        e.addPath( (ErrorString << "APOS::openFile(" << name << ", use_option, file_handle)").c_str(), LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
    return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus FileManager::closeFile(const ASAAC_PrivateId file_handle)
{
    try 
    {
        if ( close(file_handle) == -1)
             throw OSException( strerror(errno), LOCATION );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::closeFile", LOCATION);
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
        count_read = read(filehandle, buffer_address, read_count);
        
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
        case ASAAC_F:   Result = AccessRightsToMode(ASAAC_RWD); break;
        default: Result = 0;
    };
    
    return Result;
}


ASAAC_AccessRights FileManager::ModeToAccessRights( mode_t Mode ) const
{
    
}


long FileManager::UseOptionToFlags( ASAAC_UseOption UseOption ) const
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


ASAAC_UseOption FileManager::FlagsToUseOption( long Flags ) const
{
}


ASAAC_ReturnStatus FileManager::saveState( long& SavedStateHandler )
{
	try
	{
		ProtectedScope Access( "Saving the state of FileManager", m_Semaphore );
		
		char Template[20];
	
		strcpy( Template, "FileStateXXXXXX" );

		SavedStateHandler = oal_mkstemp( Template );

		oal_fchmod( SavedStateHandler, 0x777 );

		if ( SavedStateHandler == -1 ) 
			throw OSException( LOCATION );

		// remove filesystem object to disallow other programs to connect to the file
		oal_unlink( Template );
	
		ssize_t TotalSize = sizeof( FileInfoBlock ) * OS_MAX_NUMBER_OF_FILES;
	
		if ( oal_write( SavedStateHandler, m_Files, TotalSize ) != TotalSize ) 
			throw OSException( LOCATION );
	
		if ( oal_lseek( SavedStateHandler, 0, SEEK_SET ) == -1 ) 
			throw OSException( LOCATION );
	}
	catch ( OSException& Exception )
	{
		// If we encountered an error, but a file was created, clean up nicely
		if ( SavedStateHandler != -1 )
		{
			oal_close( SavedStateHandler );
			SavedStateHandler = -1;
		}
		
		throw;
	}
        
    return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus FileManager::restoreStateByHandle( long SavedStateHandler )
{
	try
	{
		if ( SavedStateHandler < 4 ) 
			return ASAAC_ERROR;
		
		ProtectedScope Access( "Restoring the state of FileManager", m_Semaphore );
	
		ssize_t TotalSize = sizeof( FileInfoBlock ) * OS_MAX_NUMBER_OF_FILES;

		if ( read( SavedStateHandler, m_Files, TotalSize ) != TotalSize ) 
			throw OSException( LOCATION );
	
		oal_close( SavedStateHandler );
		
	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_FILES; Index ++ )
	{
		if (m_Files[ Index ].Type  != FILETYPE_UNDEFINED)
		{
			//cout << CharSeq(m_Files[ Index ].Name).c_str() << endl;
		}
	}
			
	}
	catch ( ASAAC_Exception &e )
	{
		// If there was a reading error on the restore file, reset file read offset to
		// beginning of file, so we might eventually be able to try again.
		oal_lseek( SavedStateHandler, SEEK_SET, 0 );
		
		// Pass on the exception
		
		return ASAAC_ERROR;
	}

    return ASAAC_SUCCESS;
}



FileManager::FileInfoBlock* FileManager::findFileByName( const ASAAC_CharacterSequence& Name, long Flags )
{
    if (Name.size == 0)
        return 0;
    
	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_FILES; Index ++ )
	{
		if ( ( CharSeq(m_Files[ Index ].Name) == CharSeq(Name) ) &&
		     ( true || (m_Files[ Index ].Flags == Flags ) || (Flags == -1) ) ) 
			return &(m_Files[ Index ]);
	}
	
	return 0;
}


FileManager::FileInfoBlock* FileManager::findFileByHandle( long Handle )
{
	for ( long Index = 0; Index < OS_MAX_NUMBER_OF_FILES; Index ++ )
	{
		if ( m_Files[ Index ].Handle == Handle ) 
			return &(m_Files[ Index ]);
	}
	
	return 0;
}


ASAAC_ReturnStatus FileManager::saveState (ASAAC_CharacterSequence &File)
{
	long SavedStateHandler;

  	try
  	{
    	ProtectedScope Access ("Saving the state of FileManager", m_Semaphore);

    	CharacterSequence ThisFile = "/dev/shm/FileStateXXXXXX";
    	char FileData[255];
    	ThisFile.convertTo(FileData);

    	SavedStateHandler = oal_mkstemp (FileData);

		File = CharSeq(FileData).asaac_str();

    	oal_fchmod (SavedStateHandler, 0x777);

    	if (SavedStateHandler == -1)
      		throw OSException (LOCATION);

    	ssize_t TotalSize = sizeof (FileInfoBlock) * OS_MAX_NUMBER_OF_FILES;

    	if (oal_write (SavedStateHandler, m_Files, TotalSize) != TotalSize)
      		throw OSException (LOCATION);

    	if (oal_lseek (SavedStateHandler, 0, SEEK_SET) == -1)
      		throw OSException (LOCATION);

    	oal_close (SavedStateHandler);
    
    	return ASAAC_SUCCESS;
  	}
  	catch ( ASAAC_Exception &e )
  	{
    	// If we encountered an error, but a file was created, clean up nicely
    	if (SavedStateHandler != -1)
      	{
			oal_close (SavedStateHandler);
			SavedStateHandler = -1;
      	}

    	throw;
  	}
}


ASAAC_ReturnStatus FileManager::restoreStateByName ( ASAAC_CharacterSequence File )
{
  	try
  	{
    	long SavedStateHandler = oal_open(CharSeq(File).c_str(), O_RDONLY,  S_IRUSR );

    	if (SavedStateHandler < 0)
      		throw OSException (LOCATION);

    	ProtectedScope Access ("Restoring the state of FileManager", m_Semaphore);

    	ssize_t TotalSize = sizeof (FileInfoBlock) * OS_MAX_NUMBER_OF_FILES;

    	if (read (SavedStateHandler, m_Files, TotalSize) != TotalSize)
      		throw OSException (LOCATION);

    	oal_close (SavedStateHandler);

  	}
  	catch ( ASAAC_Exception &e )
    {
      	e.addPath("error in retoring File Manager Status", LOCATION);
      	// Pass on the exception
      	throw; 
    }
  	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus FileManager::reopenFile ()
{
  	ProtectedScope Access ("reopen Files", m_Semaphore);

  	for (long Index = 0; Index < OS_MAX_NUMBER_OF_FILES; Index++)
    {
    	try
      	{
			if (m_Files[Index].Handle != 0)
	  		{
	    		cout << "trying file " << CharSeq(m_Files[Index].Name).c_str() 
	    		<< " with old Handle " << m_Files[Index].Handle << endl;

	    		switch (m_Files[Index].Type)
	      		{
	      			case REGULAR_FILE:
						m_Files[Index].Handle =
		  					oal_open (CharSeq (m_Files[Index].Name).c_str (),
			    				m_Files[Index].Flags, S_IREAD | S_IWRITE);

						break;

	      			case SHARED_MEMORY_OBJECT:
						m_Files[Index].Handle =
		  					oal_shm_open (CharSeq (m_Files[Index].Name).c_str (),
								m_Files[Index].Flags, S_IREAD | S_IWRITE);
						break;

	      			case MESSAGE_QUEUE:
						m_Files[Index].Handle =
		  					oal_mq_open (CharSeq (m_Files[Index].Name).c_str (),
			       				m_Files[Index].Flags);
						break;

	      			default:
						m_Files[Index].Handle = -1;
						throw OSException (LOCATION);
	      		}

			    cout << "reopend file: " << CharSeq (m_Files[Index].Name).c_str ();
			    cout << " of type " << m_Files[Index].Type;
			    cout << " with flags " << m_Files[Index].Flags;
			    cout << " Handle = (" << m_Files[Index].Handle;
			    cout << ")\n";
	  		}
      	}
      	catch ( ASAAC_Exception &e )
      	{
            CharacterSequence ErrorString;
            
			e.addPath( (ErrorString << "reopend file: " << m_Files[Index].Name.data << "failed").c_str(), LOCATION);
            
            return ASAAC_ERROR;
      	}
    }
  	return ASAAC_SUCCESS;
}
