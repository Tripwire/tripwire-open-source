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
// fspropcalc_t.cpp -- the fs property calculator test driver
#include "fs/stdfs.h"
#include "fs/fspropcalc.h"
#include "core/debug.h"
#include "core/archive.h"
#include "core/fsservices.h"
#include "fco/fcopropset.h"
#include "fs/fspropset.h"
#include "fs/fsdatasourceiter.h"
#include "twtest/test.h"
#include "fco/fco.h"

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>


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
            d.TraceDebug("[%d] %s\t%s\n", i, pSet->GetPropName(i).c_str(), pSet->GetPropAt(i)->AsString().c_str());
        }
    }
}

void TestFSPropCalc()
{
    cDebug            d("TestFSPropCalc");
    cFSDataSourceIter ds;
    TSTRING           foo_bin = TwTestPath("foo.bin");

    //iFSServices* pFSServices = iFSServices::GetInstance();

    // oh boy! I finally get to test property calculation!
    d.TraceDebug("Creating FCO c:\\temp\\foo.bin\n");

    std::ofstream fstr(foo_bin.c_str());
    if (fstr.bad())
    {
        d.TraceError("Unable to create test file %s!\n", foo_bin.c_str());
        TEST(false);
    }
    fstr.close();

    cFileArchive arch;
    arch.OpenReadWrite(foo_bin.c_str(), true);
    arch.WriteBlob("\x1\x2\x3\x4\x5\x6\x7\x8\x9\x0", 10);
    arch.Close();

    // get the fco but none of its children...
    ds.SeekToFCO(cFCOName(foo_bin), false);
    iFCO* pFCO = ds.CreateFCO();
    TEST(pFCO);

    // create the calculator and set some properties to calculate...
    cFSPropCalc    propCalc;
    cFCOPropVector v(pFCO->GetPropSet()->GetValidVector().GetSize());
    v.AddItem(cFSPropSet::PROP_DEV);
    v.AddItem(cFSPropSet::PROP_CTIME);
    v.AddItem(cFSPropSet::PROP_SIZE);
    v.AddItem(cFSPropSet::PROP_BLOCKS);
    v.AddItem(cFSPropSet::PROP_CRC32);
    v.AddItem(cFSPropSet::PROP_MD5);
    propCalc.SetPropVector(v);

    // finally, do the calculation
    pFCO->AcceptVisitor(&propCalc);

    // see what properties were evaluated...
    PrintProps(pFCO);

    d.TraceDebug("CRC32 should be \"2ARm2G\"\n");
    d.TraceDebug("MD5 should be \"1.Oyjj1dbom.DF2KktvtQe\"\n");

    // if we do it with "Leave", then nothing should change...
    d.TraceDebug("Changing collision action to Leave; the following run should _not_ call Stat()\n");
    propCalc.SetCollisionAction(iFCOPropCalc::PROP_LEAVE);
    pFCO->AcceptVisitor(&propCalc);


    // test only calculating unevaluated props...
    d.TraceDebug("invalidating PROP_MD5 in fco, and changing the file. \n\tAll should remain the same except md5.\n");
    arch.OpenReadWrite(foo_bin.c_str(), true);
    arch.WriteString(_T("Bark Bark Bark\n"));
    arch.Close();

    // do the calculation
    pFCO->GetPropSet()->InvalidateProp(cFSPropSet::PROP_MD5);
    pFCO->AcceptVisitor(&propCalc);
    PrintProps(pFCO);


    // TODO -- is there any way to test the error queue in the prop calc?

    // release the fco
    pFCO->Release();

    return;
}

#if HAVE_SYMLINK
void TestGetSymLinkStr()
{
    std::string file = TwTestPath("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    std::string link = TwTestPath("linky");

    int fd = creat(file.c_str(), 0777);
    TEST(fd >= 0);
    close(fd);

    unlink(link.c_str());
    TEST(0 == symlink(file.c_str(), link.c_str()));

    cMemoryArchive arch(1024 * 1024);
    TEST(cFSPropCalc::GetSymLinkStr(link, arch, 8));
    TEST(arch.Length() == (int64_t)file.size());
}
#endif

void RegisterSuite_FSPropCalc()
{
    RegisterTest("FSPropCalc", "Basic", TestFSPropCalc);
#if HAVE_SYMLINK
    RegisterTest("FSPropCalc", "GetSymLinkStr", TestGetSymLinkStr);
#endif    
}
