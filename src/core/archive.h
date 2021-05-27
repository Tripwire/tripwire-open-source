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
// archive.h -- classes that abstract a raw byte archive
//
// cArchive         -- interface for single-direction (one pass) reads and writes
// cBidirArchive    -- interface for a random-access archive
// cMemArchive      -- implementation of a bidirectional archive in memory
// cFileArchive     -- implementation of a bidirectional archive as a file

#ifndef __ARCHIVE_H
#define __ARCHIVE_H

#ifndef __DEBUG_H
#include "debug.h"
#endif
#ifndef __ERRORUTIL_H
#include "errorutil.h"
#endif
#ifndef __FILEERROR_H
#include "fileerror.h"
#endif
#ifndef __FILE_H
#include "file.h"
#endif

//=============================================================================
// eArchive exception classes
//=============================================================================
TSS_FILE_EXCEPTION(eArchive, eFileError);
TSS_FILE_EXCEPTION(eArchiveOpen, eArchive);
TSS_FILE_EXCEPTION(eArchiveWrite, eArchive);
TSS_FILE_EXCEPTION(eArchiveRead, eArchive);
TSS_FILE_EXCEPTION(eArchiveEOF, eArchive);
TSS_FILE_EXCEPTION(eArchiveSeek, eArchive);
TSS_FILE_EXCEPTION(eArchiveMemmap, eArchive);
TSS_FILE_EXCEPTION(eArchiveOutOfMem, eArchive);
TSS_FILE_EXCEPTION(eArchiveInvalidOp, eArchive);
TSS_FILE_EXCEPTION(eArchiveFormat, eArchive);
TSS_FILE_EXCEPTION(eArchiveNotRegularFile, eArchive);
TSS_BEGIN_EXCEPTION(eArchiveCrypto, eArchive)

virtual TSTRING GetMsg() const;
// eCryptoArchive appends a special string to the end of
// all exception messages
TSS_END_EXCEPTION()
TSS_EXCEPTION(eArchiveStringTooLong, eArchive);

//      throw( eArchiveOpen( cErrorUtil::MakeFileError( fileError.GetMsg(), strTempFile ) ) );


//=============================================================================
// cArchive
//=============================================================================

class cArchive
{
public:
    virtual ~cArchive()
    {
    }

    // convenience methods
    //
    // Specific Read functions throw(eArchive) if EOF is reached because
    // if the caller is requesting a certain amount of data to be present,
    // reaching EOF is unexpected
    //
    // ReadBlob and WriteBlob return number of bytes read or written.  Notice
    // that ReadBlob does not throw an exception since eventually EOF is expected.
    //
    // ReadBlob can take NULL as a destination pointer
    //
    // All write functions throw exceptions for unexpected events like
    // running out of memory or disk space.
    //
    void ReadInt16(int16_t& ret);    // throw(eArchive)
    void ReadInt32(int32_t& ret);    // throw(eArchive)
    void ReadInt64(int64_t& ret);    // throw(eArchive)
    void ReadString(TSTRING& ret); // throw(eArchive)
    int  ReadBlob(void* pBlob, int count);
    void WriteInt16(int16_t i);                     // throw(eArchive)
    void WriteInt32(int32_t i);                     // throw(eArchive)
    void WriteInt64(int64_t i);                     // throw(eArchive)
    void WriteString(const TSTRING& s);                  // throw(eArchive)
    void WriteBlob(const void* pBlob, int count); // throw(eArchive)

    static int32_t GetStorageSize(const TSTRING& str);
    // this method calculates how many bytes the given string will take up in the archive and returns
    // that value
    // NOTE -- if the implementation of ReadString() or WriteString() ever changes, this method will also
    //          need to change.

    int64_t Copy(cArchive* pFrom, int64_t amt); // throw(eArchive)
        // this method copies amt bytes from pFrom to itself, throwing an eArchive if anything goes wrong.

    // only makes sense to call for reading archives
    virtual bool EndOfFile() = 0;

protected:
    // overrides
    virtual int Read(void* pDest, int count)        = 0;
    virtual int Write(const void* pDest, int count) = 0; // throw(eArchive);
};

///////////////////////////////////////////////////////////////////////////////
// class cBidirArchive --
///////////////////////////////////////////////////////////////////////////////

class cBidirArchive : public cArchive
{
public:
    enum SeekFrom
    {
        BEGINNING = 0,
        CURRENT   = 1,
        END       = -1
    };

    virtual void  Seek(int64_t offset, SeekFrom from) = 0; // throw(eArchive);
    virtual int64_t CurrentPos() const                = 0;
    virtual int64_t Length() const                    = 0;
};

///////////////////////////////////////////////////////////////////////////////
// class cMemMappedArchive -- Archive that can be memory mapped.
///////////////////////////////////////////////////////////////////////////////

class cMemMappedArchive : public cBidirArchive
{
public:
    enum
    {
        MAP_TO_EOF = -1
    };

    cMemMappedArchive();
    virtual ~cMemMappedArchive();

    virtual void MapArchive(int64_t offset, int64_t len)       = 0; // throw(eArchive);
    virtual void MapArchive(int64_t offset, int64_t len) const = 0; // throw(eArchive);
        // the const version of MapArchive() does not allow the archive to grow in size

    int64_t     GetMappedOffset() const; // throw(eArchive)
    int64_t     GetMappedLength() const; // throw(eArchive)
    void*       GetMap();                // throw(eArchive)
    const void* GetMap() const;

protected:
    mutable void*   mpMappedMem;
    mutable int64_t mMappedOffset;
    mutable int64_t mMappedLength;

    // call in derived class to set above vars
    void SetNewMap(void* pMap, int64_t offset, int64_t length) const;
};

///////////////////////////////////////////////////////////////////////////////
// class cMemoryArchive -- An archive that stores itself in a memory buffer.
//      This buffer can grow as needed up until a pre-specified maximum
//      size.  The buffer can be read and written to and can be memory
//      mapped.
///////////////////////////////////////////////////////////////////////////////

class cMemoryArchive : public cMemMappedArchive
{
public:
    cMemoryArchive(int maxSize = 0x8000000); // default max size == 128MB
    ~cMemoryArchive();

    virtual bool  EndOfFile();
    virtual void  Seek(int64_t offset, SeekFrom from); // throw(eArchive)
    virtual int64_t CurrentPos() const;
    virtual int64_t Length() const;
    virtual void  MapArchive(int64_t offset, int64_t len);       // throw(eArchive)
    virtual void  MapArchive(int64_t offset, int64_t len) const; // throw(eArchive)

    void Truncate(); // set the length to the current pos

    int8_t* GetMemory() const
    {
        return mpMemory;
    }

protected:
    int8_t* mpMemory;
    int     mAllocatedLen;
    int     mMaxAllocatedLen;
    int     mLogicalSize;
    int     mReadHead;

    virtual int  Read(void* pDest, int count);
    virtual int  Write(const void* pDest, int count); // throw(eArchive)
    virtual void AllocateMemory(int len);             // throw(eArchive)
};

///////////////////////////////////////////////////////////////////////////////
// cFixedMemArchive -- a memory archive that operates on a fixed-sized block of
//      memory that has already been allocated
///////////////////////////////////////////////////////////////////////////////
class cFixedMemArchive : public cBidirArchive
{
public:
    cFixedMemArchive();
    cFixedMemArchive(int8_t* pMem, int32_t size);
    virtual ~cFixedMemArchive();

    void Attach(int8_t* pMem, int32_t size);
    // this method associates the archive with pMem and sets the size of the
    // archive. Unlike cMemoryArchive, this may never grow or shrink in size.

    //-----------------------------------
    // cBidirArchive interface
    //-----------------------------------
    virtual void  Seek(int64_t offset, SeekFrom from); // throw(eArchive);
    virtual int64_t CurrentPos() const;
    virtual int64_t Length() const;
    virtual bool  EndOfFile();

protected:
    //-----------------------------------
    // cArchive interface
    //-----------------------------------
    virtual int Read(void* pDest, int count);        // throw(eArchive)
    virtual int Write(const void* pDest, int count); // throw(eArchive)

    int8_t* mpMemory;
    int32_t mSize;
    int32_t mReadHead;
};

class cFileArchive : public cBidirArchive
{
public:
    cFileArchive();
    virtual ~cFileArchive();

    enum OpenFlags
    {
        FA_OPEN_TEXT     = 0x1,
        FA_OPEN_TRUNCATE = 0x2,
        FA_SCANNING      = 0x4,
        FA_DIRECT        = 0x8
    };

    // TODO: Open should throw
    virtual void OpenRead(const TCHAR* filename, uint32_t openFlags = 0);
    virtual void OpenReadWrite(const TCHAR* filename, uint32_t openFlags = FA_OPEN_TRUNCATE);
    // opens a file for reading or writing; the file is always created if it doesn't exist,
    // and is truncated to zero length if truncateFile is set to true;
    TSTRING      GetCurrentFilename(void) const;
    virtual void Close(void);
    void         Truncate(); // throw(eArchive) // set the length to the current pos

    //-----------------------------------
    // cBidirArchive interface
    //-----------------------------------
    virtual bool  EndOfFile();
    virtual void  Seek(int64_t offset, SeekFrom from); // throw(eArchive)
    virtual int64_t CurrentPos() const;
    virtual int64_t Length() const;


protected:
    int64_t mFileSize; //Size of FileArchive
    int64_t mReadHead; //Current position of read/write head
    //-----------------------------------
    // cArchive interface
    //-----------------------------------
    virtual int Read(void* pDest, int count);
    virtual int Write(const void* pDest, int count); //throw(eArchive)
    bool        isWritable;
    cFile       mCurrentFile;
    TSTRING     mCurrentFilename; //current file
    uint32_t    mOpenFlags;
};

///////////////////////////////////////////////////////////////
// cLockedTemporaryFileArchive -- this class uses as an archive a file
//      that is not accessable to any other process besides the calling one.
//      the file will be deleted from the filesystem on Close();
//
//      implemented by using cFileArchive and overwriting open and close
//      functions
//
class cLockedTemporaryFileArchive : public cFileArchive
{
public:
    virtual void OpenReadWrite(const TCHAR* filename = NULL, uint32_t openFlags = FA_OPEN_TRUNCATE);
    // creates the file.  filename must not exist on the file system.
    // if filename is NULL, the class will create and use a temporary file.
    // truncateFile has no meaning
    //virtual void  OpenReadWriteThrow  ( const TCHAR* filename = NULL, bool truncateFile = true ) throw (eArchive);
    // this is the same as OpenReadWrite, except an exception is thrown on error (of type
    // cArchive::ERR_OPEN_FAILED)

    virtual void Close();
    // close and delete the file
private:
    // open for read only makes no sense if we're always creating the file,
    // so disallow read only file opens
    virtual void OpenRead(const TCHAR*, uint32_t openFlags = 0)
    {
        ASSERT(false);
        THROW_INTERNAL("archive.h");
    }
};


#endif
