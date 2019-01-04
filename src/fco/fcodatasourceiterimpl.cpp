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
// fcodatasourceiterimpl.cpp
//

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdfco.h"
#include "fcodatasourceiterimpl.h"
#include "fco.h"
#include "core/upperbound.h"
#include "core/errorbucket.h"

//=========================================================================
// METHOD CODE
//=========================================================================

cFCODataSourceIterImpl::cFCODataSourceIterImpl() : mpErrorBucket(0), mParentName(), mFlags(0)
{
}


cFCODataSourceIterImpl::~cFCODataSourceIterImpl()
{
    ClearList();
}

///////////////////////////////////////////////////////////////////////////////
// operator =
///////////////////////////////////////////////////////////////////////////////
cFCODataSourceIterImpl& cFCODataSourceIterImpl::operator=(const cFCODataSourceIterImpl& rhs)
{
    ClearList();
    mPeers        = rhs.mPeers;
    mParentName   = rhs.mParentName;
    mpErrorBucket = rhs.mpErrorBucket;
    mFlags        = rhs.mFlags;
    //
    // we need to addref all of the fcos we just got...
    //
    for (FCOList::const_iterator i = mPeers.begin(); i != mPeers.end(); ++i)
    {
        (*i)->AddRef();
    }
    //
    // figuring out where to put the iterator is a little trickier...
    // TODO -- if we ever change the mPeers data structure from a vector, this will
    //      have to change (luckily, the compiler should catch this!)
    //
    if (rhs.Done())
    {
        mCurPos = mPeers.end();
    }
    else
    {
        int offset = rhs.mCurPos - rhs.mPeers.begin();
        mCurPos    = mPeers.begin() + offset;
    }

    return *this;
}


///////////////////////////////////////////////////////////////////////////////
// GetShortName
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cFCODataSourceIterImpl::GetShortName() const
{
    ASSERT(!Done());

    return ((*mCurPos)->GetName().GetShortName());
}

///////////////////////////////////////////////////////////////////////////////
// GetName
///////////////////////////////////////////////////////////////////////////////
cFCOName cFCODataSourceIterImpl::GetName() const
{
    ASSERT(!Done());

    return ((*mCurPos)->GetName());
}

///////////////////////////////////////////////////////////////////////////////
// GetParentName
///////////////////////////////////////////////////////////////////////////////
cFCOName cFCODataSourceIterImpl::GetParentName() const
{
    return mParentName;
}

///////////////////////////////////////////////////////////////////////////////
// GetParentName
///////////////////////////////////////////////////////////////////////////////
bool cFCODataSourceIterImpl::AtRoot() const //throw (eError)
{
    return (mParentName.GetSize() == 0);
}

///////////////////////////////////////////////////////////////////////////////
// CanDescend
///////////////////////////////////////////////////////////////////////////////
bool cFCODataSourceIterImpl::CanDescend() const //throw (eError)
{
    ASSERT(!Done());
    if (Done())
    {
        return false;
    }

    return ((*mCurPos)->GetCaps() & iFCO::CAP_CAN_HAVE_CHILDREN);
}

///////////////////////////////////////////////////////////////////////////////
// Descend
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::Descend() //throw (eError)
{
    ASSERT(CanDescend());

    mParentName = GetName();
    GeneratePeers();
}

///////////////////////////////////////////////////////////////////////////////
// Ascend
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::Ascend() //throw (eError)
{
    ASSERT(!AtRoot());

    cFCOName name = mParentName;
    SeekToFCO(name); // I do this because SeekToFCO modifies mCwd and takes a reference parameter.
}

///////////////////////////////////////////////////////////////////////////////
// SeekBegin
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::SeekBegin()
{
    mCurPos = mPeers.begin();
}

///////////////////////////////////////////////////////////////////////////////
// Done
///////////////////////////////////////////////////////////////////////////////
bool cFCODataSourceIterImpl::Done() const
{
    return (mCurPos == mPeers.end());
}

///////////////////////////////////////////////////////////////////////////////
// Next
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::Next()
{
    ++mCurPos;
}

///////////////////////////////////////////////////////////////////////////////
// CreateFCO
///////////////////////////////////////////////////////////////////////////////
iFCO* cFCODataSourceIterImpl::CreateFCO() //throw (eError)
{
    ASSERT(!Done());
    InitializeTypeInfo(*mCurPos);
    (*mCurPos)->AddRef();
    return *mCurPos;
}

///////////////////////////////////////////////////////////////////////////////
// SeekToFCO
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::SeekToFCO(const cFCOName& name, bool bCreatePeers) // throw (eFCODataSourceIter)
{
    ASSERT(name.GetSize() > 0);
    ASSERT(name.IsCaseSensitive() == IsCaseSensitive());

    //
    // get the parent name and pop the short child name
    //
    cFCOName parentName = name;

    parentName.Pop(); // RAD:05/10/1999 -- Used to assign result to "shortname"

    if (parentName.GetSize() == 0)
    {
        // we are going to the root directory...
        // note that we are not responsible for iterating over the root nodes (for example,
        //      all of the drive letters in NT or all the hive names in the registry)
        //
        mParentName.Clear();
        ClearList();
        iFCO* pObj = CreateObject(name, bCreatePeers);
        if (!pObj)
        {
            // error creating object; just return.
            return;
        }

        InsertIntoPeers(pObj);
        mCurPos = mPeers.begin();
        return;
    }
    else
    {
        // if we are not creating our peers, simply create the single object and return...
        //
        if (!bCreatePeers)
        {
            ClearList();
            mParentName   = parentName;
            iFCO* pNewObj = CreateObject(name, false);
            if (pNewObj)
            {
                InsertIntoPeers(pNewObj);
                mCurPos = mPeers.begin();
            }
            return;
        }
        //
        // otherwise, load up everything. Note that we can't do the if(parentName == mParentName) optimization
        //      because if you do a SeekToFCO( c:/foo, false) and then a SeekToFCO( c:/bar, true), it won't work
        //      correctly -- 27 Jan 99 mdb
        //
        mParentName = parentName;
        ClearList();
        GeneratePeers();
    }

    if (mCurPos != mPeers.end() && *mCurPos)
        SeekToPeerByName((*mCurPos)->GetName().GetShortName());
}

///////////////////////////////////////////////////////////////////////////////
// IsCaseSensitive
///////////////////////////////////////////////////////////////////////////////
bool cFCODataSourceIterImpl::IsCaseSensitive() const
{
    return mParentName.IsCaseSensitive();
}

///////////////////////////////////////////////////////////////////////////////
// ClearList
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::ClearList()
{
    for (mCurPos = mPeers.begin(); mCurPos != mPeers.end(); ++mCurPos)
    {
        (*mCurPos)->Release();
    }

    mPeers.clear();
    mCurPos = mPeers.end();
}


///////////////////////////////////////////////////////////////////////////////
// GeneratePeers
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::GeneratePeers()
{
    ///TODO -- BIG BIG TODO -- I need to implement some lazy evaluation for these
    // fcos so that when I seek to a start point, I don't stat everything in the
    // same directory as myself.
    // NOTE: This might be alleviated by differentiating between seeks that need
    //      peers and ones that do not (or maybe lazy evaluation of the peers)

    ClearList();

    //
    // read all of the children of the parent...
    //
    std::vector<TSTRING> vChildrenNames;
    GetChildrenNames(mParentName.AsString(), vChildrenNames);

    //
    // insert the children into the set...
    //
    std::vector<TSTRING>::iterator i;
    cFCOName                       curName = mParentName;
    for (i = vChildrenNames.begin(); i != vChildrenNames.end(); ++i)
    {
        curName.Push(*i);

        iFCO* pNewObj = CreateObject(curName, true);
        if (pNewObj)
        {
            if (!InsertIntoPeers(pNewObj))
            {
                // no need for an error msg; that is handled by InsertIntoPeers;
                // just release the object.
                //
                pNewObj->Release();
            }
        }
        curName.Pop();
    }

    mCurPos = mPeers.begin();
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
bool cFCODataSourceIterImpl::SeekTo(const TCHAR* shortName)
{
    FCOList::iterator i = UpperBound(shortName);
    if (i != mPeers.end())
    {
        if (REL_EQ == Compare(shortName, (*i)->GetName().GetShortName()))
        {
            mCurPos = i;
            return true;
        }
    }

    mCurPos = mPeers.end();
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::TraceContents(int dl) const
{
    if (dl == -1)
        dl = cDebug::D_DEBUG;

    cDebug d("cFCODataSourceIterImpl::TraceContents");

    d.Trace(dl, "FCO Iterator; parent = %s\n", mParentName.AsString().c_str());
    d.Trace(dl, "---- Peers ---- (current has a * by it)\n");
    int cnt = 0;
    for (FCOList::const_iterator iter = mPeers.begin(); iter != mPeers.end(); ++iter, ++cnt)
    {
        d.Trace(dl, "[%d]%c\t:%s\n", cnt, iter == mCurPos ? _T('*') : _T(' '), (*iter)->GetName().AsString().c_str());
    }
    d.Trace(dl, "--------------- \n");
}


///////////////////////////////////////////////////////////////////////////////
// InsertIntoPeers
///////////////////////////////////////////////////////////////////////////////
// inserts the fco into peers.
// Sorted in increasing order by short name of the FCO.
bool cFCODataSourceIterImpl::InsertIntoPeers(iFCO* pFCO)
{
    FCOList::iterator i = UpperBound(pFCO->GetName().GetShortName());

    if (i != mPeers.end())
    {
        // never insert two objects that have the same name...
        //
        if (REL_EQ == Compare(pFCO->GetName().GetShortName(), (*i)->GetName().GetShortName()))
        {
            mpErrorBucket->AddError(eFCODataSourceIterDupeFCO(pFCO->GetName().AsString(), eError::NON_FATAL));
            return false;
        }
        else
            mPeers.insert(i, pFCO);
    }
    else
        mPeers.push_back(pFCO);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// SeekToPeerByName
///////////////////////////////////////////////////////////////////////////////
void cFCODataSourceIterImpl::SeekToPeerByName(const TCHAR* pchName)
{
    FCOList::iterator i = UpperBound(pchName);
    if (i != mPeers.end())
    {
        if (REL_EQ == Compare(pchName, (*i)->GetName().GetShortName()))
        {
            mCurPos = i;
            return;
        }
    }

    mCurPos = mPeers.end();
}

///////////////////////////////////////////////////////////////////////////////
// CompareShortName
///////////////////////////////////////////////////////////////////////////////

bool cFCODataSourceIterImpl::CompareForUpperBound(const iFCO* pFCO, const TCHAR* pchName) const
{
    return (Compare(pFCO->GetName().GetShortName(), pchName) == REL_LT);
}

///////////////////////////////////////////////////////////////////////////////
// cFCODataSourceIterImplCallCompare
//                 -- used to sneak a pointer-to-member-function where a
//                    pointer-to-non-member-function is expected
///////////////////////////////////////////////////////////////////////////////

class cFCODataSourceIterImplCallCompare
{
public:
    explicit cFCODataSourceIterImplCallCompare(const cFCODataSourceIterImpl* pcls) : pc(pcls){};

    bool operator()(const iFCO* a1, const TCHAR* a2)
    {
        return pc->CompareForUpperBound(a1, a2);
    }

private:
    const cFCODataSourceIterImpl* pc;
};

///////////////////////////////////////////////////////////////////////////////
// UpperBound
///////////////////////////////////////////////////////////////////////////////

cFCODataSourceIterImpl::FCOList::iterator cFCODataSourceIterImpl::UpperBound(const TCHAR* pchShortName)
{
    cFCODataSourceIterImplCallCompare comp(this);
    return ::UpperBound(mPeers.begin(), mPeers.end(), pchShortName, comp);
}
