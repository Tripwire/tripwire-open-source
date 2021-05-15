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
// fcospeclist_t.cpp

#include "fco/stdfco.h"
#include "fco/fcospeclist.h"
#include "core/debug.h"
#include "twtest/test.h"
#include "fco/fcospecimpl.h"
#include "core/archive.h"
#include "core/serializerimpl.h"
#include "fco/fcospecutil.h"
#include "fco/fcospechelper.h"

void TestFCOSpecList()
{
    cDebug d("TestFCOSpecList()");
    d.TraceDebug("Entering...\n");

    cFCOSpecList specList;

    TEST(specList.IsEmpty());

    cFCOSpecStopPointSet* pStopPts = new cFCOSpecStopPointSet;
    iFCOSpec*             pSpec    = new cFCOSpecImpl(_T("s1"), NULL, pStopPts);
    pSpec->SetStartPoint(cFCOName(_T("AAA")));
    pStopPts->Add(cFCOName(_T("AAA/SSS")));

    specList.Add(pSpec);
    pSpec->Release(); // the list now owns the spec.

    cFCOSpecStopPointSet* pStopPts2 = new cFCOSpecStopPointSet;
    pSpec                           = new cFCOSpecImpl(_T("s2"), NULL, pStopPts2);
    pSpec->SetStartPoint(cFCOName(_T("AAA")));
    pStopPts2->Add(cFCOName(_T("AAA/TTT")));

    specList.Add(pSpec);
    pSpec->Release(); // the list now owns the spec.

    pSpec = new cFCOSpecImpl(_T("s3"), NULL, new cFCOSpecStopPointSet);
    pSpec->SetStartPoint(cFCOName(_T("AAA")));

    specList.Add(pSpec);

    cFCOSpecImpl* pTestSpec = new cFCOSpecImpl(_T("test"), NULL, new cFCOSpecStopPointSet);
    pTestSpec->SetStartPoint(cFCOName(_T("AAA")));

    // Test Lookup()
    iFCOSpec* pTmp;
    TEST((pTmp = specList.Lookup(pSpec)) == pSpec);
    pTmp->Release();
    TEST((pTmp = specList.Lookup(pTestSpec)) == pSpec);
    pTmp->Release();

    // Test iterator and Insert() order
    cFCOSpecListCanonicalIter itr(specList);
    TEST(itr.Spec()->GetName().compare(_T("s3")) == 0);

    pTestSpec->Release();
    pSpec->Release();

    iFCOSpec* pPrev = NULL;
    int       i;
    for (itr.SeekBegin(), i = 0; !itr.Done(); itr.Next(), ++i)
    {
        switch (i)
        {
        case 0:
            pPrev = itr.Spec();
            break;
        default:
            TEST(iFCOSpecUtil::FCOSpecLessThan(*pPrev, *itr.Spec()));
            pPrev = itr.Spec();
            break;
        }
    }

    cFCOSpecListAddedIter itra(specList);
    TEST(itra.Spec()->GetName().compare(_T("s1")) == 0);

    for (itra.SeekBegin(), i = 0; !itra.Done(); itra.Next(), ++i)
    {
        switch (i)
        {
        case 0:
            TEST(itra.Spec()->GetName().compare(_T("s1")) == 0);
            break;
        case 1:
            TEST(itra.Spec()->GetName().compare(_T("s2")) == 0);
            break;
        case 2:
            TEST(itra.Spec()->GetName().compare(_T("s3")) == 0);
            break;
        default:
            TEST(false);
            break;
        }
    }

    // test serialization
    cMemoryArchive  a;
    cSerializerImpl s(a, cSerializerImpl::S_WRITE);
    s.Init();
    specList.Write(&s);
    s.Finit();
    a.Seek(0, cBidirArchive::BEGINNING);

    cFCOSpecList    newSpecList;
    cSerializerImpl s2(a, cSerializerImpl::S_READ);
    s2.Init();
    newSpecList.Read(&s2);
    s2.Finit();

    // now to compare the two spec lists...
    cFCOSpecListCanonicalIter itr1(specList), itr2(newSpecList);
    for (itr1.SeekBegin(), itr2.SeekBegin(); !itr1.Done(); itr1.Next(), itr2.Next())
    {
        TEST(iFCOSpecUtil::FCOSpecEqual(*itr1.Spec(), *itr2.Spec()));
    }

    // test removal...
    // I know this isn't the most thourough test in the world, but I am in a hurry!
    d.TraceDebug("Testing removal...\n");
    cFCOSpecListAddedIter ai(specList);
    ai.SeekBegin();
    while (!ai.Done())
    {
        d.TraceDebug(_T("Removing spec %s\n"), ai.Spec()->GetName().c_str());
        ai.Remove();
    }
    // better be empty!
    TEST(specList.IsEmpty());

    d.TraceDebug("TestFCOSpecList apparently worked fine!\n");
    return;
}

void RegisterSuite_FCOSpecList()
{
    RegisterTest("FCOSpecList", "Basic", TestFCOSpecList);
}
