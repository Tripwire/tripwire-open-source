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
// fcocompare_t.cpp -- the compare object's test driver
#include "fco/stdfco.h"
#include "fco/fcocompare.h"
#include "core/debug.h"
#include "fs/fsobject.h"
#include "fs/fspropcalc.h"
#include "fs/fsdatasourceiter.h"
#include "twtest/test.h"

#include <fstream>

///////////////////////////////////////////////////////////////////////////////
// PrintProps -- prints out all the valid property names and values as pairs...
///////////////////////////////////////////////////////////////////////////////

static void PrintProps(const iFCO* pFCO)
{
    cDebug                d("PrintProps");
    const iFCOPropSet*    pSet = pFCO->GetPropSet();
    const cFCOPropVector& v    = pSet->GetValidVector();

    for (int i = 0; i < pSet->GetNumProps(); i++)
    {
        if (v.ContainsItem(i))
        {
            d.TraceDebug(_T("[%d] %s\t%s\n"), i, pSet->GetPropName(i).c_str(), pSet->GetPropAt(i)->AsString().c_str());
        }
    }
}


void TestFCOCompare()
{
    std::string filename = TwTestPath("dog.txt");

    cDebug d("TestFCOCompare");
    d.TraceDebug("Entering...\n");

    // first, create an fco to compare with...

    TOFSTREAM fstr(filename.c_str());
    if (fstr.bad())
    {
        d.TraceError("Unable to create test file %s!\n", filename.c_str());
        TEST(false);
        return;
    }

    fstr << "Bark! Bark! Bark!" << std::endl;
    fstr.close();

    // create the test FCO
    cFSDataSourceIter ds;
    ds.SeekToFCO(cFCOName(filename), false);
    iFCO* pFCO = ds.CreateFCO();
    TEST(pFCO);

    // measure a couple of properties, some of which will change...
    cFSPropCalc    propCalc;
    cFCOPropVector v(pFCO->GetPropSet()->GetValidVector().GetSize());
    v.AddItem(cFSPropSet::PROP_DEV);
    v.AddItem(cFSPropSet::PROP_CTIME);
    v.AddItem(cFSPropSet::PROP_SIZE);
    v.AddItem(cFSPropSet::PROP_MTIME);
    v.AddItem(cFSPropSet::PROP_FILETYPE);
    v.AddItem(cFSPropSet::PROP_GROWING_FILE);
    propCalc.SetPropVector(v);
    pFCO->AcceptVisitor(&propCalc);
    d.TraceDebug("First FCO's properties:\n");
    PrintProps(pFCO);

    // first, try comparing it to itself...
    cFCOCompare comp;
    comp.SetPropsToCmp(v);
    unsigned int result = comp.Compare(pFCO, pFCO);
    d.TraceDebug("Compare to itself is (expect true) %s\n", result == cFCOCompare::EQUAL ? "true" : "false");
    TEST(result == cFCOCompare::EQUAL);

    // change the file...
    d.TraceDebug("Changing the file...\n");
    fstr.open(filename.c_str());
    if (fstr.bad())
    {
        d.TraceError("Unable to reopen %s!\n", filename.c_str());
        TEST(false);
        return;
    }
    fstr << "Meow! Meow! Meow! Meow!" << std::endl;
    fstr.close();

    //need a new data source iter, otherwise the existing FCO gets updated & you get a ref to it,
    // and the resulting FCOs always match.
    cFSDataSourceIter ds2;
    ds2.SeekToFCO(cFCOName(filename), false);
    iFCO* pFCO2 = ds2.CreateFCO();
    TEST(pFCO2);
    pFCO2->AcceptVisitor(&propCalc);
    d.TraceDebug("Second FCO's properties:\n");
    PrintProps(pFCO2);

    result = comp.Compare(pFCO, pFCO2);
    d.TraceDebug("Compare to new object is (expect false) %s\n", result == cFCOCompare::EQUAL ? "true" : "false");
    TEST(result == cFCOCompare::PROPS_UNEQUAL);
    d.TraceDebug("Properties that differ are:\n");
    //result.mPropVector.TraceContents();

    cFSDataSourceIter ds3;
    ds3.SeekToFCO(cFCOName(filename), false);
    // try testing properties that weren't calculated...
    d.TraceDebug("Comparing FCOs with different properties calculated\n");
    iFCO* pFCO3 = ds3.CreateFCO();
    v           = propCalc.GetPropVector();
    v.AddItem(cFSPropSet::PROP_MD5);
    propCalc.SetPropVector(v);
    pFCO3->AcceptVisitor(&propCalc);
    // do the compare
    comp.SetPropsToCmp(v);
    result = comp.Compare(pFCO2, pFCO3);
    TEST(result == cFCOCompare::PROPS_NOT_ALL_VALID);
    d.TraceDebug("Properties not valid are (should be %d):\n", cFSPropSet::PROP_MD5);
    //result.mPropVector.TraceContents();

    // release the fcos
    pFCO3->Release();
    pFCO2->Release();
    pFCO->Release();

    return;
}

void RegisterSuite_FCOCompare()
{
    RegisterTest("FCOCompare", "Basic", TestFCOCompare);
}
