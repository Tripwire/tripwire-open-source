//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
//
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
//
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
//
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
///////////////////////////////////////////////////////////////////////////////
// fsservices.h
//
// iFSServices -- interface to the file system services class
//
// objective:
//    abstract all file system calls, except for reading/writing
//       to elaborate: this class does not handle any file I/O -- we
//       intend to use streams to abstract out file I/O
//    abstract case sensitivity of underlying file system
//

#ifndef __FSSERVICES_H
#define __FSSERVICES_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TYPES_H
#include "types.h"
#endif
#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __DEBUG_H
#include "debug.h"
#endif
#ifndef __FILEERROR_H
#include "fileerror.h"
#endif

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif


//=========================================================================
// DEFINES AND MACROS
//=========================================================================

// macros for extracting the major and minor portions of int64's:
#if !defined(major)
#    if !USES_GLIBC
#        define major(x) ((int)((x) >> 8) & 0xff)
#        define minor(x) ((int)((x)&0xff))
#    else
#        ifdef WORDS_BIGENDIAN
#            define major(x) (int)(((char*)&(x))[2])
#            define minor(x) (int)(((char*)&(x))[3])
#        else
#            define major(x) (int)(((char*)&(x))[1])
#            define minor(x) (int)(((char*)&(x))[0])
#        endif
#    endif
#endif /* !major */

//=========================================================================
// TYPEDEFS
//=========================================================================

typedef int64_t cFSTime;
typedef int64_t cFSType;

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

// filesystem access control lists
//     it is the union of MAX(elem) for all the file systems that we support
/*class cACLElem {
   // TODO this is just a place holder
   // uint32_t   mUid;
};*/

// this class is used only to pass arguments to iFSServices
//     it is the union of MAX(elem) for all the file systems that we support
struct cFSStatArgs
{
    enum FileType
    {
        TY_INVALID, // lazy evaluation
        TY_FILE,
        TY_DIR,
        TY_BLOCKDEV,
        TY_CHARDEV,
        TY_SYMLINK,
        TY_FIFO,
        TY_SOCK,
        TY_DOOR,
        TY_PORT,
        TY_NAMED,
        TY_NATIVE,
        TY_MESSAGE_QUEUE,
        TY_SEMAPHORE,
        TY_SHARED_MEMORY
    };

    // attr is fs dependent?
    uint64_t  dev;     // dep
    int64_t   ino;     // dep
    int64_t   mode;    // dep
    int64_t   nlink;   // indep
    int64_t   uid;     // dep
    int64_t   gid;     // dep
    uint64_t  rdev;    // dep
    int64_t   size;    // indep
    cFSTime   atime;   // indep
    cFSTime   mtime;   // indep
    cFSTime   ctime;   // indep
    int64_t   blksize; // indep
    int64_t   blocks;  // dep
    int64_t   fstype;  // dep
    TSTRING   usid;    // dep
    TSTRING   gsid;    // dep
                     // int64_t mFileType;     // Matt's addition...

    FileType mFileType; // redundant with other information in this struct, but
                        // broken out for convenience

    //TODO: access control list should go here, too
    //std::list <cACLElem> mACL; // indep
};


//=========================================================================
//
// eFSServices -- exception class
//
//=========================================================================

TSS_FILE_EXCEPTION(eFSServices, eFileError);
TSS_FILE_EXCEPTION(eFSServicesGeneric, eFSServices);

//=========================================================================
//
// iFSServices -- abstracts all file system calls, except for opening, reading,
//    and writing, which will be handled by standard c++.
//    NOTE -- all paths returned from this class will be delimited with a '/'
//
//=========================================================================

class iFSServices
{
public:
    virtual ~iFSServices()
    {
    }
    ///////////////////////////////////////////////////////////////
    // ENUMS
    ///////////////////////////////////////////////////////////////

    ////////////////////////////////////////
    // file creation modes
    ////////////////////////////////////////
    enum Mode
    {
        MODE_DEFAULT         = 0,
        MODE_READ            = 1,
        MODE_WRITE           = 2,
        MODE_SHARE_DELETE    = 4,
        MODE_SHARE_READ      = 8,
        MODE_SHARE_WRITE     = 16,
        MODE_CREATE_EXCL     = 32,
        MODE_CREATE_TRUNCATE = 64
    };

    ////////////////////////////////////////
    // maximum path length on platform
    ////////////////////////////////////////
    enum
    {
#ifdef MAXPATHLEN
        TW_MAX_PATH = MAXPATHLEN
#else
        TW_MAX_PATH = 1024
#endif
    };

    ///////////////////////////////////////////////////////////////
    // MEMBER FUNCTIONS
    ///////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    // platform specific functions
    ////////////////////////////////////////
    virtual bool IsCaseSensitive() const = 0;
    // returns true if the file system is case sensitive
    virtual TCHAR GetPathSeparator() const = 0;
    // returns "/" for unix and "\\" for win32
    virtual const TCHAR* GetStandardBackupExtension() const = 0;
    // returns normal string to append to backup files for this os.

    ////////////////////////////////////////
    // process functions
    ////////////////////////////////////////
    virtual void Sleep(int nSeconds) const = 0;
    // makes the current process sleep for the specified number of seconds


    ////////////////////////////////////////
    // major filesystem functions
    ////////////////////////////////////////
    virtual void Stat(const TSTRING& strFileName, cFSStatArgs& pStat) const = 0;
    // fills out the cFSStatArgs structure with the stat info for the named file
    virtual void GetTempDirName(TSTRING& strName) const = 0;
    // makes directory if it doesn't exist already.  Dirname will end with a delimiter ( '/' )

    virtual void SetTempDirName(TSTRING& tmpName) = 0;

    virtual TSTRING& MakeTempFilename(TSTRING& strName) const = 0;
    // create temporary file
    //      TSTRING must have the form ("baseXXXXXX"), where the X's are replaced with
    //      characters to make it a unique file.  There must be at least 6 Xs.


    ////////////////////////////////////////
    // minor filesystem functions
    ////////////////////////////////////////
    virtual void GetHostID(TSTRING& name) const = 0;

    virtual void GetMachineName(TSTRING& name) const = 0;

    virtual void GetMachineNameFullyQualified(TSTRING& name) const = 0;

    virtual bool GetCurrentUserName(TSTRING& tstrName) const = 0;

    virtual bool GetIPAddress(uint32_t& uiIPAddress) = 0;


    ////////////////////////////////////////
    // directory specific functions
    ////////////////////////////////////////
    virtual void ReadDir(const TSTRING& strName, std::vector<TSTRING>& vDirContents, bool bFullPaths = true) const = 0;
    // puts the contents of the specified directory, except for . and .., into the supplied vector.
    // if bFullPaths is true, then the vector contains fully qualified path names; otherwise, it only contains the
    // short names.
    virtual void GetCurrentDir(TSTRING& strCurDir) const = 0;
    // returns the current working directory


    ////////////////////////////////////////
    // file specific functions
    ////////////////////////////////////////
    virtual bool FileDelete(const TSTRING& name) const = 0;

    ////////////////////////////////////////
    // directory and file functions
    ////////////////////////////////////////
    virtual bool Rename(const TSTRING& strOldName, const TSTRING& strNewName, bool fOverWrite = true) const = 0;
    // rename a file

    virtual bool GetUserName(uid_t user_id, TSTRING& tstrUser) const    = 0;
    virtual bool GetGroupName(gid_t group_id, TSTRING& tstrGroup) const = 0;

    //Set whether we try to resolve uid/gid to a name, since Linux static binaries can
    //have trouble (read: segfaulting) with name resolution given the right nsswitch.conf setup.
    //This defaults to true if not specified.
    virtual void SetResolveNames(bool resolve) = 0;

    ////////////////////////////////////////
    // miscellaneous utility functions
    ////////////////////////////////////////
    virtual void ConvertModeToString(uint64_t perm, TSTRING& tstrPerm) const = 0;
    // takes a int64_t permission (from stat) and changes it to look like UNIX's 'ls -l' (e.g. drwxrwxrwx)
    virtual bool FullPath(TSTRING& fullPath, const TSTRING& relPath, const TSTRING& pathRelFrom = _T("")) const = 0;
    // converts relPath into a fully qualified path, storing it in FullPath. If this
    // fails, false is returned.  if the path to which relPath is relative is not CWD, put it in pathRelFrom.
    // TODO: In some places we have depended on the behaviour that if relPath.empty() == true then we
    // fail or return an empty string.  Should we add this behaviour to the interface?
    virtual bool GetExecutableFilename(TSTRING& strFullPath, const TSTRING& strFilename) const = 0;
    // get the path to the current executable file
    virtual bool IsRoot(const TSTRING& strPath) const = 0;
    // returns true if strPath denotes a root path


    ////////////////////////////////////////
    // error functions
    ////////////////////////////////////////
    virtual TSTRING GetErrString() const = 0;
    // Returns an error string that is appropriate for the system, (e.g. errorstr(errno)
    // on UNIX and FormatError( ..., GetLastError(), ...) in Win32
    // Call this immediately after a failed system call to get a string
    // representation of the error event.


    ////////////////////////////////////////
    // singleton manipulation
    ////////////////////////////////////////
    static iFSServices* GetInstance();
    static void         SetInstance(iFSServices* pInst);


    ///////////////////////////////////////////////////////////////
    // PRIVATE DATA
    ///////////////////////////////////////////////////////////////
private:
    static iFSServices* mpInstance;
};


//=========================================================================
// INLINE FUNCTIONS
//=========================================================================
inline iFSServices* iFSServices::GetInstance()
{
    ASSERT(mpInstance != 0);

    return mpInstance;
}

inline void iFSServices::SetInstance(iFSServices* pInst)
{
    mpInstance = pInst;
}


#endif
