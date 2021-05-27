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
// archive.cpp -- classes that abstract a raw byte archive
//
// cArchive         -- interface for single-direction (one pass) reads and writes
// cBidirArchive    -- interface for a random-access archive
// cMemArchive      -- implementation of a bidirectional archive in memory
// cFileArchive     -- implementation of a bidirectional archive as a file

#include "stdcore.h"
#include "archive.h"
#include "fsservices.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include "file.h"
#include "stringutil.h"

#include "corestrings.h" // for: STR_ERR2_ARCH_CRYPTO_ERR

//=============================================================================
// eArchiveCrypto
//=============================================================================
TSTRING eArchiveCrypto::GetMsg() const
{
    // RAD: Updated this to use new stringtable
    return (mMsg + TSS_GetString(cCore, core::STR_ERR2_ARCH_CRYPTO_ERR));
}


//=============================================================================
// cArchive
//=============================================================================

// convenience methods
//
// Specific Read functions throw eArchive if EOF is reached because
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

void cArchive::ReadInt16(int16_t& ret) // throw(eArchive)
{
    if (ReadBlob(&ret, sizeof(int16_t)) != sizeof(int16_t))
        throw eArchiveEOF();

    ret = tw_ntohs(ret);
}

void cArchive::ReadInt32(int32_t& ret) // throw(eArchive)
{
    if (ReadBlob(&ret, sizeof(int32_t)) != sizeof(int32_t))
        throw eArchiveEOF();

    ret = tw_ntohl(ret);
}

void cArchive::ReadInt64(int64_t& ret) // throw(eArchive)
{
    if (ReadBlob(&ret, sizeof(int64_t)) != sizeof(int64_t))
        throw eArchiveEOF();

    ret = tw_ntohll(ret);
}

// NOTE:BAM 10/11/99 -- we store unsigned size, but it really only works with
// lengths < INT16_MAX due to sign extension in integral promotion during the
// resize() in ReadString().
// format for written string: 16-bit unsigned size, then a list of 16-bit UCS2 (Unicode) characters
// not including terminating NULL
void cArchive::ReadString(TSTRING& ret) // throw(eArchive)
{
    // read in size of string
    int16_t size;
    ReadInt16(size);

    // create buffer for WCHAR16 string
    wc16_string ws;
    ws.resize(size);
    WCHAR16* pwc = (WCHAR16*)ws.data();

    for (int n = 0; n < size; n++)
    {
        int16_t i16;
        ReadInt16(i16);
        *pwc++ = i16;
    }

    // convert WCHAR16 string to a TSTRING
    ret = cStringUtil::WstrToTstr(ws);
}

int cArchive::ReadBlob(void* pBlob, int count)
{
    return Read(pBlob, count);
}

void cArchive::WriteInt16(int16_t i) // throw(eArchive)
{
    i = tw_htons(i);
    WriteBlob(&i, sizeof(int16_t));
}

void cArchive::WriteInt32(int32_t i) // throw(eArchive)
{
    i = tw_htonl(i);
    WriteBlob(&i, sizeof(int32_t));
}

void cArchive::WriteInt64(int64_t i) // throw(eArchive)
{
    i = tw_htonll(i);
    WriteBlob(&i, sizeof(int64_t));
}

// NOTE:BAM 10/11/99 -- we store unsigned size, but it really only works with
// lengths < INT16_MAX due to sign extension in integral promotion during the
// resize() in ReadString().
// format for written string: 16-bit unsigned size, then a list of 16-bit UCS2 (Unicode) characters
// not including terminating NULL
void cArchive::WriteString(const TSTRING& s) // throw(eArchive)
{
    // convert string to a UCS2 string
    wc16_string ws;
    cStringUtil::Convert(ws, s); // Make convert "type-dispatched"

    // we assume that we can represent the size as a unsigned 16-bit number
    // (we actually write  it as a signed number, but we cast it)
    if (ws.length() > TSS_INT16_MAX)
        ThrowAndAssert(eArchiveStringTooLong());

    WriteInt16(static_cast<int16_t>(ws.length()));

    // write out each 16 bit character
    // RAD:09/03/99 -- Optimized for performance with "const"
    wc16_string::const_iterator at = ws.begin();
    while (at != ws.end())
        WriteInt16(*at++);
}


void cArchive::WriteBlob(const void* pBlob, int count) // throw(eArchive)
{
    if (Write(pBlob, count) < count)
        ThrowAndAssert(eArchiveWrite());
}

int32_t cArchive::GetStorageSize(const TSTRING& str)
{
    int32_t size = sizeof(int32_t); // the length is always stored
    //
    // after the length, all of the characters in the string are written as 16-bit values,
    // except for the null character
    //
    size += (str.length() * 2);

    return size;
}

int64_t cArchive::Copy(cArchive* pFrom, int64_t amt)
{
    enum
    {
        BUF_SIZE = 2048
    };
    int8_t  buf[BUF_SIZE];
    int64_t amtLeft = amt;

    while (amtLeft > 0)
    {
        int64_t amtToRead = amtLeft > (int64_t)BUF_SIZE ? (int64_t)BUF_SIZE : amtLeft;
        int64_t amtRead   = pFrom->ReadBlob(buf, static_cast<int>(amtToRead));
        amtLeft -= amtRead;
        WriteBlob(buf, static_cast<int>(amtRead));
        if (amtRead < amtToRead)
            break;
    }

    // return the amount copied ...
    return (amt - amtLeft);
}

///////////////////////////////////////////////////////////////////////////////
// class cMemMappedArchive -- Archive that can be memory mapped.
///////////////////////////////////////////////////////////////////////////////

cMemMappedArchive::cMemMappedArchive()
{
    mpMappedMem   = 0;
    mMappedOffset = 0;
    mMappedLength = 0;
}

cMemMappedArchive::~cMemMappedArchive()
{
}

int64_t cMemMappedArchive::GetMappedOffset() const // throw(eArchive)
{
    if (mpMappedMem == 0)
        ThrowAndAssert(eArchiveMemmap());

    return mMappedOffset;
}

int64_t cMemMappedArchive::GetMappedLength() const // throw(eArchive)
{
    if (mpMappedMem == 0)
        ThrowAndAssert(eArchiveMemmap());

    return mMappedLength;
}

const void* cMemMappedArchive::GetMap() const // throw(eArchive)
{
    if (mpMappedMem == 0)
        ThrowAndAssert(eArchiveMemmap());

    return mpMappedMem;
}

void* cMemMappedArchive::GetMap() // throw(eArchive)
{
    if (mpMappedMem == 0)
        ThrowAndAssert(eArchiveMemmap());

    return mpMappedMem;
}

void cMemMappedArchive::SetNewMap(void* pMap, int64_t offset, int64_t length) const
{
    if (pMap == 0)
    {
        mpMappedMem   = 0;
        mMappedOffset = 0;
        mMappedLength = 0;
    }
    else
    {
        mpMappedMem   = pMap;
        mMappedOffset = offset;
        mMappedLength = length;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class cMemoryArchive -- An archive that stores itself in a memory buffer.
//      This buffer can grow as needed up until a pre-specified maximum
//      size.  The buffer can be read and written to and can be memory
//      mapped.
///////////////////////////////////////////////////////////////////////////////

cMemoryArchive::cMemoryArchive(int maxSize) : mMaxAllocatedLen(maxSize)
{
    ASSERT(maxSize > 0);
    mpMemory      = 0;
    mAllocatedLen = 0;
    mLogicalSize  = 0;
    mReadHead     = 0;
}

cMemoryArchive::~cMemoryArchive()
{
    delete [] mpMemory;
}

bool cMemoryArchive::EndOfFile()
{
    return mReadHead >= mLogicalSize;
}

void cMemoryArchive::Seek(int64_t offset, SeekFrom from) // throw(eArchive)
{
    switch (from)
    {
    case cBidirArchive::BEGINNING:
        break;
    case cBidirArchive::CURRENT:
        offset = mReadHead + (int)offset;
        break;
    case cBidirArchive::END:
        offset = mLogicalSize + (int)offset;
        break;
    default:
        ThrowAndAssert(eArchiveSeek(TSS_GetString(cCore, core::STR_MEMARCHIVE_FILENAME),
                                    TSS_GetString(cCore, core::STR_MEMARCHIVE_ERRSTR)));
    }

    if (offset > mLogicalSize)
        ThrowAndAssert(eArchiveSeek(TSS_GetString(cCore, core::STR_MEMARCHIVE_FILENAME),
                                    TSS_GetString(cCore, core::STR_MEMARCHIVE_ERRSTR)));

    mReadHead = static_cast<int>(offset);
}

int64_t cMemoryArchive::CurrentPos() const
{
    return mReadHead;
}

int64_t cMemoryArchive::Length() const
{
    return mLogicalSize;
}

void cMemoryArchive::Truncate()
{
    ASSERT(mReadHead >= 0);

    mLogicalSize = mReadHead;
    AllocateMemory(mLogicalSize);
}

void cMemoryArchive::MapArchive(int64_t offset, int64_t len) // throw(eArchive)
{
    if (offset + (int)len > mLogicalSize)
        AllocateMemory(static_cast<int>(offset + len));

    SetNewMap(mpMemory + offset, offset, len);
}

void cMemoryArchive::MapArchive(int64_t offset, int64_t len) const // throw(eArchive)
{
    if (offset + (int)len > mLogicalSize)
        ThrowAndAssert(eArchiveMemmap());

    SetNewMap(mpMemory + offset, offset, len);
}

int cMemoryArchive::Read(void* pDest, int count)
{
    if (mReadHead + count > mLogicalSize)
        count = mLogicalSize - mReadHead;

    if (pDest != 0)
        memcpy(pDest, mpMemory + mReadHead, count);

    mReadHead += count;

    return count;
}

int cMemoryArchive::Write(const void* pDest, int count) // throw(eArchive)
{
    if (mReadHead + count > mLogicalSize)
    {
        AllocateMemory(mReadHead + count);
    }

    memcpy(mpMemory + mReadHead, pDest, count);

    mReadHead += count;

    return count;
}

void cMemoryArchive::AllocateMemory(int len) // throw(eArchive)
{
    const int MIN_ALLOCATED_SIZE = 1024;

    if (len > mAllocatedLen)
    {
        // grow the buffer
        // only error if we are in debug mode
#ifdef DEBUG
        if (len > mMaxAllocatedLen)
            ThrowAndAssert(eArchiveOutOfMem());
#endif

        if (0 == mAllocatedLen)
            mAllocatedLen = MIN_ALLOCATED_SIZE;

        while (mAllocatedLen < len)
            mAllocatedLen *= 2;

        int8_t* pNewMem = new int8_t[mAllocatedLen];
        if (mpMemory != 0)
        {
            memcpy(pNewMem, mpMemory, mLogicalSize);
            delete [] mpMemory;
        }
        mpMemory     = pNewMem;
        mLogicalSize = len;

        // update memory map if there is one
        if (mpMappedMem)
            SetNewMap(mpMemory + mMappedOffset, mMappedOffset, mMappedLength);
    }
    else
    {
        // check for memory map conflict
        if (mpMappedMem && len < mMappedOffset + mMappedLength)
            ThrowAndAssert(eArchiveMemmap());

        if (len < (mAllocatedLen >> 1) && mAllocatedLen > MIN_ALLOCATED_SIZE)
        {
            // shrink the buffer
            int8_t* pNewMem = new int8_t[len];
            ASSERT(mpMemory);
            memcpy(pNewMem, mpMemory, len);
            delete [] mpMemory;
            mpMemory     = pNewMem;
            mLogicalSize = len;

            // update memory map if there is one
            if (mpMappedMem)
                SetNewMap(mpMemory + mMappedOffset, mMappedOffset, mMappedLength);
        }
        else
        {
            // no need to grow or shrink
            mLogicalSize = len;
        }
    }
}
/*
class cFixedMemArchive : public cBidirArchive
{
public:

    int8_t*  mpMemory;
    int32_t   mSize;
    int32_t   mReadHead;
};
*/

//-----------------------------------------------------------------------------
// cFixedMemArchive
//-----------------------------------------------------------------------------
cFixedMemArchive::cFixedMemArchive() : mpMemory(0), mSize(0), mReadHead(0)
{
}

cFixedMemArchive::cFixedMemArchive(int8_t* pMem, int32_t size) : mpMemory(0), mSize(0), mReadHead(0)
{
    Attach(pMem, size);
}

cFixedMemArchive::~cFixedMemArchive()
{
}

void cFixedMemArchive::Attach(int8_t* pMem, int32_t size)
{
    mpMemory  = pMem;
    mSize     = size;
    mReadHead = 0;
}

void cFixedMemArchive::Seek(int64_t offset, SeekFrom from) // throw(eArchive)
{
    switch (from)
    {
    case cBidirArchive::BEGINNING:
        break;
    case cBidirArchive::CURRENT:
        offset = mReadHead + (int)offset;
        break;
    case cBidirArchive::END:
        offset = mSize + (int)offset;
        break;
    default:
        ThrowAndAssert(eArchiveSeek(TSS_GetString(cCore, core::STR_MEMARCHIVE_FILENAME),
                                    TSS_GetString(cCore, core::STR_MEMARCHIVE_ERRSTR)));
    }

    if (offset > mSize)
        ThrowAndAssert(eArchiveSeek(TSS_GetString(cCore, core::STR_MEMARCHIVE_FILENAME),
                                    TSS_GetString(cCore, core::STR_MEMARCHIVE_ERRSTR)));

    mReadHead = static_cast<int32_t>(offset);
}

int64_t cFixedMemArchive::CurrentPos() const
{
    return mReadHead;
}

int64_t cFixedMemArchive::Length() const
{
    return mSize;
}

bool cFixedMemArchive::EndOfFile()
{
    return (mReadHead >= mSize);
}

int cFixedMemArchive::Read(void* pDest, int count) // throw(eArchive)
{
    ASSERT(pDest);
    if (mReadHead + count > mSize)
    {
        count = static_cast<int>(mSize - mReadHead);
        if (count <= 0)
            return 0;
    }

    if (pDest != 0)
        memcpy(pDest, mpMemory + mReadHead, count);

    mReadHead += count;

    return count;
}

int cFixedMemArchive::Write(const void* pDest, int count) // throw(eArchive)
{
    if (mReadHead + count > mSize)
    {
        ASSERT(false);
        throw eArchiveWrite();
    }

    memcpy(mpMemory + mReadHead, pDest, count);

    mReadHead += count;

    return count;
}


///////////////////////////////////////////////////////////////////////////////
// class cFileArchive -- Archive for files...
///////////////////////////////////////////////////////////////////////////////

//Ctor -- Initialize member variables to 0 or NULL equivalents.
cFileArchive::cFileArchive() : mFileSize(0), mReadHead(0), isWritable(false)
{
}

cFileArchive::~cFileArchive()
{
}

bool cFileArchive::EndOfFile()
{
    return (mReadHead >= mFileSize);
}

////////////////////////////////////////////////////////////////////////
// Seek -- This is where the actual offset is performed.  The default
// for each archive will be 0.
/////////////////////////////////////////////////////////////////////////
void cFileArchive::Seek(int64_t offset, SeekFrom from) // throw(eArchive)
{
    try
    {
        switch (from)
        {
        case cBidirArchive::BEGINNING:
            break;
        case cBidirArchive::CURRENT:
            offset = mReadHead + offset;
            break;
        case cBidirArchive::END:
            offset = mFileSize + offset;
            break;
        default:
            throw eArchiveSeek(mCurrentFilename, iFSServices::GetInstance()->GetErrString());
        }

        if (offset > mFileSize)
            throw eArchiveSeek(mCurrentFilename, iFSServices::GetInstance()->GetErrString());
        mReadHead = offset;

        mCurrentFile.Seek(mReadHead, cFile::SEEK_BEGIN);
        //This is where the actual read/writehead is set!!
    } //try
    catch (eFile& fileError)
    {
        throw(eArchiveSeek(mCurrentFilename, fileError.GetDescription()));
    }
}

int64_t cFileArchive::CurrentPos(void) const
{
    return mReadHead;
}

/////////////////////////////////////////////////////////////////////////
// Length -- Returns the size of the current file archive.
/////////////////////////////////////////////////////////////////////////
int64_t cFileArchive::Length(void) const
{
    try
    {
        return mCurrentFile.GetSize();
    }
    catch (eFile& fileError)
    {
        throw(eArchiveSeek(mCurrentFilename, fileError.GetDescription()));
    }
}

/////////////////////////////////////////////////////////////////////////
// OpenRead -- Opens the file to be read only.
/////////////////////////////////////////////////////////////////////////
void cFileArchive::OpenRead(const TCHAR* filename, uint32_t openFlags)
{
    try
    {
        // set up open flags
        uint32_t flags = cFile::OPEN_READ;
        flags |= ((openFlags & FA_OPEN_TRUNCATE) ? cFile::OPEN_TRUNCATE : 0);
        flags |= ((openFlags & FA_OPEN_TEXT)     ? cFile::OPEN_TEXT     : 0);
        flags |= ((openFlags & FA_SCANNING)      ? cFile::OPEN_SCANNING : 0);
        flags |= ((openFlags & FA_DIRECT)        ? cFile::OPEN_DIRECT   : 0);

        mOpenFlags       = openFlags;
        mCurrentFilename = filename;
        mCurrentFile.Open(filename, flags);
        isWritable = false;

        mFileSize = mCurrentFile.GetSize();
        mReadHead = mCurrentFile.Seek(0, cFile::SEEK_BEGIN);
    }
    catch (eFile& fileError)
    {
        throw(eArchiveOpen(mCurrentFilename, fileError.GetDescription()));
    }
}

/////////////////////////////////////////////////////////////////////////
// OpenReadWrite -- Opens the file to be read or written to
/////////////////////////////////////////////////////////////////////////
void cFileArchive::OpenReadWrite(const TCHAR* filename, uint32_t openFlags)
{
    try
    {
        // set up open flags
        uint32_t flags = cFile::OPEN_WRITE;
        flags |= ((openFlags & FA_OPEN_TRUNCATE) ? cFile::OPEN_TRUNCATE : 0);
        flags |= ((openFlags & FA_OPEN_TEXT)     ? cFile::OPEN_TEXT     : 0);
        flags |= ((openFlags & FA_SCANNING)      ? cFile::OPEN_SCANNING : 0);
        flags |= ((openFlags & FA_DIRECT)        ? cFile::OPEN_DIRECT   : 0);

        mOpenFlags       = openFlags;
        mCurrentFilename = filename;
        mCurrentFile.Open(filename, flags);
        isWritable = true;

        mFileSize = mCurrentFile.GetSize();
        mReadHead = mCurrentFile.Seek(0, cFile::SEEK_BEGIN);
    }
    catch (eFile& fileError)
    {
        throw(eArchiveOpen(mCurrentFilename, fileError.GetDescription()));
    }
}

/////////////////////////////////////////////////////////////////////////
// GetCurrentFilename -- Returns the name of the file currently associated
//      with the FileArchive.
/////////////////////////////////////////////////////////////////////////
TSTRING cFileArchive::GetCurrentFilename(void) const
{
    return mCurrentFilename;
}

/////////////////////////////////////////////////////////////////////////
// Close -- Closes the file currently referenced by mpCurrStream
/////////////////////////////////////////////////////////////////////////
void cFileArchive::Close()
{
    try
    {
        mCurrentFile.Close();
        mFileSize = 0;
        mReadHead = 0;

        mCurrentFilename = _T("");
    }
    catch (eFile& fileError)
    {
        throw(eArchive(mCurrentFilename, fileError.GetDescription()));
    }
}

/////////////////////////////////////////////////////////////////////////
// Read -- Read places bytes in location designated by pDest.  Returns
// The actual amount read into *pDest.
/////////////////////////////////////////////////////////////////////////
int cFileArchive::Read(void* pDest, int count)
{

    try
    {
        if (mReadHead + count > mFileSize && !(mOpenFlags & FA_DIRECT))
            count = static_cast<int>(mFileSize - mReadHead);

        if (pDest != NULL)
        {
            int nbRead = static_cast<int>(mCurrentFile.Read(pDest, count));

            // 'count' may not be equal to 'nbRead' if the file is open in
            // text mode.
            count = nbRead;
            if (count < 0)
                count = 0;
        }
        else
        {
            int   i;
            int32_t dummy;
            for (i = count;; i -= sizeof(int32_t))
            {
                if (i < (int)sizeof(int32_t))
                {
                    if (i > 0)
                        mCurrentFile.Read(&dummy, i);
                    break;
                }
                mCurrentFile.Read(&dummy, i);
            }
        }

        mReadHead += count;
        return count;
    }
    catch (eFile& fileError)
    {
        throw(eArchiveRead(mCurrentFilename, fileError.GetDescription()));
    }
}

/////////////////////////////////////////////////////////////////////////
// Write -- Writes to file designated by fh.  If isWritable is not set,
// function returns 0.  Otherwise, the actual # written is returned.
/////////////////////////////////////////////////////////////////////////
int cFileArchive::Write(const void* pDest, int count) // throw(eArchive)
{
    try
    {
        int64_t actual_count = 0;
        ASSERT(mCurrentFile.isWritable);

        actual_count = mCurrentFile.Write(pDest, count);

        if (actual_count < count)
        {
            //Disk full??
            throw eArchiveWrite(mCurrentFilename, iFSServices::GetInstance()->GetErrString());
        }

        // increment the read/write head
        mReadHead += actual_count;

        // increase the size, if needed
        if (mReadHead > mFileSize)
        {
#if 0 // IS_SUNPRO
      // These two lines seem to be all there is between code that crashes and code that works for sunpro
            cDebug d("cFileArchive::Write()");
            d.TraceDebug(_T("file(%s) adjusted mFileSize = %d mReadHead = %d\n"), mCurrentFilename.c_str(), (int)mFileSize, (int)mReadHead);
#endif
            mFileSize = mReadHead;
        }

        return (int)actual_count;
    }
    catch (eFile& fileError)
    {
        throw(eArchiveWrite(mCurrentFilename, fileError.GetDescription()));
    }
}


/////////////////////////////////////////////////////////////////////////
// Truncate
/////////////////////////////////////////////////////////////////////////
void cFileArchive::Truncate() // throw(eArchive)
{
    ASSERT(mCurrentFile.IsOpen());
    ASSERT(mCurrentFile.isWritable);

    try
    {
        mCurrentFile.Truncate(mReadHead);
    }
    catch (eFile& fileError)
    {
        //TODO: create an error number for truncate...
        throw(eArchiveWrite(mCurrentFilename, fileError.GetDescription()));
    }

    mFileSize = mReadHead;
}


/////////////////////////////////////////////////////////////////////////
// OpenReadWrite -- Opens the file to be read or written to
//
// since we'll never open an existing file, the truncateFile flag is unnecessary.
/////////////////////////////////////////////////////////////////////////
void cLockedTemporaryFileArchive::OpenReadWrite(const TCHAR* filename, uint32_t openFlags)
{
    TSTRING strTempFile;

    try
    {

        ASSERT(!mCurrentFile.IsOpen()); // shouldn't be able to create a new file when we're already open
        if (mCurrentFile.IsOpen())
            throw(eArchive(mCurrentFilename, _T("Internal Error")));


        ///////////////////////////////////////////////////////////////////////////////
        // if filename is NULL, create a temp file for the caller
        if (filename == NULL)
        {
            try
            {
                iFSServices::GetInstance()->GetTempDirName(strTempFile);
                strTempFile += _T("twtempXXXXXX");
                iFSServices::GetInstance()->MakeTempFilename(strTempFile);
            }
            catch (eFSServices& fileError)
            {
                TSTRING errStr = TSS_GetString(cCore, core::STR_BAD_TEMPDIRECTORY);
                throw eArchiveOpen(strTempFile, errStr);
            }
        }
        ///////////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////////
        // create file

        // set up flags
        uint32_t flags = cFile::OPEN_WRITE | cFile::OPEN_LOCKED_TEMP | cFile::OPEN_CREATE | cFile::OPEN_EXCLUSIVE;
        if (openFlags & FA_OPEN_TRUNCATE)
            flags |= cFile::OPEN_TRUNCATE;
        if (openFlags & FA_OPEN_TEXT)
            flags |= cFile::OPEN_TEXT;

        // open file
        mCurrentFilename = filename ? filename : strTempFile.c_str();
        mCurrentFile.Open(mCurrentFilename, flags);

        isWritable = true;
        mFileSize  = mCurrentFile.GetSize();
        mReadHead  = mCurrentFile.Seek(0, cFile::SEEK_BEGIN);

#if 0 // IS_SUNPRO
    cDebug d("cLockedTemporaryFileArchive::OpenReadWrite()");
    d.TraceDebug(_T("file(%s) set mFileSize to %d mReadHead to %d\n"), mCurrentFilename.c_str(), (int)mFileSize, (int)mReadHead);
#endif

    } //try
    catch (eFile& fileError)
    {
        TSTRING      errStr = TSS_GetString(cCore, core::STR_BAD_TEMPDIRECTORY);
        eArchiveOpen e(strTempFile, errStr);
        throw e;
    }

    ///////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////
// Close -- Closes the file currently referenced by fh
void cLockedTemporaryFileArchive::Close()
{
    // Note: this deletes the file as well
    cFileArchive::Close();
}
