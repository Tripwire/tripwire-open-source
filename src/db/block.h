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
// block.h
//
#ifndef __BLOCK_H
#define __BLOCK_H

#ifndef __ARCHIVE_H
#include "core/archive.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif

//-----------------------------------------------------------------------------
// cBlock
//-----------------------------------------------------------------------------
template<int SIZE> class cBlock
{
public:
    enum
    {
        INVALID_NUM = -1
    };

    cBlock();

    void SetDirty()
    {
        mbDirty = true;
    }
    bool IsDirty() const
    {
        return mbDirty;
    }
    int GetBlockNum() const
    {
        return mBlockNum;
    }
    int8_t* GetData()
    {
        return mpData;
    }

    bool AssertValid() const;
    // this asserts and returns false if the guard bytes have been corrupted
    bool IsValidAddr(int8_t* pAddr) const;
    // returns true if pAddr falls within mpData
protected:
    enum
    {
        NUM_GUARD_BLOCKS = 8, // associated with BYTE_ALIGN: see ctor for info
        GUARD_BLOCK_VAL  = 0xAB
    }; // odd, non-zero value for debugging

    // guardMin and guardMax are used to detect bad writes
    uint8_t mGuardMin[NUM_GUARD_BLOCKS];
    int8_t  mpData[SIZE];
    uint8_t mGuardMax[NUM_GUARD_BLOCKS];
    bool  mbDirty;
    int   mBlockNum;
};

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
template<int SIZE> inline cBlock<SIZE>::cBlock() : mbDirty(false), mBlockNum(cBlock::INVALID_NUM)
{
    // To prevent misaligned memory access, the size of the data and the
    // number of guard blocks must be a multiple of the byte alignment
    // and they both must be at least as large as the byte alignment
    ASSERT(0 == (SIZE % BYTE_ALIGN));
    ASSERT(0 == (NUM_GUARD_BLOCKS % BYTE_ALIGN));
    ASSERT(SIZE >= BYTE_ALIGN);
    ASSERT(NUM_GUARD_BLOCKS >= BYTE_ALIGN);

    // init guard blocks to dummy value
    for (int i = 0; i < NUM_GUARD_BLOCKS; i++)
    {
        mGuardMin[i] = (uint8_t)GUARD_BLOCK_VAL;
        mGuardMax[i] = (uint8_t)GUARD_BLOCK_VAL;
    }

    // zero out memory
    memset(mpData, 0, SIZE);
}

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
template<int SIZE> inline bool cBlock<SIZE>::AssertValid() const
{
    // determine if guard bites have been accidentally overwritten
    for (int i = 0; i < NUM_GUARD_BLOCKS; i++)
    {
        if ((mGuardMin[i] != (uint8_t)GUARD_BLOCK_VAL) || (mGuardMax[i] != (uint8_t)GUARD_BLOCK_VAL))
        {
            ASSERT(false);
            return false;
        }
    }

    return true;
}

template<int SIZE> inline bool cBlock<SIZE>::IsValidAddr(int8_t* pAddr) const
{
    return ((pAddr >= &mpData[0]) && (pAddr <= &mpData[SIZE - 1]));
}


//-----------------------------------------------------------------------------
// cBlockImpl
//-----------------------------------------------------------------------------
template<int SIZE> class cBlockImpl : public cBlock<SIZE>
{
public:
    enum
    {
        INVALID_NUM = -1
    };

    cBlockImpl();

    void SetBlockNum(int blockNum)
    {
        cBlock<SIZE>::mBlockNum = blockNum;
    }
    void SetTimestamp(uint32_t timestamp)
    {
        mTimestamp = timestamp;
    }
    uint32_t GetTimestamp() const
    {
        return mTimestamp;
    }

    void Write(cBidirArchive& arch);                            //throw( eArchive )
    void Read(cBidirArchive& arch, int blockNum = INVALID_NUM); //throw( eArchive )
        // if blockNum is INVALID_NUM, then it reads in the current block number
protected:
    uint32_t mTimestamp;
};

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
template<int SIZE> inline cBlockImpl<SIZE>::cBlockImpl() : cBlock<SIZE>(), mTimestamp(0)
{
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
template<int SIZE> inline void cBlockImpl<SIZE>::Write(cBidirArchive& arch) //throw( eArchive )
{
    ASSERT(cBlock<SIZE>::mbDirty);
    ASSERT((cBlock<SIZE>::mBlockNum >= 0) && (((cBlock<SIZE>::mBlockNum + 1) * SIZE) <= arch.Length()));

    arch.Seek((cBlock<SIZE>::mBlockNum * SIZE), cBidirArchive::BEGINNING);
    arch.WriteBlob(cBlock<SIZE>::mpData, SIZE);

    cBlock<SIZE>::mbDirty = false;
}

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
template<int SIZE> inline void cBlockImpl<SIZE>::Read(cBidirArchive& arch, int blockNum) //throw( eArchive )
{
    if (blockNum != INVALID_NUM)
        cBlock<SIZE>::mBlockNum = blockNum;

    ASSERT((cBlock<SIZE>::mBlockNum >= 0) && (((cBlock<SIZE>::mBlockNum + 1) * SIZE) <= arch.Length()));

    //    std::cout << "cBlockImpl<SIZE>::Read() mBlockNum = " << mBlockNum << " arch.Length() = " << arch.Length() << std::endl;

    arch.Seek((cBlock<SIZE>::mBlockNum * SIZE), cBidirArchive::BEGINNING);
    arch.ReadBlob(cBlock<SIZE>::mpData, SIZE);

    cBlock<SIZE>::mbDirty = false;
}

#endif
