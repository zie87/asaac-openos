#ifndef FILEMANAGER_HH_
#define FILEMANAGER_HH_



#include "OpenOSIncludes.hh"

#include "Exceptions/Exceptions.hh"

#include "IPC/Semaphore.hh"
#include "IPC/ProtectedScope.hh"

#include "Allocator/LocalMemory.hh"


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
public:

        struct FileInfoBlock 
        {
            ASAAC_CharacterSequence  Name;
            long             		 Handle;
            long             		 Flags;
            FileType        		 Type;
        };

		//! get FileManager instance
        static FileManager* getInstance();
        /*!< \returns reference to instance of FileManager
         */

		//! open a filesystem object
        ASAAC_ReturnStatus openFile( const ASAAC_CharacterSequence Name, FileType Type, long Flags, long& FileHandle );
        /*!< The openFile operation looks for the indicated file in the table of open handles,
         *   and returns its value, if the FileType and Flags for access match those
         *   stored for the filesystem object.
         *   
         *   \param[in] Name	Name of filesystem object to be opened
         *   \param[in] Type    Type of filesystem object to be opened (REGULAR_FILE, SHARED_MEMORY_OBJECT, MESSAGE_QUEUE)
         *   \param[in] Flags	Flags for the filesystem object to be opened
         *   \param[out] FileHandle Filehandle returned by OS for the respective filesystem object
         * 
         *   \returns ASAAC_SUCCESS if the filesystem object is already open or could be opened.
         *            ASAAC_ERROR if the indicated filesystem object could not be opened.
         */
         
         
        //! close a filesystem object indicated by Name
        ASAAC_ReturnStatus closeFile( const ASAAC_CharacterSequence& Name, long Flags = -1 );
                
        //! close a filesystem object indicated by file handle
        ASAAC_ReturnStatus closeFile( long FileHandle );

		//! closes all filesystem objects stored in the FileHandler
        ASAAC_ReturnStatus closeAllFiles();


        //! remove a filesystem object indicated by Name
        ASAAC_ReturnStatus removeFile( const ASAAC_CharacterSequence& Name );
                
        //! remove a filesystem object indicated by FileHandle
        ASAAC_ReturnStatus removeFile( long FileHandle );
                

		//! get the file handle of
        long getFileHandle( const ASAAC_CharacterSequence& Name, FileType Type, long Flags, long Size = 0);
		/*!< and if found, returns the appropriate handle. If the file has not already been
         *   opened, and hence is not found in the table, the FileManager attempts to open
         *   the file at the moment of the function call and add to to the table of open files.
         */

		ASAAC_ResourceReturnStatus createDirectory(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access);
		
		ASAAC_TimedReturnStatus deleteDirectory(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout);
		
		ASAAC_ResourceReturnStatus createFile(const ASAAC_CharacterSequence name, const ASAAC_AccessRights access, const unsigned long file_size);
		
		ASAAC_TimedReturnStatus deleteFile(const ASAAC_CharacterSequence name, const ASAAC_DeleteOption del_opt, const ASAAC_TimeInterval timeout);
		
		ASAAC_ReturnStatus openFile(const ASAAC_CharacterSequence name, const ASAAC_UseOption use_option, ASAAC_PrivateId &file_handle);
		
		ASAAC_ReturnStatus closeFile(const ASAAC_PrivateId file_handle);
		
		ASAAC_TimedReturnStatus lockFile(const ASAAC_PrivateId file_handle, const ASAAC_TimeInterval timeout);
		
		ASAAC_ReturnStatus unlockFile(const ASAAC_PrivateId filehandle);
		
		ASAAC_ReturnStatus getFileAttributes(const ASAAC_PrivateId filehandle, ASAAC_AccessRights &access, ASAAC_LockStatus &lock_status);
		
		ASAAC_ReturnStatus seekFile(const ASAAC_PrivateId filehandle, const ASAAC_SeekMode seek_mode, const long set_pos, unsigned long &new_pos);
		
		ASAAC_TimedReturnStatus readFile(const ASAAC_PrivateId filehandle, ASAAC_Address buffer_address, const long read_count, long &count_read, const ASAAC_TimeInterval timeout);
		
		ASAAC_TimedReturnStatus writeFile(const ASAAC_PrivateId file_handle, const ASAAC_Address buffer_address, const unsigned long write_count, unsigned long &count_written, const ASAAC_TimeInterval timeout);
		
		ASAAC_TimedReturnStatus getFileBuffer(const unsigned long buffer_size, ASAAC_Address &buffer_address, const ASAAC_TimeInterval timeout);
		
		ASAAC_ReturnStatus releaseFileBuffer(const ASAAC_Address buffer_address);

        mode_t AccessRightsToMode( ASAAC_AccessRights AccessRights ) const;
        ASAAC_AccessRights ModeToAccessRights( mode_t Mode ) const;

        long UseOptionToFlags( ASAAC_UseOption UseOption ) const;
        ASAAC_UseOption FlagsToUseOption( long Flags ) const;

        ASAAC_ReturnStatus saveState( long& SavedStateHandle );

  		//! save the File Manager State in a File and not only in an Filehandler. The file will be closed. 
        ASAAC_ReturnStatus saveState( ASAAC_CharacterSequence &File );

  		//! restore the File Manager State from a File given by a file name. The file will be opened and closed. 
        ASAAC_ReturnStatus restoreStateByName( ASAAC_CharacterSequence File );
        ASAAC_ReturnStatus restoreStateByHandle( long SavedStateHandle );

        //! reopen all filesystem objects
        ASAAC_ReturnStatus reopenFile();

        virtual ~FileManager();

protected:
        FileManager();

        FileInfoBlock*  findFileByName( const ASAAC_CharacterSequence& Name, long Flags = -1);
        FileInfoBlock*  findFileByHandle( long Handle );

        Semaphore       m_Semaphore;
        LocalMemory		m_SemaphoreAllocator;

        FileInfoBlock   m_Files[ OS_MAX_NUMBER_OF_FILES ];

};


#endif /*FILEMANAGER_HH_*/
