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
// fcospecimpl.cpp
//

#include "stdfco.h"
#include "fcospecimpl.h"
#include "core/debug.h"
#include "core/serializer.h"
#include "iterproxy.h"
#include "fcospechelper.h"
#include "core/errorutil.h"

#ifdef DEBUG
int gFCOSpecImplCreated   = 0;
int gFCOSpecImplDestroyed = 0;
#endif

///////////////////////////////////////////////////////////////////////////////
// ctor and dtor
///////////////////////////////////////////////////////////////////////////////
cFCOSpecImpl::cFCOSpecImpl(const TSTRING& name, void* pSrc, iFCOSpecHelper* pHelper) : mName(name), mpHelper(pHelper)
{
#ifdef DEBUG
    ++gFCOSpecImplCreated;
    cDebug d("cFCOSpecImpl::cFCOSpecImpl()");
    d.TraceDetail("Allocated cFCOSpecImpl(%p) creat=%d dest=%d\n", this, gFCOSpecImplCreated, gFCOSpecImplDestroyed);
#endif
}

cFCOSpecImpl::cFCOSpecImpl() : iFCOSpec(), mName(_T("Unnamed_FCOSpecImpl")), mpHelper(0)
{
#ifdef DEBUG
    ++gFCOSpecImplCreated;
    cDebug d("cFCOSpecImpl::cFCOSpecImpl()");
    d.TraceDetail("Allocated cFCOSpecImpl(%p) creat=%d dest=%d\n", this, gFCOSpecImplCreated, gFCOSpecImplDestroyed);
#endif
}

cFCOSpecImpl::cFCOSpecImpl(const cFCOSpecImpl& rhs) : iFCOSpec(), mpHelper(0)
{
    *this = rhs;
#ifdef DEBUG
    ++gFCOSpecImplCreated;
    cDebug d("cFCOSpecImpl::cFCOSpecImpl()");
    d.TraceDetail("Allocated cFCOSpecImpl(%p) creat=%d dest=%d\n", this, gFCOSpecImplCreated, gFCOSpecImplDestroyed);
#endif
}

cFCOSpecImpl::~cFCOSpecImpl()
{
#ifdef DEBUG
    ++gFCOSpecImplDestroyed;
    cDebug d("cFCOSpecImpl::cFCOSpecImpl()");
    d.TraceDetail("Deleted cFCOSpecImpl(%p) creat=%d dest=%d\n", this, gFCOSpecImplCreated, gFCOSpecImplDestroyed);
#endif

    delete mpHelper;
}

IMPLEMENT_SERREFCOUNT(cFCOSpecImpl, _T("cFCOSpecImpl"), 0, 1);

///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
cFCOSpecImpl& cFCOSpecImpl::operator=(const cFCOSpecImpl& rhs)
{
    mName       = rhs.mName;
    mPropVector = rhs.mPropVector;
    mpHelper    = rhs.mpHelper ? rhs.mpHelper->Clone() : 0;

    return *this;
}
///////////////////////////////////////////////////////////////////////////////
// Clone -- make a copy of this spec
///////////////////////////////////////////////////////////////////////////////
iFCOSpec* cFCOSpecImpl::Clone() const
{
    cFCOSpecImpl* pNewSpec = new cFCOSpecImpl(mName);
    *pNewSpec              = *this;
    return pNewSpec;
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents -- debug output of this object's contents
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecImpl::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug d("cFCOSpecImpl::TraceContents");
    d.Trace(dl, _T("FCOSpecImpl     :\t%s\n"), mName.c_str());
    mPropVector.TraceContents(dl);
    if (mpHelper)
        mpHelper->TraceContents(dl);
    else
        d.Trace(dl, _T("<NULL spec helper>\n"));

    // NOTE -- tracing out the data source associated with this spec has no
    //      real value -- mdb
}

const TSTRING& cFCOSpecImpl::GetName() const
{
    return mName;
}

void cFCOSpecImpl::SetName(const TSTRING& name)
{
    mName = name;
}

const cFCOName& cFCOSpecImpl::GetStartPoint() const
{
    ASSERT(mpHelper != 0);
    return mpHelper->GetStartPoint();
}

void cFCOSpecImpl::SetStartPoint(const cFCOName& name)
{
    cDebug d("cFCOSpecImpl::SetStartPoint");

    ASSERT(mpHelper != 0);
    mpHelper->SetStartPoint(name);
}

const cFCOPropVector& cFCOSpecImpl::GetPropVector(const iFCOSpecMask* pFCOSpecMask) const
{
    // NOTE: for now, there will only be one property vector associated with a given specifier
    return mPropVector;
}

bool cFCOSpecImpl::SetPropVector(const iFCOSpecMask* pMask, const cFCOPropVector& vector)
{
    // for now, the default mask is the only mask the spec has.
    if (pMask != iFCOSpecMask::GetDefaultMask())
        return false;

    mPropVector = vector;
    return true;
}

const iFCOSpecMask* cFCOSpecImpl::GetSpecMask(const iFCO* pFCO) const
{
    // for now, I will just return the default property mask
    return iFCOSpecMask::GetDefaultMask();
}


void cFCOSpecImpl::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("FS Spec Read")));

    pSerializer->ReadString(mName);
    mPropVector.Read(pSerializer);

    // read the helper
    mpHelper = (iFCOSpecHelper*)pSerializer->ReadObjectDynCreate();
}

void cFCOSpecImpl::Write(iSerializer* pSerializer) const
{
    ASSERT(mpHelper != 0);

    pSerializer->WriteString(mName);
    mPropVector.Write(pSerializer);

    pSerializer->WriteObjectDynCreate(mpHelper);
}

///////////////////////////////////////////////////////////////////////////////
// SpecContainsFCO
///////////////////////////////////////////////////////////////////////////////
bool cFCOSpecImpl::SpecContainsFCO(const cFCOName& name) const
{
    ASSERT(mpHelper != 0);

    return mpHelper->ContainsFCO(name);
}

///////////////////////////////////////////////////////////////////////////////
// ShouldStopDescent
///////////////////////////////////////////////////////////////////////////////
bool cFCOSpecImpl::ShouldStopDescent(const cFCOName& name) const
{
    ASSERT(mpHelper != 0);
    return mpHelper->ShouldStopDescent(name);
}
