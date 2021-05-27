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
// growheap.cpp
#include "stdcore.h"
#include "growheap.h"
#include "debug.h"
#include <vector>

//-----------------------------------------------------------------------------
// cGrowHeap_i
//-----------------------------------------------------------------------------
class cGrowHeap_i
{
public:
    class cHeap
    {
    public:
        size_t mSize;
        int8_t*  mpData;

        cHeap(size_t size) : mSize(size), mpData(new int8_t[size])
        {
            ASSERT(mpData != 0);
        }
        // Note: The above ASSERT should never occur!  If the new failed we should have thrown a bad_alloc().
    };
    typedef std::vector<cHeap> HeapList;

    size_t   mInitialSize;
    size_t   mGrowBy;
    HeapList mHeaps;
    TSTRING  mName;
    size_t   mCurOff;

    cGrowHeap_i(size_t initialSize, size_t growBy, const TCHAR* name);
    ~cGrowHeap_i()
    {
        Clear();
    }

    size_t AlignSizeRequest(size_t size, size_t alignSize);
    void*  Malloc(size_t size);
    void   Clear();
};


cGrowHeap_i::cGrowHeap_i(size_t initialSize, size_t growBy, const TCHAR* name)
    : mInitialSize(initialSize), mGrowBy(growBy), mName(name), mCurOff(0)
{
    // assure that initial size and growby are aligned
    ASSERT(0 == (initialSize % BYTE_ALIGN));
    ASSERT(0 == (growBy % BYTE_ALIGN));
}

size_t cGrowHeap::TotalMemUsage() const
{
    size_t usage = 0;
    for (cGrowHeap_i::HeapList::const_iterator i = mpData->mHeaps.begin(); i != mpData->mHeaps.end(); i++)
    {
        usage += i->mSize;
    }
    if (!mpData->mHeaps.empty())
    {
        //  take off the unused portion...
        usage -= (mpData->mHeaps.back().mSize - mpData->mCurOff);
    }
    return usage;
}


void* cGrowHeap_i::Malloc(size_t size)
{
    size = AlignSizeRequest(size, BYTE_ALIGN);

    ASSERT((size > 0) && (size < mGrowBy));

    if (size >= mGrowBy)
        return NULL;

    if (mHeaps.empty())
    {
        mHeaps.push_back(cHeap(mInitialSize));
        ASSERT(mHeaps.back().mpData != 0);
        mCurOff = 0;
    }

    if (mCurOff + size < mHeaps.back().mSize)
    {
        // we have room to add this to the current heap.
        //
        ASSERT(mHeaps.back().mpData);
        int8_t* ret = mHeaps.back().mpData + mCurOff;
        mCurOff += size;

        return ret;
    }
    else
    {
        mHeaps.push_back(cHeap(mGrowBy));
        ASSERT(mHeaps.back().mpData != 0);
        mCurOff = 0;

#ifdef _DEUBG
        void* ret = Malloc(size);
        ASSERT(ret != 0);
        return ret;
#else
        return Malloc(size);
#endif
    }
}

size_t cGrowHeap_i::AlignSizeRequest(size_t size, size_t alignSize)
{
    // The two's complement algorithm requires a non-zero size request size,
    // so make make all requests require it so that this function
    // acts the same no matter what the integer representation
    if (0 == size)
        size = 1;

#if USES_2S_COMPLEMENT
    // This efficient algorithm assumes alignSize is power of two AND a
    // 2's complement representation. Requires non-zero size request
    ASSERT(0 == (alignSize % 2));
    ASSERT(size > 0);
    return ((size + alignSize - 1) & ~(alignSize - 1));
#else
    // this makes no assumption about BYTE_ALIGN or hardware integer representation
    return (((size / alignSize) + ((size % alignSize) ? 1 : 0)) * alignSize);
#endif
}

void cGrowHeap_i::Clear()
{
    for (HeapList::iterator i = mHeaps.begin(); i != mHeaps.end(); i++)
    {
        delete [] i->mpData;
    }
    mHeaps.clear();
}

//-----------------------------------------------------------------------------
// cGrowHeap
//-----------------------------------------------------------------------------
cGrowHeap::cGrowHeap(size_t initialSize, size_t growBy, const TCHAR* name)
    : mpData(new cGrowHeap_i(initialSize, growBy, name))
{
}

cGrowHeap::~cGrowHeap()
{
    cDebug d("FCO name heap stats");
    d.TraceDebug(_T("Total heap memory usage for %s: %d\n"), mpData->mName.c_str(), TotalMemUsage());

    delete mpData;
}

void* cGrowHeap::Malloc(size_t size)
{
    return mpData->Malloc(size);
}

void cGrowHeap::Clear()
{
    mpData->Clear();
}
