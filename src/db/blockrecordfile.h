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
// blockrecordfile.h
//
#ifndef __BLOCKRECORDFILE_H
#define __BLOCKRECORDFILE_H

#ifndef __BLOCKFILE_H
#include "blockfile.h"
#endif
#ifndef __BLOCKRECORDARRAY_H
#include "blockrecordarray.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif
#ifndef __ERROR_H
#include "core/error.h"
#endif

class eArchive;

class cBlockRecordFile
{
public:
    //-------------------------------------------------------------------------
    // Construction and Destruction
    //-------------------------------------------------------------------------
    cBlockRecordFile();
    virtual ~cBlockRecordFile();

    //-------------------------------------------------------------------------
    // Opening and Closing Files
    //-------------------------------------------------------------------------
    virtual void Open(const TSTRING& fileName, int numPages, bool bTruncate = false); //throw (eArchive)
    virtual void Open(cBidirArchive* pArch, int numPages);                            //throw (eArchive)
        // for the second Open(), this class owns the destruction of the archive
    virtual void Close(); //throw (eArchive)
    virtual void Flush(); //throw (eArchive)

    //-------------------------------------------------------------------------
    // tAddr -- struct that is used to identify the location of a data item in
    //          the file
    //-------------------------------------------------------------------------
    struct tAddr
    {
        int32_t mBlockNum;
        int32_t mIndex;

        tAddr(int32_t block = -1, int32_t addr = -1) : mBlockNum(block), mIndex(addr)
        {
        }
    };

    //-------------------------------------------------------------------------
    // Adding and Removing Data
    //-------------------------------------------------------------------------
    tAddr AddItem(int8_t* pData, int dataSize); //throw (eArchive)
        // adds the given data to the file, growing it as necessary. Return value
        // can be used in the future to retrieve the data
    void RemoveItem(tAddr dataAddr); //throw (eArchive)
        // removes the named data from the file. This will assert that the given
        // address is valid, and will shrink the end of the file if there are
        // empty blocks at the end after the removal TODO -- it doesn't actually
        // do the latter.

    //-------------------------------------------------------------------------
    // Accessing Data
    //-------------------------------------------------------------------------
    bool IsValidAddr(tAddr addr); //throw (eArchive)
        // returns true if the given address points to valid data
    int8_t* GetDataForReading(tAddr dataAddr, int32_t& dataSize); //throw (eArchive)
        // returns a pointer to the named data. This method will assert that the address is
        // valid. The data pointer returned is guarenteed to be valid only until the next
        // method call into this class.
    int8_t* GetDataForWriting(tAddr dataAddr, int32_t& dataSize); //throw (eArchive)
        // this is the same as the previous function, except the dirty bit for the page is also set

    cBidirArchive* GetArchive()
    {
        return mBlockFile.GetArchive();
    }
    // NOTE -- be very careful with this; only read from it and do not write to it.

    //-------------------------------------------------------------------------
    // Private Implementation
    //-------------------------------------------------------------------------
protected:
    typedef std::vector<cBlockRecordArray> BlockArray;
    //
    // NOTE -- use all of these methods with much care!
    //
    cBlockFile* GetBlockFile()
    {
        return &mBlockFile;
    }
    BlockArray* GetBlockArray()
    {
        return &mvBlocks;
    }

private:
    int32_t      mLastAddedTo; // optimization that keeps track of last block added to
    bool       mbOpen;       // are we currently associated with a file?
    cBlockFile mBlockFile;
    BlockArray mvBlocks;

    cBlockRecordFile(const cBlockRecordFile& rhs); //not impl
    void operator=(const cBlockRecordFile& rhs);   //not impl

    int FindRoomForData(int32_t dataSize); //throw (eArchive)
        // searches through all the blocks, starting with mLastAddedTo, looking
        // for one with dataSize free space. This asserts that the size is valid
        // for storage in a block
    void OpenImpl(bool bTruncate); //throw (eArchive)
                                   // implementation of the Open() methods above; both end up calling this.

    //-------------------------------------------------------------------------
    // Profiling / Debugging Interface
    //-------------------------------------------------------------------------
#ifdef _BLOCKFILE_DEBUG
public:
    void TraceContents(int dl = -1) const;
    // traces out all the info we can on the current state of this class
    // dl is the debug level to trace it at; -1 means to use D_DEBUG
    void AssertValid() const;
    // ASSERTs as much as we can about the consistancy of our internal state.
    void AssertAllBlocksValid();
    // time consuming function that calls AssertValid() on all of the blocks
#endif
};


#endif //__BLOCKRECORDFILE_H
