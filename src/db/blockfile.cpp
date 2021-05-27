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
// blockfile.cpp

#include "stddb.h"

#ifndef __TCHAR_H
#    error
#endif

#include "blockfile.h"
#include "core/archive.h"
#include "core/debug.h"
#include "core/archive.h"

#include <vector>

//-----------------------------------------------------------------------------
// cBlockFile
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cBlockFile::cBlockFile() : mNumPages(-1), mNumBlocks(-1), mTimer(0), mpArchive(0)
{
#ifdef _BLOCKFILE_DEBUG

    mNumBlockWrite   = 0;
    mNumBlockRead    = 0;
    mNumPageFault    = 0;
    mNumPageRequests = 0;

#endif //_BLOCKFILE_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cBlockFile::~cBlockFile()
{
    Close();
}

///////////////////////////////////////////////////////////////////////////////
// Open
///////////////////////////////////////////////////////////////////////////////
void cBlockFile::Open(const TSTRING& fileName, int numPages, bool bTruncate) //throw (eArchive)
{
    ASSERT(numPages > 0);
    ASSERT(!mpArchive);

    cFileArchive* pArch = new cFileArchive;
    pArch->OpenReadWrite(fileName.c_str(), (bTruncate ? cFileArchive::FA_OPEN_TRUNCATE : 0));

    Open(pArch, numPages);
}

void cBlockFile::Open(cBidirArchive* pArch, int numPages) //throw (eArchive)
{
    ASSERT(numPages > 0);
    ASSERT(!mpArchive);
    //
    // keep my sanity...
    //
    pArch->Seek(0, cBidirArchive::BEGINNING);
    //
    // initialize the paged blocks list...
    //
    mpArchive = pArch;
    mNumPages = numPages;
    mvPagedBlocks.resize(mNumPages);
    ASSERT(mvPagedBlocks.capacity() == mNumPages); // make sure that we didn't alloc too many!
    //
    // if the file is newly created, set its size to that of a single block
    //
    if (mpArchive->Length() == 0)
    {
        mpArchive->WriteBlob(mvPagedBlocks[0].GetData(), GetBlockSize());
    }
    //
    // make sure that the file is an appropriate length
    // TODO -- what is the correct thing to do if this fails? right now, I assert, but
    //      perhaps I should throw an exception or increase the file size to the next block interval
    ASSERT(mpArchive->Length() % GetBlockSize() == 0);
    mNumBlocks = mpArchive->Length() / GetBlockSize();
}


///////////////////////////////////////////////////////////////////////////////
// Close
///////////////////////////////////////////////////////////////////////////////
void cBlockFile::Close()
{
    if (mpArchive)
    {
        Flush();
        delete mpArchive;
        mpArchive = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Flush
///////////////////////////////////////////////////////////////////////////////
void cBlockFile::Flush()
{
    ASSERT(mpArchive);
    for (BlockVector::iterator i = mvPagedBlocks.begin(); i != mvPagedBlocks.end(); ++i)
    {
        FlushBlock(&(*i));
    }
}

///////////////////////////////////////////////////////////////////////////////
// GetBlock
///////////////////////////////////////////////////////////////////////////////
cBlockFile::Block* cBlockFile::GetBlock(int blockNum) //throw (eArchive)
{
#ifdef _BLOCKFILE_DEBUG
    mNumPageRequests++;
    AssertValid();
#endif
    //
    // debug stuff
    //
    cDebug d("cBlockFile::GetBlock");
    d.TraceNever("Request for block %d\n", blockNum);

    //TODO -- I should really throw an exception here...
    ASSERT((blockNum >= 0) && (blockNum < GetNumBlocks()));
    ASSERT(mpArchive);

    //
    // first, increment the timer; if it flips, reset everybody's access times to 0.
    //
    mTimer++;
    if (mTimer == 0)
    {
        for (BlockVector::iterator i = mvPagedBlocks.begin(); i != mvPagedBlocks.end(); ++i)
        {
            i->SetTimestamp(0);
        }
    }
    //
    // now, see if the desired block is in memory...
    //
    for (BlockVector::iterator i = mvPagedBlocks.begin(); i != mvPagedBlocks.end(); ++i)
    {
        if (i->GetBlockNum() == blockNum)
        {
            // this is easy; just return the block.
            i->SetTimestamp(mTimer);
            d.TraceNever("\tBlock %d was in memory.\n", blockNum);
            return &(*i);
        }
    }
    //
    // ok, we are going to have to page it into memory; look for the page with the earliest timestamp to
    // remove...
    //
#ifdef _BLOCKFILE_DEBUG
    mNumPageFault++;
#endif
    d.TraceNever("\tBlock %d was not in memory; paging it in\n", blockNum);

    uint32_t              earliestTime = mvPagedBlocks[0].GetTimestamp();
    BlockVector::iterator it           = mvPagedBlocks.begin();
    BlockVector::iterator earliestIter = it;
    ++it; // since we don't want to check the first one
    ASSERT(it != mvPagedBlocks.end());
    for (; it != mvPagedBlocks.end(); ++it)
    {
        if (it->GetTimestamp() < earliestTime)
        {
            earliestIter = it;
            earliestTime = it->GetTimestamp();
        }
    }
    ASSERT((earliestIter >= mvPagedBlocks.begin()) && (earliestIter < mvPagedBlocks.end()));
    //
    // flush the page data to disk if it is dirty...
    //
    FlushBlock(&(*earliestIter));
    d.TraceNever("\tPaging out block %d\n", earliestIter->GetBlockNum());
    //
    // ok, now we can read in the new page...
    //
    earliestIter->SetBlockNum(blockNum);
    earliestIter->Read(*mpArchive);
    earliestIter->SetTimestamp(mTimer);
    //
    // this variable keeps track of how many block reads we do.
#ifdef _BLOCKFILE_DEBUG
    mNumBlockRead++;
    AssertValid();
#endif
    return &(*earliestIter);
}

///////////////////////////////////////////////////////////////////////////////
// CreateBlock
///////////////////////////////////////////////////////////////////////////////
cBlockFile::Block* cBlockFile::CreateBlock()
{
    // debug stuff
    //
    ASSERT(mpArchive);
    cDebug d("cBlockFile::CreateBlock()");
    d.TraceDetail("Creating new block (number %d)\n", GetNumBlocks());

    char emptyBlock[BLOCK_SIZE];
    memset(emptyBlock, 0, BLOCK_SIZE);
    //
    // write empty data in the new block's location
    //
    //ASSERT(GetNumBlocks() * GetBlockSize() <= mpArchive->Length());
    d.TraceDetail("Seeking to %d * %d = %d\n", GetNumBlocks(), GetBlockSize(), GetNumBlocks() * GetBlockSize());
    mpArchive->Seek(GetNumBlocks() * GetBlockSize(), cBidirArchive::BEGINNING);
    mpArchive->WriteBlob(emptyBlock, BLOCK_SIZE);
    //
    // now, page it in...
    //
    mNumBlocks++;
    return GetBlock(GetNumBlocks() - 1);
}

///////////////////////////////////////////////////////////////////////////////
// DestroyLastBlock
///////////////////////////////////////////////////////////////////////////////
void cBlockFile::DestroyLastBlock()
{
    //TODO -- implement this!!!
    ASSERT(false);
    /*
    mNumBlocks--;
    mpArchive->Seek( mNumBlocks * BLOCK_SIZE, cBidirArchive::BEGINNING );
    mpArchive->Truncate();

    AssertValid();
*/
}


//-----------------------------------------------------------------------------
// Debug methods...
//-----------------------------------------------------------------------------
#ifdef _BLOCKFILE_DEBUG

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cBlockFile::TraceContents(int dl) const
{
    cDebug d("cBlockFile::TraceContents");
    if (dl == -1)
        dl = cDebug::D_DEBUG;

    d.Trace(dl, "Block size:             %d\n", GetBlockSize());
    d.Trace(dl, "Number of blocks:       %d\n", mNumBlocks);
    d.Trace(dl, "Current timer:          %d\n", mTimer);
    //
    // trace out the profiling info...
    //
    d.Trace(dl, "Number of page writes:  %d\n", mNumBlockWrite);
    d.Trace(dl, "Number of page reads:   %d\n", mNumBlockRead);
    d.Trace(dl, "Number of page faults:  %d\n", mNumPageFault);
    d.Trace(dl, "Number of page requests:%d\n", mNumPageRequests);

    d.Trace(dl, "Number of pages:        %d\n", mNumPages);
    d.Trace(dl, "-------------------------\n");
    //
    // trace out all the block information...
    //
    int cnt = 0;
    for (BlockVector::const_iterator i = mvPagedBlocks.begin(); i != mvPagedBlocks.end(); i++, cnt++)
    {
        d.Trace(dl,
                "Page[%d] Block Num=%d Timestamp=%d Dirty=%s\n",
                cnt,
                i->GetBlockNum(),
                i->GetTimestamp(),
                i->IsDirty() ? "true" : "false");
    }
    d.Trace(dl, "-------------------------\n");
}

///////////////////////////////////////////////////////////////////////////////
// CreateBlock
///////////////////////////////////////////////////////////////////////////////
void cBlockFile::AssertValid() const
{
    if (!mpArchive)
        return;
        //
        // make sure the archive length and block count match up
        //
#    ifdef DEBUG
    if (mpArchive->Length() != (GetBlockSize() * GetNumBlocks()))
    {
        cDebug d("cBlockFile::AssertValid");
        d.Trace(cDebug::D_DEBUG, "FATAL ERROR!!!\n");
        d.Trace(cDebug::D_DEBUG,
                "mpArchive->Length() = %d \n\t!= blocksize * numblocks = %d * %d\n",
                int(mpArchive->Length()),
                int(GetBlockSize()),
                int(GetNumBlocks()));
        d.Trace(cDebug::D_DEBUG, "now tracing the contents...\n");
        TraceContents(cDebug::D_DEBUG);
    }
#    endif
    ASSERT(mpArchive->Length() == (GetBlockSize() * GetNumBlocks()));

    //
    // iterate through the pages...
    //
    for (BlockVector::const_iterator i = mvPagedBlocks.begin(); i != mvPagedBlocks.end(); i++)
    {
        // make sure the block number is valid
        //
        int blockNum = i->GetBlockNum();
        ASSERT((blockNum == cBlock<BLOCK_SIZE>::INVALID_NUM) || ((blockNum >= 0) && (blockNum < GetNumBlocks())));
        //
        // make sure the time stamp is less than or equal to the timer time...
        //
        ASSERT(i->GetTimestamp() <= mTimer);
        //
        // assert that the guard bytes haven't been modified
        i->AssertValid();
    }
}

#endif //_BLOCKFILE_DEBUG
