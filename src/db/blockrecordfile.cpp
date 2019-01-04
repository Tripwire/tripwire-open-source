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
// blockrecordfile.cpp

#include "stddb.h"

#include "blockrecordfile.h"
#include "core/archive.h"

///////////////////////////////////////////////////////////////////////////////
// util_InitBlockArray
///////////////////////////////////////////////////////////////////////////////
static inline void util_InitBlockArray(cBlockRecordArray& block)
{
    if (!block.Initialized())
    {
        block.InitForExistingBlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cBlockRecordFile::cBlockRecordFile() : mLastAddedTo(-1), mbOpen(false)
{
}

///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cBlockRecordFile::~cBlockRecordFile()
{
    Close();
}

///////////////////////////////////////////////////////////////////////////////
// Open
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::Open(const TSTRING& fileName, int numPages, bool bTruncate) //throw (eArchive)
{
    ASSERT(!mbOpen);

    // first, open the file..
    //
    mBlockFile.Open(fileName, numPages, bTruncate);
}

void cBlockRecordFile::Open(cBidirArchive* pArch, int numPages) //throw (eArchive)
{
    ASSERT(!mbOpen);
    bool bTruncate = (pArch->Length() == 0);

    mBlockFile.Open(pArch, numPages);

    OpenImpl(bTruncate);
}

///////////////////////////////////////////////////////////////////////////////
// OpenImpl
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::OpenImpl(bool bTruncate) //throw (eArchive)
{
    ASSERT(!mbOpen);
    //
    // create the vector of block record arrays...avoid initializing them all
    //  right now so that we don't page the whole file into memory
    //
    mvBlocks.clear();
    for (int i = 0; i < mBlockFile.GetNumBlocks(); i++)
    {
        mvBlocks.push_back(cBlockRecordArray(&mBlockFile, i));
        //
        // if we are creating a new file, initialize the block for first usage...
        //
        if (bTruncate)
        {
            mvBlocks.back().InitNewBlock();
        }
    }
    //
    // use the integrity of the first block as a q&d check for file integrity...
    //
    ASSERT(mvBlocks.size() > 0);
    if (!mvBlocks.begin()->Initialized())
    {
        mvBlocks.begin()->InitForExistingBlock();
    }
    if (!mvBlocks.begin()->IsClassValid())
    {
        TSTRING str = _T("Bad file format for the Block Record File");
        throw eArchiveFormat(str);
    }
    // set the last item added to to zero (TODO -- is this the right thing to do or
    // should I store it in the archive?
    //
    mLastAddedTo = 0;

    mbOpen = true;
}

///////////////////////////////////////////////////////////////////////////////
// Close
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::Close() //throw (eArchive)
{
    Flush();
    mBlockFile.Close();
    mbOpen = false;
    mvBlocks.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Flush
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::Flush() //throw (eArchive)
{
    mBlockFile.Flush();
}

///////////////////////////////////////////////////////////////////////////////
// AddItem
///////////////////////////////////////////////////////////////////////////////
cBlockRecordFile::tAddr cBlockRecordFile::AddItem(int8_t* pData, int dataSize) //throw (eArchive)
{
    ASSERT(mbOpen);
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif
    tAddr rtn(0, 0);
    //
    // first, let's find room for this data item...
    //
    rtn.mBlockNum = FindRoomForData(dataSize);
    //
    // now, insert the data...
    //
    rtn.mIndex = mvBlocks[rtn.mBlockNum].AddItem(pData, dataSize, 1);
    //
    // update the last added to pointer and return the location...
    //
    mLastAddedTo = rtn.mBlockNum;
    return rtn;
}

///////////////////////////////////////////////////////////////////////////////
// RemoveItem
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::RemoveItem(cBlockRecordFile::tAddr dataAddr) //throw (eArchive)
{
    ASSERT(mbOpen);
    ASSERT(IsValidAddr(dataAddr));
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif

    mvBlocks[dataAddr.mBlockNum].DeleteItem(dataAddr.mIndex);
    //
    // remove unneeded blocks at the end...
    //
    //TODO -- not implemented yet!
    /*
    while( mvBlocks.back().GetNumItems() == 0 )
    {
        // if there are >2 blocks, and the back two blocks are both
        // empty, remove one. We always want to be sure there is at
        // least one block in the database, and we don't want to remove
        // single trailing empty blocks because it would perform poorly if 
        // there were a bunch of additions and removals at the end of the file.
        if( (mvBlocks.size() > 2) && (mvBlocks[ mvBlocks.size()-2 ].GetNumItems() == 0 )
        {
            // TODO -- uncomment these to implement file shrinking...
            mvBlocks.pop_back();
            mBlockFile.DestroyLastBlock();
        }
    }
    */
}

///////////////////////////////////////////////////////////////////////////////
// IsValidAddr
///////////////////////////////////////////////////////////////////////////////
bool cBlockRecordFile::IsValidAddr(cBlockRecordFile::tAddr addr) //throw (eArchive)
{
    ASSERT(mbOpen);

    if ((addr.mBlockNum < 0) || (addr.mBlockNum >= mBlockFile.GetNumBlocks()))
        return false;

    util_InitBlockArray(mvBlocks[addr.mBlockNum]);

    return (mvBlocks[addr.mBlockNum].IsItemValid(addr.mIndex));
}

///////////////////////////////////////////////////////////////////////////////
// GetDataForReading
///////////////////////////////////////////////////////////////////////////////
int8_t* cBlockRecordFile::GetDataForReading(cBlockRecordFile::tAddr dataAddr, int32_t& dataSize) //throw (eArchive)
{
    ASSERT(mbOpen);
    ASSERT(IsValidAddr(dataAddr));
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif

    return (mvBlocks[dataAddr.mBlockNum].GetDataForReading(dataAddr.mIndex, dataSize));
}

///////////////////////////////////////////////////////////////////////////////
// GetDataForWriting
///////////////////////////////////////////////////////////////////////////////
int8_t* cBlockRecordFile::GetDataForWriting(cBlockRecordFile::tAddr dataAddr, int32_t& dataSize) //throw (eArchive)
{
    ASSERT(mbOpen);
    ASSERT(IsValidAddr(dataAddr));
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif

    return (mvBlocks[dataAddr.mBlockNum].GetDataForWriting(dataAddr.mIndex, dataSize));
}

///////////////////////////////////////////////////////////////////////////////
// FindRoomForData
///////////////////////////////////////////////////////////////////////////////
int cBlockRecordFile::FindRoomForData(int32_t dataSize) //throw (eArchive)
{
    ASSERT((dataSize > 0) && (dataSize <= cBlockRecordArray::MAX_DATA_SIZE));
    ASSERT(mbOpen);
#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif
    cDebug d("cBlockRecordFile::FindRoomForData");

    // first, try the last added to block...
    //
    d.TraceDetail("Looking for room for %d bytes; first trying mLastAddedTo (%d)\n", dataSize, mLastAddedTo);
    if (mLastAddedTo >= 0)
    {
        util_InitBlockArray(mvBlocks[mLastAddedTo]);
        if (mvBlocks[mLastAddedTo].GetAvailableSpace() >= dataSize)
        {
            d.TraceDetail("---Found room in block %d\n", mLastAddedTo);
            return mLastAddedTo;
        }
    }
    //
    // ok, I guess we will have to iterate through all the blocks...
    //
    BlockArray::iterator it;
    int                  cnt = 0;
    for (it = mvBlocks.begin(); it != mvBlocks.end(); ++it, ++cnt)
    {
        util_InitBlockArray(*it);
        if (it->GetAvailableSpace() >= dataSize)
        {
            d.TraceDetail("---Found room in block %d\n", cnt);
            return cnt;
        }
    }
    //
    // if we got here, then we need to add a new block
    //
    d.TraceDetail("---We need to add new block(%d)\n", cnt);
    mBlockFile.CreateBlock();
    ASSERT((mBlockFile.GetNumBlocks() == (mvBlocks.size() + 1)) && (mvBlocks.size() == cnt));
    mvBlocks.push_back(cBlockRecordArray(&mBlockFile, cnt));
    mvBlocks.back().InitNewBlock();

    ASSERT(mvBlocks.back().GetAvailableSpace() >= dataSize);

    return cnt;
}


#ifdef _BLOCKFILE_DEBUG

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::TraceContents(int dl) const
{
    // TODO -- this is probably not what I want to do, but it helps me right now...
    //
    for (BlockArray::const_iterator i = mvBlocks.begin(); i != mvBlocks.end(); ++i)
    {
        i->TraceContents(dl);
    }
}

///////////////////////////////////////////////////////////////////////////////
// AssertValid
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::AssertValid() const
{
    ASSERT((mLastAddedTo >= 0) && (mLastAddedTo < mvBlocks.size()));
    ASSERT(mvBlocks.size() == mBlockFile.GetNumBlocks());
}

///////////////////////////////////////////////////////////////////////////////
// AssertAllBlocksValid
///////////////////////////////////////////////////////////////////////////////
void cBlockRecordFile::AssertAllBlocksValid()
{
    for (BlockArray::iterator i = mvBlocks.begin(); i != mvBlocks.end(); ++i)
    {
        i->AssertValid();
    }
}


#endif
