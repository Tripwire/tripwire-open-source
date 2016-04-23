//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
// objectpool.cpp
#include "stdcore.h"
#include "objectpool.h"
#include "debug.h"

//-----------------------------------------------------------------------------
// cBlockLink
//-----------------------------------------------------------------------------
class cBlockLink
{
public:
    cBlockLink(cBlockLink* pNext) : mpNext(pNext)   {}
    cBlockLink* Next()                              { return mpNext; }
private:
    cBlockLink* mpNext; // pointer to the next link, or NULL
};


//-----------------------------------------------------------------------------
// cBlockList
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cBlockList::cBlockList() :
    mpBlocks(0)
{
}

cBlockList::~cBlockList()
{
    Clear();
}

///////////////////////////////////////////////////////////////////////////////
// Allocate
///////////////////////////////////////////////////////////////////////////////
void* cBlockList::Allocate(int size)
{
    char* mem   = new char[size + sizeof(cBlockLink)];
    mpBlocks    = new(mem) cBlockLink(mpBlocks);
    return mem + sizeof(cBlockLink);
}

///////////////////////////////////////////////////////////////////////////////
// Clear
///////////////////////////////////////////////////////////////////////////////
void cBlockList::Clear()
{
    while(mpBlocks)
    {
        cBlockLink* pLink = mpBlocks;
        mpBlocks = mpBlocks->Next();
        pLink->~cBlockLink();
        delete [] (char*)(pLink);
    }
}

//-----------------------------------------------------------------------------
// cObjectPoolBase
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cObjectPoolBase::cObjectPoolBase(int objSize, int chunkSize) :
    mObjectSize(objSize),
    mChunkSize(chunkSize),
    mpNextFree(0)
{
}

cObjectPoolBase::~cObjectPoolBase()
{
    //TODO -- assert that the number of instances left are 0.
    Clear();
}

///////////////////////////////////////////////////////////////////////////////
// AllocNewChunk
///////////////////////////////////////////////////////////////////////////////
void cObjectPoolBase::AllocNewChunk()
{
    ASSERT(mpNextFree == 0);

    int size        = mObjectSize * mChunkSize;
    char* pBlock    = (char*)mBlockList.Allocate(size);

    char* pLast = pBlock + size - mObjectSize;
    for(char* pc = pBlock; pc < pLast; pc += mObjectSize)
    {
        ((cLink*)pc)->mpNext = (cLink*)(pc + mObjectSize);
    }
    ((cLink*)pLast)->mpNext = 0;

    mpNextFree = (cLink*)pBlock;
}


