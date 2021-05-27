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
// fcospechelper.cpp
//

#include "stdfco.h"
#include "fcospechelper.h"
#include "core/debug.h"
#include "core/serializer.h"
#include "twfactory.h"
#include "fconametranslator.h"

//#############################################################################
// iFCOSpecHelper
//#############################################################################
///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
void iFCOSpecHelper::Read(iSerializer* pSerializer, int32_t version)
{
    // read the start point
    pSerializer->ReadObject(&mStartPoint);
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
void iFCOSpecHelper::Write(iSerializer* pSerializer) const
{
    // write the start point
    pSerializer->WriteObject(&mStartPoint);
}


//#############################################################################
// cFCOSpecStopPointSet
//#############################################################################
IMPLEMENT_TYPEDSERIALIZABLE(cFCOSpecStopPointSet, _T("cFCOSpecStopPointSet"), 0, 1)


///////////////////////////////////////////////////////////////////////////////
// cFCOSpecStopPointSet
///////////////////////////////////////////////////////////////////////////////
cFCOSpecStopPointSet::cFCOSpecStopPointSet() : mRecurseDepth(cFCOSpecStopPointSet::RECURSE_INFINITE)
{
}

///////////////////////////////////////////////////////////////////////////////
// ContainsFCO
///////////////////////////////////////////////////////////////////////////////
bool cFCOSpecStopPointSet::ContainsFCO(const cFCOName& name) const
{
    // make sure name is at or below start point
    cFCOName::Relationship rel = name.GetRelationship(GetStartPoint());
    if ((rel != cFCOName::REL_BELOW) && (rel != cFCOName::REL_EQUAL))
        return false;

    // chack all the stop points
    std::set<cFCOName>::const_iterator itr;
    for (itr = mStopPoints.begin(); itr != mStopPoints.end(); ++itr)
    {
        rel = name.GetRelationship(*itr);
        if ((rel == cFCOName::REL_BELOW) || (rel == cFCOName::REL_EQUAL))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// SetStartPoint
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecStopPointSet::SetStartPoint(const cFCOName& startPoint)
{
    cDebug d("cFCOSpecStopPointSet::SetStartPoint");

    // make sure the start point is above all stop points...
    std::set<cFCOName>::iterator i;
    for (i = mStopPoints.begin(); i != mStopPoints.end(); ++i)
    {
        if (i->GetRelationship(startPoint) != cFCOName::REL_BELOW)
        {
            TOSTRINGSTREAM str;
            str << "Bad start point [" << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(startPoint)
                << "] added to spec with stop point "
                << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(*i);
	    tss_mkstr(errText, str);
	    
            d.TraceError("%s\n", errText.c_str());
            throw eSerializerInputStreamFmt(errText);
        }
    }

    // ok, it is acceptable to add this
    inherited::SetStartPoint(startPoint);
}

///////////////////////////////////////////////////////////////////////////////
// ShouldStopDescent -- this returns true if we have just crossed the
//      "boundary" between what is in and what is out of the spec's set. For
//      StopPointSets, this means that the fco name matches a stop point
///////////////////////////////////////////////////////////////////////////////
bool cFCOSpecStopPointSet::ShouldStopDescent(const cFCOName& name) const
{
    cDebug d("cFCOSpecStopPointSet::ShouldStopDescent");
    //
    // first, check the recurse depth...
    //
    if (GetRecurseDepth() != RECURSE_INFINITE)
    {
        if ((name.GetSize() - mStartPoint.GetSize()) > GetRecurseDepth())
            return true;
    }


    std::set<cFCOName>::const_iterator i = mStopPoints.find(name);
    return (i != mStopPoints.end());
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecStopPointSet::TraceContents(int dl) const
{
    cDebug d("cFCOSpecStopPointSet::TraceContents");
    if (dl == -1)
        dl = cDebug::D_DEBUG;

    d.Trace(dl, _T("Start Point:\t%s\n"), mStartPoint.AsString().c_str());
    for (std::set<cFCOName>::const_iterator i = mStopPoints.begin(); i != mStopPoints.end(); ++i)
    {
        d.Trace(dl, _T("Stop Point :\t%s\n"), i->AsString().c_str());
    }
}

///////////////////////////////////////////////////////////////////////////////
// Add
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecStopPointSet::Add(const cFCOName& name)
{
    cDebug d("cFCOSpecImpl::AddStopPoint");

    // first, make sure this stop point is below the start point
    if (name.GetRelationship(mStartPoint) != cFCOName::REL_BELOW)
    {
        d.TraceError("Attempt to add stop point that is not below start point!\n");
        d.TraceError("\tstart point = %s stop point = %s\n", mStartPoint.AsString().c_str(), name.AsString().c_str());

        TOSTRINGSTREAM str;
        str << "Attempt to add stop point that is not below start point!"
            << " start point = " << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(mStartPoint)
            << " stop point = " << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(name);

	tss_mkstr(errText, str);
	
        d.TraceError(_T("%s\n"), errText.c_str());
        throw eSerializerInputStreamFmt(errText);
    }

    std::set<cFCOName>::iterator i;
    for (i = mStopPoints.begin(); i != mStopPoints.end();)
    {
        cFCOName::Relationship rel = name.GetRelationship(*i);
        switch (rel)
        {
        case cFCOName::REL_EQUAL:
            d.TraceDebug(_T("Attempt to add stop point %s that already exists in spec %s\n"),
                         name.AsString().c_str(),
                         mStartPoint.AsString().c_str());
            return;
        case cFCOName::REL_BELOW:
            d.TraceDebug(_T("Attempt to add stop point %s to spec %s, but stop point %s already exists!\n"),
                         name.AsString().c_str(),
                         mStartPoint.AsString().c_str(),
                         i->AsString().c_str());
            return;
        case cFCOName::REL_ABOVE:
            // this stop point will replace the encountered stop point, so remove it!
            d.TraceDebug(_T("Attempt to add stop point %s to spec %s, so removing stop point %s\n"),
                         name.AsString().c_str(),
                         mStartPoint.AsString().c_str(),
                         i->AsString().c_str());
            // we have to be careful we don't decrement past the beginning of the set!
            if (i == mStopPoints.begin())
            {
                mStopPoints.erase(i);
                i = mStopPoints.begin();
            }
            else
                mStopPoints.erase(i++);
            break;

        default:
            ++i;
        }
    }

    // finally, insert the name
    mStopPoints.insert(name);
}

///////////////////////////////////////////////////////////////////////////////
// Clone
///////////////////////////////////////////////////////////////////////////////
iFCOSpecHelper* cFCOSpecStopPointSet::Clone() const
{
    cFCOSpecStopPointSet* pNew = new cFCOSpecStopPointSet;

    pNew->mStartPoint = mStartPoint;

    std::set<cFCOName>::const_iterator itr;
    for (itr = mStopPoints.begin(); itr != mStopPoints.end(); ++itr)
    {
        pNew->Add(*itr);
    }

    return pNew;
}

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecStopPointSet::Read(iSerializer* pSerializer, int32_t version)
{
    // read the start point
    //pSerializer->ReadObject(&mStartPoint);
    inherited::Read(pSerializer, version);

    // read all the stop points
    int32_t size;
    pSerializer->ReadInt32(size);
    ASSERT(size >= 0);
    for (int i = 0; i < size; ++i)
    {
        cFCOName fcoName;
        pSerializer->ReadObject(&fcoName);
        mStopPoints.insert(fcoName);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecStopPointSet::Write(iSerializer* pSerializer) const
{
    // write the start point
    //pSerializer->WriteObject(&mStartPoint);
    inherited::Write(pSerializer);

    // write all the stop points
    pSerializer->WriteInt32(mStopPoints.size());
    std::set<cFCOName>::const_iterator itr;
    for (itr = mStopPoints.begin(); itr != mStopPoints.end(); ++itr)
    {
        pSerializer->WriteObject(&(*itr));
    }
}


///////////////////////////////////////////////////////////////////////////////
// Compare
///////////////////////////////////////////////////////////////////////////////
iFCOSpecHelper::CompareResult cFCOSpecStopPointSet::Compare(const iFCOSpecHelper* pRhs) const
{
    // if different types, order on mType address :-)
    if (pRhs->GetType() != GetType())
        return ((&pRhs->GetType() < &GetType()) ? CMP_LT : CMP_GT);

    const cFCOSpecStopPointSet* pStopPtSet = static_cast<const cFCOSpecStopPointSet*>(pRhs);
    if (GetStartPoint() != pStopPtSet->GetStartPoint())
        return ((GetStartPoint() < pStopPtSet->GetStartPoint()) ? CMP_LT : CMP_GT);

    if (GetSize() != pStopPtSet->GetSize())
        return ((GetSize() < pStopPtSet->GetSize()) ? CMP_LT : CMP_GT);

    std::set<cFCOName>::const_iterator myIter = mStopPoints.begin(), rhsIter = pStopPtSet->mStopPoints.begin();
    for (; myIter != mStopPoints.end(); ++myIter, ++rhsIter)
    {
        if (*myIter != *rhsIter)
            return ((*myIter < *rhsIter) ? CMP_LT : CMP_GT);
    }

    // they must be equal!
    return CMP_EQ;
}

//#############################################################################
// cFCOSpecNoChildren
//#############################################################################
IMPLEMENT_TYPEDSERIALIZABLE(cFCOSpecNoChildren, _T("cFCOSpecNoChildren"), 0, 1)

///////////////////////////////////////////////////////////////////////////////
// ContainsFCO
///////////////////////////////////////////////////////////////////////////////
bool cFCOSpecNoChildren::ContainsFCO(const cFCOName& name) const
{
    return (name == GetStartPoint());
}

///////////////////////////////////////////////////////////////////////////////
// Clone
///////////////////////////////////////////////////////////////////////////////
iFCOSpecHelper* cFCOSpecNoChildren::Clone() const
{
    iFCOSpecHelper* pNew = new cFCOSpecNoChildren;
    pNew->SetStartPoint(GetStartPoint());
    return pNew;
}

///////////////////////////////////////////////////////////////////////////////
// ShouldStopDescent
///////////////////////////////////////////////////////////////////////////////
bool cFCOSpecNoChildren::ShouldStopDescent(const cFCOName& name) const
{
    // we should stop for everything except the start point...
    return (name != GetStartPoint());
}

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecNoChildren::Read(iSerializer* pSerializer, int32_t version)
{
    inherited::Read(pSerializer, version);
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecNoChildren::Write(iSerializer* pSerializer) const
{
    inherited::Write(pSerializer);
}

///////////////////////////////////////////////////////////////////////////////
// Compare
///////////////////////////////////////////////////////////////////////////////
iFCOSpecHelper::CompareResult cFCOSpecNoChildren::Compare(const iFCOSpecHelper* pRhs) const
{
    // if different types, order on mType address :-)
    if (pRhs->GetType() != GetType())
        return ((&pRhs->GetType() < &GetType()) ? CMP_LT : CMP_GT);

    if (GetStartPoint() != pRhs->GetStartPoint())
        return ((GetStartPoint() < pRhs->GetStartPoint()) ? CMP_LT : CMP_GT);

    return CMP_EQ;
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecNoChildren::TraceContents(int dl) const
{
    cDebug d("cFCOSpecNoChildren::TraceContents");
    if (dl == -1)
        dl = cDebug::D_DEBUG;

    d.Trace(dl, "Single FCO Spec (all children are stop points): %s\n", mStartPoint.AsString().c_str());
}
