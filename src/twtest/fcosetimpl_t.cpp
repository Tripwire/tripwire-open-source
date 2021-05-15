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
// fcosetimpl_t -- FCOSetImpl test driver

#include "fco/stdfco.h"
#include "fco/fcosetimpl.h"
#include "fs/fsobject.h"
#include "core/debug.h"
#include "twtest/test.h"
#include "fco/iterproxy.h"
#include "core/archive.h"
#include "core/serializerimpl.h"

static void PrintIter(const iFCOIter* pIter, cDebug& d)
{
    TSTRING str;
    for (; !pIter->Done(); pIter->Next())
    {
        str += pIter->FCO()->GetName().AsString().c_str();
        str += _T(" ");
    }
    d.TraceDebug(_T("%s\n"), str.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// TestFCOSetImpl -- FCOSetImpl component test harness
///////////////////////////////////////////////////////////////////////////////
void TestFCOSetImpl()
{
    cDebug d("TestFCOSetImpl()");
    d.TraceDebug("Entering...\n");


    iFCO* pFCO1 = new cFSObject(cFCOName(_T("fco1")));
    iFCO* pFCO2 = new cFSObject(cFCOName(_T("fco2")));
    iFCO* pFCO3 = new cFSObject(cFCOName(_T("fco3")));

    cFCOSetImpl set;
    set.Insert(pFCO1);
    set.Insert(pFCO2);
    set.Insert(pFCO3);

    // the set should have AddRef()ed, so I can release these now.
    pFCO1->Release();
    pFCO2->Release();
    pFCO3->Release();

    // let's iterate over the fcos
    iFCOIter*            pIter = set.GetIter();
    cIterProxy<iFCOIter> pit(pIter);
    pit->SeekBegin();
    PrintIter(pit, d);


    // lookup a specific fco
    cIterProxy<iFCOIter> pit2(set.Lookup(cFCOName(_T("fco2"))));
    if (!(iFCOIter*)pit2)
    {
        d.TraceError("Lookup failed for fco2!\n");
        TEST(false);
    }

    d.TraceDebug("Iterating from fco2 to end...\n");
    PrintIter(pit2, d);

    // Insert something
    d.TraceDebug("Inserting dog...\n");
    pFCO1 = new cFSObject(cFCOName(_T("dog")));
    set.Insert(pFCO1);
    pFCO1->Release();
    pit->SeekBegin();
    PrintIter(pit, d);

    // ...and then remove it
    d.TraceDebug("Removing fco3\n");
    cIterProxy<iFCOIter> pit3(set.Lookup(cFCOName(_T("fco3"))));
    if (!(iFCOIter*)pit3)
    {
        d.TraceError("Lookup failed for fco3!\n");
        TEST(false);
    }
    pit3->Remove();
    pit3->SeekBegin();
    PrintIter(pit3, d);

    // test operator=
    cFCOSetImpl set2;
    set2 = set;

    pIter->DestroyIter();
    pIter = set2.GetIter();
    pit   = pIter;
    d.TraceDebug("Made a new set and set it equal to the first with operator=; printing out...\n");
    PrintIter(pit, d);


    // test IsEmpty
    set.Clear();
    TEST(set.IsEmpty());

    // test refrence counting...
    d.TraceDebug("Set 1 was cleared out; printing set 2 to ensure ref counting worked\n");
    pit->SeekBegin();
    PrintIter(pit, d);

    // test serialization
    cFCOSetImpl     set3;
    cMemoryArchive  a;
    cSerializerImpl writeSer(a, cSerializerImpl::S_WRITE);
    writeSer.Init();
    set2.Write(&writeSer);
    writeSer.Finit();
    a.Seek(0, cBidirArchive::BEGINNING);
    cSerializerImpl readSer(a, cSerializerImpl::S_READ);
    readSer.Init();
    set3.Read(&readSer);
    readSer.Finit();
    d.TraceDebug("Serialized the set out and read it back in; this should be the same as above...\n");
    pit = set3.GetIter();
    PrintIter(pit, d);

    pIter->DestroyIter();

    d.TraceDebug("Leaving...\n");
    return;
}

void RegisterSuite_FCOSetImpl()
{
    RegisterTest("FCOSetImpl", "Basic", TestFCOSetImpl);
}
