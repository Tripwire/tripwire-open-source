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
// dbdatasource.cpp
//

#include "stdtw.h"
#include "dbdatasource.h"
#include "fco/twfactory.h"
#include "fco/fconameinfo.h"
#include "core/archive.h"
#include "core/serializerimpl.h"
#include "fco/fcopropset.h"
#include "fco/fco.h"
#include "fco/fcodatasourceiter.h"

// TODO -- get rid of this include somehow!
#include "fco/genreswitcher.h"

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cDbDataSourceIter::cDbDataSourceIter(cHierDatabase* pDb, int genreNum)
    : iFCODataSourceIter(), mDbIter(pDb), mFlags(0), mpErrorBucket(0)
{
    //
    // remember the fco creation function...
    // TODO -- Note that this couples this file with cGenreSwitcher; perhaps this should take the fco creation
    //      function instead...
    //
    if (genreNum == -1)
        genreNum = cGenreSwitcher::GetInstance()->CurrentGenre();
    mFCOCreateFunc = cGenreSwitcher::GetInstance()->GetFactoryForGenre((cGenre::Genre)genreNum)->GetCreateFunc();

#ifdef DEBUG
    //
    // make some assertions about the current genre's name info
    //
    iFCONameInfo* pNameInfo = cGenreSwitcher::GetInstance()->GetFactoryForGenre((cGenre::Genre)genreNum)->GetNameInfo();
    ASSERT(pDb->IsCaseSensitive() == pNameInfo->IsCaseSensitive());
    ASSERT(pDb->GetDelimitingChar() == pNameInfo->GetDelimitingChar());
#endif //#ifdef DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// cDbDataSourceIter
///////////////////////////////////////////////////////////////////////////////
cDbDataSourceIter::cDbDataSourceIter(const cDbDataSourceIter& rhs)
    : iFCODataSourceIter(),
      mDbIter(rhs.mDbIter),
      mFCOCreateFunc(rhs.mFCOCreateFunc),
      mFlags(rhs.mFlags),
      mpErrorBucket(rhs.mpErrorBucket)
{
}

///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
cDbDataSourceIter& cDbDataSourceIter::operator=(const cDbDataSourceIter& rhs)
{
    mDbIter        = rhs.mDbIter;
    mFCOCreateFunc = rhs.mFCOCreateFunc;
    mFlags         = rhs.mFlags;
    mpErrorBucket  = rhs.mpErrorBucket;
    return (*this);
}


///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cDbDataSourceIter::~cDbDataSourceIter()
{
}

///////////////////////////////////////////////////////////////////////////////
// CreateCopy
///////////////////////////////////////////////////////////////////////////////
iFCODataSourceIter* cDbDataSourceIter::CreateCopy() const
{
    //TODO -- implement this with a memory pool?
    return new cDbDataSourceIter(*this);
}


///////////////////////////////////////////////////////////////////////////////
// GetName
///////////////////////////////////////////////////////////////////////////////
cFCOName cDbDataSourceIter::GetName() const
{
    ASSERT(!Done());

    cFCOName rtn(mDbIter.GetCwd());
    rtn.Push(mDbIter.GetName());
    return rtn;
}


///////////////////////////////////////////////////////////////////////////////
// CreateFCO
//
// note that cHierDatabase::GetData() will throw if we are Done() or the node
//      doesn't have data
///////////////////////////////////////////////////////////////////////////////
iFCO* cDbDataSourceIter::CreateFCO() //throw (eError)
{
    ASSERT(!Done());
    //
    // ok, first, we will create the new fco...
    //
    iFCO* pFCO = static_cast<iFCO*>((*mFCOCreateFunc)());
    pFCO->SetName(GetName());
    try
    {
        ASSERT(mDbIter.HasData());
        int32_t length;
        int8_t* pData = mDbIter.GetData(length);
        //
        // associate a serializer with this memory and read in the property set...
        //
        cFixedMemArchive arch(pData, length);
        cSerializerImpl  ser(arch, cSerializerImpl::S_READ);
        ser.Init();
        ser.ReadObject(pFCO->GetPropSet());
        ser.Finit();
    }
    catch (...)
    {
        pFCO->Release();
        throw;
    }

    return pFCO;
}

///////////////////////////////////////////////////////////////////////////////
// HasFCOData
///////////////////////////////////////////////////////////////////////////////
bool cDbDataSourceIter::HasFCOData() const
{
    ASSERT(!Done());
    if (Done())
        return false;

    return (mDbIter.HasData());
}


///////////////////////////////////////////////////////////////////////////////
// SeekToFCO
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::SeekToFCO(const cFCOName& name, bool bCreatePeers) //throw(eFCODataSourceIter)
{
    ASSERT(name.GetSize() > 0);
    cFCOName parentName = name;
    parentName.Pop();
    if (!SeekToDirectory(parentName, false))
    {
        // make myself Done() and return...
        //
        while (!Done())
            Next();
        return;
    }
    //
    // note that this is Done() if it fails, so we are good...
    //
    mDbIter.SeekTo(name.GetShortName());
}

///////////////////////////////////////////////////////////////////////////////
// AddFCO
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::AddFCO(const TSTRING& shortName, const iFCO* pFCO) //throw (eFCODataSourceIter, eError)
{
    mDbIter.CreateEntry(shortName);
    //
    // we are now pointing at the entry we just created, so now let's add the data...
    //
    SetFCOData(pFCO);
}

///////////////////////////////////////////////////////////////////////////////
// RemoveFCO
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::RemoveFCO() //throw (eError)
{
    if ((!Done()) && (!HasFCOData()))
    {
        mDbIter.DeleteEntry();
    }
    else
    {
        // this was called in inappropriate circumastances
        ASSERT(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
// RemoveFCOData
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::RemoveFCOData() //throw (eError)
{
    ASSERT(!Done());
    ASSERT(HasFCOData());
    if (HasFCOData())
    {
        mDbIter.RemoveData();
    }
}

///////////////////////////////////////////////////////////////////////////////
// SetFCOData
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::SetFCOData(const iFCO* pFCO) //throw (eError)
{
    ASSERT(!Done());
    if (Done())
    {
        throw eHierDatabase(_T("Attempt to set FCO data when the iterator is done."));
    }
    // TODO -- assert and throw if the fco's type is not the same as our creation function
    //      There is no way to do this through the serializable interface, but when there is,
    //      we should do the above assertion.
    //
    // if data already exists here, we first remove it;
    //
    if (mDbIter.HasData())
    {
        mDbIter.RemoveData();
    }

    if (pFCO)
    {
        //
        // write the fco's property set to a memory archive...
        //
        // TODO -- does this need to be static?
        static cMemoryArchive arch;
        arch.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl ser(arch, cSerializerImpl::S_WRITE);
        ser.Init();
        ser.WriteObject(pFCO->GetPropSet());
        ser.Finit();
        //
        // write this to the archive...
        //
        mDbIter.SetData(arch.GetMemory(), arch.CurrentPos());
    }
}

///////////////////////////////////////////////////////////////////////////////
// AddChildArray
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::AddChildArray() //throw (eError)
{
    ASSERT(!Done());
    ASSERT(!CanDescend());

    mDbIter.CreateChildArray();
}

///////////////////////////////////////////////////////////////////////////////
// RemoveChildArray
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::RemoveChildArray() //throw (eError)
{
    //NOTE -- the hier db iter does all of the proper asserting...
    //
    mDbIter.DeleteChildArray();
}

///////////////////////////////////////////////////////////////////////////////
// CreatePath
///////////////////////////////////////////////////////////////////////////////
void cDbDataSourceIter::CreatePath(const cFCOName& name) //throw (eError)
{
    ASSERT(name.GetSize() > 0);

    cFCOName parentName = name;
    parentName.Pop();
    SeekToDirectory(parentName, true);
    if (!mDbIter.SeekTo(name.GetShortName()))
    {
        mDbIter.CreateEntry(name.GetShortName());
    }
}


///////////////////////////////////////////////////////////////////////////////
// SeekToDirectory
///////////////////////////////////////////////////////////////////////////////
bool cDbDataSourceIter::SeekToDirectory(const cFCOName& parentName, bool bCreate)
{
    cDebug d("cDbDataSourceIter::SeekToDirectory");
    //
    // the first task is to ascend until we are in a directory that we can descend into to
    // reach parentName...
    //
    cFCOName curParent = GetParentName();
    d.TraceDebug(
        _T("Entering... Seeking to %s (cwd = %s)\n"), parentName.AsString().c_str(), curParent.AsString().c_str());
    int ascendCount;
    switch (curParent.GetRelationship(parentName))
    {
    case cFCOName::REL_BELOW:
        //
        // we must ascend...
        //
        ascendCount = curParent.GetSize() - parentName.GetSize();
        d.TraceDetail(_T("\tAscending %d times...\n"), ascendCount);
        ASSERT(ascendCount > 0);
        for (; ascendCount > 0; ascendCount--)
            Ascend();
        break;
    case cFCOName::REL_ABOVE:
        //
        // we are above the needed directory; nothing else to do here...
        //
        d.TraceDetail(_T("\tAbove; not ascending...\n"));
        break;
    case cFCOName::REL_EQUAL:
        //
        // we need to do nothing else here...
        //
        d.TraceDetail(_T("\tEqual; doing nothing...\n"));
        SeekBegin();
        return true;
    case cFCOName::REL_UNRELATED:
        //
        // we have to go all the way to the root...
        //
        d.TraceDetail(_T("\tUnrelated; seeking to root...\n"));
        SeekToRoot();
        break;
    default:
        // unreachable
        ASSERT(false);
        return false;
    }

    curParent = GetParentName();
    if (parentName.GetSize() == curParent.GetSize())
        return true;
    //
    // now we will descend to the parent directory we are interested in...
    //
    cFCOName::iterator i(parentName);
    i.SeekTo(curParent.GetSize());
    for (; (!i.Done()); i.Next())
    {
        if (!mDbIter.SeekTo(i.GetName()))
        {
            // this needs to be created!
            if (bCreate)
                mDbIter.CreateEntry(i.GetName());
            else
                return false;
        }
        //
        // create the child array and descend
        //
        if (!mDbIter.CanDescend())
        {
            if (bCreate)
                mDbIter.CreateChildArray();
            else
                return false;
        }
        mDbIter.Descend();
    }
    return true;
}
