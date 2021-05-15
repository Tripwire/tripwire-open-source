//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
// fcospecattr_t

#include "fco/stdfco.h"
#include "fco/fcospecattr.h"
#include "twtest/test.h"
#include "core/archive.h"
#include "core/serializerimpl.h"
#include "twtest/test.h"

static void TraceSpecAttr(const cFCOSpecAttr* pAttr, cDebug d)
{
    d.TraceDebug("--- Printing contents ---\n");
    d.TraceDebug(_T("Name: %s Severity: %d\n"), pAttr->GetName().c_str(), pAttr->GetSeverity());
    d.TraceDebug("Email addresses:\n");

    cFCOSpecAttrEmailIter i(*pAttr);
    for (i.SeekBegin(); !i.Done(); i.Next())
    {
        d.TraceDebug(_T("\t%s\n"), i.EmailAddress().c_str());
    }
}


void TestFCOSpecAttr()
{
    cDebug d("TestFCOSpecAttr");

    d.TraceDebug("Entering\n");
    cFCOSpecAttr* pAttr = new cFCOSpecAttr;

    pAttr->SetName(_T("My Name"));
    d.TraceDebug("Setting Name     = My Name\n");
    pAttr->SetSeverity(53);
    d.TraceDebug("Setting Severity = 53\n");
    pAttr->AddEmail(_T("dog@bark.com"));
    d.TraceDebug("Adding email     = dog@bark.com\n");
    pAttr->AddEmail(_T("cow@moo.com"));
    d.TraceDebug("Adding email     = cow@moo.com\n");
    pAttr->AddEmail(_T("cat@meow.com"));
    d.TraceDebug("Adding email     = cat@meow.com\n");

    // trace contents...
    TraceSpecAttr(pAttr, d);

    // test serialization...
    d.TraceDebug("Testing Serialization; next output should be the same as the previous\n");
    cMemoryArchive  a;
    cSerializerImpl s(a, cSerializerImpl::S_WRITE);
    s.Init();
    pAttr->Write(&s);
    s.Finit();
    a.Seek(0, cBidirArchive::BEGINNING);
    cFCOSpecAttr*   pNew = new cFCOSpecAttr;
    cSerializerImpl s2(a, cSerializerImpl::S_READ);
    s2.Init();
    pNew->Read(&s2);
    s2.Finit();

    // trace contents...
    TraceSpecAttr(pNew, d);

    TEST(*pAttr == *pNew);

    pNew->Release();
    pAttr->Release();
}

void RegisterSuite_FCOSpecAttr()
{
    RegisterTest("FCOSpecAttr", "Basic", TestFCOSpecAttr);
}
