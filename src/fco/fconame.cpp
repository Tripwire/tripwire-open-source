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
// fconame.cpp
///////////////////////////////////////////////////////////////////////////////
#include "stdfco.h"
#include "fconame.h"
#include "core/serializer.h"
#include "fconametbl.h"
#include "core/refcountobj.h"
#include "twfactory.h"
#include "core/errorutil.h"
#include "core/ntmbs.h"

#include <vector>

//#############################################################################
// cFCOName_i -- an implementation of a cFCOName -- this object is refrence
//      counted (so copies are cheap) and contains a linked list of cFCOTableNodes
//      as well as the static instance of the cFCONameTbl.
//#############################################################################
class cFCOName_i : public cRefCountObj
{
public:
    cFCOName::ListType mNames;

    ~cFCOName_i()
    {
        ClearList();
    }
    void ClearList();
    // releases all the names in mNames and clears the list

    // the single name table
    static cFCONameTbl msNameTbl;
};

///////////////////////////////////////////////////
// ClearList()
///////////////////////////////////////////////////
inline void cFCOName_i::ClearList()
{
    cFCOName::ListType::iterator i;
    for (i = mNames.begin(); i != mNames.end(); ++i)
        (*i)->Release();
    mNames.clear();
}

cFCONameTbl cFCOName_i::msNameTbl;

//#############################################################################
// cFCOName
//#############################################################################
IMPLEMENT_TYPEDSERIALIZABLE(cFCOName, _T("cFCOName"), 0, 1)

///////////////////////////////////////////////////////////////////////////////
// ClearNameTable -- IMPORTANT!!! This should only be called when you are 100%
//      positive no more operations involving fco names will occur.
///////////////////////////////////////////////////////////////////////////////
void cFCOName::ClearNameTable()
{
    cFCOName_i::msNameTbl.Clear();
}


///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cFCOName::cFCOName(iFCONameInfo* pNI) : iTypedSerializable(), mpPathName(0), mDelimiter('/')
{
    SetNameInfo(pNI);
    mpPathName = new cFCOName_i;
#ifdef DEBUG
    mDebugStrName = AsString();
    cDebug d("cFCOName::cFCOName(iFCONameInfo*)");
    d.TraceNever(_T("constructing %X:%X %s (refcount=%d)\n"),
                 this,
                 mpPathName,
                 mDebugStrName.c_str(),
                 mpPathName->GetRefCount());
#endif
}

cFCOName::cFCOName(const cFCOName& rhs)
    : iTypedSerializable(), mpPathName(rhs.mpPathName), mDelimiter(rhs.mDelimiter), mbCaseSensitive(rhs.mbCaseSensitive)
{
    mpPathName->AddRef();
#ifdef DEBUG
    mDebugStrName = AsString();
    cDebug d("cFCOName::cFCOName(cFCOName&)");
    d.TraceNever(_T("constructing %X:%X %s (refcount=%d)\n"),
                 this,
                 mpPathName,
                 mDebugStrName.c_str(),
                 mpPathName->GetRefCount());
#endif
}

cFCOName::cFCOName(const TSTRING& rhs, iFCONameInfo* pNI) : iTypedSerializable(), mpPathName(0), mDelimiter('/')
{
    SetNameInfo(pNI);
    mpPathName = new cFCOName_i;
    ParseString(rhs.c_str());

#ifdef DEBUG
    mDebugStrName = AsString();
    cDebug d("cFCOName::cFCOName(cFCOName&,iFCONameInfo*)");
    d.TraceNever(_T("constructing %X:%X %s (refcount=%d)\n"),
                 this,
                 mpPathName,
                 mDebugStrName.c_str(),
                 mpPathName->GetRefCount());
#endif
}

cFCOName::cFCOName(const TCHAR* rhs, iFCONameInfo* pNI) : iTypedSerializable(), mpPathName(0), mDelimiter('/')
{
    SetNameInfo(pNI);
    mpPathName = new cFCOName_i;
    ParseString(rhs);

#ifdef DEBUG
    mDebugStrName = AsString();
    cDebug d("cFCOName::cFCOName(cFCOName&,iFCONameInfo*)");
    d.TraceNever(_T("constructing %X:%X %s (refcount=%d)\n"),
                 this,
                 mpPathName,
                 mDebugStrName.c_str(),
                 mpPathName->GetRefCount());
#endif
}

cFCOName::~cFCOName()
{
#ifdef DEBUG
    cDebug d("cFCOName::~cFCOName()");
    d.TraceNever(
        _T("destructing %X:%X %s (refcount=%d)\n"), this, mpPathName, mDebugStrName.c_str(), mpPathName->GetRefCount());
#endif

    mpPathName->Release();
}

void cFCOName::SetNameInfo(iFCONameInfo* pNI)
{
    if (pNI)
    {
        mbCaseSensitive = pNI->IsCaseSensitive();
        mDelimiter      = pNI->GetDelimitingChar();
    }
    else
    {
        mbCaseSensitive = iTWFactory::GetInstance()->GetNameInfo()->IsCaseSensitive();
        mDelimiter      = iTWFactory::GetInstance()->GetNameInfo()->GetDelimitingChar();
    }
#ifdef DEBUG
    if (mpPathName != NULL) // this could be called from the constructor before this is initialized.
        mDebugStrName = AsString();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
cFCOName& cFCOName::operator=(const cFCOName& rhs)
{
    mpPathName->Release();
    // TODO -- I am sure this won't work (const-ness)
    mpPathName = rhs.mpPathName;
    mpPathName->AddRef();
    mDelimiter      = rhs.mDelimiter;
    mbCaseSensitive = rhs.mbCaseSensitive;
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
    return *this;
}

cFCOName& cFCOName::operator=(const TSTRING& rhs)
{
    *this = rhs.c_str();
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
    return *this;
}

cFCOName& cFCOName::operator=(const TCHAR* rhs)
{
    // if I have the only handle on this vector, I can reuse it
    // otherwise, I have to release it.
    if (mpPathName->GetRefCount() != 1)
    {
        mpPathName->Release();
        mpPathName = new cFCOName_i;
    }
    ParseString(rhs);
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
    return *this;
}

void cFCOName::ParseString(const TCHAR* pszin)
{
    ASSERT(mpPathName != 0);
    ASSERT(pszin != 0);

    mpPathName->ClearList();

    const TCHAR* at         = (pszin + 0);
    const TCHAR* begin      = at;
    const TCHAR* end        = at;
    int          components = 0;

    while (*end)
        ++end;

    while (at < end)
    {
        while (*at && !(*at == mDelimiter) && (at < end))
            at++;

        TSTRING name(begin, at);

//NOTE: To be truly standards compliant we ought to turn >2 slashes into 1, not 2.
#if SUPPORTS_DOUBLE_SLASH_PATH
        if (name.length() > 0 || components < 2)
#else
        if (name.length() > 0 || components == 0)
#endif
        {
            cFCONameTblNode* pNode = cFCOName_i::msNameTbl.CreateNode(name);

            mpPathName->mNames.push_back(pNode);
        }

        components++;
        at++;
        begin = at;
        //begin = (at = tss::strinc(at));
    }
}


///////////////////////////////////////////////////////////////////////////////
// AsString
///////////////////////////////////////////////////////////////////////////////
TSTRING cFCOName::AsString() const
{
    TSTRING str;
    str = _T("");
    ASSERT(mpPathName != 0);

    // this kind of stinks, but I have to special case the root dir ("/")
    // if I don't, it appears as an empty string
    // 15 Oct -- I also had to add a yucky hack for c:\ in windows
    //
    // 13 Jan 99 mdb -- I have decided that all fconames that are one item long should be
    // considered "root items" and should thus be displayed with a trailing delimiting character.
    //
    if (mpPathName->mNames.size() == 1)
    {
        str = (*mpPathName->mNames.begin())->GetString();
        str += mDelimiter;
        return str;
    }
    // end ugly root dir hacks ...
    ListType::iterator i = mpPathName->mNames.begin();
    while (i != mpPathName->mNames.end())
    {
        TSTRING current = (*i)->GetString();
        // the loop is constructed in this odd fashion because I don't want a trailing mDelimiter
        str += current;
        ++i;

        if (i != mpPathName->mNames.end() && current != "/")
            str += mDelimiter;
    }

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// GetShortName
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cFCOName::GetShortName() const
{
    ASSERT(!mpPathName->mNames.empty());
    if (mpPathName->mNames.empty())
        return 0;

    return (mpPathName->mNames.back()->GetString());
}

///////////////////////////////////////////////////////////////////////////////
// Clear
///////////////////////////////////////////////////////////////////////////////
void cFCOName::Clear()
{
    //TODO -- I could probably implement this a little cleaner...
    //
    while (!mpPathName->mNames.empty())
    {
        Pop();
    }
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
}


///////////////////////////////////////////////////////////////////////////////
// GetRelationship
///////////////////////////////////////////////////////////////////////////////
cFCOName::Relationship cFCOName::GetRelationship(const cFCOName& rhs) const
{
    ListType::iterator myIter, rhsIter;

    // get the easy equality out of the case first...
    if (mpPathName == rhs.mpPathName)
        return REL_EQUAL;

    // if either name is case sensitive, we will do a case sensitive compare
    bool bCaseSensitive = (IsCaseSensitive() || rhs.IsCaseSensitive());
    bool bEqual;

    for (myIter = mpPathName->mNames.begin(), rhsIter = rhs.mpPathName->mNames.begin();
         (myIter != mpPathName->mNames.end() && rhsIter != rhs.mpPathName->mNames.end());
         ++myIter, ++rhsIter)
    {
        if (bCaseSensitive)
            bEqual = (*myIter == *rhsIter);
        else
            bEqual = ((*myIter)->GetLowercaseNode() == (*rhsIter)->GetLowercaseNode());
        if (!bEqual)
            return REL_UNRELATED;
    }

    // if we got this far, one is above another, or they are equal...
    if ((myIter == mpPathName->mNames.end()) && (rhsIter == rhs.mpPathName->mNames.end()))
        return REL_EQUAL;
    else if (myIter == mpPathName->mNames.end())
        // I am shorter; I am above rhs
        return REL_ABOVE;
    else
        return REL_BELOW;
}


///////////////////////////////////////////////////////////////////////////////
// Read
// TODO -- serialize the hash table and nodes instead of reading and writing
//      as a string
///////////////////////////////////////////////////////////////////////////////
void cFCOName::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("FCO Name Read")));

    TSTRING str;
    pSerializer->ReadString(str);

    int16_t dummy = 0;

    // serialize the delimiter
    pSerializer->ReadInt16(dummy); // delimiter, but it's always '/' anyway in OST.
    mDelimiter = '/';

    // read the case-sensitiveness
    pSerializer->ReadInt16(dummy);
    if (dummy == 0)
        mbCaseSensitive = false;
    else
        mbCaseSensitive = true;

    ParseString(str.c_str());
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Write
// TODO -- serialize the hash table and nodes instead of reading and writing
//      as a string
///////////////////////////////////////////////////////////////////////////////
void cFCOName::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteString(AsString());

    // serialize the delimiter
    unsigned short wc = (unsigned short)'/';
    pSerializer->WriteInt16(wc);
    pSerializer->WriteInt16(mbCaseSensitive ? (int16_t)1 : (int16_t)0);
}

///////////////////////////////////////////////////////////////////////////////
// CopyOnModify -- if the refrence count on mpPathName is > 1, release it and
//      allocate our own copy of it
///////////////////////////////////////////////////////////////////////////////
void cFCOName::CopyOnModify()
{
    if (mpPathName->GetRefCount() > 1)
    {
        cFCOName_i* pOld = mpPathName;
        mpPathName       = new cFCOName_i;
        ListType::iterator i;
        for (i = pOld->mNames.begin(); i != pOld->mNames.end(); ++i)
        {
            (*i)->AddRef();
            mpPathName->mNames.push_back(*i);
        }
        pOld->Release();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Push
///////////////////////////////////////////////////////////////////////////////
void cFCOName::Push(const TSTRING& str)
{
    // we must copy the fconame if there is more than one refrence to it...
    CopyOnModify();

    cFCONameTblNode* pNode = cFCOName_i::msNameTbl.CreateNode(str);
    mpPathName->mNames.push_back(pNode);
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Pop
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cFCOName::Pop()
{
    // we must copy the fconame if there is more than one refrence to it...
    CopyOnModify();

    ASSERT(GetSize() > 0);

    cFCONameTblNode* pNode = mpPathName->mNames.back();
    mpPathName->mNames.pop_back();

    // I do this assertion because it should also be in the hash table
    ASSERT(pNode->GetRefCount() > 1);
    const TCHAR* ret = pNode->GetString();
    pNode->Release();
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// PopFront
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cFCOName::PopFront()
{
    // we must copy the fconame if there is more than one reference to it...
    CopyOnModify();

    ASSERT(GetSize() > 0);

    cFCONameTblNode*             pNode = mpPathName->mNames.front();
    cFCOName::ListType::iterator i     = mpPathName->mNames.begin();
    mpPathName->mNames.erase(i);

    // I do this assertion because it should also be in the hash table
    ASSERT(pNode->GetRefCount() > 1);
    const TCHAR* ret = pNode->GetString();
    pNode->Release();
#ifdef DEBUG
    mDebugStrName = AsString();
#endif
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// GetSize
///////////////////////////////////////////////////////////////////////////////
int cFCOName::GetSize() const
{
    return mpPathName->mNames.size();
}

///////////////////////////////////////////////////////////////////////////////
// operator< -- provides an arbitrary ordering to cFCONames. The algorithm I chose
//      is like strcmp, except instead of comparing characters, I compare
//      cFCONameTblNode* addresses
///////////////////////////////////////////////////////////////////////////////
bool cFCOName::operator<(const cFCOName& rhs) const
{
    ListType::iterator myIter, rhsIter;

    // if either name is case sensitive, we will do a case sensitive compare
    bool bCaseSensitive = (IsCaseSensitive() || rhs.IsCaseSensitive());

    // get the easy equality out of the case first...
    if (mpPathName == rhs.mpPathName)
        return false;

    for (myIter = mpPathName->mNames.begin(), rhsIter = rhs.mpPathName->mNames.begin();
         (myIter != mpPathName->mNames.end() && rhsIter != rhs.mpPathName->mNames.end());
         ++myIter, ++rhsIter)
    {
        if (bCaseSensitive)
        {
            if (*myIter > *rhsIter)
                return false;
            else if (*myIter < *rhsIter)
                return true;
        }
        else
        {
            // not case sensitive
            if ((*myIter)->GetLowercaseNode() > (*rhsIter)->GetLowercaseNode())
                return false;
            else if ((*myIter)->GetLowercaseNode() < (*rhsIter)->GetLowercaseNode())
                return true;
        }
        // if they are equal, keep going
    }

    // if we got this far, one is above another, or they are equal...
    if (rhsIter == rhs.mpPathName->mNames.end())
        // either I am longer of we are equal; so return false
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// cFCONameIter
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// cFCONameIter
///////////////////////////////////////////////////////////////////////////////
cFCONameIter::cFCONameIter(const cFCOName& name) : mName(name)
{
    SeekBegin();
}

///////////////////////////////////////////////////////////////////////////////
// ~cFCONameIter
///////////////////////////////////////////////////////////////////////////////
cFCONameIter::~cFCONameIter()
{
}

///////////////////////////////////////////////////////////////////////////////
// GetSize
///////////////////////////////////////////////////////////////////////////////
int cFCONameIter::GetSize() const
{
    return mName.mpPathName->mNames.size();
}

///////////////////////////////////////////////////////////////////////////////
// SeekBegin
///////////////////////////////////////////////////////////////////////////////
void cFCONameIter::SeekBegin()
{
    mIter = mName.mpPathName->mNames.begin();
}

///////////////////////////////////////////////////////////////////////////////
// Next
///////////////////////////////////////////////////////////////////////////////
void cFCONameIter::Next()
{
    ++mIter;
}

///////////////////////////////////////////////////////////////////////////////
// Done
///////////////////////////////////////////////////////////////////////////////
bool cFCONameIter::Done() const
{
    return (mIter == mName.mpPathName->mNames.end());
}

///////////////////////////////////////////////////////////////////////////////
// GetName
///////////////////////////////////////////////////////////////////////////////
const TCHAR* cFCONameIter::GetName() const
{
    ASSERT(!Done());

    return (*mIter)->GetString();
}

///////////////////////////////////////////////////////////////////////////////
// Prev
///////////////////////////////////////////////////////////////////////////////
void cFCONameIter::Prev()
{
    --mIter;
}

///////////////////////////////////////////////////////////////////////////////
// Index
///////////////////////////////////////////////////////////////////////////////
int cFCONameIter::Index() const
{
    ASSERT(!Done());
    return (mIter - mName.mpPathName->mNames.begin());
}

///////////////////////////////////////////////////////////////////////////////
// SeekTo
///////////////////////////////////////////////////////////////////////////////
void cFCONameIter::SeekTo(int index)
{
    ASSERT((index >= 0) && (index < mName.GetSize()));

    mIter = (mName.mpPathName->mNames.begin() + index);
}
