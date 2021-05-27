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
// fsobject.cpp

#include "stdfs.h"

#include "fco/fconame.h"
#include "fsobject.h"
#include "fsvisitor.h"
#include "core/serializer.h"
#include "core/error.h"
#include "core/errorutil.h"

// FCO Type information...
IMPLEMENT_SERREFCOUNT(cFSObject, _T("FSObject"), 0, 1)

// Debug stuff
#ifdef DEBUG
static int gNumFSObjectCreate  = 0;
static int gNumFSObjectDestroy = 0;

void cFSObject::TraceStats()
{
    cDebug d("cFSObject::TraceStats");

    d.TraceDebug(
        "cFSObject Stats:\n\tNum Created:\t%d\n\tNum Destroyed:\t%d\n", gNumFSObjectCreate, gNumFSObjectDestroy);
}


class cFSNameStatPrinter
{
public:
    ~cFSNameStatPrinter()
    {
        cDebug d("cFSNameStatPrinter");
        d.TraceDebug("*** Num fs objects created: %d Num destroyed: %d ***\n", gNumFSObjectCreate, gNumFSObjectDestroy);
    }

} gFSNameStatPrinter;

#endif //_DEBUG


cFSObject::cFSObject(const cFCOName& name) : mName(name)
{
#ifdef DEBUG
    gNumFSObjectCreate++;
#endif
}

cFSObject::cFSObject() : mName(_T("undefined"))
{
#ifdef DEBUG
    gNumFSObjectCreate++;
#endif
}

cFSObject::~cFSObject()
{
#ifdef DEBUG
    gNumFSObjectDestroy++;
#endif
}

const cFCOName& cFSObject::GetName() const
{
    return mName;
}

void cFSObject::SetName(const cFCOName& name)
{
    mName = name;
}


///////////////////////////////////////////////////////////////////////////////
// GetCaps
///////////////////////////////////////////////////////////////////////////////
uint32_t cFSObject::GetCaps() const
{
    uint32_t cap = mName.GetSize() > 1 ? CAP_CAN_HAVE_PARENT : 0;

    ASSERT(GetFSPropSet().GetValidVector().ContainsItem(cFSPropSet::PROP_FILETYPE));
    if (GetFSPropSet().GetFileType() == cFSPropSet::FT_DIR)
    {
        cap |= CAP_CAN_HAVE_CHILDREN;
    }

    return cap;
}


///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFSObject::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug d("cFSObject::TraceContents");

    d.Trace(dl, _T("%s\n"), GetName().AsString().c_str());
    // trace the property set's stats
    GetPropSet()->TraceContents(dl);
}

///////////////////////////////////////////////////////////////////////////////
// retrieving the property set and children...
///////////////////////////////////////////////////////////////////////////////
const iFCOPropSet* cFSObject::GetPropSet() const
{
    return &mPropSet;
}

iFCOPropSet* cFSObject::GetPropSet()
{
    return &mPropSet;
}

///////////////////////////////////////////////////////////////////////////////
// Clone
///////////////////////////////////////////////////////////////////////////////
iFCO* cFSObject::Clone() const
{
    cFSObject* pNew = new cFSObject(GetName());
    // copy all the properties...
    pNew->GetFSPropSet() = GetFSPropSet();
    // note that we don't get the children...
    return pNew;
}

///////////////////////////////////////////////////////////////////////////////
// Visitor Interface
///////////////////////////////////////////////////////////////////////////////
void cFSObject::AcceptVisitor(iFCOVisitor* pVisitor)
{
    // first, make sure this is the right type...
    ASSERT(pVisitor->GetType() == iFSVisitor::mType);
    iFSVisitor* pfsv = static_cast<iFSVisitor*>(pVisitor);
    pfsv->VisitFSObject(*this);
}


void cFSObject::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("File System Object")));


    pSerializer->ReadObject(&mName);
    pSerializer->ReadObject(&mPropSet);
}

void cFSObject::Write(iSerializer* pSerializer) const
{

    pSerializer->WriteObject(&mName);
    pSerializer->WriteObject(&mPropSet);
}
