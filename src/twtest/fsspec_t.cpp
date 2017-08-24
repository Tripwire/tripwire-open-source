//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2017 Tripwire,
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
// fcospecimpl test driver

#include "fco/stdfco.h"
#include "fco/fcospecimpl.h"
#include "core/debug.h"
#include "fs/fsdatasourceiter.h"
#include "fco/iterproxy.h"
#include "core/error.h"
#include "twtest/test.h"
#include "fco/fcospechelper.h"
#include "core/fsservices.h"


///////////////////////////////////////////////////////////////////////////////
// PrintFCOTree -- recursively prints an fco's name and all of it's children's
///////////////////////////////////////////////////////////////////////////////
/*
static void PrintFCOTree(const iFCO* pFCO, cDebug d, int depth)
{
    TSTRING prefix;
    for(int i=0; i<depth; i++)
    {
        prefix += _T("--");
    }

    d.TraceDebug(_T("%s%s\n"), prefix.c_str(), pFCO->GetName().AsString().c_str());

    const cIterProxy<iFCOIter> pi = pFCO->GetChildSet()->GetIter();

    for(pi->SeekBegin(); ! pi->Done(); pi->Next())
    {
        PrintFCOTree(pi->FCO(), d, depth+1);
    }
}
*/

void TestFCOSpecImpl()
{
    cDebug d("TestFCOSpecImpl");
    d.TraceDebug("Entering...\n");

    cFSDataSourceIter dataSrc;

    // test AllChildStopPoint fcos...
    d.TraceDebug("Now testing a spec whose start point is the only thing it maps to (%s)\n", TwTestDir().c_str());
    cFCOSpecImpl* pSpec2 = new cFCOSpecImpl(TwTestDir(), &dataSrc, new cFCOSpecNoChildren);
    pSpec2->SetStartPoint(cFCOName(TwTestDir()));
    dataSrc.SeekToFCO(pSpec2->GetStartPoint(), false);
    iFCO* pFCO = dataSrc.CreateFCO();
    TEST(pFCO);
    //PrintFCOTree(pFCO, d, 0);
    pFCO->Release();

    // create an FSSpec and set up some start and stop points...
    cFCOSpecStopPointSet* pSet = new cFCOSpecStopPointSet;
    cFCOSpecImpl* pSpec = new cFCOSpecImpl(_T("Test FSSpec"), &dataSrc, pSet);
    pSpec->SetStartPoint(cFCOName(_T("/etc")));
    pSet->Add(cFCOName(_T("/etc/open gl")));
    pSet->Add(cFCOName(_T("/etc/pclient")));

    // create all the fcos...
    cFSDataSourceIter dataSrc2;
    dataSrc2.SeekToFCO(pSpec->GetStartPoint(), false);
    iFCO* pFCO2 = dataSrc2.CreateFCO();
    TEST(pFCO2);
    //PrintFCOTree(pFCO, d, 0);
    pFCO2->Release();

    // TODO -- test Clone(), copy ctor, operator=

    pSpec->Release();
}

