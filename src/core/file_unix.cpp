
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
// file_unix.cpp : Specific implementation of file operations for Unix.

#include "core/stdcore.h"
#include "core/file.h"

//#include <stdio.h>
#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
//#include <errno.h>

#if HAVE_SYS_FS_VX_IOCTL_H
#include <sys/fs/vx_ioctl.h>
#endif

#include "core/debug.h"
#include "core/corestrings.h"
#include "core/fsservices.h"
#include "core/errorutil.h"

#if HAVE_UNIXLIB_LOCAL_H
#include <unixlib/local.h>
#endif

///////////////////////////////////////////////////////////////////////////
// cFile_i : Insulated implementation for cFile objects.
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
struct cFile_i
{
    cFile_i();
    ~cFile_i();

    int     m_fd;         //underlying file descriptor
    FILE*   mpCurrStream; //currently defined file stream
    TSTRING mFileName;    //the name of the file we are currently referencing.
    uint32_t  mFlags;       //Flags used to open the file
};

//Ctor
cFile_i::cFile_i() : m_fd(-1), mpCurrStream(NULL), mFlags(0)
{
}

//Dtor
cFile_i::~cFile_i()
{
    if (mpCurrStream != NULL)
    {
        fclose(mpCurrStream);
        mpCurrStream = NULL;

#if !CAN_UNLINK_WHILE_OPEN // so unlink after close instead
        if (mFlags & cFile::OPEN_LOCKED_TEMP)
        {
            // unlink this file
            if (0 != unlink(mFileName.c_str()))
            {
                throw(eFileOpen(mFileName, iFSServices::GetInstance()->GetErrString()));
            }
        }
#endif
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// cFile () -- Implements file operations
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

cFile::cFile() : mpData(NULL), isWritable(false)
{
    mpData = new cFile_i;
}

cFile::~cFile()
{
    if (mpData != NULL)
    {
        delete mpData;
        mpData = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Open
///////////////////////////////////////////////////////////////////////////////

#if !USES_DEVICE_PATH
void cFile::Open(const TSTRING& sFileName, uint32_t flags)
{
#else
void cFile::Open(const TSTRING& sFileNameC, uint32_t flags)
{
    TSTRING sFileName = cDevicePath::AsNative(sFileNameC);
#endif
    mode_t openmode = 0664;
    if (mpData->mpCurrStream != NULL)
        Close();

    mpData->mFlags = flags;

    //
    // set up the open permissions
    //
    int perm = 0;

    TSTRING mode;

    if (flags & OPEN_WRITE)
    {
        perm |= O_RDWR;
        isWritable = true;
        mode       = _T("rb");
        if (flags & OPEN_TRUNCATE)
        {
            perm |= O_TRUNC;
            perm |= O_CREAT;
            mode = _T("w+b");
        }
        else
            mode = _T("r+b");
    }
    else
    {
        perm |= O_RDONLY;
        isWritable = false;
        mode       = _T("rb");
    }

    if (flags & OPEN_EXCLUSIVE)
    {
        perm |= O_CREAT | O_EXCL;
        openmode = (mode_t)0600; // Make sure only root can read the file
    }

    if (flags & OPEN_CREATE)
        perm |= O_CREAT;

#ifdef O_NONBLOCK
    if (flags & OPEN_SCANNING)
        perm |= O_NONBLOCK;
#endif

#ifdef O_NOATIME
    if (flags & OPEN_SCANNING)
        perm |= O_NOATIME;
#endif

#ifdef O_DIRECT
    //Only use O_DIRECT for scanning, since cfg/policy/report reads
    // don't happen w/ a nice round block size.
    if ((flags & OPEN_DIRECT) && (flags & OPEN_SCANNING))
        perm |= O_DIRECT;
#endif

    //
    // actually open the file
    //
    int fh = _topen(sFileName.c_str(), perm, openmode);
    if (fh == -1)
    {
        throw(eFileOpen(sFileName, iFSServices::GetInstance()->GetErrString()));
    }
    mpData->m_fd = fh;

#if CAN_UNLINK_WHILE_OPEN
    if (flags & OPEN_LOCKED_TEMP)
    {
        // unlink this file
        if (0 != unlink(sFileName.c_str()))
        {
            // we weren't able to unlink file, so close handle and fail
            close(fh);
            throw(eFileOpen(sFileName, iFSServices::GetInstance()->GetErrString()));
        }
    }
#endif

    //
    // turn the file handle into a FILE*
    //
    mpData->mpCurrStream = _tfdopen(fh, mode.c_str());

    mpData->mFileName = sFileName; //Set mFileName to the newly opened file.

    cFile::Rewind();

#ifdef F_NOCACHE //OSX
    if ((flags & OPEN_DIRECT) && (flags & OPEN_SCANNING))
        fcntl(fh, F_NOCACHE, 1);
#endif

#if HAVE_DIRECTIO // Solaris
    if ((flags & OPEN_DIRECT) && (flags & OPEN_SCANNING))
        directio(fh, DIRECTIO_ON);
#endif

#if SUPPORTS_POSIX_FADVISE
    if (flags & OPEN_SCANNING && !(flags & OPEN_DIRECT))
    {
#ifdef POSIX_FADV_SEQUENTIAL
        posix_fadvise(fh, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

#ifdef POSIX_FADV_NOREUSE
        posix_fadvise(fh, 0, 0, POSIX_FADV_NOREUSE);
#endif
    }

#elif HAVE_SYS_FS_VX_IOCTL_H
    if (flags & OPEN_SCANNING)
    {
        if (flags & OPEN_DIRECT)
            ioctl(fh, VX_SETCACHE, VX_DIRECT);
        else
            ioctl(fh, VX_SETCACHE, VX_SEQ | VX_NOREUSE);
    }
#endif
}


///////////////////////////////////////////////////////////////////////////
// Close -- Closes mpCurrStream and sets the pointer to NULL
///////////////////////////////////////////////////////////////////////////
void cFile::Close() //throw(eFile)
{
    if (mpData->mpCurrStream != NULL)
    {
#if (SUPPORTS_POSIX_FADVISE && defined(POSIX_FADV_DONTNEED))
        posix_fadvise(fileno(mpData->mpCurrStream), 0, 0, POSIX_FADV_DONTNEED);
#endif

        fclose(mpData->mpCurrStream);
        mpData->mpCurrStream = NULL;
    }

    //mpData->mFileName.clear();
}

bool cFile::IsOpen(void) const
{
    return (mpData->mpCurrStream != NULL);
}

// Autoconf docs say HAVE_FSEEKO applies to both fseeko & ftello
#if HAVE_FSEEKO
    #define tss_fseek fseeko
    #define tss_ftell ftello
#else
    #define tss_fseek fseek
    #define tss_ftell ftell
#endif

///////////////////////////////////////////////////////////////////////////
// Seek -- Positions the read/write offset in mpCurrStream.  Returns the
//      current offset upon completion.  Returns 0 if no stream is defined.
///////////////////////////////////////////////////////////////////////////
cFile::File_t cFile::Seek(File_t offset, SeekFrom From) const //throw(eFile)
{
    //Check to see if a file as been opened yet...
    ASSERT(mpData->mpCurrStream != 0);

    int apiFrom;

    switch (From)
    {
    case cFile::SEEK_BEGIN:
        apiFrom = SEEK_SET;
        break;
    case cFile::SEEK_CURRENT:
        apiFrom = SEEK_CUR;
        break;
    case cFile::SEEK_EOF:
        apiFrom = SEEK_END;
        break;
    default:
        //An invalid SeekFrom parameter was passed.
        throw(eInternal(_T("file_unix")));
    }

// this is a hack to simulate running out of disk space
#if 0
    static int blowupCount = 1;
    if (++blowupCount == 1075)
    {
        fputs("***** faking seek failure!\n", stderr);
        //throw eFileSeek();
        throw std::bad_alloc();
    }
    fprintf(stderr, "%d\n", blowupCount);
#endif


    
    if (tss_fseek(mpData->mpCurrStream, offset, apiFrom) != 0)
    {
#ifdef DEBUG
        cDebug d("cFile::Seek");
        d.TraceDebug("Seek failed!\n");
#endif
        throw eFileSeek();
    }

    return tss_ftell(mpData->mpCurrStream);
}

///////////////////////////////////////////////////////////////////////////
// Read -- Returns the actual bytes read from mpCurrStream.  Returns 0 if
//      mpCurrStream is undefined.
///////////////////////////////////////////////////////////////////////////
cFile::File_t cFile::Read(void* buffer, File_t nBytes) const //throw(eFile)
{
    File_t iBytesRead;

    // Has a file been opened?
    ASSERT(mpData->mpCurrStream != NULL);

    // Is the nBytes parameter 0?  If so, return without touching buffer:
    if (nBytes == 0)
        return 0;

    if (mpData->mFlags & OPEN_DIRECT)
    {
#if READ_TAKES_CHAR_PTR
        iBytesRead = read(mpData->m_fd, (char*)buffer, nBytes);
#else
        iBytesRead = read(mpData->m_fd, buffer, nBytes);
#endif
        if (iBytesRead < 0)
        {
            throw eFileRead(mpData->mFileName, iFSServices::GetInstance()->GetErrString());
        }
    }
    else
    {
        iBytesRead = fread(buffer, sizeof(uint8_t), nBytes, mpData->mpCurrStream);
        if (ferror(mpData->mpCurrStream) != 0)
        {
            throw eFileRead(mpData->mFileName, iFSServices::GetInstance()->GetErrString());
        }
    }

    return iBytesRead;
}

///////////////////////////////////////////////////////////////////////////
// Write -- Returns the actual number of bytes written to mpCurrStream
//      Returns 0 if no file has been opened.
///////////////////////////////////////////////////////////////////////////
cFile::File_t cFile::Write(const void* buffer, File_t nBytes) //throw(eFile)
{
    File_t actual_count = 0;

    // Has a file been opened? Is it writable?
    ASSERT(mpData->mpCurrStream != NULL);
    ASSERT(isWritable);

    if ((actual_count = fwrite(buffer, sizeof(uint8_t), nBytes, mpData->mpCurrStream)) < nBytes)
        throw eFileWrite(mpData->mFileName, iFSServices::GetInstance()->GetErrString());
    else
        return actual_count;
}

///////////////////////////////////////////////////////////////////////////
// Tell -- Returns the current file offset.  Returns 0 if no file has been
//      opened.
///////////////////////////////////////////////////////////////////////////
cFile::File_t cFile::Tell() const
{
    ASSERT(mpData->mpCurrStream != 0);

    return ftell(mpData->mpCurrStream);
}

///////////////////////////////////////////////////////////////////////////
// Flush -- Flushes the current stream.
///////////////////////////////////////////////////////////////////////////
bool cFile::Flush() //throw(eFile)
{
    if (mpData->mpCurrStream == NULL)
        throw eFileFlush(mpData->mFileName, iFSServices::GetInstance()->GetErrString());

    return (fflush(mpData->mpCurrStream) == 0);
}
///////////////////////////////////////////////////////////////////////////
// Rewind -- Sets the offset to the beginning of the file.  If mpCurrStream
//      is NULL, this method returns false. If the rewind operation fails,
//      an exception is thrown.
///////////////////////////////////////////////////////////////////////////
void cFile::Rewind() const //throw(eFile)
{
    ASSERT(mpData->mpCurrStream != 0);

    rewind(mpData->mpCurrStream);
    if (ftell(mpData->mpCurrStream) != 0)
        throw(eFileRewind(mpData->mFileName, iFSServices::GetInstance()->GetErrString()));
}

///////////////////////////////////////////////////////////////////////////
// GetSize -- Returns the size of the current stream, if one has been
//      opened. If no stream has been opened, returns -1.
///////////////////////////////////////////////////////////////////////////
cFile::File_t cFile::GetSize() const
{
    File_t vCurrentOffset = Tell(); //for saving the current offset
    File_t ret;

    //Has a file been opened? If not, return -1
    if (mpData->mpCurrStream == NULL)
        return -1;

    ret = Seek(0, cFile::SEEK_EOF);
    Seek(vCurrentOffset, cFile::SEEK_BEGIN);
    //return the offset to it's position prior to GetSize call.

    return ret;
}

/////////////////////////////////////////////////////////////////////////
// Truncate
/////////////////////////////////////////////////////////////////////////
void cFile::Truncate(File_t offset) // throw(eFile)
{
    ASSERT(mpData->mpCurrStream != 0);
    ASSERT(isWritable);

    ftruncate(fileno(mpData->mpCurrStream), offset);
    if (GetSize() != offset)
        throw(eFileTrunc(mpData->mFileName, iFSServices::GetInstance()->GetErrString()));
}


/////////////////////////////////////////////////////////////////////////
// Platform path conversion methods
/////////////////////////////////////////////////////////////////////////

bool cDosPath::IsAbsolutePath(const TSTRING& in)
{
    if (in.empty())
        return false;

    if (in[0] == '/')
        return true;

    if (in.length() >= 2 && in[1] == ':')
        return true;

    return false;
}

// For paths of type C:\DOS
TSTRING cDosPath::AsPosix(const TSTRING& in)
{
#if (defined(__MINGW32__) || defined(__OS2__))
    return in;
#else  
    if (in[0] == '/')
    {
        return in;
    }

    TSTRING out = (cDosPath::IsAbsolutePath(in)) ? ("/dev/" + in) : in;
    std::replace(out.begin(), out.end(), '\\', '/');
    out.erase(std::remove(out.begin(), out.end(), ':'), out.end());

    return out;
#endif    
}

TSTRING cDosPath::AsNative(const TSTRING& in)
{
    if (in[0] != '/')
    {
        return in;
    }

    if (in.find("/dev") != 0 || in.length() < 6)
        return in;

    TSTRING out = "?:/";
    out[0]      = in[5];

    if (in.length() >= 8)
        out.append(in.substr(7));

    std::replace(out.begin(), out.end(), '/', '\\');

    return out;
}

TSTRING cDosPath::BackupName(const TSTRING& in)
{
    TSTRING                out = in;
    std::string::size_type pos = out.find_last_of("\\");
    if (std::string::npos == pos)
        return in;

    TSTRING path = in.substr(0, pos);
    TSTRING name = in.substr(pos, 9);
    std::replace(name.begin(), name.end(), '.', '_');
    path.append(name);

    return path;
}

/////////////////////////////////////////////////////////////////////////
bool cArosPath::IsAbsolutePath(const TSTRING& in)
{
    if (in.empty())
        return false;

    if (in[0] == '/')
        return true;

    if (in.find(":") != std::string::npos)
        return true;

    return false;
}

// For paths of type DH0:dir/file
TSTRING cArosPath::AsPosix(const TSTRING& in)
{
    if (in[0] == '/')
    {
        return in;
    }

    TSTRING out = IsAbsolutePath(in) ? '/' + in : in;
    std::replace(out.begin(), out.end(), ':', '/');

    return out;
}

TSTRING cArosPath::AsNative(const TSTRING& in)
{
    if (in[0] != '/')
    {
        return in;
    }

    std::string::size_type drive = in.find_first_not_of("/");
    TSTRING                out   = (drive != std::string::npos) ? in.substr(drive) : in;
    TSTRING::size_type     t     = out.find_first_of('/');
    if (t != std::string::npos)
        out[t] = ':';
    else
        out.append(":");

    return out;
}

/////////////////////////////////////////////////////////////////////////
bool cRiscosPath::IsAbsolutePath(const TSTRING& in)
{
    if (in.empty())
        return false;

    if (in[0] == '/')
        return true;

    if (in.find("$") != std::string::npos)
        return true;

    return false;
}

// For paths of type SDFS::Volume.$.dir.file
TSTRING cRiscosPath::AsPosix(const TSTRING& in)
{
#if HAVE_UNIXLIB_LOCAL_H
    if (in[0] == '/')
    {
        return in;
    }

    TSTRING out;
    char*   unixified = __unixify(in.c_str(), 0, 0, 0, 0);
    if (unixified)
    {
        out.assign(unixified);
        free(unixified);
        return out;
    }
    return in;

#else
    return in;
#endif
}

TSTRING cRiscosPath::AsNative(const TSTRING& in)
{
#if HAVE_UNIXLIB_LOCAL_H
    if (in[0] != '/')
    {
        return in;
    }

    TSTRING           out;
    int               buf_size = in.length() + 100; // examples pad by 100
    std::vector<char> buf(buf_size);
    __riscosify(in.c_str(), 0, 0, &buf[0], buf_size, 0);
    if (buf[0])
    {
        out.assign(&buf[0]);
        return out;
    }
    return in;
#else
    return in;
#endif
}


/////////////////////////////////////////////////////////////////////////
bool cRedoxPath::IsAbsolutePath(const TSTRING& in)
{
    if (in.empty())
        return false;

    if (in[0] == '/')
        return true;

    if (in.find(":") != std::string::npos)
        return true;

    return false;
}

// For paths of type file:/dir/file
TSTRING cRedoxPath::AsPosix(const TSTRING& in)
{
    if (in[0] == '/')
    {
        return in;
    }

    TSTRING                out   = IsAbsolutePath(in) ? '/' + in : in;
    std::string::size_type colon = out.find_first_of(":");
    if (colon != std::string::npos)
        out.erase(colon, 1);
    return out;
}

TSTRING cRedoxPath::AsNative(const TSTRING& in)
{
    if (in[0] != '/')
    {
        return in;
    }

    std::string::size_type drive = in.find_first_not_of("/");
    TSTRING                out   = (drive != std::string::npos) ? in.substr(drive) : in;
    TSTRING::size_type     slash = out.find_first_of('/');
    if (slash != std::string::npos)
        out.insert(slash, ":");
    else
        out.append(":/");

    return out;
}
