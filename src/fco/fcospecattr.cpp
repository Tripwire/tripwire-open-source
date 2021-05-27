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
// fcospecattr.cpp
//

#include "stdfco.h"
#include "fcospecattr.h"
#include "core/serializer.h"

IMPLEMENT_SERREFCOUNT(cFCOSpecAttr, _T("cFCOSpecAttr"), 0, 1)

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecAttr::Read(iSerializer* pSerializer, int32_t version)
{
    pSerializer->ReadString(mName);
    pSerializer->ReadInt32(mSeverity);

    int32_t size;
    TSTRING str;
    pSerializer->ReadInt32(size);
    mEmailAddrs.clear();
    for (int i = 0; i < size; i++)
    {
        pSerializer->ReadString(str);
        mEmailAddrs.push_back(str);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecAttr::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteString(mName);
    pSerializer->WriteInt32(mSeverity);

    pSerializer->WriteInt32(mEmailAddrs.size());
    std::list<TSTRING>::const_iterator i;
    for (i = mEmailAddrs.begin(); i != mEmailAddrs.end(); ++i)
        pSerializer->WriteString(*i);
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFCOSpecAttr::TraceContents(int dl) const
{
    cDebug d("cFCOSpecAttr::TraceContents");
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    d.Trace(dl, "--- cFCOSpecAttr ---\n");
    d.Trace(dl, "Name: %s Severity: %d\n", mName.c_str(), mSeverity);
    d.Trace(dl, "Email addresses:\n");

    std::list<TSTRING>::const_iterator i;
    for (i = mEmailAddrs.begin(); i != mEmailAddrs.end(); ++i)
    {
        d.Trace(dl, "\t%s\n", i->c_str());
    }
}

bool cFCOSpecAttr::operator==(const cFCOSpecAttr& rhs) const
{
    return ((mEmailAddrs == rhs.mEmailAddrs) && (mName == rhs.mName) && (mSeverity == rhs.mSeverity));
}
