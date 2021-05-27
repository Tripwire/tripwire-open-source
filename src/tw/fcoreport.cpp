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
// fcoreport.h
///////////////////////////////////////////////////////////////////////////////

#include "stdtw.h"
#include "fcoreport.h"
#include "fcoreportutil.h"
#include "fco/fcospeclist.h"
#include "fco/fcosetimpl.h"
#include "fco/fcosetws.h"
#include "fco/fcospecutil.h"
#include "fco/fcopropvector.h"
#include "core/serializer.h"
#include "core/debug.h"
#include "core/errorbucketimpl.h"
#include "fco/iterproxy.h"
#include "core/hashtable.h"
#include "core/fsservices.h"
#include "fco/fcospecattr.h"
#include "core/fileheader.h"
#include "headerinfo.h"

#include <time.h>

//=============================================================================
// class cFCOReport_i
//=============================================================================

class cFCOReport_i
{
public:
    cFCOReport_i();
    ~cFCOReport_i();

    // note that all iFCOs and all iFCOSpecs in this structure are AddRef()ed when added
    // and will be Release()d when deleted.
    struct cChangeNode
    {
        const iFCO*    mpOldFCO;
        const iFCO*    mpNewFCO;
        cFCOPropVector mChangeVector;

        cChangeNode();
        cChangeNode(const cChangeNode& rhs);
        ~cChangeNode();
        void Clear();
    };

    struct cNode
    {
        iFCOSpec*              mpSpec;     // _must_ be valid (non-0)
        const cFCOSpecAttr*    mpSpecAttr; // _must_ be valid (non-0)
        cFCOSetWS              mAdded;     // these two sets make sure their fcos are in mpSpec.
        cFCOSetWS              mRemoved;
        std::list<cChangeNode> mChanged;
        cErrorQueue            mErrorQueue;
        int32_t               mnObjectsScanned;

        cNode();
        cNode(const cNode& rhs);
        ~cNode();
        void Clear();
    };


    typedef std::list<cNode> SpecList;

    struct cGenreNode
    {
        cGenreNode(){};
        cGenreNode(const cGenreNode& rhs);
        cFCOReportGenreHeader mGenreHeader;
        cGenre::Genre         mGenre;
        SpecList              mSpecList;
    };

    typedef std::list<cGenreNode> GenreSpecList;

    // data members
    cErrorQueue   mErrorQueue; // the general error queue
    GenreSpecList mGenreList;
};


cFCOReport_i::cGenreNode::cGenreNode(const cFCOReport_i::cGenreNode& rhs)
    : mGenreHeader(rhs.mGenreHeader), mGenre(rhs.mGenre), mSpecList(rhs.mSpecList)
{
}

cFCOReport_i::cFCOReport_i()
{
}

cFCOReport_i::~cFCOReport_i()
{
}


cFCOReport_i::cChangeNode::cChangeNode() : mpOldFCO(0), mpNewFCO(0)
{
}

cFCOReport_i::cChangeNode::cChangeNode(const cFCOReport_i::cChangeNode& rhs)
{
    mpOldFCO = rhs.mpOldFCO;
    if (mpOldFCO)
        mpOldFCO->AddRef();

    mpNewFCO = rhs.mpNewFCO;
    if (mpNewFCO)
        mpNewFCO->AddRef();

    mChangeVector = rhs.mChangeVector;
}

cFCOReport_i::cChangeNode::~cChangeNode()
{
    Clear();
}

void cFCOReport_i::cChangeNode::Clear()
{
    if (mpOldFCO)
        mpOldFCO->Release();
    if (mpNewFCO)
        mpNewFCO->Release();
    mpOldFCO = mpNewFCO = 0;
}

cFCOReport_i::cNode::cNode() : mpSpec(0), mpSpecAttr(0), mAdded(0), mRemoved(0), mnObjectsScanned(0)
{
}

cFCOReport_i::cNode::cNode(const cFCOReport_i::cNode& rhs) : mAdded(rhs.mAdded), mRemoved(rhs.mRemoved)
{
    mpSpec = rhs.mpSpec;
    if (mpSpec)
        mpSpec->AddRef();

    mpSpecAttr = rhs.mpSpecAttr;
    if (mpSpecAttr)
        mpSpecAttr->AddRef();

    mChanged         = rhs.mChanged;
    mErrorQueue      = rhs.mErrorQueue;
    mnObjectsScanned = rhs.mnObjectsScanned;
}

cFCOReport_i::cNode::~cNode()
{
    Clear();
}

void cFCOReport_i::cNode::Clear()
{
    mAdded.Clear();
    mAdded.SetSpec(0);
    mRemoved.Clear();
    mRemoved.SetSpec(0);
    mChanged.clear();
    mErrorQueue.Clear();
    if (mpSpec)
        mpSpec->Release();
    if (mpSpecAttr)
        mpSpecAttr->Release();
    mpSpec           = 0;
    mpSpecAttr       = 0;
    mnObjectsScanned = 0;
}

//=============================================================================
// cFCOReportGenreIter
//=============================================================================

class cFCOReportGenreIter_i
{
public:
    cFCOReport_i::GenreSpecList*          mpList;
    cFCOReport_i::GenreSpecList::iterator mIter;
};

cFCOReportGenreIter::cFCOReportGenreIter(const cFCOReport& report)
{
    mpData = new cFCOReportGenreIter_i;

    mpData->mpList = &report.mpData->mGenreList;
    mpData->mIter  = mpData->mpList->begin();
}

cFCOReportGenreIter::cFCOReportGenreIter(const cFCOReportGenreIter& rhs)
{
    mpData = new cFCOReportGenreIter_i;

    *this = rhs;
}

cFCOReportGenreIter::~cFCOReportGenreIter()
{
    delete mpData;
}

cFCOReportGenreIter& cFCOReportGenreIter::operator=(const cFCOReportGenreIter& rhs)
{
    mpData->mpList = rhs.mpData->mpList;
    mpData->mIter  = rhs.mpData->mIter;

    return *this;
}

// iteration methods
void cFCOReportGenreIter::SeekBegin() const
{
    mpData->mIter = mpData->mpList->begin();
}

void cFCOReportGenreIter::Next() const
{
    ++mpData->mIter;
}

bool cFCOReportGenreIter::Done() const
{
    return mpData->mIter == mpData->mpList->end();
}

// seeks to specific genre.  Returns false if genre does not exist in report
bool cFCOReportGenreIter::SeekToGenre(cGenre::Genre genre)
{
    for (mpData->mIter = mpData->mpList->begin();; ++mpData->mIter)
    {
        if (mpData->mIter == mpData->mpList->end())
            return false;

        if (mpData->mIter->mGenre == genre)
            break;
    }

    return true;
}

// removes the entire genre from report
void cFCOReportGenreIter::Remove()
{
    ASSERT(!Done());

    mpData->mIter = mpData->mpList->erase(mpData->mIter);
}

cFCOReportGenreHeader& cFCOReportGenreIter::GetGenreHeader() const
{
    return mpData->mIter->mGenreHeader;
}

// Get current genre
cGenre::Genre cFCOReportGenreIter::GetGenre() const
{
    if (mpData->mIter == mpData->mpList->end())
    {
        ASSERT(false);
        return cGenre::GENRE_INVALID;
    }

    return mpData->mIter->mGenre;
}

//=============================================================================
// cFCOReportSpecIter
//=============================================================================

class cFCOReportSpecIter_i
{
public:
    cFCOReport_i::SpecList*          mpList;
    cFCOReport_i::SpecList::iterator mIter;
};

/////////////////////////
// ctor and dtor
/////////////////////////
cFCOReportSpecIter::cFCOReportSpecIter(const cFCOReport& report, cGenre::Genre genre)
{
    mpData = 0;

    cFCOReport_i::GenreSpecList::iterator genreIter;
    for (genreIter = report.mpData->mGenreList.begin();; ++genreIter)
    {
        if (genreIter == report.mpData->mGenreList.end())
        {
            ASSERT(false);
            THROW_INTERNAL("fcoreport.cpp");
        }

        if (genreIter->mGenre == genre)
            break;
    }

    mpData         = new cFCOReportSpecIter_i();
    mpData->mpList = &genreIter->mSpecList;
    mpData->mIter  = mpData->mpList->begin();
}

cFCOReportSpecIter::cFCOReportSpecIter(cFCOReport& report, cGenre::Genre genre)
{
    mpData = 0;

    cFCOReport_i::GenreSpecList::iterator genreIter;
    for (genreIter = report.mpData->mGenreList.begin();; ++genreIter)
    {
        if (genreIter == report.mpData->mGenreList.end())
        {
            // add an empty spec to the report
            report.AddSpec(genre, 0, 0, 0);
            genreIter = report.mpData->mGenreList.end();
            --genreIter;
            ASSERT(genreIter->mGenre == genre);
        }

        if (genreIter->mGenre == genre)
            break;
    }

    mpData         = new cFCOReportSpecIter_i();
    mpData->mpList = &genreIter->mSpecList;
    mpData->mIter  = mpData->mpList->begin();
}

cFCOReportSpecIter::cFCOReportSpecIter(const cFCOReportGenreIter& genreIter)
{
    mpData         = new cFCOReportSpecIter_i();
    mpData->mpList = &genreIter.mpData->mIter->mSpecList;
    mpData->mIter  = mpData->mpList->begin();
}

cFCOReportSpecIter::~cFCOReportSpecIter()
{
    delete mpData;
}

cFCOReportSpecIter::cFCOReportSpecIter(const cFCOReportSpecIter& rhs)
{
    mpData = new cFCOReportSpecIter_i();
    *this  = rhs;
}

cFCOReportSpecIter& cFCOReportSpecIter::operator=(const cFCOReportSpecIter& rhs)
{
    if (mpData == 0)
        mpData = new cFCOReportSpecIter_i();

    mpData->mpList = rhs.mpData->mpList;
    mpData->mIter  = rhs.mpData->mIter;

    return *this;
}

int cFCOReportSpecIter::GetNumChanged() const
{
    ASSERT(!Done());
    return mpData ? mpData->mIter->mChanged.size() : 0;
}

void cFCOReportSpecIter::SetObjectsScanned(int nObjectsScanned)
{
    ASSERT(mpData != 0);
    if (mpData == 0)
        THROW_INTERNAL("fcoreport.cpp");

    mpData->mIter->mnObjectsScanned = nObjectsScanned;
}

int cFCOReportSpecIter::GetObjectsScanned() const
{
    ASSERT(!Done());
    return mpData ? mpData->mIter->mnObjectsScanned : 0;
}

void cFCOReportSpecIter::Remove()
{
    ASSERT(!Done());
    mpData->mIter->Clear();
    mpData->mIter = mpData->mpList->erase(mpData->mIter);
}

/////////////////////////
// iteration methods
/////////////////////////
void cFCOReportSpecIter::SeekBegin() const
{
    if (mpData)
        mpData->mIter = mpData->mpList->begin();
}

void cFCOReportSpecIter::Next() const
{
    ASSERT(mpData != 0);
    ++(mpData->mIter);
}

bool cFCOReportSpecIter::Done() const
{
    return mpData ? (mpData->mIter == mpData->mpList->end()) : true;
}

const iFCOSpec* cFCOReportSpecIter::GetSpec() const
{
    ASSERT(!Done());
    return mpData ? (mpData->mIter->mpSpec) : 0;
}

bool cFCOReportSpecIter::SeekToSpec(const iFCOSpec* pSpec)
{
    if (mpData)
        for (mpData->mIter = mpData->mpList->begin(); mpData->mIter != mpData->mpList->end(); ++(mpData->mIter))
        {
            if (iFCOSpecUtil::FCOSpecEqual(*mpData->mIter->mpSpec, *pSpec))
                return true;
        }

    return false;
}

const cFCOSpecAttr* cFCOReportSpecIter::GetAttr() const
{
    ASSERT(!Done());
    return mpData ? (mpData->mIter->mpSpecAttr) : 0;
}

const cErrorQueue* cFCOReportSpecIter::GetErrorQueue() const
{
    ASSERT(!Done());
    return mpData ? (&mpData->mIter->mErrorQueue) : 0;
}

cErrorQueue* cFCOReportSpecIter::GetErrorQueue()
{
    ASSERT(!Done());
    return mpData ? (&mpData->mIter->mErrorQueue) : 0;
}

const iFCOSet* cFCOReportSpecIter::GetAddedSet() const
{
    ASSERT(!Done());
    return mpData ? (&mpData->mIter->mAdded) : 0;
}

iFCOSet* cFCOReportSpecIter::GetAddedSet()
{
    ASSERT(!Done());
    return mpData ? (&mpData->mIter->mAdded) : 0;
}

const iFCOSet* cFCOReportSpecIter::GetRemovedSet() const
{
    ASSERT(!Done());
    return mpData ? (&mpData->mIter->mRemoved) : 0;
}

iFCOSet* cFCOReportSpecIter::GetRemovedSet()
{
    ASSERT(!Done());
    return mpData ? (&mpData->mIter->mRemoved) : 0;
}

//=============================================================================
// class cFCOReportChangeIter
//=============================================================================
class cFCOReportChangeIter_i
{
public:
    std::list<cFCOReport_i::cChangeNode>*          mpList;
    std::list<cFCOReport_i::cChangeNode>::iterator mIter;
};

cFCOReportChangeIter::cFCOReportChangeIter(const cFCOReportSpecIter& specIter)
{
    ASSERT(!specIter.Done());
    mpData = new cFCOReportChangeIter_i;
    SetSpecIter(specIter);
}

cFCOReportChangeIter::cFCOReportChangeIter(const cFCOReportChangeIter& rhs)
{
    mpData = new cFCOReportChangeIter_i;
    *this  = rhs;
}

cFCOReportChangeIter::~cFCOReportChangeIter()
{
    delete mpData;
}

cFCOReportChangeIter& cFCOReportChangeIter::operator=(const cFCOReportChangeIter& rhs)
{
    mpData->mpList = rhs.mpData->mpList;
    mpData->mIter  = rhs.mpData->mIter;

    return *this;
}

void cFCOReportChangeIter::SetSpecIter(const cFCOReportSpecIter& specIter)
{
    ASSERT(!specIter.Done());
    mpData->mpList = &specIter.mpData->mIter->mChanged;
    mpData->mIter  = mpData->mpList->begin();
}

///////////////////////////////////////////////////////////////////////////////
// Remove
///////////////////////////////////////////////////////////////////////////////
void cFCOReportChangeIter::Remove()
{
    ASSERT(!Done());
    mpData->mIter->Clear();
    mpData->mIter = mpData->mpList->erase(mpData->mIter);
}

void cFCOReportChangeIter::SeekBegin() const
{
    ASSERT(mpData->mpList != 0);
    mpData->mIter = mpData->mpList->begin();
}

void cFCOReportChangeIter::Next() const
{
    ASSERT(mpData->mpList != 0);
    ++(mpData->mIter);
}

bool cFCOReportChangeIter::Done() const
{
    ASSERT(mpData->mpList != 0);
    if (!mpData->mpList)
        return true;
    return (mpData->mIter == mpData->mpList->end());
}

const iFCO* cFCOReportChangeIter::GetOld() const
{
    ASSERT(!Done());
    return (mpData->mIter->mpOldFCO);
}


const iFCO* cFCOReportChangeIter::GetNew() const
{
    ASSERT(!Done());
    return (mpData->mIter->mpNewFCO);
}


const cFCOPropVector& cFCOReportChangeIter::GetChangeVector() const
{
    ASSERT(!Done());
    return (mpData->mIter->mChangeVector);
}

//=============================================================================
// class cFCOReport
//=============================================================================

IMPLEMENT_TYPEDSERIALIZABLE(cFCOReport, _T("cFCOReport"), 0, 1);

cFCOReport::cFCOReport()
{
    mpData = new cFCOReport_i;
}

cFCOReport::~cFCOReport()
{
    ClearReport();
    delete mpData;
}

////////////////////////////////////////////////////////////

cErrorQueue* cFCOReport::GetErrorQueue()
{
    return &mpData->mErrorQueue;
}

const cErrorQueue* cFCOReport::GetErrorQueue() const
{
    return &mpData->mErrorQueue;
}

///////////////////////////////////////////////////////////////////////////////
// GetNumSpecs
///////////////////////////////////////////////////////////////////////////////
int cFCOReport::GetNumSpecs(cGenre::Genre genre) const
{
    cFCOReport_i::GenreSpecList::iterator genreIter;
    for (genreIter = mpData->mGenreList.begin();; ++genreIter)
    {
        if (genreIter == mpData->mGenreList.end())
        {
            return 0;
        }

        if (genreIter->mGenre == genre)
            break;
    }

    return genreIter->mSpecList.size();
}

///////////////////////////////////////////////////////////////////////////////
// ClearReport
///////////////////////////////////////////////////////////////////////////////
void cFCOReport::ClearReport()
{
    mpData->mGenreList.clear();
}

///////////////////////////////////////////////////////////////////////////////
// AddSpec
///////////////////////////////////////////////////////////////////////////////
void cFCOReport::AddSpec(cGenre::Genre       genre,
                         const iFCOSpec*     pSpec,
                         const cFCOSpecAttr* pAttr,
                         cFCOReportSpecIter* pIter)
{
    // look up the genre
    cFCOReport_i::GenreSpecList::iterator genreIter;
    for (genreIter = mpData->mGenreList.begin();; ++genreIter)
    {
        if (genreIter == mpData->mGenreList.end())
        {
            cFCOReport_i::cGenreNode newGenre;
            newGenre.mGenre = genre;

            mpData->mGenreList.push_back(newGenre);

            genreIter = mpData->mGenreList.end();
            --genreIter;
            break;
        }

        if (genreIter->mGenre == genre)
            break;
    }

    if (pSpec == 0)
    {
        ASSERT(pAttr == 0);
        ASSERT(pIter == 0);
        return;
    }

    // make sure this spec doesn't overlap any others
    cFCOReport_i::SpecList::iterator specIter;
    for (specIter = genreIter->mSpecList.begin(); specIter != genreIter->mSpecList.end(); ++specIter)
    {
        // TODO -- what is the right action to take?
        if (iFCOSpecUtil::SpecsOverlap(pSpec, specIter->mpSpec))
        {
            ASSERT(false);
            THROW_INTERNAL("fcoreport.cpp");
        }
    }

    genreIter->mSpecList.push_back(cFCOReport_i::cNode());
    cFCOReport_i::cNode& node = genreIter->mSpecList.back();
    node.mpSpec               = pSpec->Clone();
    node.mpSpecAttr           = pAttr;
    pAttr->AddRef();
    node.mAdded.SetSpec(node.mpSpec);
    node.mRemoved.SetSpec(node.mpSpec);

    // fill out the iterator
    if (pIter && pIter->mpData && pIter->mpData->mpList == &genreIter->mSpecList)
    {
        pIter->mpData->mIter = genreIter->mSpecList.end();
        --(pIter->mpData->mIter);
        ASSERT(pIter->GetSpec() == node.mpSpec);
    }
}


///////////////////////////////////////////////////////////////////////////////
// AddChangedFCO
///////////////////////////////////////////////////////////////////////////////
void cFCOReport::AddChangedFCO(const cFCOReportSpecIter& iter,
                               const iFCO*               pOldFCO,
                               const iFCO*               pNewFCO,
                               const cFCOPropVector&     changedProps)
{
    cDebug d("cFCOReport::AddChangedFCO");

    // make some assertions about the iterator

#ifdef DEBUG
    // make sure iter points to one of our spec lists
    cFCOReport_i::GenreSpecList::iterator genreIter;
    for (genreIter = mpData->mGenreList.begin();; ++genreIter)
    {
        if (genreIter == mpData->mGenreList.end())
        {
            ASSERT(false);
            THROW_INTERNAL("fcoreport.cpp");
        }

        if (&genreIter->mSpecList == iter.mpData->mpList)
            break;
    }
#endif

    ASSERT(!iter.Done()); // make sure it points to something valid

    // some sanity checking for the fco names...
    ASSERT(pOldFCO->GetName().IsEqual(pNewFCO->GetName()));
    ASSERT((iter.GetSpec()->SpecContainsFCO(pOldFCO->GetName())));

    iter.mpData->mIter->mChanged.push_back(cFCOReport_i::cChangeNode());
    cFCOReport_i::cChangeNode& changeNode = iter.mpData->mIter->mChanged.back();
    changeNode.mpOldFCO                   = pOldFCO;
    changeNode.mpNewFCO                   = pNewFCO;
    changeNode.mChangeVector              = changedProps;
    pOldFCO->AddRef();
    pNewFCO->AddRef();
}

///////////////////////////////////////////////////////////////////////////////
// iSerializable interface
///////////////////////////////////////////////////////////////////////////////
void cFCOReport::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("Report Read")));

    ClearReport();

    // read in main error queue
    pSerializer->ReadObject(&mpData->mErrorQueue);

    // read in the genres
    int32_t genreIter, genreCount;
    int32_t specIter, specCount;

    pSerializer->ReadInt32(genreCount);
    for (genreIter = 0; genreIter < genreCount; genreIter++)
    {
        cFCOReport_i::cGenreNode newGenre;
        int32_t                  genre;

        // TODO:BAM -- this used to be int16, so take care of backwards compatability
        pSerializer->ReadInt32(genre);
        newGenre.mGenre = (cGenre::Genre)genre;

        pSerializer->ReadObject(&newGenre.mGenreHeader);

        pSerializer->ReadInt32(specCount);
        for (specIter = 0; specIter < specCount; specIter++)
        {
            newGenre.mSpecList.push_back(cFCOReport_i::cNode());
            cFCOReport_i::cNode& node = newGenre.mSpecList.back();

            node.mpSpec     = static_cast<iFCOSpec*>(pSerializer->ReadObjectDynCreate());
            node.mpSpecAttr = static_cast<cFCOSpecAttr*>(pSerializer->ReadObjectDynCreate());

            pSerializer->ReadObject(&node.mErrorQueue);
            pSerializer->ReadObject(&node.mAdded);
            pSerializer->ReadObject(&node.mRemoved);
            pSerializer->ReadInt32((int32_t&)node.mnObjectsScanned);

            node.mAdded.SetSpec(node.mpSpec);
            node.mRemoved.SetSpec(node.mpSpec);

            int32_t changeSize;
            pSerializer->ReadInt32(changeSize);
            for (int j = 0; j < changeSize; j++)
            {
                node.mChanged.push_back(cFCOReport_i::cChangeNode());
                cFCOReport_i::cChangeNode& cnode = node.mChanged.back();
                cnode.mpOldFCO                   = static_cast<iFCO*>(pSerializer->ReadObjectDynCreate());
                cnode.mpNewFCO                   = static_cast<iFCO*>(pSerializer->ReadObjectDynCreate());
                cnode.mChangeVector.Read(pSerializer);
            }
        }

        mpData->mGenreList.push_back(newGenre);
    }

    // TODO -- we should do some kind of test here to ensure that all the data we read
    // is consistant (ie -- no overlapping specs)
}

void cFCOReport::Write(iSerializer* pSerializer) const
{
    cFCOReport_i::GenreSpecList::iterator genreIter;
    cFCOReport_i::SpecList::iterator      specIter;

    // write out the main error queue
    pSerializer->WriteObject(&mpData->mErrorQueue);

    // iteratate over genres
    pSerializer->WriteInt32(mpData->mGenreList.size());

    for (genreIter = mpData->mGenreList.begin(); genreIter != mpData->mGenreList.end(); ++genreIter)
    {
        // TODO:BAM -- this used to be int16, so take care of backwards compatability
        pSerializer->WriteInt32(genreIter->mGenre);
        pSerializer->WriteObject(&genreIter->mGenreHeader);


        // write each node out...
        pSerializer->WriteInt32(genreIter->mSpecList.size());

        for (specIter = genreIter->mSpecList.begin(); specIter != genreIter->mSpecList.end(); ++specIter)
        {
            pSerializer->WriteObjectDynCreate(specIter->mpSpec);
            pSerializer->WriteObjectDynCreate(specIter->mpSpecAttr);
            pSerializer->WriteObject(&specIter->mErrorQueue);
            pSerializer->WriteObject(&specIter->mAdded);
            pSerializer->WriteObject(&specIter->mRemoved);
            pSerializer->WriteInt32(specIter->mnObjectsScanned);

            std::list<cFCOReport_i::cChangeNode>::iterator changedIter;
            pSerializer->WriteInt32(specIter->mChanged.size());
            for (changedIter = specIter->mChanged.begin(); changedIter != specIter->mChanged.end(); ++changedIter)
            {
                pSerializer->WriteObjectDynCreate(changedIter->mpOldFCO);
                pSerializer->WriteObjectDynCreate(changedIter->mpNewFCO);
                changedIter->mChangeVector.Write(pSerializer);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// GetFileHeaderID()
///////////////////////////////////////////////////////////////////////////////

struct cFCOReportFHID
{
    cFileHeaderID* ReportID;

    cFCOReportFHID()
    {
        ReportID = 0;
    }
    ~cFCOReportFHID()
    {
        delete ReportID;
    }
} gFCOReportFHID;

const cFileHeaderID& cFCOReport::GetFileHeaderID()
{
    if (gFCOReportFHID.ReportID == 0)
        gFCOReportFHID.ReportID = new cFileHeaderID(CLASS_TYPE(cFCOReport).AsString());

    // sanity check
    ASSERT(*gFCOReportFHID.ReportID == cFileHeaderID(CLASS_TYPE(cFCOReport).AsString()));

    return *gFCOReportFHID.ReportID;
}


void cFCOReport::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug d("cFCOReport:");

    d.TraceDebug("Global Error Queue:\n");
    mpData->mErrorQueue.TraceContents(dl);

    cFCOReport_i::GenreSpecList::iterator genreIter;
    cFCOReport_i::SpecList::iterator      specIter;

    for (genreIter = mpData->mGenreList.begin(); genreIter != mpData->mGenreList.end(); ++genreIter)
    {
        d.Trace(dl, "> Genre [%d]:\n", (int)genreIter->mGenre);

        int specCount = 0;
        for (specIter = genreIter->mSpecList.begin(); specIter != genreIter->mSpecList.end(); ++specIter, ++specCount)
        {
            d.Trace(dl, ">>> Spec [%d]:\n", specCount);
            ASSERT(specIter->mpSpec != 0);
            specIter->mpSpec->TraceContents(dl);
            specIter->mpSpecAttr->TraceContents(dl);
            specIter->mErrorQueue.TraceContents(dl);

            // trace out added & removed files
            d.TraceDebug(">>> Added Files:\n");
            specIter->mAdded.TraceContents(dl);
            d.TraceDebug(">>> Removed Files:\n");
            specIter->mRemoved.TraceContents(dl);

            // trace out changed files
            d.TraceDebug(">>> Changed Files:\n");
            std::list<cFCOReport_i::cChangeNode>::iterator changedIter;
            int                                            changeCounter = 0;
            for (changedIter = specIter->mChanged.begin(); changedIter != specIter->mChanged.end();
                 ++changedIter, ++changeCounter)
            {
                d.Trace(dl, ">>>>> Changed fco [%d]\n", changeCounter);
                d.Trace(dl, ">>>>>   Old FCO:\n");
                (*changedIter).mpOldFCO->TraceContents(dl);
                d.Trace(dl, ">>>>>   New FCO:\n");
                (*changedIter).mpNewFCO->TraceContents(dl);
                (*changedIter).mChangeVector.TraceContents(dl);
            }
        }
    }
}
