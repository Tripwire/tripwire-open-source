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
// fcospecutil_t.cpp

#include "fco/stdfco.h"
#include "fco/fcospecutil.h"
#include "core/debug.h"
#include "twtest/test.h"
#include "fco/fcospecimpl.h"
#include "fco/iterproxy.h"
#include "fco/fcospechelper.h"

void TestFcoSpecUtil()
{
    cDebug d("TestFcoSpecUtil()");

    // first check the equality operator...
    cFCOSpecStopPointSet* pSet1  = new cFCOSpecStopPointSet;
    cFCOSpecStopPointSet* pSet2  = new cFCOSpecStopPointSet;
    cFCOSpecImpl*         pSpec1 = new cFCOSpecImpl(_T("Spec1"), NULL, pSet1);
    cFCOSpecImpl*         pSpec2 = new cFCOSpecImpl(_T("Spec2"), NULL, pSet2);

    pSpec1->SetStartPoint(cFCOName(_T("Dog")));
    pSpec2->SetStartPoint(cFCOName(_T("Dog")));
    pSet1->Add(cFCOName(_T("Dog/Bark")));
    pSet2->Add(cFCOName(_T("Dog/Bark")));
    pSet1->Add(cFCOName(_T("Dog/Pant")));
    pSet2->Add(cFCOName(_T("Dog/Pant")));
    TEST(iFCOSpecUtil::FCOSpecEqual(*pSpec1, *pSpec2));

    // make them a little different...
    cFCOSpecStopPointIter iter(*pSet1);
    iter.SeekBegin();
    cFCOName removedName = iter.StopPoint();
    iter.Remove();
    pSet1->Add(cFCOName(_T("Dog/Scratch")));
    TEST(!iFCOSpecUtil::FCOSpecEqual(*pSpec1, *pSpec2));

    pSet1->Add(removedName);
    TEST(!iFCOSpecUtil::FCOSpecEqual(*pSpec1, *pSpec2));

    pSpec1->SetStartPoint(cFCOName(_T("Dog")));
    pSet2->Add(cFCOName(_T("Dog/Howl")));
    TEST(!iFCOSpecUtil::FCOSpecEqual(*pSpec1, *pSpec2));
    TEST(iFCOSpecUtil::FCOSpecLessThan(*pSpec1, *pSpec2));
    pSet1->Add(cFCOName(_T("Dog/Howm")));
    TEST(iFCOSpecUtil::FCOSpecLessThan(*pSpec2, *pSpec1));

    pSpec1->Release();
    pSpec2->Release();

    d.TraceDebug("Leaving..\n");
}

void RegisterSuite_FcoSpecUtil()
{
    RegisterTest("FcoSpecUtil", "Basic", TestFcoSpecUtil);
}
