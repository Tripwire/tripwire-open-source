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
// blockrecordarray.cpp

#include "stddb.h"

#include "blockrecordarray.h"


///////////////////////////////////////////////////////////////////////////////
// util_GetIndexArray -- returns a reference to the index array inside a block
//      Right now, this is just a cast, but if the blocks ever had headers, this
//      would also take that offset into account
///////////////////////////////////////////////////////////////////////////////
inline cBlockRecordArray::tIndexArray& util_GetIndexArray(cBlockFile::Block* pBlock)
{
    return (*reinterpret_cast<cBlockRecordArray::tIndexArray*>(pBlock->GetData()));
}

///////////////////////////////////////////////////////////////////////////////
// util_OffsetToAddr -- given an offset from the end of a block (such as
//      is stored in tRecordIndex) return a pointer to memory inside the block
//      that corresponds to the given offset
///////////////////////////////////////////////////////////////////////////////
inline int8_t* util_OffsetToAddr(cBlockFile::Block* pBlock, int offset)
{
    return reinterpret_cast<int8_t*>((pBlock->GetData() + (cBlockFile::BLOCK_SIZE - offset)));
}


///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cBlockRecordArray::cBlockRecordArray(cBlockFile* pBlockFile, int blockNum)
    : mpBlockFile(pBlockFile), mBlockNum(blockNum), mSpaceAvailable(-1), mNumItems(0), mbInit(false)
{
}

cBlockRecordArray::cBlockRecordArray() : mpBlockFile(0), mBlockNum(-1), mSpaceAvailable(-1), mNumItems(0), mbInit(false)
{
}

void cBlockRecordArray::Init(cBlockFile* pBlockFile, int blockNum)
{
    ASSERT(!pBlockFile);

    mpBlockFile     = pBlockFile;
    mBlockNum       = blockNum;
    mSpaceAvailable = -1;
    mNumItems       = 0;
    mbInit          = false;
}


///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cBlockRecordArray::~cBlockRecordArray()
{
}

///////////////////////////////////////////////////////////////////////////////
// InitNewBlock
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::InitNewBlock() //throw (eArchive)
{
    ASSERT(mpBlockFile != 0);
    ASSERT((mBlockNum >= 0) && (mBlockNum < mpBlockFile->GetNumBlocks()));
#ifdef _BLOCKFILE_DEBUG
    mpBlockFile->AssertValid();
#endif

    cBlockFile::Block* pBlock = mpBlockFile->GetBlock(mBlockNum);
    tIndexArray&       array  = util_GetIndexArray(pBlock);
    //
    // set everything to zero..
    //
    memset(pBlock->GetData(), 0, cBlockFile::BLOCK_SIZE);
    pBlock->SetDirty();
    //
    // set up the guard byte for the index array...
    //
    array.maRecordIndex[0].SetOffset(-1);
    array.maRecordIndex[0].SetMainIndex(INVALID_INDEX);
    //
    // remember that we have been initialized
    //
    mbInit = true;
    //
    // update the free space and max index variables...
    //
    mNumItems = 0;
    UpdateFreeSpace(pBlock);
    WriteHeaderInfo(pBlock);
}

///////////////////////////////////////////////////////////////////////////////
// InitForExistingBlock
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::InitForExistingBlock() //throw (eArchive)
{
    ASSERT(mpBlockFile != 0);
    ASSERT((mBlockNum >= 0) && (mBlockNum < mpBlockFile->GetNumBlocks()));
    ASSERT(!mbInit);
    //
    // read in our header info...
    //
    cBlockFile::Block* pBlock = mpBlockFile->GetBlock(mBlockNum);
    ReadHeaderInfo(pBlock);
    //
    // remember that we have been initialized
    //
    mbInit = true;
    //
    // some debug out...
    //
    cDebug d("cBlockRecordArray::InitForExistingBlock");
    d.TraceDetail(" Executed for block %d; NumItems=%d, SpaceAvail=%d\n", mBlockNum, mNumItems, mSpaceAvailable);
}


///////////////////////////////////////////////////////////////////////////////
// WriteHeaderInfo
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::WriteHeaderInfo(cBlockFile::Block* pBlock)
{
    ASSERT(mpBlockFile != 0);
    ASSERT(Initialized());

    util_GetIndexArray(pBlock).mHeader.SetNumItems(mNumItems);
    util_GetIndexArray(pBlock).mHeader.SetSpaceAvail(mSpaceAvailable);

    pBlock->SetDirty();
}

///////////////////////////////////////////////////////////////////////////////
// ReadHeaderInfo
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::ReadHeaderInfo(cBlockFile::Block* pBlock)
{
    ASSERT(mpBlockFile != 0);

    mNumItems       = util_GetIndexArray(pBlock).mHeader.GetNumItems();
    mSpaceAvailable = util_GetIndexArray(pBlock).mHeader.GetSpaceAvail();
}


///////////////////////////////////////////////////////////////////////////////
// UpdateFreeSpace
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::UpdateFreeSpace(cBlockFile::Block* pBlock)
{
    ASSERT(Initialized());
    ASSERT(mpBlockFile != 0);
    // first, take into account the size of all of the index structures. Note that
    // we add one to the current number because we want to account for the new one
    // that might be added if another item were added to the array
    //
    int availSpace = cBlockFile::BLOCK_SIZE - sizeof(tHeader) - ((GetNumItems() + 1) * sizeof(tRecordIndex));
    //
    // always take the MARGIN away, to give ourselves some breathing room between the indexes and
    // the data
    //
    availSpace -= MARGIN;
    //
    // subtract from availSpace the amount of room that the actual data is taking up...
    // note that it is always true that the top index points to a valid value
    //
    if (GetNumItems() > 0)
    {
        int topOffset = util_GetIndexArray(pBlock).maRecordIndex[GetNumItems() - 1].GetOffset();
        availSpace -= topOffset;
    }

    mSpaceAvailable = availSpace < 0 ? 0 : availSpace;

#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// IsItemValid
///////////////////////////////////////////////////////////////////////////////
bool cBlockRecordArray::IsItemValid(int index) const //throw (eArchive)
{
    ASSERT(Initialized());

    if ((index < 0) || (index >= GetNumItems()))
        return false;

    cBlockFile::Block* pBlock = mpBlockFile->GetBlock(mBlockNum);

    return (util_GetIndexArray(pBlock).maRecordIndex[index].GetMainIndex() != INVALID_INDEX);
}


///////////////////////////////////////////////////////////////////////////////
// AddItem
///////////////////////////////////////////////////////////////////////////////
int cBlockRecordArray::AddItem(int8_t* pData, int dataSize, int mainIndex) //throw (eArchive)
{
    // make ourselves initialized, if we are not right now...
    //
    if (!Initialized())
    {
        InitForExistingBlock();
    }


    ASSERT(dataSize <= GetAvailableSpace());
    ASSERT(GetNumItems() <= MAX_RECORDS);
    ASSERT(dataSize > 0); // I am not sure if this is the right thing to do !
    ASSERT(mpBlockFile != 0);
    ASSERT(mainIndex != INVALID_INDEX);
    ASSERT(Initialized());
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
    mpBlockFile->AssertValid();
#endif
    cDebug d("cBlockRecordArray::AddItem");
    d.TraceDetail("Entering... adding item of size %d to block %d\n", dataSize, mBlockNum);
    //
    // get the block we need and a reference into the index array....
    //
    cBlockFile::Block* pBlock     = mpBlockFile->GetBlock(mBlockNum);
    tIndexArray&       indexArray = util_GetIndexArray(pBlock);
    //
    // find an unused entry to put the item into...
    //
    tRecordIndex* pIndex   = indexArray.maRecordIndex;
    int           newIndex = 0;
    for (; newIndex < mNumItems; newIndex++, pIndex++)
    {
        if (pIndex->GetMainIndex() == INVALID_INDEX)
            break;
    }
    d.TraceDetail("Adding new item to index %d (max index + 1 = %d)\n", newIndex, mNumItems);
    // write the data we know for sure at this point...
    //
    pIndex->SetMainIndex(mainIndex);
    pBlock->SetDirty();
    //
    // calculate the offset for the new data
    //
    int prevOffset = (newIndex == 0 ? 0 : indexArray.maRecordIndex[newIndex - 1].GetOffset());
    pIndex->SetOffset(prevOffset + dataSize);

    if (newIndex == mNumItems)
    {
        //
        // increment the max number of indexes...
        //
        mNumItems++;
        indexArray.maRecordIndex[mNumItems].SetOffset(-1);
        indexArray.maRecordIndex[mNumItems].SetMainIndex(INVALID_INDEX);
    }
    else
    {
        // we are going to have to shift up the data that is above us...
        //
        int   topOffset = indexArray.maRecordIndex[GetNumItems() - 1].GetOffset();
        int8_t* pTop    = util_OffsetToAddr(pBlock, topOffset);
        int   amtToMove = topOffset - prevOffset;
        ASSERT(amtToMove >= 0);

        memmove(pTop - dataSize, pTop, amtToMove);
        //
        // now, we need to iterate over all of the existing entries, updating their offsets...
        //
        tRecordIndex* pMovedIndex = pIndex + 1;
        for (int i = newIndex + 1; i < mNumItems; i++, pMovedIndex++)
        {
            pMovedIndex->SetOffset(pMovedIndex->GetOffset() + dataSize);
        }
    }
    //
    // write the data...
    //
    memcpy(util_OffsetToAddr(pBlock, pIndex->GetOffset()), pData, dataSize);
    //
    // update the free space...
    //
    indexArray.maRecordIndex[mNumItems].SetOffset(-1);
    indexArray.maRecordIndex[mNumItems].SetMainIndex(INVALID_INDEX);
    UpdateFreeSpace(pBlock);
    WriteHeaderInfo(pBlock);

#ifdef _BLOCKFILE_DEBUG
    AssertValid();
    mpBlockFile->AssertValid();
    d.TraceDetail("*** Add Complete; here are the new contents ***\n");
    TraceContents(cDebug::D_NEVER);
#endif

    return newIndex;
}

///////////////////////////////////////////////////////////////////////////////
// DeleteItem
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::DeleteItem(int index) //throw (eArchive)
{
    // make ourselves initialized, if we are not right now...
    //
    if (!Initialized())
    {
        InitForExistingBlock();
    }


    ASSERT(mpBlockFile != 0);
    ASSERT(IsItemValid(index));
    ASSERT(Initialized());
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
    mpBlockFile->AssertValid();
#endif
    cDebug d("cBlockRecordArray::DeleteItem");
    d.TraceDetail("Entering... Removing item index %d from block %d\n", index, mBlockNum);

    //
    // get the block we need and a reference into the index array....
    //
    cBlockFile::Block* pBlock     = mpBlockFile->GetBlock(mBlockNum);
    tIndexArray&       indexArray = util_GetIndexArray(pBlock);
    pBlock->SetDirty();
    //
    // if this is the last entry, then our job is easy...
    //
    if ((index + 1) == mNumItems)
    {
        mNumItems--;
        // we need to clean out all the entries that are empty at the end of the index list...
        //
        while ((mNumItems > 0) && (indexArray.maRecordIndex[mNumItems - 1].GetMainIndex() == INVALID_INDEX))
        {
            mNumItems--;
        }
        //
        // set the guard bytes...
        //
        indexArray.maRecordIndex[mNumItems].SetOffset(-1);
        indexArray.maRecordIndex[mNumItems].SetMainIndex(INVALID_INDEX);
    }
    else
    {
        // we are going to shift all entries above the deleted entry down;
        // first, figure out how far we are shifting and the source address...
        //
        int sizeToShift =
            indexArray.maRecordIndex[GetNumItems() - 1].GetOffset() - indexArray.maRecordIndex[index].GetOffset();
        int distToShift = indexArray.maRecordIndex[index].GetOffset();
        if (index > 0)
        {
            distToShift -= indexArray.maRecordIndex[index - 1].GetOffset();
        }
        int8_t* pSrc = util_OffsetToAddr(pBlock, indexArray.maRecordIndex[GetNumItems() - 1].GetOffset());
        //
        // copy the data..
        //
        ASSERT(pBlock->IsValidAddr(pSrc) && pBlock->IsValidAddr(pSrc + distToShift + sizeToShift - 1));
        ASSERT(sizeToShift >= 0);
        ASSERT(distToShift >= 0);
        memmove(pSrc + distToShift, pSrc, sizeToShift);
        //
        // mark this entry as unused...
        // note that the offset is set to be the same as the item before it.
        //
        tRecordIndex* pIndex = &indexArray.maRecordIndex[index];
        pIndex->SetMainIndex(INVALID_INDEX);
        pIndex->SetOffset((index == 0 ? 0 : indexArray.maRecordIndex[index - 1].GetOffset()));
        //
        // now, iterate over the items, moving their offsets down...
        //
        pIndex++;
        for (int i = index + 1; i < GetNumItems(); i++, pIndex++)
        {
            pIndex->SetOffset(pIndex->GetOffset() - distToShift);
        }
    }

    UpdateFreeSpace(pBlock);
    WriteHeaderInfo(pBlock);

#ifdef _BLOCKFILE_DEBUG
    AssertValid();
    mpBlockFile->AssertValid();
    d.TraceDetail("*** Removal Complete; here are the new contents ***\n");
    TraceContents(cDebug::D_NEVER);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// GetDataForReading
///////////////////////////////////////////////////////////////////////////////
int8_t* cBlockRecordArray::GetDataForReading(int index, int32_t& dataSize) //throw (eArchive)
{
    // make ourselves initialized, if we are not right now...
    //
    if (!Initialized())
    {
        InitForExistingBlock();
    }

    ASSERT(IsItemValid(index));
    cBlockFile::Block* pBlock     = mpBlockFile->GetBlock(mBlockNum);
    tIndexArray&       indexArray = util_GetIndexArray(pBlock);
    //
    // figure out the size of the block
    //
    dataSize = indexArray.maRecordIndex[index].GetOffset();
    if (index != 0)
    {
        dataSize -= indexArray.maRecordIndex[index - 1].GetOffset();
    }

#ifdef _BLOCKFILE_DEBUG
    AssertValid();
    mpBlockFile->AssertValid();
#endif


    return (util_OffsetToAddr(pBlock, indexArray.maRecordIndex[index].GetOffset()));
}

///////////////////////////////////////////////////////////////////////////////
// GetDataForWriting
///////////////////////////////////////////////////////////////////////////////
int8_t* cBlockRecordArray::GetDataForWriting(int index, int32_t& dataSize) //throw (eArchive)
{
    // make ourselves initialized, if we are not right now...
    //
    if (!Initialized())
    {
        InitForExistingBlock();
    }

    ASSERT(IsItemValid(index));
    cBlockFile::Block* pBlock     = mpBlockFile->GetBlock(mBlockNum);
    tIndexArray&       indexArray = util_GetIndexArray(pBlock);
    pBlock->SetDirty();
    //
    // figure out the size of the block
    //
    dataSize = indexArray.maRecordIndex[index].GetOffset();
    if (index != 0)
    {
        dataSize -= indexArray.maRecordIndex[index - 1].GetOffset();
    }

#ifdef _BLOCKFILE_DEBUG
    AssertValid();
    mpBlockFile->AssertValid();
#endif

    return (util_OffsetToAddr(pBlock, indexArray.maRecordIndex[index].GetOffset()));
}


///////////////////////////////////////////////////////////////////////////////
// IsClassValid
///////////////////////////////////////////////////////////////////////////////
#define BRA_ASSERT(x)  \
    if (x)             \
        ;              \
    else               \
    {                  \
        ASSERT(false); \
        return false;  \
    }

bool cBlockRecordArray::IsClassValid() const
{
    ASSERT(mpBlockFile != 0);
    cDebug d("cBlockRecordArray::AssertValid");
    if (!mbInit)
    {
        // if we haven't been initialized yet, then I assume that we are valid...
        //
        return true;
    }

    //
    // get the block we need and a reference into the index array....
    //
    try
    {
        cBlockFile::Block* pBlock = mpBlockFile->GetBlock(mBlockNum);
        tIndexArray&       array  = util_GetIndexArray(pBlock);
        //
        // make sure that the num items and available space make sense...
        //
        BRA_ASSERT((mNumItems >= 0) && (mNumItems <= MAX_RECORDS));
        BRA_ASSERT((mSpaceAvailable >= 0) && (mSpaceAvailable <= MAX_DATA_SIZE));
        //
        // assert that the top index points to a valid value...
        //
        if (mNumItems != 0)
        {
            BRA_ASSERT(array.maRecordIndex[mNumItems - 1].GetMainIndex() != INVALID_INDEX);
            BRA_ASSERT(array.maRecordIndex[mNumItems - 1].GetOffset() != -1);
        }

        int prevOff = 0;
        int i;
        for (i = 0; i < mNumItems; i++)
        {
            BRA_ASSERT(array.maRecordIndex[i].GetOffset() >= prevOff);
            prevOff = array.maRecordIndex[i].GetOffset();
        }
        // make sure the guard byte is there...
        //
        BRA_ASSERT(array.maRecordIndex[i].GetOffset() == -1);
        //
        // make sure the final offset is less than the highest record index offset.
        //
        BRA_ASSERT((int8_t*)&array.maRecordIndex[i] < util_OffsetToAddr(pBlock, prevOff));
        //
        // TODO -- is there anything else that is worth checking?
    }
    catch (eArchive& e)
    {
        d.TraceError("*** cBlockFile::GetBlock threw an eArchive: %d %s\n", e.GetID(), e.GetMsg().c_str());
        BRA_ASSERT(false);
    }
    // if we got here, then everything worked ok.
    //
    return true;
}


#ifdef _BLOCKFILE_DEBUG

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::TraceContents(int dl) const
{
    if (dl == -1)
        dl = cDebug::D_DEBUG;

    cDebug d("cBlockRecordArray::TraceContents");

    d.Trace(dl, "---------- Block Record Array ----------\n");
    d.Trace(dl, "----------------------------------------\n");
    d.Trace(dl, "Block Number    %d\n", mBlockNum);
    d.Trace(dl, "Space Available %d\n", mSpaceAvailable);
    d.Trace(dl, "Num Entries     %d\n", mNumItems);

    cBlockFile::Block* pBlock     = mpBlockFile->GetBlock(mBlockNum);
    tIndexArray&       indexArray = util_GetIndexArray(pBlock);

    for (int i = 0; i < mNumItems; i++)
    {
        d.Trace(dl,
                "(%d)\tOffset %d\tMain Idx %d\n",
                i,
                indexArray.maRecordIndex[i].GetOffset(),
                indexArray.maRecordIndex[i].GetMainIndex());
    }

    d.Trace(dl, "----------------------------------------\n");
}

///////////////////////////////////////////////////////////////////////////////
// AssertValid
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordArray::AssertValid() const
{
    ASSERT(IsClassValid());
}

#endif
