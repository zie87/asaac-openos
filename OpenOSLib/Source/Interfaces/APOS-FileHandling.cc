#include "OpenOS.hh"

#include "Managers/FileManager.hh"


ASAAC_ResourceReturnStatus ASAAC_APOS_createDirectory(const ASAAC_CharacterSequence* name, const ASAAC_AccessRights access)
{
	return FileManager::getInstance()->createDirectory( *name, access );
}


ASAAC_TimedReturnStatus ASAAC_APOS_deleteDirectory(const ASAAC_CharacterSequence* name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval* timeout)
{
	return FileManager::getInstance()->deleteDirectory( *name, del_opt, *timeout );
}


ASAAC_ResourceReturnStatus ASAAC_APOS_createFile(const ASAAC_CharacterSequence* name, const ASAAC_AccessRights access, const unsigned long file_size)
{
	return FileManager::getInstance()->createFile( *name, access, file_size );
}


ASAAC_TimedReturnStatus ASAAC_APOS_deleteFile(const ASAAC_CharacterSequence* name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval* timeout)
{
	return FileManager::getInstance()->deleteFile( *name, del_opt, *timeout );
}


ASAAC_ReturnStatus ASAAC_APOS_openFile(const ASAAC_CharacterSequence* name, const ASAAC_UseOption* use_option, ASAAC_PrivateId* file_handle)
{
	return FileManager::getInstance()->openFile( *name, *use_option, *file_handle );
}


ASAAC_ReturnStatus ASAAC_APOS_closeFile(const ASAAC_PrivateId file_handle)
{
	return FileManager::getInstance()->closeFile( file_handle );
}


ASAAC_TimedReturnStatus ASAAC_APOS_lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval* timeout)
{
	return FileManager::getInstance()->lockFile( file_handle, *timeout );
}


ASAAC_ReturnStatus ASAAC_APOS_unlockFile(const ASAAC_PrivateId filehandle)
{
	return FileManager::getInstance()->unlockFile( filehandle );
}


ASAAC_ReturnStatus ASAAC_APOS_getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights* access, ASAAC_LockStatus* lock_status)
{
	return FileManager::getInstance()->getFileAttributes( filehandle, *access, *lock_status );
}


ASAAC_ReturnStatus ASAAC_APOS_seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long* new_pos)
{
	return FileManager::getInstance()->seekFile( filehandle, seek_mode, set_pos, *new_pos );
}


ASAAC_TimedReturnStatus ASAAC_APOS_readFile(const ASAAC_PrivateId filehandle, ASAAC_Address* buffer_address, const long read_count, long* count_read, const ASAAC_TimeInterval* timeout)
{
	return FileManager::getInstance()->readFile( filehandle, buffer_address, read_count, *count_read, *timeout );
}


ASAAC_TimedReturnStatus ASAAC_APOS_writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long* count_written, const ASAAC_TimeInterval* timeout)
{
	return FileManager::getInstance()->writeFile( file_handle, buffer_address, write_count, *count_written, *timeout );
}


ASAAC_TimedReturnStatus ASAAC_APOS_getFileBuffer(const unsigned long buffer_size, ASAAC_Address* buffer_address, const ASAAC_TimeInterval* timeout)
{
	return FileManager::getInstance()->getFileBuffer( buffer_size, *buffer_address, *timeout );
}


ASAAC_ReturnStatus ASAAC_APOS_releaseFileBuffer(const ASAAC_Address buffer_address)
{
	return FileManager::getInstance()->releaseFileBuffer( buffer_address );
}

