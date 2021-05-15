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
///////////////////////////////////////////////////////////////////////////////
// fileutil_t.cpp
//
// This is just to test my Copy() method.
// Feel free to add if you want.

#include "util/stdutil.h"
#include "util/fileutil.h"
#include "core/debug.h"
#include "test.h"
#include <unistd.h>

using namespace std;

//=========================================================================
// FUNCTION CODE
//=========================================================================

void TestFileUtil()
{
    TSTRING source = TwTestPath("copy_src");

    //Create a temporary file for testing:
    FILE* testStream;
    testStream = _tfopen(source.c_str(), _T("w+b"));
    TEST(testStream);

    TSTRING testString(_T("This is a test"));
    int     iTestStringLength = testString.length();

    //Write some data to the stream...
    fwrite(testString.c_str(), sizeof(TCHAR), iTestStringLength, testStream);
    fclose(testStream);

    TSTRING dest = TwTestPath("copy_dest");

    TEST(cFileUtil::Copy(source, dest));

    unlink(dest.c_str());
    unlink(source.c_str());
}

void RegisterSuite_FileUtil()
{
    RegisterTest("FileUtil", "Basic", TestFileUtil);
}
