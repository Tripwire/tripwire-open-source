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
// blockrecordarray.h
//
// cBlockRecordArray
//
#ifndef __BLOCKRECORDARRAY_H
#define __BLOCKRECORDARRAY_H

#ifndef __BLOCKFILE_H
#include "blockfile.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif

//
// TODO -- I need to guarantee that all of the structures in this class do not have any pad bytes.
//      This is true under win32, but I am not sure if it will always be true. -- 10 feb 99 mdb
//

class eArchive;

class cBlockRecordArray
{
public:
    //-------------------------------------------------------------------------------------
    // Construction
    //-------------------------------------------------------------------------------------
    cBlockRecordArray(cBlockFile* pBlockFile, int blockNum);
    // this object is associated with a single block in a cBlockFile. It is _not_
    // asserted that the blockNum is valid for the associated file at this point,
    // since it is possible that the file has not been loaded yet
    cBlockRecordArray();
    // if this version of the ctor is called, then Init() must be called before it can be used.
    void Init(cBlockFile* pBlockFile, int blockNum);

    virtual ~cBlockRecordArray();

    //-------------------------------------------------------------------------------------
    // Initialization ... exactly one of these methods must be called before this class can be used
    //-------------------------------------------------------------------------------------
    void InitNewBlock(); //throw (eArchive)
        // this method should be called when a new block has been created, and before this
        // class operates on it.
    void InitForExistingBlock(); //throw (eArchive)
        // this method should be called to initialize this class for use with a block that has
        // previously been created and initialized with InitNewBlock().

    //-------------------------------------------------------------------------------------
    // Data Manipulation
    //-------------------------------------------------------------------------------------
    int AddItem(int8_t* pData, int dataSize, int mainIndex); //throw (eArchive)
        // inserts the given item into the array; returns the index that it was inserted into.
        // this asserts that there is room for the new element, and updates the avail. space and
        // max index as necessary.
    void DeleteItem(int index); //throw (eArchive)
        // deletes the specified item; this asserts that the index is valid, and updates the avail.
        // space and max index as necessary.
    int8_t* GetDataForReading(int index, int32_t& dataSize); //throw (eArchive)
        // returns a pointer to the named data. This method will assert that the address is
        // valid. The data pointer returned is guarenteed to be valid only until the next
        // method call into this class.
    int8_t* GetDataForWriting(int index, int32_t& dataSize); //throw (eArchive)
        // this is the same as the previous function, except the dirty bit for the page is also set
    bool IsItemValid(int index) const; //throw (eArchive)
        // returns true if the given index has a valid value.

    bool IsClassValid() const;
    // returns true if the class has an internally consistant state. If an archive exception
    // occurs, false is returned. This method is good for determining if a newly opened block
    // file is actually a block file

    bool Initialized() const
    {
        return mbInit;
    }
    int GetAvailableSpace() const
    {
        ASSERT(Initialized());
        return mSpaceAvailable;
    }
    int GetNumItems() const
    {
        ASSERT(Initialized());
        return mNumItems;
    }
    // returns the number of items currently being stored in the array

    ///////////////////////////////////////////////////////////////////
    // tRecordIndex -- this is what each record in the array is
    //      indexed by. These come at the beginning of the block and
    //      the record data comes at the end.
    ///////////////////////////////////////////////////////////////////
    struct tRecordIndex
    {
    private:
        int32_t mOffset;    // offset from the end of the block that my data is at; offset 1 is the last byte in the block
        int32_t mMainIndex; // my main array index
    public:
        // byte order safe access methods...
        //
        void SetOffset(int32_t off)
        {
            mOffset = tw_htonl(off);
        }
        int32_t GetOffset()
        {
            return tw_ntohl(mOffset);
        }
        void SetMainIndex(int32_t idx)
        {
            mMainIndex = tw_htonl(idx);
        }
        int32_t GetMainIndex()
        {
            return tw_ntohl(mMainIndex);
        }
    };
    ///////////////////////////////////////////////////////////////////
    // tHeader -- the info that appears at the beginning of the block
    ///////////////////////////////////////////////////////////////////
    struct tHeader
    {
    private:
        int32_t mSpaceAvailable;
        int32_t mNumItems;

    public:
        // byte order safe access methods...
        //
        void SetSpaceAvail(int32_t sa)
        {
            mSpaceAvailable = tw_htonl(sa);
        }
        int32_t GetSpaceAvail()
        {
            return tw_ntohl(mSpaceAvailable);
        }
        void SetNumItems(int32_t ni)
        {
            mNumItems = tw_htonl(ni);
        }
        int32_t GetNumItems()
        {
            return tw_ntohl(mNumItems);
        }
    };

    enum
    {
        MAX_RECORDS = cBlockFile::BLOCK_SIZE / (sizeof(tRecordIndex) + sizeof(uint32_t)),
        // MAX_RECORDS signifies the maximum number of records that can be stored in a single block.
        // It is rationalized like this: each used record needs a tRecordIndex plus an extra uint32
        // as a minimum storage requirement (even though it is ok to have a record that is filled in
        // with zero bytes)
        MARGIN = 10,
        // MARGIN is the min. amount of room that should exist between the indexes and the data. This
        // is used when calculating the available space, and also in assertions.
        MAX_DATA_SIZE = cBlockFile::BLOCK_SIZE - sizeof(tRecordIndex) - sizeof(tHeader) - MARGIN,
        // this is the largest single piece of data that could theoreticaly be stored (if it were the
        // only thing stored in the block). We assume that the single record index will take up space,
        // the header will take up space, and the margin will take up space.
        INVALID_INDEX = -1
        // when tRecordIndex::mMainIndex is set to this, it indicates that the index is not being used.
    };

    ///////////////////////////////////////////////////////////////////
    // tIndexArray -- the beginning of a block is treated
    //      as one of these
    ///////////////////////////////////////////////////////////////////
    struct tIndexArray
    {
        tHeader      mHeader;
        tRecordIndex maRecordIndex[MAX_RECORDS];
    };

private:
    cBlockFile* mpBlockFile;
    int         mBlockNum;       // the block that I am operating on
    int         mSpaceAvailable; // amount of storage I have left
    int         mNumItems;       // number of items currently being stored in this array
    bool        mbInit;          // has InitXXX() been called?

    void UpdateFreeSpace(cBlockFile::Block* pBlock);
    // calculates the total free space available in this array and assigns
    // it to mSpaceAvailable. Note that the value of mSpaceAvailable signifies
    // the size of the largest single item that could be added to the array.
    void WriteHeaderInfo(cBlockFile::Block* pBlock);
    // gets the header information at the top of our block in sync with our member vars
    void ReadHeaderInfo(cBlockFile::Block* pBlock);
    // reads the header info from the block and sets our member vars to what was read

    ///////////////////////////////////////////////////////////////////////////
    // Profiling / Debugging interface
    ///////////////////////////////////////////////////////////////////////////
#ifdef _BLOCKFILE_DEBUG
public:
    void TraceContents(int dl = -1) const;
    // traces out all the info we can on the current state of this class
    // dl is the debug level to trace it at; -1 means to use D_DEBUG
    void AssertValid() const;
    // ASSERTs as much as we can about the consistency of our internal state.
#endif
};


#endif //__BLOCKRECORDARRAY_H
