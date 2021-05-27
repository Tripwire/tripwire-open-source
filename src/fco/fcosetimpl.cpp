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
// fcosetimpl.cpp
//

#include "stdfco.h"
#include "fconame.h"
#include "fcosetimpl.h"
#include "core/serializer.h"
#include "core/debug.h"
#include "core/errorutil.h"


//################################################################
// cFCOSetImpl
//################################################################

IMPLEMENT_TYPEDSERIALIZABLE(cFCOSetImpl, _T("cFCOSetImpl"), 0, 1)

///////////////////////////////////////////////////////////////////////////////
// CreateIterator -- creates a new iterator for the passed class
///////////////////////////////////////////////////////////////////////////////
inline static cFCOIterImpl* CreateIterator(cFCOSetImpl* pSet)
{
    return new cFCOIterImpl(pSet);
}

inline static const cFCOIterImpl* CreateIterator(const cFCOSetImpl* pSet)
{
    return new cFCOIterImpl(pSet);
}

///////////////////////////////////////////////////////////////////////////////
// ctor and dtor
///////////////////////////////////////////////////////////////////////////////
cFCOSetImpl::cFCOSetImpl() : iFCOSet()
{
}

cFCOSetImpl::~cFCOSetImpl()
{
    Clear();
}

cFCOSetImpl::cFCOSetImpl(const cFCOSetImpl& rhs) : iFCOSet()
{
    *this = rhs;
}

///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
cFCOSetImpl& cFCOSetImpl::operator=(const cFCOSetImpl& rhs)
{
    std::set<cFCONode>::const_iterator i;
    for (i = rhs.mFCOSet.begin(); i != rhs.mFCOSet.end(); ++i)
    {
        i->mpFCO->AddRef();
        mFCOSet.insert(cFCONode(i->mpFCO));
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Clear -- remove all elements from the set...
///////////////////////////////////////////////////////////////////////////////
void cFCOSetImpl::Clear()
{
    // release our grip on all the fcos.
    std::set<cFCONode>::iterator i;
    for (i = mFCOSet.begin(); i != mFCOSet.end(); ++i)
    {
        i->mpFCO->Release();
    }

    mFCOSet.clear();
}

///////////////////////////////////////////////////////////////////////////////
// IsEmpty -- Anything in the set?
///////////////////////////////////////////////////////////////////////////////
bool cFCOSetImpl::IsEmpty() const
{
    return mFCOSet.empty();
}

///////////////////////////////////////////////////////////////////////////////
// Lookup -- TODO: figure out how I can implement const and non-const versions
//      without duplicating code.
///////////////////////////////////////////////////////////////////////////////
const iFCOIter* cFCOSetImpl::Lookup(const cFCOName& name) const
{
    const cFCOIterImpl* pIter = CreateIterator(this);
    if (!pIter->SeekToFCO(name))
    {
        pIter->DestroyIter();
        pIter = NULL;
    }
    return pIter;
}

iFCOIter* cFCOSetImpl::Lookup(const cFCOName& name)
{
    cFCOIterImpl* pIter = CreateIterator(this);
    if (!pIter->SeekToFCO(name))
    {
        pIter->DestroyIter();
        pIter = NULL;
    }
    return pIter;
}

///////////////////////////////////////////////////////////////////////////////
// GetIter
///////////////////////////////////////////////////////////////////////////////
const iFCOIter* cFCOSetImpl::GetIter() const
{
    return CreateIterator(this);
}

iFCOIter* cFCOSetImpl::GetIter()
{
    return CreateIterator(this);
}

void cFCOSetImpl::ReturnIter(const cFCOIterImpl* pIter) const
{
    delete pIter;
}

///////////////////////////////////////////////////////////////////////////////
// Insertion
///////////////////////////////////////////////////////////////////////////////
void cFCOSetImpl::Insert(iFCO* pFCO)
{

    //TODO -- how do I handle the insertion of the same fco into the set?
    // should it be a no-op, added twice, asserted, or an exception?
    std::pair<std::set<cFCONode>::iterator, bool> p;
    p = mFCOSet.insert(cFCONode(pFCO));
    // if the element already existed in the set, p.second is false.
    ASSERT(p.second);
    if (p.second)
        pFCO->AddRef();
}

///////////////////////////////////////////////////////////////////////////////
// AcceptSerializer
///////////////////////////////////////////////////////////////////////////////
void cFCOSetImpl::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("FCO Set Read")));

    Clear();

    int   i;
    int32_t size;
    pSerializer->ReadInt32(size);

    // TODO -- don't assert; throw an exception or noop -- mdb
    //ASSERT(size >= 0);

    for (i = 0; i < size; ++i)
    {
        iTypedSerializable* pObj = pSerializer->ReadObjectDynCreate();
        mFCOSet.insert(static_cast<iFCO*>(pObj));
    }
}

void cFCOSetImpl::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(mFCOSet.size());

    std::set<cFCONode>::const_iterator itr;
    for (itr = mFCOSet.begin(); itr != mFCOSet.end(); ++itr)
    {
        pSerializer->WriteObjectDynCreate(itr->mpFCO);
    }
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFCOSetImpl::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug                             d("cFCOSetImpl::TraceContents");
    std::set<cFCONode>::const_iterator i;
    int                                count = 0;
    for (i = mFCOSet.begin(); i != mFCOSet.end(); ++i, ++count)
    {
        d.Trace(dl, "[FCO %d]\n", count);
        i->mpFCO->TraceContents(dl);
    }
}


//################################################################
// cFCOIterImpl
//################################################################

///////////////////////////////////////////////////////////////////////////////
// ctor and dtor
///////////////////////////////////////////////////////////////////////////////
cFCOIterImpl::cFCOIterImpl(cFCOSetImpl* pSet) : mpSet(pSet), mIter()
{
    mIter = mpSet->mFCOSet.begin();
}

cFCOIterImpl::cFCOIterImpl(const cFCOSetImpl* pSet) : mpSet((cFCOSetImpl*)pSet), mIter()
{
    mIter = mpSet->mFCOSet.begin();
}

cFCOIterImpl::~cFCOIterImpl()
{
}

///////////////////////////////////////////////////////////////////////////////
// SeekToFCO
///////////////////////////////////////////////////////////////////////////////
bool cFCOIterImpl::SeekToFCO(const cFCOName& name) const
{
    mIter = mpSet->mFCOSet.find(cFCOSetImpl::cFCONode(name));
    return (mIter != mpSet->mFCOSet.end());
}


void cFCOIterImpl::SeekBegin() const
{
    ASSERT(mpSet != 0);
    mIter = mpSet->mFCOSet.begin();
}


bool cFCOIterImpl::Done() const
{
    ASSERT(mpSet != 0);
    return (mIter == mpSet->mFCOSet.end());
}

bool cFCOIterImpl::IsEmpty() const
{
    ASSERT(mpSet != 0);
    return (mpSet->mFCOSet.empty());
}

void cFCOIterImpl::Next() const
{
    ASSERT(mpSet != 0);
    ++mIter;
}


const iFCO* cFCOIterImpl::FCO() const
{
    return mIter->mpFCO;
}

iFCO* cFCOIterImpl::FCO()
{
    return mIter->mpFCO;
}

void cFCOIterImpl::Remove()
{
    ASSERT(mpSet != 0);
    ASSERT(mIter != mpSet->mFCOSet.end());
    mIter->mpFCO->Release();
    mpSet->mFCOSet.erase(mIter++);
}


void cFCOIterImpl::Remove() const
{
    ASSERT(mpSet != 0);
    ASSERT(mIter != mpSet->mFCOSet.end());
    mIter->mpFCO->Release();
    mpSet->mFCOSet.erase(mIter++);
}


void cFCOIterImpl::DestroyIter() const
{
    ASSERT(mpSet != 0);
    mpSet->ReturnIter(this);
}
