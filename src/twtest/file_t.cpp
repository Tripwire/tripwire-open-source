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
// file_t.cpp : A test harness for cFile, a class for abstracting
//      file operations between different platforms.

#include "core/stdcore.h"
#include "core/file.h"
#include "twtest/test.h"
#include <stdio.h>

void TestFile()
{
    TSTRING fileName = TwTestPath("file_test.bin");

    //Create a temporary file for testing:
    FILE* testStream;
    testStream = _tfopen(fileName.c_str(), _T("w+b"));
    TEST(testStream);

    TSTRING testString(_T("This is a test"));
    int     iTestStringLength = testString.length();

    //Write some data to the stream...
    fwrite(testString.c_str(), sizeof(TCHAR), iTestStringLength, testStream);
    fclose(testStream);

    //Open the file again, for reading only this time.
    testStream = _tfopen(fileName.c_str(), _T("rb"));
    TEST(testStream);
}

////////////////////////////////////////////////////////////////////////

void testDosAsPosix(const std::string& in, const std::string& expected)
{
    TEST(expected == cDosPath::AsPosix(in));
}

void TestDosAsPosix()
{
    testDosAsPosix("c:\\foo", "/dev/c/foo");
    testDosAsPosix("c:\\foo\\bar\\baz.txt", "/dev/c/foo/bar/baz.txt");
    testDosAsPosix("c:/foo/bar/baz.txt", "/dev/c/foo/bar/baz.txt");

    testDosAsPosix("c:\\", "/dev/c/");
    testDosAsPosix("c:", "/dev/c");

    testDosAsPosix("foo.txt", "foo.txt");
    testDosAsPosix("bar\\foo.txt", "bar/foo.txt");
    testDosAsPosix("bar/foo.txt", "bar/foo.txt");

    testDosAsPosix("/foo/bar/baz.txt", "/foo/bar/baz.txt");
}


void testDosAsNative(const std::string& in, const std::string& expected)
{
    TEST(expected == cDosPath::AsNative(in));
}

void TestDosAsNative()
{
    testDosAsNative("/dev/c/foo", "c:\\foo");
    testDosAsNative("/dev/c/", "c:\\");
    testDosAsNative("/dev/c", "c:\\");

    testDosAsNative("/foo/bar/baz", "/foo/bar/baz");
}


void testDosIsAbsolute(const std::string& in, bool expected)
{
    TEST(expected == cDosPath::IsAbsolutePath(in));
}

void TestDosIsAbsolute()
{
    testDosIsAbsolute("C:\\", true);
    testDosIsAbsolute("C:", true);
    testDosIsAbsolute("C:\\foo", true);
    testDosIsAbsolute("C:\\foo\\bar\\baz.txt", true);

    testDosIsAbsolute("/foo", true);

    testDosIsAbsolute("foo.txt", false);
    testDosIsAbsolute("bar\\foo.txt", false);
    testDosIsAbsolute("bar/foo.txt", false);
}

void testDosBackupName(const std::string& in, const std::string& expected)
{
    TEST(expected == cDosPath::BackupName(in));
}

void TestDosBackupName()
{
    testDosBackupName("C:\\12345678.123", "C:\\12345678");
    testDosBackupName("C:\\12345678", "C:\\12345678");
    testDosBackupName("C:\\1.123", "C:\\1_123");
    testDosBackupName("C:\\1", "C:\\1");

    testDosBackupName("C:\\FOO\\12345678.123", "C:\\FOO\\12345678");
    testDosBackupName("C:\\FOO.BAR\\1234.123", "C:\\FOO.BAR\\1234_123");
}


////////////////////////////////////////////////////////////////////////
void testArosAsPosix(const std::string& in, const std::string& expected)
{
    TEST(expected == cArosPath::AsPosix(in));
}

void TestArosAsPosix()
{
    testArosAsPosix("DH0:", "/DH0/");
    testArosAsPosix("DH0:Foo", "/DH0/Foo");
    testArosAsPosix("DH0:Foo/Bar", "/DH0/Foo/Bar");

    testArosAsPosix("/DH0/Foo/Bar", "/DH0/Foo/Bar");

    testArosAsPosix("Foo", "Foo");
    testArosAsPosix("Foo/Bar", "Foo/Bar");
}

void testArosAsNative(const std::string& in, const std::string& expected)
{
    TEST(expected == cArosPath::AsNative(in));
}

void TestArosAsNative()
{
    testArosAsNative("/DH0", "DH0:");
    testArosAsNative("/DH0/Foo", "DH0:Foo");
    testArosAsNative("/DH0/Foo/Bar", "DH0:Foo/Bar");

    testArosAsNative("DH0:Foo/Bar", "DH0:Foo/Bar");

    testArosAsNative("Foo", "Foo");
    testArosAsNative("Foo/Bar", "Foo/Bar");
}

void testArosIsAbsolute(const std::string& in, bool expected)
{
    TEST(expected == cArosPath::IsAbsolutePath(in));
}

void TestArosIsAbsolute()
{
    testArosIsAbsolute("DH0:", true);
    testArosIsAbsolute("DH0:Foo", true);
    testArosIsAbsolute("DH0:Foo/bar", true);

    testArosIsAbsolute("/DH0/Foo/bar", true);

    testArosIsAbsolute("Foo/bar", false);
    testArosIsAbsolute("Foo", false);
}


////////////////////////////////////////////////////////////////////////
void testRedoxAsPosix(const std::string& in, const std::string& expected)
{
    TEST(expected == cRedoxPath::AsPosix(in));
}

void TestRedoxAsPosix()
{
    testRedoxAsPosix("file:/", "/file/");
    testRedoxAsPosix("file:/Foo", "/file/Foo");
    testRedoxAsPosix("file:/Foo/Bar", "/file/Foo/Bar");

    testRedoxAsPosix("/file/Foo/Bar", "/file/Foo/Bar");

    testRedoxAsPosix("Foo", "Foo");
    testRedoxAsPosix("Foo/Bar", "Foo/Bar");
}

void testRedoxAsNative(const std::string& in, const std::string& expected)
{
    TEST(expected == cRedoxPath::AsNative(in));
}

void TestRedoxAsNative()
{
    testRedoxAsNative("/file", "file:/");
    testRedoxAsNative("/file/Foo", "file:/Foo");
    testRedoxAsNative("/file/Foo/Bar", "file:/Foo/Bar");

    testRedoxAsNative("file:/Foo/Bar", "file:/Foo/Bar");

    testRedoxAsNative("Foo", "Foo");
    testRedoxAsNative("Foo/Bar", "Foo/Bar");
}

void testRedoxIsAbsolute(const std::string& in, bool expected)
{
    TEST(expected == cRedoxPath::IsAbsolutePath(in));
}

void TestRedoxIsAbsolute()
{
    testRedoxIsAbsolute("file:", true);
    testRedoxIsAbsolute("file:/Foo", true);
    testRedoxIsAbsolute("file:/Foo/bar", true);

    testRedoxIsAbsolute("/file/Foo/bar", true);

    testRedoxIsAbsolute("Foo/bar", false);
    testRedoxIsAbsolute("Foo", false);
}


void RegisterSuite_File()
{
    RegisterTest("File", "Basic", TestFile);
    RegisterTest("File", "DosAsPosix", TestDosAsPosix);
    RegisterTest("File", "DosAsNative", TestDosAsNative);
    RegisterTest("File", "DosIsAbsolute", TestDosIsAbsolute);
    RegisterTest("File", "DosBackupName", TestDosBackupName);

    RegisterTest("File", "ArosAsPosix", TestArosAsPosix);
    RegisterTest("File", "ArosAsNative", TestArosAsNative);
    RegisterTest("File", "ArosIsAbsolute", TestArosIsAbsolute);

    RegisterTest("File", "RedoxAsPosix", TestRedoxAsPosix);
    RegisterTest("File", "RedoxAsNative", TestRedoxAsNative);
    RegisterTest("File", "RedoxIsAbsolute", TestRedoxIsAbsolute);
}
