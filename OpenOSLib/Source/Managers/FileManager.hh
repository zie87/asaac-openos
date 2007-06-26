#ifndef FILEMANAGER_HH_
#define FILEMANAGER_HH_



#include "OpenOSIncludes.hh"

#include "Exceptions/Exceptions.hh"

#include "IPC/Semaphore.hh"
#include "IPC/ProtectedScope.hh"

#include "Allocator/LocalMemory.hh"
#include "Common/Templates/SharedMap.hh"

using namespace std;

enum FileType {

		FILETYPE_UNDEFINED,
        REGULAR_FILE,
        SHARED_MEMORY_OBJECT,
        MESSAGE_QUEUE

};


//! singleton class to handle file access
/*! the FileManager handles the opening and closing of files in a way so
 *  the file handles can be held across the transition of one process to
 *  another via the exec() command. It is used to open relevant files
 *  in a privileged mode already, then store those open file handles,
 *  so they can still be accessed by the unprivileged application code
 *  taking control after the exec().
 */

class FileManager
{
private:
        FileManager();

public:
        static FileManager* getInstance();
        static size_t predictSize();
           
        void initialize();
        void deinitialize( const bool do_throw = false );     
        virtual ~FileManager();

		void executeFile( const ASAAC_CharacterSequence name, const ProcessAlias alias );

		void createDirectory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access);				
        void createFile(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size);
        void createSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size);
        void createMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long queue_size, const unsigned long message_size);
		
		void deleteDirectory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout);
        void deleteFile(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout);
        void deleteSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout);
        void deleteMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout);
		
        void openFile(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle);
        void openSharedMemory(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle);
        void openMessageQueue(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle);
		
		void closeFile(const ASAAC_PrivateId file_handle);
        void closeAllFiles();
		
		void lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval timeout);		
		void unlockFile(const ASAAC_PrivateId filehandle);
		
		void getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights &access, ASAAC_LockStatus &lock_status);
				
		void seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long &new_pos);
		void readFile(const ASAAC_PrivateId filehandle, ASAAC_Address buffer_address, const long read_count, long &count_read, const ASAAC_TimeInterval timeout);				
		void writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long &count_written, const ASAAC_TimeInterval timeout);
		void mapFile(const ASAAC_PrivateId file_handle, const unsigned long size, const unsigned long offset, ASAAC_Address &address);
		void unmapFile(const ASAAC_Address address, const unsigned long size);
		
		void getFileBuffer(const unsigned long buffer_size, ASAAC_Address &buffer_address, const ASAAC_TimeInterval timeout);		
		void releaseFileBuffer(const ASAAC_Address buffer_address);

protected:
        void                setDefaultFlagsToPosixHandle( int handle ) const;

        mode_t              AccessRightsToMode( ASAAC_AccessRights AccessRights ) const;
        ASAAC_AccessRights  ModeToAccessRights( mode_t Mode ) const;

        int                 UseOptionToFlags( ASAAC_UseOption UseOption ) const;
        ASAAC_UseOption     FlagsToUseOption( int Flags ) const;

        ASAAC_UseOption     AccessRightsToUseOption( ASAAC_AccessRights AccessRights ) const;
        ASAAC_AccessRights  UseOptionToAccessRights( ASAAC_UseOption UseOption ) const;

public:
        void reopenFiles();


protected:
		typedef struct {
            ASAAC_CharacterSequence  Name;
            FileType                 Type;
            ASAAC_UseOption          UseOption;
            int                      PosixHandle;
            bool					 Derived;
		} FileInfoData;
		
        bool            m_IsInitialized;

        LocalMemory     m_Allocator;

        Semaphore       m_Semaphore;

protected:
        ASAAC_PrivateId 	generateAsaacHandle();
        
        void            	storeFileData( const ASAAC_PrivateId asaac_handle, const ASAAC_UseOption use_option, ASAAC_CharacterSequence name, FileType type, const int posix_handle );

		long				indexOf( const int posix_handle );
		long				indexOf( const ASAAC_CharacterSequence name, FileType type, ASAAC_UseOption use_option );
        
        FileInfoData   		getFileDataByAsaacHandle( const ASAAC_PrivateId asaac_handle );
        ASAAC_PrivateId		getAsaacHandleByName( const ASAAC_CharacterSequence name, FileType type, ASAAC_UseOption use_option, bool derived );
        
        void            	releaseFileData( const ASAAC_PrivateId asaac_handle );
        
        ASAAC_CharacterSequence getFileInfoString( FileInfoData Data );
        FileInfoData		    getFileInfoData( ASAAC_CharacterSequence Sequence );
        
        void writeFileInfoListToEnvironment();
        void readFileInfoListFromEnvironment();

private:
		typedef SharedMap<ASAAC_PrivateId, FileInfoData> FileInfoList;
		
        FileInfoList m_FileInfoList;
};


#endif /*FILEMANAGER_HH_*/
