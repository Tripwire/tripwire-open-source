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
// file.h : Interface for cFile class, which abstracts file operations across
//      different platforms (currently just Windows and Unix...)

#ifndef __FILE_H
#define __FILE_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __FILEERROR_H
#include "fileerror.h"
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

//=============================================================================
// eFile exception class
//=============================================================================

TSS_FILE_EXCEPTION(eFile, eFileError);
TSS_FILE_EXCEPTION(eFileOpen, eFile);
TSS_FILE_EXCEPTION(eFileWrite, eFile);
TSS_FILE_EXCEPTION(eFileRead, eFile);
TSS_FILE_EXCEPTION(eFileEOF, eFile); // never used!
TSS_FILE_EXCEPTION(eFileSeek, eFile);
TSS_FILE_EXCEPTION(eFileInvalidOp, eFile); // never used!
TSS_FILE_EXCEPTION(eFileTrunc, eFile);
TSS_FILE_EXCEPTION(eFileClose, eFile); // never used!
TSS_FILE_EXCEPTION(eFileFlush, eFile);
TSS_FILE_EXCEPTION(eFileRewind, eFile);

//=============================================================================
// cFile
//=============================================================================
struct cFile_i;
class cFile
{
public:
    typedef off_t File_t;

    enum SeekFrom
    {
        SEEK_BEGIN = 0,
        SEEK_CURRENT,
        SEEK_EOF
    };

    enum OpenFlags
    {
        // note that reading from the file is implicit
        OPEN_READ  = 0x00000001, // not needed, but makes calls nice...
        OPEN_WRITE = 0x00000002, // we will be writing to the file
        OPEN_LOCKED_TEMP =
            0x00000004, // the file should not be readable by other processes and should be removed when closed
        OPEN_TRUNCATE =
            0x00000008, // opens an empty file. creates it if it doesn't exist. Doesn't make much sense without OF_WRITE
        OPEN_CREATE    = 0x00000010, // create the file if it doesn't exist; this is implicit if OF_TRUNCATE is set
        OPEN_TEXT      = 0x00000020,
        OPEN_EXCLUSIVE = 0x00000040, // Use O_CREAT | O_EXCL
        OPEN_SCANNING  = 0x00000080, // Open for scanning; set nonblocking & caching accordingly, where available
        OPEN_DIRECT    = 0x00000100  // Use O_DIRECT or platform equivalent
    };

    //Ctor, Dtor, CpyCtor, Operator=:
    cFile(void);
    ~cFile(void);

    /************ User Interface **************************/

    // Both Open methods ALWAYS open files in BINARY mode!
    void Open(const TSTRING& sFileName, uint32_t flags = OPEN_READ); //throw(eFile)
    void Close(void);                                              //throw(eFile)
    bool IsOpen(void) const;

    File_t Seek(File_t offset, SeekFrom From) const; //throw(eFile)
        // Seek returns the current offset after completion
    File_t Read(void* buffer, File_t nBytes) const; //throw(eFile)
        // Read returns the number of bytes that are actually read.  If the nBytes
        // parameter is 0, 0 bytes will be read and buffer will remain untouched.
        // If the read head is at EOF, no bytes will be read and 0 will be returned.
    File_t Write(const void* buffer, File_t nBytes); //throw(eFile)
        // Write returns the number of bytes that are actually written.
    File_t Tell(void) const;
    // Tell returns the current offset.
    bool Flush(void); //throw(eFile)
        // Flush returns 0 if the currently defined stream is successfully flushed.
    void Rewind(void) const; //throw(eFile)
        // Sets the offset to 0.
    File_t GetSize(void) const;
    // Returns the size of the current file in bytes.  Returns -1 if no file is defined.
    void Truncate(File_t offset); // throw(eFile)

private:
    cFile(const cFile& rhs);            //not impl.
    cFile& operator=(const cFile& rhs); //not impl.

    //Pointer to the insulated implementation
    cFile_i* mpData;

public:
    bool isWritable;
};


class cDosPath
{
public:
    static TSTRING AsPosix(const TSTRING& in);
    static TSTRING AsNative(const TSTRING& in);
    static bool    IsAbsolutePath(const TSTRING& in);
    static TSTRING BackupName(const TSTRING& in);
};

class cArosPath
{
public:
    static TSTRING AsPosix(const TSTRING& in);
    static TSTRING AsNative(const TSTRING& in);
    static bool    IsAbsolutePath(const TSTRING& in);
};

class cRiscosPath
{
public:
    static TSTRING AsPosix(const TSTRING& in);
    static TSTRING AsNative(const TSTRING& in);
    static bool    IsAbsolutePath(const TSTRING& in);
};

class cRedoxPath
{
public:
    static TSTRING AsPosix(const TSTRING& in);
    static TSTRING AsNative(const TSTRING& in);
    static bool    IsAbsolutePath(const TSTRING& in);
};

#    if USES_DOS_DEVICE_PATH
#        define cDevicePath cDosPath
#    elif IS_AROS
#        define cDevicePath cArosPath
#    elif IS_RISCOS
#        define cDevicePath cRiscosPath
#    elif IS_REDOX
#        define cDevicePath cRedoxPath
#    endif


#endif //__FILE_H
