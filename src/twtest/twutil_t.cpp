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
// twutil_t.cpp

#include "tw/stdtw.h"
#include "tw/twutil.h"
#include "util/fileutil.h"
#include "twtest/test.h"
#include <fstream>

//#include <statbuf.h>
#include <unistd.h>
#include <sys/stat.h>

std::string WideToNarrow(const TSTRING& strWide);

void TestTWUtil()
{
    // TODO: we should test more than the file exists stuff, but that
    // is all I need to do for right now.
    cDebug d("TestTWUtil");

    /////////////////////////////////////////////////////////////////
    // Test FileExists() and FileWritable()

    d.TraceAlways("Testing FileExists() and FileWritable()\n");

    // assuming the current dir is writable, this test should succeed
    TEST(cFileUtil::FileWritable(_T("afilethatdoesnotexist.tmp")) == true);

    TSTRING tmpDir = TwTestPath("fileexistdir");
    TSTRING tmpFN  = TwTestPath("fileexiststest.tmp");

    // make a subdir in the TEMP_DIR
    tw_mkdir(tmpDir.c_str(), 0700);
    chmod(tmpDir.c_str(), 0700);

    // make sure file is not there
    chmod(tmpFN.c_str(), 0777);
    unlink(tmpFN.c_str());

    // make sure exists tests false, writable is true
    // and checking writable should not create the file
    TEST(cFileUtil::FileExists(tmpFN) == false);
    TEST(cFileUtil::FileWritable(tmpFN) == true)
    TEST(cFileUtil::FileExists(tmpFN) == false);

#if !HAVE_GETUID
    bool is_root = true;
#else
    
#if IS_AROS
    bool is_root = (65534 == getuid()); //AROS doesn't really have users, & posixy fns use this pseudo value.
#else
    bool is_root = (0 == getuid());
#endif
    
#endif
    // make the dir read only and make sure write tests false
    // windows fails this test, perhaps because I am an administrator?
    //  chmod(tmpDir.c_str(), 0500);
    //  TODO - is this valid now that we don't use /tmp?
    //  TEST(cFileUtil::FileWritable(tmpFN) == is_root);
    //  chmod(tmpDir.c_str(), 0700);

    // create the file
    {
        std::ofstream ostr(WideToNarrow(tmpFN).c_str());
        ostr << "Hey there.\n";
    }

    // test a read only file
    chmod(tmpFN.c_str(), 0400);
    TEST(cFileUtil::FileWritable(tmpFN) == is_root);

    // test a writable file
    chmod(tmpFN.c_str(), 0666);
    TEST(cFileUtil::FileWritable(tmpFN) == true);

    // delete the test file and dir
    unlink(tmpFN.c_str());
    unlink(tmpDir.c_str());
}

std::string WideToNarrow(const TSTRING& strWide)
{
    return strWide;
}

void RegisterSuite_TWUtil()
{
    RegisterTest("TWUtil", "Basic", TestTWUtil);
}
