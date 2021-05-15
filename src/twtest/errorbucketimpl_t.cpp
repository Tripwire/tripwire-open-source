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
// errorbucketimpl_t.cpp

#include "core/stdcore.h"
#include "core/errorbucketimpl.h"
#include "twtest/test.h"
#include "core/debug.h"
#include "core/archive.h"
#include "core/errorgeneral.h"
#include "core/errortable.h"

TSS_EXCEPTION(eTestErrorBucketImpl, eError);

// test option 7
void TestErrorBucketImpl()
{
    cDebug d("TestErrorBucketImpl");

    //This whole function is in sorry shape... TODO: Fix this DRA
    d.TraceDebug("Entering...\n");

    cErrorReporter er;
    cErrorQueue    eq;

    er.SetChild(&eq);


    //These calls to PrintErrorMsg are broken.  The code is probably old. -DRA

    // Test error reporting
    cErrorReporter::PrintErrorMsg(eErrorGeneral(_T("This should have a single line.")));
    cErrorReporter::PrintErrorMsg(eErrorGeneral(_T("This should have a mulitiple lines since I have")
                                                _T(" put so much text here.  But it does have a lot")
                                                _T(" of spaces so cErrorReporter should have no")
                                                _T(" problem breaking it up.")));
    cErrorReporter::PrintErrorMsg(eErrorGeneral(
        _T("This has many long words: ")
        _T("40chars_________________________________")
        _T(" short words ")
        _T("50chars___________________________________________")
        _T(" short words ")
        _T("90chars___________________________________________________________________________________")
        _T(" short words short words short words short words ")
        _T("90chars___________________________________________________________________________________")));
    cErrorReporter::PrintErrorMsg(eErrorGeneral(
        _T("The error reporter should handle newlines.\n")
        _T("Newlines should break up the text appropriately.  Who knows when they will occur.  Can't have them ")
        _T("getting in the way.\n")
        _T("And one last line with a big char strings:  ")
        _T("90chars___________________________________________________________________________________ ")
        _T("40chars_________________________________ 50chars___________________________________________")));


    // TODO -- test the error table
    // TODO -- test the error filter
    // set some errors...

    eTestErrorBucketImpl error1(_T("Error A")), error2(_T("Error B")), error3(_T("Error C"));

    er.AddError(error1);
    d.TraceDebug("Adding error 0 -- Error A\n");
    er.AddError(error2);
    d.TraceDebug("Adding error 1 -- Error B\n");
    er.AddError(error3);
    d.TraceDebug("Adding error 100 -- Error C\n");

    // those should have gone out to stderr; let's check the queue
    cErrorQueueIter i(eq);
    int             counter = 0;
    for (i.SeekBegin(); !i.Done(); i.Next(), counter++)
    {
        switch (counter)
        {
        case 0:
            TEST(i.GetError().GetID() == error1.GetID());
            TEST(i.GetError().GetMsg().compare(_T("Error A")) == 0);
            break;
        case 1:
            TEST(i.GetError().GetID() == error2.GetID());
            TEST(i.GetError().GetMsg().compare(_T("Error B")) == 0);
            break;
        case 2:
            TEST(i.GetError().GetID() == error3.GetID());
            TEST(i.GetError().GetMsg().compare(_T("Error C")) == 0);
            break;
        default:
            TEST(false);
        }
    }

    //TODO - test this stuff that's commented out

    //    TCOUT << _T("Following string should be a cArchive::ERR_OPEN_FAILED error:\n");
    //    TCOUT << cErrorTable::GetErrorString(cArchive::ERR_OPEN_FAILED) << std::endl;

    /* This isn't going to work anymore, given that we don't have numeric errror constants
// print out all error strings

    // Look up all errors. 
    // Note: our current error printing format limits us to 4 digit errors, so this should work for a while.
    int errornum;
    for (errornum = 0; errornum <= 9999; ++errornum) 
    {
        TSTRING errorString = cErrorTable::GetErrorString(errornum);
        if (errorString.compare(0, 26, _T("**** Invalid Error Number ")) != 0)
        {
            TCOUT << _T("### Error ");

            TCOUT.width(4);
            TCHAR oldfill = TCOUT.fill(_T('0'));
            TCOUT << errornum;
            TCOUT.fill(oldfill);

            TCOUT << _T(": ") << errorString << std::endl;
        }
    }
*/

    d.TraceDebug("Leaving...\n");
}

void RegisterSuite_ErrorBucketImpl()
{
    RegisterTest("ErrorBucketImpl", "Basic", TestErrorBucketImpl);
}
