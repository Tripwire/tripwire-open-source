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
//////////////////////////////////////////////////////////////////
// unixfsservices.cpp
//
//  Implements cUnixFSServices class in unixfsservices.h
//

#include "core/stdcore.h"
#include "core/corestrings.h"
#include "core/file.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

//#include <ctype.h>
//#include <iostream>

//#include <sys/types.h>
#if HAVE_UNISTD_H
#   include <unistd.h>
#endif

#if HAVE_DIRENT_H
#   include <dirent.h>
#endif

//#include <sys/stat.h>

//#if HAVE_SYS_TIME_H
//#include <sys/time.h>
//#endif

//#if HAVE_SYS_PARAM_H
//#include <sys/param.h>
//#endif

//#if HAVE_SYS_MOUNT_H
//#include <sys/mount.h>
//#endif

/*#if HAVE_SYS_USTAT_H
#include <sys/ustat.h>
#endif*/

//#if HAVE_WCHAR_H
//#include <wchar.h>
//#endif

//#if HAVE_SYS_SYSMACROS_H
//#include <sys/sysmacros.h>
//#endif

#if HAVE_SYS_UTSNAME_H
#   include <sys/utsname.h>
#endif

#if HAVE_PWD_H
#   include <pwd.h>
#endif

#if HAVE_SYS_SOCKET_H
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#endif

#if HAVE_GRP_H
#   include <grp.h>
#endif

#//include <fcntl.h>
#include <errno.h>

//=========================================================================
// INCLUDES
//=========================================================================

#include "unixfsservices.h"

// commented out definition of _TWNBITSMAJOR because we should use the
// makedev macro in sys/sysmacros.h for portability.
// except linux has the body of sysmacros.h commented out. why?
// -jeb 7/26/99
// reduced to sys/statfs.h.  Linux is OK and doesn't deserve
// special treatment.  20010317-PH
/*#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif //HAVE_SYS_STATFS_H
*/

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

#define TW_SLASH _T('/')

#if (SUPPORTS_NATIVE_OBJECTS)
  #ifndef _S_IFNATIVE
    #define _S_IFNATIVE 02000000 /* OS/400 native object, of any object type that isn't treated as a directory */
  #endif

  #ifndef S_ISNATIVE
    #define S_ISNATIVE(x) (((x) & 0370000) == _S_IFNATIVE)
  #endif
#endif

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

using namespace std;

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

static bool                             util_FileIsExecutable(const TSTRING&);
static bool                             util_PathFind(TSTRING& strFullPath, const TSTRING& strFilename);
static void                             util_RemoveLastPathElement(TSTRING& strPath, TSTRING& strElem);
static bool                             util_GetNextPathElement(const TSTRING& strPathC, TSTRING& strElem, int index);
static void                             util_RemoveDuplicateSeps(TSTRING& strPath);
static bool                             util_TrailingSep(TSTRING& str, bool fLeaveSep);
static void                             util_RemoveTrailingSeps(TSTRING& str);
template<typename T> static inline void util_ZeroMemory(T& obj);

//=========================================================================
// PUBLIC METHOD CODE
//=========================================================================

cUnixFSServices::cUnixFSServices() : mResolveNames(true)
{
}

cUnixFSServices::~cUnixFSServices()
{
}

//=========================================================================
// *** VIRTUAL FUNCTION CODE ***
//=========================================================================

///////////////////////////////////////////////////////////////////////////////
// GetErrString
///////////////////////////////////////////////////////////////////////////////
TSTRING cUnixFSServices::GetErrString() const
{
    TSTRING ret;
    char*   pErrorStr = strerror(errno);
    ret               = pErrorStr;
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// GetHostID
///////////////////////////////////////////////////////////////////////////////
void cUnixFSServices::GetHostID(TSTRING& name) const
{
    TOSTRINGSTREAM ret;

    ret.setf(std::ios::hex, std::ios::basefield);
 
#if HAVE_GETHOSTID
    ret << gethostid();
#else
    ret << 999999;
#endif

    tss_stream_to_string(ret, name);
}

// returns "/" for unix and "\\" for win32
TCHAR cUnixFSServices::GetPathSeparator() const
{
    return '/';
}

#if !USES_DEVICE_PATH
void cUnixFSServices::ReadDir(const TSTRING& strFilename, std::vector<TSTRING>& v, bool bFullPaths) const
{
#else
void cUnixFSServices::ReadDir(const TSTRING& strFilenameC, std::vector<TSTRING>& v, bool bFullPaths) const
{
    TSTRING strFilename = cDevicePath::AsNative(strFilenameC);
#endif

    //Get all the filenames
    DIR* dp = 0;

#if defined(O_DIRECTORY) && defined(O_NOATIME)
    //dfd will be autoclosed by closedir(), should not be explicitly closed.
    int dfd = open(strFilename.c_str(), O_RDONLY | O_DIRECTORY | O_NOATIME);
    if (dfd > 0)
        dp = fdopendir(dfd);
#else
    dp = opendir(strFilename.c_str());
#endif

    if (dp == NULL)
    {
        throw eFSServicesGeneric(strFilename, iFSServices::GetInstance()->GetErrString());
        return;
    }

    struct dirent* d;

    while ((d = readdir(dp)) != NULL)
    {
        if ((strcmp(d->d_name, _T(".")) != 0) && (strcmp(d->d_name, _T("..")) != 0))
        {
            if (bFullPaths)
            {
                //Create the full pathname
                TSTRING strNewName = strFilename;

                // get full path of dir entry
                util_TrailingSep(strNewName, true);
                strNewName += d->d_name;

                // save full path name
                v.push_back(strNewName);
            }
            else
                v.push_back(d->d_name);
        }
    }

    //Close the directory
    closedir(dp);
}

/* needs to and with S_IFMT, check EQUALITY with S_*, and return more types
cFSStatArgs::FileType cUnixFSServices::GetFileType(const cFCOName &filename) 
{
    cFSStatArgs stat;
    Stat(filename, stat);
    return stat.mFileType;
}
*/

void cUnixFSServices::GetCurrentDir(TSTRING& strCurDir) const
{
    TCHAR pathname[iFSServices::TW_MAX_PATH];
    pathname[0] = '\0';
    TCHAR* ret  = getcwd(pathname, sizeof(TCHAR) * iFSServices::TW_MAX_PATH);

    if (ret == NULL)
        throw eFSServicesGeneric(strCurDir, iFSServices::GetInstance()->GetErrString());

    strCurDir = pathname;
}


TSTRING& cUnixFSServices::MakeTempFilename(TSTRING& strName) const
{
    char  szTemplate[iFSServices::TW_MAX_PATH];

    strncpy(szTemplate, strName.c_str(), iFSServices::TW_MAX_PATH);

#if HAVE_MKSTEMP
    // create temp filename and check to see if mkstemp failed
    int fd = mkstemp(szTemplate);
    if (fd < 0)
        throw eFSServicesGeneric(strName);

    close(fd);
    char* pchTempFileName = szTemplate;

#elif HAVE_MKTEMP

    // create temp filename
    char* pchTempFileName = mktemp(szTemplate);

    //check to see if mktemp failed
    if (pchTempFileName == NULL || strlen(pchTempFileName) == 0)
    {
        throw eFSServicesGeneric(strName);
    }

#elif HAVE_TMPNAM
    char* pchTempFileName = tmpnam(szTemplate);
#else
    // none of the standard temp fns exist?  should this be an error?
    #error "No standard tempfile naming functions are available"
#endif

    // change name so that it has the XXXXXX part filled in
    strName = pchTempFileName;

    // TODO: below is a very old comment, is it still accurate?
    // Linux creates the file!!  Doh!
    // So I'll always attempt to delete it -bam
    FileDelete(strName.c_str());

    return (strName);
}

void cUnixFSServices::GetTempDirName(TSTRING& strName) const
{
    strName = mTempPath;
}

void cUnixFSServices::SetTempDirName(TSTRING& tmpPath)
{
    mTempPath = tmpPath;
}


#if !USES_DEVICE_PATH
void cUnixFSServices::Stat(const TSTRING& strName, cFSStatArgs& statArgs) const
{
#else
void cUnixFSServices::Stat(const TSTRING& strNameC, cFSStatArgs& statArgs) const
{
    TSTRING strName = cDevicePath::AsNative(strNameC);
#endif
    //local variable for obtaining info on file.
    struct stat statbuf;

    int ret;
#if HAVE_LSTAT
    ret = lstat(strName.c_str(), &statbuf);
#elif HAVE_STAT
    ret = stat(strName.c_str(), &statbuf);
#endif

    cDebug d("cUnixFSServices::Stat");
    d.TraceDetail("Executing on file %s (result=%d)\n", strName.c_str(), ret);

    if (ret < 0)
        throw eFSServicesGeneric(strName, iFSServices::GetInstance()->GetErrString());

#if HAVE_STRUCT_STAT_ST_RDEV
    // new stuff 7/17/99 - BAM
    // if the file is not a device set rdev to zero by hand (most OSs will
    // do this for us, but some don't)
    if (!S_ISBLK(statbuf.st_mode) && !S_ISCHR(statbuf.st_mode))
    {
        // must zero memory instead of '= 0' since we don't know the
        // actual type of the object -- could be a struct (requiring '= {0}' )
        util_ZeroMemory(statbuf.st_rdev);
    }
#endif

    //copy information returned by lstat call into the structure passed in
    statArgs.gid   = statbuf.st_gid;
    statArgs.atime = statbuf.st_atime;
    statArgs.ctime = statbuf.st_ctime;
    statArgs.mtime = statbuf.st_mtime;
    statArgs.dev   = statbuf.st_dev;

#if HAVE_STRUCT_STAT_ST_RDEV
    statArgs.rdev = statbuf.st_rdev;
#else
    statArgs.rdev = 0;
#endif

    statArgs.ino     = statbuf.st_ino;
    statArgs.mode    = statbuf.st_mode;
    statArgs.nlink   = statbuf.st_nlink;
    statArgs.size    = statbuf.st_size;
    statArgs.uid     = statbuf.st_uid;

#if HAVE_STRUCT_STAT_ST_BLKSIZE    
    statArgs.blksize = statbuf.st_blksize;
#else
    statArgs.blksize = 0;
#endif
    
#if HAVE_STRUCT_STAT_ST_BLOCKS
    statArgs.blocks = statbuf.st_blocks;
#else
    statArgs.blocks = 0;
#endif

    // set the file type
    if (S_ISREG(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_FILE;
#ifdef S_ISDIR    
    else if (S_ISDIR(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_DIR;
#endif    
#ifdef S_ISLNK    
    else if (S_ISLNK(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_SYMLINK;
#endif
#ifdef S_ISBLK
    else if (S_ISBLK(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_BLOCKDEV;
#endif
#ifdef S_ISCHR    
    else if (S_ISCHR(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_CHARDEV;
#endif
#ifdef S_ISFIFO
    else if (S_ISFIFO(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_FIFO;
#endif    
#ifdef S_ISSOCK
    else if (S_ISSOCK(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_SOCK;
#endif
#ifdef S_ISDOOR
    else if (S_ISDOOR(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_DOOR;
#endif
#ifdef S_ISPORT
    else if (S_ISPORT(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_PORT;
#endif
#ifdef S_ISNAM
    else if (S_ISNAM(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_NAMED;
#endif
#ifdef S_ISNATIVE
    else if (S_ISNATIVE(statbuf.st_mode))
        statArgs.mFileType = cFSStatArgs::TY_NATIVE;
#endif
#ifdef S_TYPEISMQ
    else if (S_TYPEISMQ(&statbuf))
        statArgs.mFileType = cFSStatArgs::TY_MESSAGE_QUEUE;
#endif
#ifdef S_TYPEISSEM
    else if (S_TYPEISSEM(&statbuf))
        statArgs.mFileType = cFSStatArgs::TY_SEMAPHORE;
#endif
#ifdef S_TYPEISSHM
    else if (S_TYPEISSHM(&statbuf))
        statArgs.mFileType = cFSStatArgs::TY_SHARED_MEMORY;
#endif
    else
        statArgs.mFileType = cFSStatArgs::TY_INVALID;
}

void cUnixFSServices::GetMachineName(TSTRING& strName) const
{
#if HAVE_SYS_UTSNAME_H
    struct utsname namebuf;
    if (uname(&namebuf) == -1)
        throw eFSServicesGeneric(strName);
    else
        strName = namebuf.nodename;
#else
    strName = "localhost";
#endif
}

void cUnixFSServices::GetMachineNameFullyQualified(TSTRING& strName) const
{
#if HAVE_SYS_UTSNAME_H
    char buf[256];
    if (gethostname(buf, 256) != 0)
    {
#    if defined(SOLARIS_NO_GETHOSTBYNAME) || !SUPPORTS_NETWORKING
        strName = buf;
        return;
#    else
        struct hostent* ret;
        ret = gethostbyname(buf);

        if (ret != NULL)
        {
            strName = ret->h_name;
            return;
        }
#    endif
    }
#endif
    try
    {
        cUnixFSServices::GetMachineName(strName);
    }
    catch (eFSServices&)
    {
        strName = TSS_GetString(cCore, core::STR_UNKNOWN);
    }
}

bool cUnixFSServices::FileDelete(const TSTRING& strName) const
{
    return (0 == remove(strName.c_str()));
}

bool cUnixFSServices::GetCurrentUserName(TSTRING& strName) const
{
#if HAVE_PWD_H  
    bool fSuccess = false;
    uid_t          uid = getuid();
    struct passwd* pp  = getpwuid(uid);

    if (pp)
    {
        strName  = pp->pw_name;
        fSuccess = true;
    }
    else
        strName = _T("");

    return (fSuccess);
#else
    strName = _T("");
    return true;
#endif
}


// returns IP address in network byte order
bool cUnixFSServices::GetIPAddress(uint32_t& uiIPAddress)
{
    bool   fGotAddress = false;
    cDebug d(_T("cUnixFSServices::GetIPAddress"));

#if SUPPORTS_NETWORKING && HAVE_SYS_UTSNAME_H
    struct utsname utsnameBuf;
    if (EFAULT != uname(&utsnameBuf))
    {
        d.TraceDetail("uname returned nodename: %s\n", utsnameBuf.nodename);

        struct hostent* phostent = gethostbyname(utsnameBuf.nodename);

        if (phostent)
        {
            ASSERT(AF_INET == phostent->h_addrtype);
            ASSERT(sizeof(int32_t) == phostent->h_length);

            if (phostent->h_length)
            {
                if (phostent->h_addr_list[0])
                {
                    int32_t* pAddress = reinterpret_cast<int32_t*>(phostent->h_addr_list[0]);
                    uiIPAddress     = *pAddress;
                    fGotAddress     = true;
                }
                else
                    d.TraceError(_T("phostent->h_addr_list[0] was zero"));
            }
            else
                d.TraceError(_T("phostent->h_length was zero"));
        }
        else
            d.TraceError(_T("gethostbyname failed"));
    }
    else
        d.TraceError(_T("uname failed"));
#endif

    return (fGotAddress);
}

bool cUnixFSServices::IsCaseSensitive() const
{
    return true;
}


void cUnixFSServices::SetResolveNames(bool resolve)
{
    mResolveNames = resolve;
}


bool cUnixFSServices::GetUserName(uid_t user_id, TSTRING& tstrUser) const
{
    bool fSuccess = true;

#if HAVE_PWD_H    
    if (mResolveNames)
    {
        struct passwd* pp = getpwuid(user_id);
        if (pp == NULL)
        {
            fSuccess = false;
            tstrUser = TSS_GetString(cCore, core::STR_UNKNOWN);
        }
        else
            tstrUser = pp->pw_name;
    }
    else
    {
#endif      
        TOSTRINGSTREAM sstr;
        sstr << user_id;
        tss_stream_to_string(sstr, tstrUser);
	
#if HAVE_PWD_H	
    }
#endif
    return (fSuccess);
}


bool cUnixFSServices::GetGroupName(gid_t group_id, TSTRING& tstrGroup) const
{
    bool fSuccess = true;

#if !IS_REDOX && HAVE_GRP_H
    if (mResolveNames)
    {
        struct group* pg = getgrgid(group_id);
        if (pg == NULL)
        {
            fSuccess  = false;
            tstrGroup = TSS_GetString(cCore, core::STR_UNKNOWN);
        }
        else
            tstrGroup = pg->gr_name;
    }
    else
    {
#endif
        TOSTRINGSTREAM sstr;
        sstr << group_id;
        tss_stream_to_string(sstr, tstrGroup);
	
#if !IS_REDOX && HAVE_GRP_H
    }
#endif

    return (fSuccess);
}


#ifndef S_ISVTX // DOS/DJGPP doesn't have this
#    define S_ISVTX 0
#endif
#ifndef S_ISUID
#    define S_ISUID 0
#endif
#ifndef S_ISGID
#    define S_ISGID 0
#endif

////////////////////////////////////////////////////////////////////////
// Function name    : cUnixFSServices::ConvertModeToString
// Description      : takes a TSTRING and fills it with an "ls -l" representation
//                    of the object's permission bits ( e.g. "drwxr-x--x" ).
//
// Returns          : void -- no errors are reported
//
// Argument         : uint64_t perm -- st_mode from "stat"
// Argument         : TSTRING& tstrPerm -- converted permissions, ls -l style
//
void cUnixFSServices::ConvertModeToString(uint64_t perm, TSTRING& tstrPerm) const
{
    TCHAR szPerm[12]; //10 permission bits plus the NULL
    strncpy(szPerm, _T("----------"), 11);

    ASSERT(sizeof(unsigned short) <= sizeof(uint32_t));
    // We do this in case an "unsigned short" is ever larger than the
    // value we are switching on, since the size of the mode parameter
    // will be unsigned short (whatever that means, for the given platform...)

    // check file type
    switch ((uint32_t)perm & S_IFMT) //some versions of Unix don't like to switch on
                                   //64 bit values.
    {
#ifdef S_IFDIR
    case S_IFDIR:
        szPerm[0] = _T('d');
        break;
#endif
#ifdef S_IFCHR
    case S_IFCHR:
        szPerm[0] = _T('c');
        break;
#endif
#ifdef S_IFBLK
    case S_IFBLK:
        szPerm[0] = _T('b');
        break;
#endif
#ifdef S_IFIFO
    case S_IFIFO:
        szPerm[0] = _T('p');
        break;
#endif
#ifdef S_IFLNK
    case S_IFLNK:
        szPerm[0] = _T('l');
        break;
#endif
#ifdef S_IFDOOR // Solaris doors
    case S_IFDOOR:
        szPerm[0] = _T('D');
        break;
#endif
#ifdef S_IFPORT // Solaris event ports
    case S_IFPORT:
        szPerm[0] = _T('P');
        break;
#endif
#ifdef S_IFNAM
    case S_IFNAM:
        szPerm[0] = _T('n');
        break;
#endif
#ifdef S_IFNATIVE
    case S_IFNATIVE:
        szPerm[0] = _T('?'); // TODO: check how this looks in qsh
        break;
#endif
        break;
    }

    // check owner read and write
    if (perm & S_IRUSR)
        szPerm[1] = _T('r');
    if (perm & S_IWUSR)
        szPerm[2] = _T('w');

    // check owner execute
    if (perm & S_ISUID && perm & S_IXUSR)
        szPerm[3] = _T('s');
    else if (perm & S_IXUSR)
        szPerm[3] = _T('x');
    else if (perm & S_ISUID)
        szPerm[3] = _T('S');

    // check group read and write
    if (perm & S_IRGRP)
        szPerm[4] = _T('r');
    if (perm & S_IWGRP)
        szPerm[5] = _T('w');

    // check group execute
    if (perm & S_ISGID && perm & S_IXGRP)
        szPerm[6] = _T('s');
    else if (perm & S_IXGRP)
        szPerm[6] = _T('x');
    else if (perm & S_ISGID)
        szPerm[6] = _T('l');

    // check other read and write
    if (perm & S_IROTH)
        szPerm[7] = _T('r');
    if (perm & S_IWOTH)
        szPerm[8] = _T('w');

    // check other execute
    if (perm & S_ISVTX && perm & S_IXOTH)
        szPerm[9] = _T('t');
    else if (perm & S_IXOTH)
        szPerm[9] = _T('x');
    else if (perm & S_ISVTX)
        szPerm[9] = _T('T');

    tstrPerm = szPerm;

    return;
}

////////////////////////////////////////////////////////////////////////
// Function name    : cUnixFSServices::Rename
// Description      : Rename a file.  Overwrites newname if it exists.and overwrite is true
//
// Returns          : false if failure, true on success
bool cUnixFSServices::Rename(const TSTRING& strOldName, const TSTRING& strNewName, bool overwrite) const
{
#ifdef _UNICODE
#    error UNICODE Rename not implemented
#endif

    // delete new file if overwriting
    if (overwrite)
        if (access(strNewName.c_str(), F_OK) == 0 && remove(strNewName.c_str()) != 0)
            return false;

    if (rename(strOldName.c_str(), strNewName.c_str()) == 0)
        return true;

    // Note: errno will be set
    return false;
}


bool cUnixFSServices::GetExecutableFilename(TSTRING& strFullPath, const TSTRING& strFilename) const
{
    bool fGotName = false;

    if (strFilename.empty())
        return false;

    // if there is a slash in the filename, it's absolute or relative to cwd
    if (TSTRING::npos != strFilename.find(_T('/')))
    {
        // if absolute path
        if (strFilename[0] == _T('/'))
        {
            strFullPath = strFilename;
            fGotName    = true;
        }
        else // is relative path; find path from cwd
        {
            fGotName = FullPath(strFullPath, strFilename);
        }
    }
    else // it's just a filename: should be found in path
    {
        fGotName = util_PathFind(strFullPath, strFilename);

        TSTRING strFP;
        if (fGotName && FullPath(strFP, strFullPath))
            strFullPath = strFP;
    }

    return (fGotName);
}


///////////////////////////////////////////////////////////////////////////////
// Function name    : cUnixFSServices::FullPath
// Description      :
//
// Return type      : bool
// Argument         :  TSTRING& strFullPath
// Argument         : const TSTRING& strRelPathC
// Argument         : const TSTRING& pathRelFromC
//
// TODO -- is throwing an exception the more appropriate alternative to returning
//      a bool? I think it is ... mdb
///////////////////////////////////////////////////////////////////////////////
bool cUnixFSServices::FullPath(TSTRING& strFullPath, const TSTRING& strRelPathC, const TSTRING& pathRelFromC) const
{
    cDebug d("cUnixFSServices::FullPath");
    d.TraceDebug("strRelPathC = %s, pathRelFromC = %s\n", strRelPathC.c_str(), pathRelFromC.c_str());

    // don't do anything with an empty path
    if (strRelPathC.empty())
        return false;

#if USES_DEVICE_PATH
    TSTRING strRelPath = cDevicePath::AsPosix(strRelPathC); // make non-const temp var
#else
    TSTRING strRelPath = strRelPathC; // make non-const temp var
#endif
    //
    // get base name (where strRelPath will be relative to), which will either be;
    //  1. the root directory if strRelPath is an absolute path
    //  2. pathRelFrom if it's not empty
    //  3. otherwise ( not abs path AND no rel path ) the current working directory
    //

    if (strRelPath[0] == TW_SLASH) // if is absolute path
    {
        if (IsRoot(strRelPath)) // if it's root, don't monkey with it, just return it.
        {
            strFullPath = strRelPath;
            d.TraceDebug("Is root; returning %s\n", strFullPath.c_str());
            return true;
        }
        else
        {
            strFullPath = _T(""); // push root, then add path elements from strRelPathC
                                  // one by one (in while loop below)
        }
    }
    else // is a relative path, so check pathRelFromC
    {
        if (pathRelFromC.empty()) // if we're relative to CWD...
        {
            //
            // get the current working directory
            //
            try
            {
                GetCurrentDir(strFullPath);
#if USES_DEVICE_PATH
                strFullPath = cDevicePath::AsPosix(strFullPath);
#endif
                util_TrailingSep(strFullPath, false);
            }
            catch (eFSServices&)
            {
                return false;
            }

            d.TraceDebug("Creating prefix relative to CWD: %s\n", strFullPath.c_str());
        }
        else // we're relative to a given dir
        {

#if USES_DEVICE_PATH
            strFullPath = cDevicePath::AsPosix(pathRelFromC);
#else
            strFullPath = pathRelFromC;
#endif
            util_RemoveDuplicateSeps(strFullPath);
            util_TrailingSep(strFullPath, false);

            d.TraceDebug("Creating prefix from supplied path: %s\n", strFullPath.c_str());
        }
    }

    //
    // start adding path elements from strRelPath to the base name
    // ( which already has an absolute starting point.  see above. )
    //

    TSTRING strElem;
    int     index = 0;
    while (util_GetNextPathElement(strRelPath, strElem, index++))
    {
        d.TraceDebug("Path element = %s\n", strElem.c_str());
        if (0 == strElem.compare(_T(".")))
        {
            // ignore it
        }
        else if (0 == strElem.compare(_T("..")))
        {
            // go up a dir ( the function takes care of root dir case )
            TSTRING strDummy;
            util_RemoveLastPathElement(strFullPath, strDummy);
        }
        else // just a regular old path element
        {
            strFullPath += TW_SLASH;
            strFullPath += strElem;
        }

        d.TraceDebug("FullPath is now %s\n", strFullPath.c_str());
    }

#if IS_AROS || IS_REDOX
    strFullPath = cDevicePath::AsNative(strFullPath);
#endif

    d.TraceDebug("Done, returning %s\n", strFullPath.c_str());
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// GetStandardBackupExtension()
//
// Returns normal string to append to backup files for this os.
// (e.g. "~" for unix and ".bak" for winos)
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cUnixFSServices::GetStandardBackupExtension() const
{
    return _T(".bak");
}


void cUnixFSServices::Sleep(int nSeconds) const
{
    sleep(nSeconds);
}


////////////////////////////////////////////////////////////////////////////////
// Function name    : IsRoot
// Description      : A root path is all '/'s
//
// Return type      : bool
// Argument         : const TSTRING& strPath
///////////////////////////////////////////////////////////////////////////////
bool cUnixFSServices::IsRoot(const TSTRING& strPath) const
{
    // and empty path is NOT the root path
    if (strPath.empty())
        return false;

    // check to see if all characters are a slash
    for (TSTRING::const_iterator iter = strPath.begin(); iter != strPath.end(); iter++)
    {
        // if we've found a char that's not '/', then it's not the root path
        if (*iter != TW_SLASH)
            return false;
    }

    return true;
}

//*************************************************************************
//*************************************************************************
//                      UTIL FUNCTION CODE
//*************************************************************************
//*************************************************************************


///////////////////////////////////////////////////////////////////////////////
// Function name    : util_PathFind
// Description      :
//      takes single-element executible filename and looks in path env var for it
//      assumes path is colon-delimited string of directories.
//
// Return type      : bool
// Argument         :  TSTRING& strFullPath
// Argument         : const TSTRING& strFilename
///////////////////////////////////////////////////////////////////////////////
bool util_PathFind(TSTRING& strFullPath, const TSTRING& strFilename)
{
    bool fFoundFile = false;

    if (strFilename.empty())
        return false;

    //
    // get the path environment variable
    //
    TCHAR* pszPathVar = getenv("PATH");
    if (pszPathVar != NULL)
    {
        //
        // cycle over characters in path looking for the ':'
        //
        TSTRING strCurPath;
        TCHAR*  pchTemp    = pszPathVar;
        bool    fMorePaths = true;
        do // while still more paths and haven't found file
        {
            //
            // are we at the ':'?
            //
            if (*pchTemp && *pchTemp != _T(':')) // if we're not at the end of the path
            {
                strCurPath += *pchTemp;
            }
            else // we have found the ':'
            {
                //
                // expand current path into a fully qualified path
                // if it's empty, use current directory
                //
                TSTRING strFP;
                if (strCurPath.empty())
                    strCurPath = _T(".");
                if (iFSServices::GetInstance()->FullPath(strFP, strCurPath))
                    strCurPath = strFP;

                //
                // put the file together with the path dir
                //
                TSTRING strFullName = strCurPath;
                util_TrailingSep(strFullName, true);
                strFullName += strFilename;

                //
                // the file must exist and be executable
                //
                if (util_FileIsExecutable(strFullName))
                {
                    strFullPath = strFullName;
                    fFoundFile  = true;
                }
                else
                    strCurPath.erase(); // start over
            }

            //
            // keep searching if we're not at the end of the path string
            //

            if (*pchTemp)
                pchTemp++;
            else
                fMorePaths = false;
        } while (!fFoundFile && fMorePaths);
    }

    return (fFoundFile);
}


///////////////////////////////////////////////////////////////////////////////
// Function name    : util_FileIsExecutable
// Description      : file ( or file a link points to ) must be a regular
//                    file and executable by someone
//
// Return type      : bool
// Argument         : const TSTRING& strFile
///////////////////////////////////////////////////////////////////////////////
bool util_FileIsExecutable(const TSTRING& strFile)
{
    if (strFile.empty())
        return false;

    struct stat s;
    if (stat(strFile.c_str(), &s) < 0) // this call handles links
        return false;

    return (S_ISREG(s.st_mode) && (s.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))); // can someone execute it?
}


////////////////////////////////////////////////////////////////////////////////
// Function name    : util_RemoveDuplicateSeps
// Description      :
// takes all adjacent slashes and replaces them with a single slash
//      ///root//foo -> /root/foo
//      rel//foo///  -> rel/foo/
//
// Return type      : void
// Argument         : TSTRING& strPath
///////////////////////////////////////////////////////////////////////////////
void util_RemoveDuplicateSeps(TSTRING& strPath)
{
    bool              fLastCharWasSep = false;
    TSTRING::iterator iter            = strPath.begin();
    while (iter != strPath.end())
    {
        bool fErasedChar = false;
        // if we've found a char that's not '/', then it's not the root
        if (*iter == TW_SLASH)
        {
            // if this char is a duplicate sep, erase it
            if (fLastCharWasSep)
            {
                iter        = strPath.erase(iter);
                fErasedChar = true;
            }

            fLastCharWasSep = true;
        }
        else
        {
            fLastCharWasSep = false;
        }

        // don't go past end of string (could happen with erase)
        if (!fErasedChar)
            iter++;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// Function name    : util_RemoveLastPathElement
// Description      :
//      effectively pops off a path element from the end, except for the root dir, where it does nothing
//      it removes any slashes before and after the element
//      ///root//foo/    -> leaves "///root" ("foo"  is strElem)
//      ///root          -> leaves ""        ("root" is strElem)
//      //               -> leaves ""        (""     is strElem)
//
// Return type      : void
// Argument         :  TSTRING& strPath
// Argument         : TSTRING& strElem
/////////////////////////////////////////////////////////////////////////////////
void util_RemoveLastPathElement(TSTRING& strPath, TSTRING& strElem)
{

    // remove all trailing separators
    util_RemoveTrailingSeps(strPath);

    // find the last separator
    TSTRING::size_type lastSep = strPath.rfind(TW_SLASH);

    // if separator was found, take all chars after it
    if (lastSep != TSTRING::npos)
    {
        strElem = strPath.substr(lastSep + 1);
        strPath.resize(lastSep + 1);
    }
    else // no seps in name, take whole string
    {
        // last element
        strElem = strPath;
        strPath.erase();
    }

    // remove all trailing separators
    util_RemoveTrailingSeps(strPath);
}


////////////////////////////////////////////////////////////////////////////////////
// Function name    : util_GetNextPathElement
// Description      :
//      starting from the left side of the path string, returns the index'th path element
//      returns true if the element exists, false if there aren't <index + 1> many elements
//
//      index is ZERO BASED
//
//      2rd element of   ABC/DEF/GH -> GH
//      1st element of //ABC/DEF/GH -> DEF
//
// Return type      : bool : got path element? ( i.e. was there index path elements? )
// Argument         : const TSTRING& strPathC
// Argument         : TSTRING& strElem
// Argument         : int index
/////////////////////////////////////////////////////////////////////////////////
bool util_GetNextPathElement(const TSTRING& strPathC, TSTRING& strElem, int index)
{

    // don't do anything if root or empty
    if (strPathC.empty() || iFSServices::GetInstance()->IsRoot(strPathC))
        return false;

    TSTRING strPath = strPathC; // writable local version

    bool               fMoreSeps = true;
    TSTRING::size_type nextSep, nextNonSep;
    nextSep = nextNonSep = (TSTRING::size_type)-1;
    for (int i = 0; i <= index && fMoreSeps; i++)
    {
        // go past leading separators
        nextNonSep = strPath.find_first_not_of(TW_SLASH, nextSep + 1);

        if (nextNonSep != TSTRING::npos)
        {
            // find index'th slash (start of index'th element)
            nextSep = strPath.find(TW_SLASH, nextNonSep);

            // if we're at the end and we haven't found the index'th element
            // left, then tell the caller that there aren't that many elemnts
            if (nextSep == TSTRING::npos && i < index)
                fMoreSeps = false;
        }
        else
            fMoreSeps = false;
    }

    // get the element and remove it from the path
    if (fMoreSeps)
        strElem = strPath.substr(nextNonSep, nextSep - nextNonSep);

    return (fMoreSeps);
}

/////////////////////////////////////////////////////////////////////////
// Function name    : util_TrailingSep
// Description      : ensure that a path ( fLeaveSep ? "has" : "does not have" ) a trailing slash
//
// Return type      : bool : was there a trailing slash?
// Argument         : TSTRING& str
// Argument         : bool fLeaveSep
/////////////////////////////////////////////////////////////////////////////////
bool util_TrailingSep(TSTRING& str, bool fLeaveSep)
{
    bool fWasSep = false;

    // if there's a trailing sep
    if (!str.empty() && str[str.size() - 1] == TW_SLASH)
    {
        if (!fLeaveSep)
            str.resize(str.size() - 1);
        fWasSep = true;
    }
    else // else no trailing sep
    {
        if (fLeaveSep)
            str += TW_SLASH;
        fWasSep = false;
    }

    return (fWasSep);
}

/////////////////////////////////////////////////////////////////////////
// Function name    : util_RemoveTrailingSeps
// Description      : removes all trailing separators
//
// Return type      : void
// Argument         : TSTRING& str
/////////////////////////////////////////////////////////////////////////////////
void util_RemoveTrailingSeps(TSTRING& str)
{
    while (util_TrailingSep(str, false))
    {
    }
}

template<typename T> static inline void util_ZeroMemory(T& obj)
{
    memset(&obj, 0, sizeof(obj));
}
