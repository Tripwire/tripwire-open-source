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
// hierdatabase.cpp

#include "stddb.h"

#include "hierdatabase.h"
#include "hierdbnode.h"
#include "core/archive.h"
#include "core/upperbound.h"
#include "core/errorbucket.h"
#include "core/errorbucketimpl.h"

// TODO -- all of these util_ functions should throw an eArchive if an attempt is made to
//      write to a null address

static inline void util_ThrowIfNull(const cHierAddr& addr, const TSTRING& context)
{
    ASSERT(!addr.IsNull());
    if (addr.IsNull())
    {
        TSTRING msg(_T("Attempt to access null address"));
        if (!context.empty())
        {
            msg += _T(" in ") + context;
        }
        throw eHierDatabase(msg);
    }
}

///////////////////////////////////////////////////////////////////////////////
// util_ReadObject -- this will read the given object from the database from the
//      given address. eArchive is thrown on exception.
///////////////////////////////////////////////////////////////////////////////
static void util_ReadObject(cHierDatabase* pDb, cHierNode* pNode, const cHierAddr& addr)
{
    int32_t dataSize;
    int8_t* pData = pDb->GetDataForReading(cBlockRecordFile::tAddr(addr.mBlockNum, addr.mIndex), dataSize);
    //
    // make sure we aren't trying to read a null object
    //
    util_ThrowIfNull(addr, _T("util_ReadObject"));

    cFixedMemArchive arch(pData, dataSize);
    pNode->Read(arch);
}

///////////////////////////////////////////////////////////////////////////////
// util_WriteObject -- this will write the given object to the database, returning
//      the address that it was written to, and throwing eArchive on error.
///////////////////////////////////////////////////////////////////////////////
static cHierAddr util_WriteObject(cHierDatabase* pDb, cHierNode* pNode)
{
    static cMemoryArchive arch;
    arch.Seek(0, cBidirArchive::BEGINNING);
    pNode->Write(arch);

    cBlockRecordFile::tAddr addr = pDb->AddItem(arch.GetMemory(), arch.CurrentPos());
    return cHierAddr(addr.mBlockNum, addr.mIndex);
}

///////////////////////////////////////////////////////////////////////////////
// util_RewriteObject -- call this when the data in a structure has changed, but
//      its size has not; this will update the database without changing the
//      object's address.
///////////////////////////////////////////////////////////////////////////////
static void util_RewriteObject(cHierDatabase* pDb, cHierNode* pNode, const cHierAddr& addr)
{
    int32_t dataSize;
    int8_t* pData = pDb->GetDataForWriting(cBlockRecordFile::tAddr(addr.mBlockNum, addr.mIndex), dataSize);

    util_ThrowIfNull(addr, _T("util_RewriteObject"));

    cFixedMemArchive arch(pData, dataSize);
    pNode->Write(arch);
}


///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cHierDatabase::cHierDatabase(bool bCaseSensitive, TCHAR delChar)
    : mRootArrayAddr(), mbCaseSensitive(bCaseSensitive), mDelimitingChar(delChar)
{
}

///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cHierDatabase::~cHierDatabase()
{
}

///////////////////////////////////////////////////////////////////////////////
// Open
///////////////////////////////////////////////////////////////////////////////
void cHierDatabase::Open(cBidirArchive* pArch, int numPages) //throw (eArchive, eHierDatabase)
{
    bool bTruncate = (pArch->Length() == 0);
    inherited::Open(pArch, numPages);

    OpenImpl(bTruncate);
}


void cHierDatabase::Open(const TSTRING& fileName, int numPages, bool bTruncate) //throw (eArchive, eHierDatabase)
{
    inherited::Open(fileName, numPages, bTruncate);

#ifdef _BLOCKFILE_DEBUG
    AssertValid();
#endif

    OpenImpl(bTruncate);
}

///////////////////////////////////////////////////////////////////////////////
// OpenImpl
///////////////////////////////////////////////////////////////////////////////
void cHierDatabase::OpenImpl(bool bTruncate) //throw (eArchive, eHierDatabase)
{
    // now, we should write the root node at (0,0) if we are creating, and assert that the root node
    // is there if we are opening an existing one...
    //
    if (bTruncate)
    {
        //
        // make the root node that we are creating...
        //
        cHierRoot rootNode;
        rootNode.mChild          = cHierAddr(0, 1); // the first array will be placed at (0,1)
        rootNode.mbCaseSensitive = mbCaseSensitive;
        rootNode.mDelimitingChar = mDelimitingChar;
        cHierAddr addr           = util_WriteObject(this, &rootNode);
        //
        // assert that it was written to (0, 0)
        //
        ASSERT((addr.mBlockNum == 0) && (addr.mIndex == 0));
        //
        // make an empty array to serve as the root array.
        //
        cHierArrayInfo arrayInfo;
        arrayInfo.mParent = cHierAddr();
        arrayInfo.mArray  = cHierAddr();
        //
        // actually write the objects and assert that they were written to the right places.
        //
        addr = util_WriteObject(this, &arrayInfo);
        ASSERT((addr.mBlockNum == 0) && (addr.mIndex == 1));

        mRootArrayAddr = cHierAddr(0, 1);
    }
    else
    {
        cHierRoot rootNode;
        util_ReadObject(this, &rootNode, cHierAddr(0, 0));
        //
        // set the location of the top-level array and the other root parameters...
        //
        mRootArrayAddr  = rootNode.mChild;
        mbCaseSensitive = rootNode.mbCaseSensitive;
        mDelimitingChar = rootNode.mDelimitingChar;
    }
}

//-----------------------------------------------------------------------------
// cHierDatabaseIter
//-----------------------------------------------------------------------------

cHierDatabaseIter::cHierDatabaseIter(cHierDatabase* pDb) //throw(eArchive)
    : mpDb(pDb), mCurPath()
{
    ASSERT(pDb != 0);
    mCurPath.SetCaseSensitive(mpDb->IsCaseSensitive());
    mCurPath.SetDelimiter(mpDb->GetDelimitingChar());
    //
    // load in the root array...
    //
    SeekToRoot();
}

cHierDatabaseIter::~cHierDatabaseIter()
{
}

///////////////////////////////////////////////////////////////////////////////
// SeekToRoot
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::SeekToRoot() //throw (eArchive)
{
    ASSERT(mpDb != 0);
    ASSERT(!mpDb->mRootArrayAddr.IsNull());
    mCurPath.Clear();
    LoadArrayAt(mpDb->mRootArrayAddr);
}

///////////////////////////////////////////////////////////////////////////////
// copy ctor
///////////////////////////////////////////////////////////////////////////////
cHierDatabaseIter::cHierDatabaseIter(const cHierDatabaseIter& rhs)
{
    *this = rhs;
}

///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
cHierDatabaseIter& cHierDatabaseIter::operator=(const cHierDatabaseIter& rhs)
{
    mpDb      = rhs.mpDb;
    mEntries  = rhs.mEntries;
    mInfo     = rhs.mInfo;
    mInfoAddr = rhs.mInfoAddr;
    mCurPath  = rhs.mCurPath;
    //
    // the iterator is a little trickier
    //
    if (rhs.Done())
    {
        mIter = mEntries.end();
    }
    else
    {
        EntryArray::const_iterator p1, p2;
        p1    = rhs.mIter;
        p2    = rhs.mEntries.begin();
        mIter = mEntries.begin() + (p1 - p2);
    }

    return (*this);
}


///////////////////////////////////////////////////////////////////////////////
// LoadArrayAt
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::LoadArrayAt(const cHierAddr& addr) //throw (eArchive, eHierDatabase)
{
    ASSERT(mpDb != 0);
    //
    // make sure the address is non-null
    //
    util_ThrowIfNull(addr, _T("cHierDatabaseIter::LoadArrayAt"));
    //
    // load the info and the array
    //
    mInfoAddr = addr;
    util_ReadObject(mpDb, &mInfo, mInfoAddr);
    cHierAddr curAddr = mInfo.mArray;
    //
    // load in all the array entries...
    //
    mEntries.clear();
    while (!curAddr.IsNull())
    {
        mEntries.push_back(cHierEntry());
        //Catch db errors, in case only part of db is broken
        try
        {
            util_ReadObject(mpDb, &mEntries.back(), curAddr);
        }
        catch (eError& e)
        {
            e.SetFatality(false);
            cErrorReporter::PrintErrorMsg(e);
            mEntries.pop_back();
            break;
        }

        curAddr = mEntries.back().mNext;
    }

    //
    // seek to the beginning of the array
    //
    SeekBegin();
}


///////////////////////////////////////////////////////////////////////////////
// AtRoot
///////////////////////////////////////////////////////////////////////////////
bool cHierDatabaseIter::AtRoot() const //throw (eHierDatabase)
{
    bool bResult = (mInfoAddr == mpDb->mRootArrayAddr);

    // TODO -- maybe I want to put this in a #ifdef?
    //
    if (bResult)
    {
        // we are at the root; assert that we have no parent...
        //
        ASSERT(mInfo.mParent.IsNull());
        if (!mInfo.mParent.IsNull())
        {
            throw eHierDatabase(_T("Root node of db has a non-null parent"));
        }
    }
    else
    {
        // if we are not at the root, assert that we have a parent...
        ASSERT(!mInfo.mParent.IsNull());
        if (mInfo.mParent.IsNull())
        {
            throw eHierDatabase(_T("Non-root node of db has a null parent!"));
        }
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// CanDescend
///////////////////////////////////////////////////////////////////////////////
bool cHierDatabaseIter::CanDescend() const
{
    if (Done())
        return false;

    return (!mIter->mChild.IsNull());
}

///////////////////////////////////////////////////////////////////////////////
// Descend
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::Descend()
{
    ASSERT(CanDescend());
    //
    // alter the cwd...
    //
    mCurPath.Push(mIter->mName);

    LoadArrayAt(mIter->mChild);
}

///////////////////////////////////////////////////////////////////////////////
// Ascend
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::Ascend()
{
    ASSERT(!AtRoot());
    ASSERT(mCurPath.GetSize() > 0);
    //
    // alter the cwd...
    //
    mCurPath.Pop();

    LoadArrayAt(mInfo.mParent);
}

///////////////////////////////////////////////////////////////////////////////
// SeekBegin
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::SeekBegin()
{
    mIter = mEntries.begin();
}

///////////////////////////////////////////////////////////////////////////////
// Done
///////////////////////////////////////////////////////////////////////////////
bool cHierDatabaseIter::Done() const
{
    return (mIter == mEntries.end());
}

///////////////////////////////////////////////////////////////////////////////
// Next
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::Next()
{
    ++mIter;
}

///////////////////////////////////////////////////////////////////////////////
// SeekTo
///////////////////////////////////////////////////////////////////////////////
bool cHierDatabaseIter::SeekTo(const TCHAR* pName)
{
    EntryArray::iterator i = UpperBound(pName);
    if (i != mEntries.end())
    {
        if (0 == Compare(pName, (*i).mName.c_str()))
        {
            mIter = i;
            return true;
        }
    }

    mIter = mEntries.end();
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// GetName
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cHierDatabaseIter::GetName() const
{
    ASSERT(!Done());
    if (Done())
    {
        throw eHierDatabase(_T("Attempt to call iter::GetName() when done is true"));
    }

    return mIter->mName.c_str();
}

///////////////////////////////////////////////////////////////////////////////
// GetCwd
///////////////////////////////////////////////////////////////////////////////
const TSTRING cHierDatabaseIter::GetCwd() const
{
    return mCurPath.AsString();
}

///////////////////////////////////////////////////////////////////////////////
// GetCurrentAddr
///////////////////////////////////////////////////////////////////////////////
cHierAddr cHierDatabaseIter::GetCurrentAddr() const
{
    ASSERT(!Done());
    if (Done())
    {
        return cHierAddr();
    }

    if (mIter == mEntries.begin())
    {
        return mInfo.mArray;
    }

    return (mIter - 1)->mNext;
}

///////////////////////////////////////////////////////////////////////////////
// CreateChildArray
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::CreateChildArray() //throw (eArchive, eHierDatabase)
{
    ASSERT(!Done());
    ASSERT(!CanDescend());
    if (Done())
    {
        throw eHierDatabase(_T("Attempt to call iter::CreateChildArray() when done is true"));
    }
    if (CanDescend())
    {
        throw eHierDatabase(_T("Attempt to call iter::CreateChildArray() when child already exists"));
    }

    cHierArrayInfo newInfo;
    cHierAddr      infoAddr;

    // write the new info
    newInfo.mParent = mInfoAddr;
    infoAddr        = util_WriteObject(mpDb, &newInfo);
    mIter->mChild   = infoAddr;
    //
    // rewrite the current object, since its child info just changed
    //
    util_RewriteObject(mpDb, &(*mIter), GetCurrentAddr());
}

///////////////////////////////////////////////////////////////////////////////
// CreateEntry
//
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::CreateEntry(const TSTRING& name) //throw (eArchive, eHierDatabase)
{
    cHierEntry newEntry;
    newEntry.mName = name;

    //
    // insert this in order in the set...
    //
    mIter = UpperBound(newEntry.mName.c_str());

    //
    // make sure that no duplicate names are added to the database...
    //
    if (mIter != mEntries.end())
    {
        if (0 == Compare(mIter->mName.c_str(), newEntry.mName.c_str()))
        {
            return;
            //throw eHierDbDupeName( name );
        }
    }

    // Note -- insert() inserts directly _before_ the iterator
    // we need to get the current address so we can set the new object's
    //      next pointer appropriately
    //
    cHierAddr nextAddr;
    if (!Done())
    {
        nextAddr = GetCurrentAddr();
    }
    mIter = mEntries.insert(mIter, newEntry);
    //
    // first, we should write the new object to the database
    //
    mIter->mNext      = nextAddr;
    cHierAddr newAddr = util_WriteObject(mpDb, &(*mIter));
    //
    // set the previous object's next pointer, and rewrite that object to disk...
    //
    if (mIter == mEntries.begin())
    {
        // we need to rewrite the array info...
        //
        mInfo.mArray = newAddr;
        util_RewriteObject(mpDb, &mInfo, mInfoAddr);
    }
    else
    {
        // altering the previous node...
        //
        --mIter;
        mIter->mNext = newAddr;
        util_RewriteObject(mpDb, &(*mIter), GetCurrentAddr());
        ++mIter;
    }
}

///////////////////////////////////////////////////////////////////////////////
// GetData
///////////////////////////////////////////////////////////////////////////////
int8_t* cHierDatabaseIter::GetData(int32_t& length) const //throw (eArchive, eHierDatabase)
{
    ASSERT(HasData());
    if (!HasData())
    {
        throw eHierDatabase(_T("Attempt to get data from a node when HasData() is false"));
    }

    //
    // note that we can only get data for reading; perhaps in the future I will add
    // support for retrieving data for writing as well.
    //
    return mpDb->GetDataForReading(cBlockRecordFile::tAddr(mIter->mData.mBlockNum, mIter->mData.mIndex), length);
}

///////////////////////////////////////////////////////////////////////////////
// HasData
///////////////////////////////////////////////////////////////////////////////
bool cHierDatabaseIter::HasData() const
{
    ASSERT(!Done());
    if (Done())
        return false;

    return (!mIter->mData.IsNull());
}

///////////////////////////////////////////////////////////////////////////////
// SetData
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::SetData(int8_t* pData, int32_t length) //throw (eArchive, eHierDatabase)
{
    ASSERT(!Done());
    ASSERT(!HasData());

    // remove the item's data, if there is any...
    //
    if (HasData())
    {
        RemoveData();
    }
    //
    // add the data and set the entry's data pointer appropriately
    //
    cBlockRecordFile::tAddr addr = mpDb->AddItem(pData, length);
    mIter->mData                 = cHierAddr(addr.mBlockNum, addr.mIndex);
    //
    // update the entry on disk...
    //
    util_RewriteObject(mpDb, &(*mIter), GetCurrentAddr());
}

///////////////////////////////////////////////////////////////////////////////
// RemoveData
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::RemoveData() //throw (eArchive, eHierDatabase)
{
    ASSERT(!Done());
    if (!HasData())
    {
        ASSERT(false);
        return;
    }

    mpDb->RemoveItem(cBlockRecordFile::tAddr(mIter->mData.mBlockNum, mIter->mData.mIndex));
    //
    // now, we need to update the node's data pointer and save the node to disk
    //
    mIter->mData = cHierAddr();
    util_RewriteObject(mpDb, &(*mIter), GetCurrentAddr());
}


///////////////////////////////////////////////////////////////////////////////
// DeleteEntry
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::DeleteEntry() //throw (eArchive, eHierDatabase)
{
    ASSERT(!Done());
    ASSERT(!CanDescend()); // this node can't have any children.
    cDebug d("cHierDatabaseIter::DeleteEntry");

    if (Done())
    {
        d.TraceDetail("Attempt to DeleteEntry() when Done() == true; returning\n");
        return;
    }
    if (CanDescend())
    {
        throw eHierDatabase(_T("Attempt to delete an entry that still has children.\n"));
    }

    //
    // first, we should set the previous node's next pointer...
    //
    cHierAddr curAddr = GetCurrentAddr();
    if (mIter == mEntries.begin())
    {
        // we are changing the info's mArray pointer
        //
        mInfo.mArray = mIter->mNext;
        util_RewriteObject(mpDb, &mInfo, mInfoAddr);
    }
    else
    {
        // altering the previous node...
        //
        --mIter;
        mIter->mNext = (mIter + 1)->mNext;
        util_RewriteObject(mpDb, &(*mIter), GetCurrentAddr());
        ++mIter;
    }
    //
    // now, delete the node from the file and from our array...
    //
    mpDb->RemoveItem(cBlockRecordFile::tAddr(curAddr.mBlockNum, curAddr.mIndex));
    mIter = mEntries.erase(mIter);
}

///////////////////////////////////////////////////////////////////////////////
// CanDeleteChildArray
///////////////////////////////////////////////////////////////////////////////
bool cHierDatabaseIter::ChildArrayEmpty()
{
    ASSERT(!Done());
    ASSERT(CanDescend());

    cHierArrayInfo info;
    util_ReadObject(mpDb, &info, mIter->mChild);
    return (info.mArray.IsNull());
}

///////////////////////////////////////////////////////////////////////////////
// DeleteChildArray
///////////////////////////////////////////////////////////////////////////////
void cHierDatabaseIter::DeleteChildArray() //throw (eArchive, eHierDatabase)
{
    ASSERT(!Done());
    ASSERT(CanDescend());
    cDebug d("cHierDatabaseIter::DeleteChildArray");

    if (Done())
    {
        d.TraceDetail("Attempt to DeleteChildArray() when Done() == true; returning\n");
        return;
    }
    if (!CanDescend())
    {
        d.TraceDetail("Attempt to DeleteChildArray() when none exists; returning\n");
        return;
    }
    //
    // make sure that the child index is empty...
    //
    if (!ChildArrayEmpty())
    {
        ASSERT(false);
        throw eHierDatabase(_T("Attempt to delete a child array that still has entries"));
    }
    //
    // ok, no we can remove it...
    //
    mpDb->RemoveItem(cBlockRecordFile::tAddr(mIter->mChild.mBlockNum, mIter->mChild.mIndex));
    mIter->mChild = cHierAddr();
    util_RewriteObject(mpDb, &(*mIter), GetCurrentAddr());
}


///////////////////////////////////////////////////////////////////////////////
// CompareForUpperBound
///////////////////////////////////////////////////////////////////////////////

bool cHierDatabaseIter::CompareForUpperBound(const cHierEntry& he, const TCHAR* pchName) const
{
    return (0 > Compare(he.mName.c_str(), pchName));
}

///////////////////////////////////////////////////////////////////////////////
// cHierDatabaseIterCallCompare
//                 -- used to sneak a pointer-to-member-function where a
//                    pointer-to-non-member-function is expected
///////////////////////////////////////////////////////////////////////////////
class cHierDatabaseIterCallCompare
{
public:
    explicit cHierDatabaseIterCallCompare(const cHierDatabaseIter* pcls) : pc(pcls){};

    bool operator()(const cHierEntry& a1, const TCHAR* a2)
    {
        return pc->CompareForUpperBound(a1, a2);
    }

private:
    const cHierDatabaseIter* pc;
};

///////////////////////////////////////////////////////////////////////////////
// UpperBound
///////////////////////////////////////////////////////////////////////////////

cHierDatabaseIter::EntryArray::iterator cHierDatabaseIter::UpperBound(const TCHAR* pchName)
{
    cHierDatabaseIterCallCompare comp(this);
    return ::UpperBound(mEntries.begin(), mEntries.end(), pchName, comp);
}
