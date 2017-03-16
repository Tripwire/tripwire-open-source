//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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

using namespace std;

//Tests the functions that are currently implemented in win32fsservices.
void TestUnixFSServices()
{
    cDebug d("TestUnixFSServices");
    //  d.RemoveOutTarget(cDebug::OUT_STDOUT);

    try
    {
        iFSServices* pFSServices = iFSServices::GetInstance();

        // working primarily with the temp dir.
        cFCOName name(_T("/tmp"));  // dies here

        // Check to make sure /tmp is a dir
        //TEST(pFSServices->GetFileType(name) == cFSStatArgs::TY_DIR);
        
        // get directory contents (test readdir)
        std::vector <TSTRING> v;
        pFSServices->ReadDir(name.AsString(), v);

        {
            d.TraceDebug("name: %d entries\n", v.size());

            std::vector <TSTRING>::iterator p;
            int n = 0;
            for (p = v.begin(); p != v.end(); p++) {
                d.TraceDetail("    %s\n", p->c_str());
                n++;
            }

            TEST(n == v.size());
        }

        //Test the Stat method
        cFSStatArgs stat;

        //TO DO: use archive to create this file
        TSTRING testfile = "/tmp/tmp.tmp";
        cFileArchive filearch;
        filearch.OpenReadWrite(testfile.c_str());
        filearch.Seek(0, cBidirArchive::BEGINNING);
        filearch.WriteString(_T("This is a test"));
        filearch.Close();
        
        pFSServices->Stat(testfile, stat);
        
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

        //Test GetCurrentDir:
        TSTRING currpath;
        pFSServices->GetCurrentDir(currpath);
        d.TraceDetail("GetCurrentDir returned %s\n", currpath.c_str());
        //TEST(currpath == _T("~"));
            //they should both be ~!!

        //Test MakeTempFilename
        TSTRING _template(_T("twtempXXXXXX"));
        pFSServices->MakeTempFilename(_template);
        d.TraceDetail("Testing MakeTempFilename: \n");
        d.TraceDetail("%s \n", _template.c_str() );

        // Test GetMachineName
        d.TraceDetail("Testing GetMachineName:\n");
        TSTRING uname;
        pFSServices->GetMachineName(uname);
        d.TraceDetail("GetMachineName returned: %s\n", uname.c_str());
        
        // Test GetHostID
        d.TraceDetail("Testing GetHostID:\n");
        TSTRING hostid;
        pFSServices->GetHostID(hostid);
        d.TraceDetail("GetHostID returned: %s\n", hostid.c_str());

        // Test GetCurrentUserName
        d.TraceDetail("Testing GetCurrentUserName:\n");
        TSTRING username;
        TEST( pFSServices->GetCurrentUserName(username) );
        d.TraceDetail("GetCurrentUserName returned: %s\n", username.c_str());

        TCERR << "TODO: unixfsservices_t.cpp, Test GetIPAddress segfaults mysteriously." << std::endl;
        // Test GetIPAddress
        /*d.TraceDetail("Testing GetIPAddress:\n");
        uint32 *ipaddr;
        TEST( pFSServices->GetIPAddress( *ipaddr ) );
        d.TraceDetail("GetIPAddress returned: %d\n", ipaddr);
        */
        // test GetExecutableFilename
        d.TraceDetail("Testing GetExecutableFilename: \n");
        TSTRING filename = _T("sh");
        TSTRING fullpath = _T("/bin/");
        TEST(pFSServices->GetExecutableFilename(fullpath, filename));
        filename = _T("/bin/sh");
        TEST(pFSServices->GetExecutableFilename(fullpath, filename));

        // test Rename
        d.TraceDetail("Testing Rename:\n");
        TSTRING newtestfile = _T("/tmp/new.tmp");
        TEST( pFSServices->Rename( testfile, newtestfile ) );

        // test GetOwnerForFile
        d.TraceDetail("Testing GetOwnerForFile:\n");
        TSTRING ownername;
        TEST( pFSServices->GetOwnerForFile( newtestfile, ownername ) );
        d.TraceDetail("GetOwnerForFile returned owner %s.\n", ownername.c_str());

        // test GetGroupForFile
        d.TraceDetail("Testing GetGroupForFile:\n");
        TSTRING groupname;
        TEST( pFSServices->GetGroupForFile( newtestfile, groupname ) );
        d.TraceDetail("GetGroupForFile returned group %s.\n", groupname.c_str());

        // test FileDelete
        d.TraceDetail("Testing FileDelete:\n");
        TEST( pFSServices->FileDelete( newtestfile ) );



    }//end try block
    catch (eError& e)
    {
      d.TraceError("Exception caught: %s\n", e.GetMsg().c_str());
    }

}








