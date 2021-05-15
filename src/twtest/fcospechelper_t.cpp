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
// fcospechelper_t.cpp

#include "fco/stdfco.h"
#include "fco/fcospechelper.h"
#include "twtest/test.h"
#include "core/error.h"
#include "core/serializerimpl.h"
#include "core/archive.h"

void TestFCOSpecHelper()
{
    cDebug d("TestFCOSpecHelper");
    d.TraceDebug("Entering...\n");

    // test the start and stop point fringe cases...
    d.TraceDebug("Testing start and stop point stuff...\n");
    cFCOSpecStopPointSet* pSet3 = new cFCOSpecStopPointSet;
    pSet3->SetStartPoint(cFCOName(_T("/etc")));
    d.TraceDebug("*** Added start point /etc\n");
    pSet3->Add(cFCOName(_T("/etc/dog/bark")));
    d.TraceDebug("*** Added stop  point /etc/dog/bark\n");
    pSet3->Add(cFCOName(_T("/etc/dog/pant")));
    d.TraceDebug("*** Added stop  point /etc/dog/pant\n\n");
    pSet3->Add(cFCOName(_T("/etc/cat/meow")));
    d.TraceDebug("*** Added stop  point /etc/cat/meow\n");
    pSet3->TraceContents();
    pSet3->Add(cFCOName(_T("/etc/dog")));
    d.TraceDebug("*** Added stop  point /etc/dog\n");
    pSet3->Add(cFCOName(_T("/etc/cat/purr")));
    d.TraceDebug("*** Added stop  point /etc/cat/purr\n");
    pSet3->Add(cFCOName(_T("/etc/cat/purr/loud")));
    d.TraceDebug("*** Added stop  point /etc/cat/purr/loud\n");
    pSet3->TraceContents();
    pSet3->Add(cFCOName(_T("/etc/dog")));
    d.TraceDebug("*** Added stop  point /etc/dog\n");
    try
    {
        pSet3->Add(cFCOName(_T("/var/spool")));
        d.TraceDebug("*** Added stop  point /var/spool\n");
    }
    catch (eError& e)
    {
        d.TraceDebug(_T("Caught exception : %s\n"), e.GetMsg().c_str());
    }
    try
    {
        pSet3->SetStartPoint(cFCOName(_T("/var")));
        d.TraceDebug("*** Added start point /var\n");
    }
    catch (eError& e)
    {
        d.TraceDebug(_T("Caught exception : %s\n"), e.GetMsg().c_str());
    }
    try
    {
        pSet3->SetStartPoint(cFCOName(_T("/")));
        d.TraceDebug("*** Added start point /\n");
    }
    catch (eError& e)
    {
        d.TraceDebug(_T("Caught exception : %s\n"), e.GetMsg().c_str());
    }
    pSet3->TraceContents();

    // test SpecContainsFCO()
    cFCOSpecStopPointSet* pSet4 = new cFCOSpecStopPointSet;
    pSet4->SetStartPoint(cFCOName(_T("/etc")));
    pSet4->Add(cFCOName(_T("/etc/dog")));
    pSet4->Add(cFCOName(_T("/etc/cat/meow")));
    TEST(pSet4->ContainsFCO(cFCOName(_T("/etc/frog"))));
    TEST(pSet4->ContainsFCO(cFCOName(_T("/etc/cat/paw"))));
    TEST(!pSet4->ContainsFCO(cFCOName(_T("/etc/dog"))));
    TEST(!pSet4->ContainsFCO(cFCOName(_T("/var/spool/mail"))));
    TEST(!pSet4->ContainsFCO(cFCOName(_T("/etc/dog/bark"))));

    // test the All Children Stop Points case
    cFCOSpecNoChildren noChildren;
    noChildren.SetStartPoint(cFCOName(_T("/etc")));
    TEST(noChildren.ContainsFCO(cFCOName(_T("/etc"))));
    TEST(!noChildren.ContainsFCO(cFCOName(_T("/etc/frog"))));

    // TODO -- test Compare()

    // test serialization
    // test serialization
    cMemoryArchive  a;
    cSerializerImpl s(a, cSerializerImpl::S_WRITE);
    s.Init();
    s.WriteObjectDynCreate(pSet3);
    s.WriteObjectDynCreate(&noChildren);
    s.Finit();
    a.Seek(0, cBidirArchive::BEGINNING);

    iFCOSpecHelper *pHelp1, *pHelp2;
    cSerializerImpl s2(a, cSerializerImpl::S_READ);
    s2.Init();
    pHelp1 = (iFCOSpecHelper*)s2.ReadObjectDynCreate();
    pHelp2 = (iFCOSpecHelper*)s2.ReadObjectDynCreate();
    s2.Finit();

    TEST(pHelp1->Compare(pSet3) == iFCOSpecHelper::CMP_EQ);
    TEST(pHelp2->Compare(&noChildren) == iFCOSpecHelper::CMP_EQ);
    TEST(pHelp1->Compare(&noChildren) != iFCOSpecHelper::CMP_EQ);
    TEST(pHelp2->Compare(pSet3) != iFCOSpecHelper::CMP_EQ);


    delete pSet3;
    delete pSet4;
    delete pHelp1;
    delete pHelp2;
}

void RegisterSuite_FCOSpecHelper()
{
    RegisterTest("FCOSpecHelper", "Basic", TestFCOSpecHelper);
}
