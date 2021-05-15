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
// fcoreport_t.cpp

#include "tw/stdtw.h"
#include "tw/fcoreport.h"
#include "fco/fcospecimpl.h"
#include "fco/fcosetimpl.h"
#include "fs/fsobject.h"
#include "core/serializerimpl.h"
#include "core/archive.h"
#include "twtest/test.h"
#include "core/errorbucketimpl.h"
#include "fco/fcospecattr.h"
#include "fco/fcospechelper.h"
#include "fs/fs.h"
#include <ctime>

// we use this instead of TraceContents() so we can test the report iterators.
static void TraceReport(const cFCOReport& r, cDebug& d)
{
    d.TraceDebug("Global Error Queue:\n");
    r.GetErrorQueue()->TraceContents();

    cFCOReportGenreIter genreIter(r);
    int                 genreCount = 0;

    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next(), ++genreCount)
    {
        d.TraceDebug("> Genre [%d]:\n", genreCount);

        cFCOReportSpecIter specIter(genreIter);
        int                specCount = 0;
        for (specIter.SeekBegin(); !specIter.Done(); specIter.Next(), ++specCount)
        {
            d.TraceDebug(">>> Spec [%d]:\n", specCount);
            TEST(specIter.GetSpec());
            specIter.GetSpec()->TraceContents();
            specIter.GetAttr()->TraceContents();
            specIter.GetErrorQueue()->TraceContents();

            d.TraceDebug(">>> Added Files:\n");
            specIter.GetAddedSet()->TraceContents();
            d.TraceDebug(">>> Removed Files:\n");
            specIter.GetRemovedSet()->TraceContents();

            // trace out changed files
            cFCOReportChangeIter changeIter(specIter);
            int                  changeCtr = 0;
            for (changeIter.SeekBegin(); !changeIter.Done(); changeIter.Next(), changeCtr++)
            {
                d.TraceDebug(">>>>> Changed fco [%d]\n", changeCtr);
                d.TraceDebug(">>>>>   Old FCO:\n");
                changeIter.GetOld()->TraceContents();
                d.TraceDebug(">>>>>   New FCO:\n");
                changeIter.GetNew()->TraceContents();
                changeIter.GetChangeVector().TraceContents();
            }
        }
    }
}


void TestFCOReport()
{
    cDebug d("TestFCOReport");

    cFCOSpecImpl*  pSpec         = new cFCOSpecImpl(_T("/etc"), NULL, new cFCOSpecStopPointSet);
    cFCOSpecAttr*  pAttr         = new cFCOSpecAttr;
    cFSObject*     addedFCO      = new cFSObject(cFCOName(_T("/etc/added_file")));
    cFSObject*     removedFCO    = new cFSObject(cFCOName(_T("/etc/removed_file")));
    cFSObject*     changedFCO    = new cFSObject(cFCOName(_T("/etc/changed_file")));
    cFSObject*     oldChangedFCO = new cFSObject(cFCOName(_T("/etc/changed_file")));
    cFSObject*     newChangedFCO = new cFSObject(cFCOName(_T("/etc/changed_file")));
    cFCOPropVector changedPropVector;

    /*
    //Calculate the time taken to generate the test report:
    time_t* dummy_arg = NULL;
    time_t  time_finish;
    time_t  time_begin = time(dummy_arg);
    */

    {
        cFCOReport report;

        changedPropVector.AddItem(cFSPropSet::PROP_SIZE);
        pSpec->SetStartPoint(cFCOName(_T("/etc")));
        pAttr->SetName(_T("/etc"));
        pAttr->SetSeverity(53);

        report.AddSpec(cFS::GenreID(), pSpec, pAttr);
        cFCOReportSpecIter it(report, cFS::GenreID());
        it.GetAddedSet()->Insert(addedFCO);
        it.GetRemovedSet()->Insert(removedFCO);
        report.AddChangedFCO(it, oldChangedFCO, newChangedFCO, changedPropVector);

        /*
        //Store the time taken to generate the test report:
        time_finish = time(dummy_arg);
        report.SetCreationTime( (int64_t)difftime(time_finish, time_begin));
        d.TraceDebug("Report calculation time = %I64i seconds.\n", report.GetCreationTime());
        */

        d.TraceDebug("Before serializing report:\n");
        TraceReport(report, d);
        {
            std::string  filepath = TwTestPath("tmp.twr");
            cFileArchive outFile;
            outFile.OpenReadWrite(filepath.c_str());
            cSerializerImpl outSer(outFile, cSerializerImpl::S_WRITE);

            outSer.Init();
            outSer.WriteObject(&report);
            outSer.Finit();

            outFile.Close();

            cFileArchive inFile;
            inFile.OpenRead(filepath.c_str());
            cSerializerImpl inSer(inFile, cSerializerImpl::S_READ);

            cFCOReport inReport;

            inSer.Init();
            inSer.ReadObject(&inReport);
            inSer.Finit();

            d.TraceDebug("Read in serialized report:\n");
            TraceReport(inReport, d);
        }
    }

    // TODO -- test cFCOReportSpecIter::Remove()
    // TODO -- test cFCOReportChangeIter::Remove()
    d.TraceDebug("*** We still need to test Remove() for the two iterator classes!\n");

    pSpec->Release();
    pAttr->Release();
    addedFCO->Release();
    removedFCO->Release();
    changedFCO->Release();
    oldChangedFCO->Release();
    newChangedFCO->Release();

    d.TraceDebug("Leaving...\n");
}

void RegisterSuite_FCOReport()
{
    RegisterTest("FCOReport", "Basic", TestFCOReport);
}
