#include "OpenOSObject.hh"

#include "Managers/FileManager.hh"


ASAAC_ResourceReturnStatus ASAAC_APOS_createDirectory(const ASAAC_CharacterSequence* name, const ASAAC_AccessRights access)
{
    try 
    {
		FileManager::getInstance()->createDirectory( *name, access );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createDirectory", LOCATION);
        e.raiseError();
        
        return e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;
    }
    
	return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_deleteDirectory(const ASAAC_CharacterSequence* name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval* timeout)
{
    try 
    {
		FileManager::getInstance()->deleteDirectory( *name, del_opt, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::deleteDirectory", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_ResourceReturnStatus ASAAC_APOS_createFile(const ASAAC_CharacterSequence* name, const ASAAC_AccessRights access, const unsigned long file_size)
{
    try 
    {
		FileManager::getInstance()->createFile( *name, access, file_size );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::createFile", LOCATION);
        e.raiseError();
        
        return e.isResource()?ASAAC_RS_RESOURCE:ASAAC_RS_ERROR;
    }
    
	return ASAAC_RS_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_deleteFile(const ASAAC_CharacterSequence* name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval* timeout)
{
    try 
    {
		FileManager::getInstance()->deleteFile( *name, del_opt, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::deleteFile", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_openFile(const ASAAC_CharacterSequence* name, const ASAAC_UseOption* use_option, ASAAC_PrivateId* file_handle)
{
    try 
    {
		FileManager::getInstance()->openFile( *name, *use_option, *file_handle );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::openFile", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_closeFile(const ASAAC_PrivateId file_handle)
{
    try 
    {
		FileManager::getInstance()->closeFile( file_handle );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::closeFile", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval* timeout)
{
    try 
    {
		FileManager::getInstance()->lockFile( file_handle, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::lockFile", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_unlockFile(const ASAAC_PrivateId filehandle)
{
    try 
    {
		FileManager::getInstance()->unlockFile( filehandle );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::unlockFile", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights* access, ASAAC_LockStatus* lock_status)
{
    try 
    {
		FileManager::getInstance()->getFileAttributes( filehandle, *access, *lock_status );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getFileAttributes", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long* new_pos)
{
    try 
    {
		FileManager::getInstance()->seekFile( filehandle, seek_mode, set_pos, *new_pos );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::seekFile", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_readFile(const ASAAC_PrivateId filehandle, ASAAC_Address* buffer_address, const long read_count, long* count_read, const ASAAC_TimeInterval* timeout)
{
    try 
    {
		FileManager::getInstance()->readFile( filehandle, buffer_address, read_count, *count_read, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::readFile", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long* count_written, const ASAAC_TimeInterval* timeout)
{
    try 
    {
		FileManager::getInstance()->writeFile( file_handle, buffer_address, write_count, *count_written, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::writeFile", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_TimedReturnStatus ASAAC_APOS_getFileBuffer(const unsigned long buffer_size, ASAAC_Address* buffer_address, const ASAAC_TimeInterval* timeout)
{
    try 
    {
		FileManager::getInstance()->getFileBuffer( buffer_size, *buffer_address, *timeout );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::getFileBuffer", LOCATION);
        e.raiseError();
        
        return e.isTimeout()?ASAAC_TM_TIMEOUT:ASAAC_TM_ERROR;
    }
    
	return ASAAC_TM_SUCCESS;
}


ASAAC_ReturnStatus ASAAC_APOS_releaseFileBuffer(const ASAAC_Address buffer_address)
{
    try 
    {
		FileManager::getInstance()->releaseFileBuffer( buffer_address );
    }
    catch ( ASAAC_Exception &e )
    {
        e.addPath("APOS::releaseFileBuffer", LOCATION);
        e.raiseError();
        
        return ASAAC_ERROR;
    }
    
	return ASAAC_SUCCESS;
}

