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
// unixfsservices_t.cpp: Tests cUnixFSServices

//#include <fcntl.h>
#include "core/stdcore.h"
#include "core/unixfsservices.h"
#include <iostream>
#include "core/archive.h"
#include "fco/fconame.h"

#ifndef __TEST_H
#include "twtest/test.h"
#endif

#include <unistd.h>

using namespace std;


std::string makeTestFile(const std::string& filename)
{
    TSTRING      testfile = TwTestPath(filename);
    cFileArchive filearch;
    filearch.OpenReadWrite(testfile.c_str());
    filearch.Seek(0, cBidirArchive::BEGINNING);
    filearch.WriteString(_T("This is a test"));
    filearch.Close();

    return testfile;
}

//Tests the functions that are currently implemented in win32fsservices.
void TestReadDir()
{
    cDebug d("TestReadDir");
    //  d.RemoveOutTarget(cDebug::OUT_STDOUT);

    iFSServices* pFSServices = iFSServices::GetInstance();

    // working primarily with the temp dir.
    cFCOName name(TwTestDir());

    // Check to make sure test dir is a dir
    //TEST(pFSServices->GetFileType(name) == cFSStatArgs::TY_DIR);

    // get directory contents (test readdir)
    std::vector<TSTRING> v;
    pFSServices->ReadDir(name.AsString(), v);

    {
        d.TraceDebug("name: %d entries\n", v.size());

        std::vector<TSTRING>::iterator p;
        size_t                         n = 0;
        for (p = v.begin(); p != v.end(); ++p)
        {
            d.TraceDetail("    %s\n", p->c_str());
            n++;
        }

        TEST(n == v.size());
    }
}

void TestStat()
{
    //Test the Stat method
    cFSStatArgs stat;

    std::string testfile = makeTestFile("stat.tmp");

    iFSServices::GetInstance()->Stat(testfile, stat);

    TEST("Stat() did not throw");
    /*
    cDebug d("TestStat");
    //print out the information returned by Stat
    d.TraceDetail("Information returned by Stat: \n");
    d.TraceDetail("Group ID : %-5d \n", stat.gid);
    //d.TraceDetail("Last access time: %d \n", stat.atime);
    d.TraceDetail("Last inode change: %d \n", stat.ctime);
    d.TraceDetail("Last modified: %d \n", stat.mtime);
    d.TraceDetail("Major/minor device nums: %d \n", stat.dev);
    d.TraceDetail("Inode # of file : %d \n", stat.ino);
    d.TraceDetail("Mode bits: %d \n", stat.mode);
    d.TraceDetail("Num links: %d \n", stat.nlink);
    d.TraceDetail("Major/minor dev if special: %d \n", stat.rdev);
    d.TraceDetail("File size: %d \n", stat.size);
    d.TraceDetail("User ID: %d \n", stat.uid);
*/
}

void TestGetCurrentDir()
{
    TSTRING currpath;
    iFSServices::GetInstance()->GetCurrentDir(currpath);

    TEST("GetCurrentDir() did not throw");

    //d.TraceDetail("GetCurrentDir returned %s\n", currpath.c_str());
    //TEST(currpath == _T("~"));
    //they should both be ~!!
}

void TestMakeTempFilename()
{
    TSTRING _template(_T("twtempXXXXXX"));
    iFSServices::GetInstance()->MakeTempFilename(_template);

    TEST("MakeTempFilename() did not throw");
}

void TestGetMachineName()
{
    TSTRING uname;
    iFSServices::GetInstance()->GetMachineName(uname);

    TEST("GetMachineName() did not throw");
}

void TestGetHostID()
{
    TSTRING hostid;
    iFSServices::GetInstance()->GetHostID(hostid);

    TEST("GetHostID() did not throw:");
}

void TestGetCurrentUserName()
{
#if !IS_SKYOS // SkyOS breaks on this, for as-yet-unknown reasons
    TSTRING username;
    bool    success = iFSServices::GetInstance()->GetCurrentUserName(username);
    if (!success)
        skip("GetCurrentUserName test skipped, usually caused by system configuration problems");

    TEST("GetCurrentUserName() did not throw");
#endif
}

void TestGetIPAddress()
{
    uint32_t ipaddr;
    bool   success = iFSServices::GetInstance()->GetIPAddress(ipaddr);
    if (!success)
        skip("GetIPAddress test skipped, usually caused by hostname/IP configuration problems");

    TEST("GetIPAddress() did not throw");
}

void TestGetExecutableFilename()
{
    if (-1 == access("/bin/sh", F_OK))
        skip("/bin/sh not found/accessible");

    TSTRING filename = _T("sh");
    TSTRING fullpath = _T("/bin/");
    TEST(iFSServices::GetInstance()->GetExecutableFilename(fullpath, filename));

    filename = _T("/bin/sh");
    TEST(iFSServices::GetInstance()->GetExecutableFilename(fullpath, filename));
}

void TestRename()
{
    std::string testfile = makeTestFile("rename_from");

    TSTRING newtestfile = TwTestPath("rename_to");
    TEST(iFSServices::GetInstance()->Rename(testfile, newtestfile));
}

void TestFileDelete()
{
    std::string to_rm = makeTestFile("to_rm");

    TEST(iFSServices::GetInstance()->FileDelete(to_rm));
}

// This looks silly, but we've run into situations where basic exception handling fails due to
// fun linker issues, so here's a test to verify that we were built correctly.
void TestCatch()
{
    bool threw = false;
    try
    {
        throw eFSServices("a thing happened");
    }
    catch (const eFSServices& e)
    {
        threw = true;
    }

    TEST(threw);
}


void RegisterSuite_UnixFSServices()
{
    RegisterTest("UnixFSServices", "ReadDir", TestReadDir);
    RegisterTest("UnixFSServices", "Stat", TestStat);
    RegisterTest("UnixFSServices", "GetCurrentDir", TestGetCurrentDir);
    RegisterTest("UnixFSServices", "MakeTempFilename", TestMakeTempFilename);
    RegisterTest("UnixFSServices", "GetMachineName", TestGetMachineName);
    RegisterTest("UnixFSServices", "GetHostID", TestGetHostID);
    RegisterTest("UnixFSServices", "GetCurrentUserName", TestGetCurrentUserName);
    RegisterTest("UnixFSServices", "GetIPAddress", TestGetIPAddress);
    RegisterTest("UnixFSServices", "GetExecutableFilename", TestGetExecutableFilename);
    RegisterTest("UnixFSServices", "Rename", TestRename);
    RegisterTest("UnixFSServices", "FileDelete", TestFileDelete);
    RegisterTest("UnixFSServices", "TestCatch", TestCatch);
}
