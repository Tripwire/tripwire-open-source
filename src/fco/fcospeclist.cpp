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
// fcospeclist.cpp
//
// cFCOSpecList -- a list of iFCOSpec's with in a well defined order.
///////////////////////////////////////////////////////////////////////////////

#include "stdfco.h"
#include "fcospeclist.h"
#include "fcospecutil.h"
#include "core/serializer.h"
#include "core/debug.h"
#include "core/errorutil.h"

///////////////////////////////////////////////////////////////////////////////
// class cFCOSpecList-- a list of iFCOSpec's with a well defined order.
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cFCOSpecList, _T("cFCOSpecList"), 0, 1)

cFCOSpecList::cFCOSpecList()
{
}

cFCOSpecList::~cFCOSpecList()
{
    Clear();
}

void cFCOSpecList::Clear()
{
    std::list<PairType>::iterator itr;

    for (itr = mAddedList.begin(); itr != mAddedList.end(); ++itr)
    {
        itr->first->Release();
        itr->second->Release();
    }

    mCanonicalList.clear();
    mAddedList.clear();
}

bool cFCOSpecList::IsEmpty() const
{
    return mAddedList.empty();
}

int cFCOSpecList::Size() const
{
    return mAddedList.size();
}

const cFCOSpecList& cFCOSpecList::operator=(const cFCOSpecList& rhs)
{
    Clear();

    cFCOSpecListAddedIter itr(rhs);

    for (itr.SeekBegin(); !itr.Done(); itr.Next())
    {
        Add(itr.Spec(), itr.Attr());
    }

    return *this;
}

void cFCOSpecList::Add(iFCOSpec* pSpec, cFCOSpecAttr* pAttr)
{
    std::list<PairType>::iterator itr;
    pSpec->AddRef();
    if (pAttr == 0)
        pAttr = new cFCOSpecAttr;
    else
        pAttr->AddRef();
        
    for (itr = mCanonicalList.begin();; ++itr)
    {
        if (itr == mCanonicalList.end() || iFCOSpecUtil::FCOSpecLessThan(*pSpec, *itr->first))
        {
            mCanonicalList.insert(itr, PairType(pSpec, pAttr));
            mAddedList.push_back(PairType(pSpec, pAttr));
            break;
        }
    }
}

iFCOSpec* cFCOSpecList::Lookup(iFCOSpec* pSpec) const
{
    std::list<PairType>::iterator itr;
    for (itr = mCanonicalList.begin(); itr != mCanonicalList.end(); ++itr)
    {
        if (itr->first == pSpec)
        {
            pSpec->AddRef();
            return itr->first;
        }
    }

    for (itr = mCanonicalList.begin(); itr != mCanonicalList.end(); ++itr)
    {
        if (iFCOSpecUtil::FCOSpecEqual(*pSpec, *itr->first))
        {
            itr->first->AddRef();
            return itr->first;
        }
    }

    return NULL;
}

void cFCOSpecList::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("FCO Spec List")));

    Clear();

    int32_t size;
    pSerializer->ReadInt32(size);

    for (int i = 0; i < size; ++i)
    {
        iFCOSpec*     pReadInSpec = static_cast<iFCOSpec*>(pSerializer->ReadObjectDynCreate());
        cFCOSpecAttr* pSpecAttr   = static_cast<cFCOSpecAttr*>(pSerializer->ReadObjectDynCreate());
        Add(pReadInSpec, pSpecAttr);
        pReadInSpec->Release(); // Add() will increase the ref count by 1
        pSpecAttr->Release();
    }
}

void cFCOSpecList::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(Size());

    cFCOSpecListAddedIter itr(*this);
    for (itr.SeekBegin(); !itr.Done(); itr.Next())
    {
        pSerializer->WriteObjectDynCreate(itr.Spec());
        pSerializer->WriteObjectDynCreate(itr.Attr());
    }
}

///////////////////////////////////////////////////////////////////////////////
// class cFCOSpecListAddedIter -- iterate using the order in which items were
//      added to this list.
///////////////////////////////////////////////////////////////////////////////

cFCOSpecListAddedIter::cFCOSpecListAddedIter(const cFCOSpecList& list)
{
    mpSpecList = (cFCOSpecList*)&list;
    mIter      = mpSpecList->mAddedList.begin();
}

cFCOSpecListAddedIter::~cFCOSpecListAddedIter()
{
}

void cFCOSpecListAddedIter::SeekBegin() const
{
    mIter = mpSpecList->mAddedList.begin();
}

bool cFCOSpecListAddedIter::Done() const
{
    return mIter == mpSpecList->mAddedList.end();
}

bool cFCOSpecListAddedIter::IsEmpty() const
{
    return mpSpecList->mAddedList.empty();
}

void cFCOSpecListAddedIter::Next() const
{
    ++mIter;
}

const iFCOSpec* cFCOSpecListAddedIter::Spec() const
{
    return mIter->first;
}

iFCOSpec* cFCOSpecListAddedIter::Spec()
{
    return mIter->first;
}
const cFCOSpecAttr* cFCOSpecListAddedIter::Attr() const
{
    return mIter->second;
}
cFCOSpecAttr* cFCOSpecListAddedIter::Attr()
{
    return mIter->second;
}

void cFCOSpecListAddedIter::Remove()
{
    ASSERT(!Done());

    // the tricky part is finding the spec in the other list...
    std::list<cFCOSpecList::PairType>::iterator i;
    for (i = mpSpecList->mCanonicalList.begin(); i != mpSpecList->mCanonicalList.end(); ++i)
    {
        if (i->first == mIter->first)
            break;
    }
    ASSERT(i != mpSpecList->mCanonicalList.end());

    // now to do the deleteing!
    mIter = mpSpecList->mAddedList.erase(mIter);
    mpSpecList->mCanonicalList.erase(i);
}


///////////////////////////////////////////////////////////////////////////////
// class cFCOSpecListCanonicalIter -- iterate using the order in which items were
//      added to this list.
///////////////////////////////////////////////////////////////////////////////

cFCOSpecListCanonicalIter::cFCOSpecListCanonicalIter(const cFCOSpecList& list)
{
    mpSpecList = (cFCOSpecList*)&list;
    mIter      = mpSpecList->mCanonicalList.begin();
}

cFCOSpecListCanonicalIter::~cFCOSpecListCanonicalIter()
{
}

void cFCOSpecListCanonicalIter::SeekBegin() const
{
    mIter = mpSpecList->mCanonicalList.begin();
}

bool cFCOSpecListCanonicalIter::Done() const
{
    return mIter == mpSpecList->mCanonicalList.end();
}

bool cFCOSpecListCanonicalIter::IsEmpty() const
{
    return mpSpecList->mCanonicalList.empty();
}

void cFCOSpecListCanonicalIter::Next() const
{
    ++mIter;
}

const iFCOSpec* cFCOSpecListCanonicalIter::Spec() const
{
    return mIter->first;
}

iFCOSpec* cFCOSpecListCanonicalIter::Spec()
{
    return mIter->first;
}
const cFCOSpecAttr* cFCOSpecListCanonicalIter::Attr() const
{
    return mIter->second;
}
cFCOSpecAttr* cFCOSpecListCanonicalIter::Attr()
{
    return mIter->second;
}
