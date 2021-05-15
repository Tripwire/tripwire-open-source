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
// archive_t.cpp
//
// test the archive component

#include "core/stdcore.h"
#include "core/archive.h"
#include "twtest/test.h"
#include "core/error.h"
#include "tw/twutil.h"
#include <stdio.h>

TSS_EXCEPTION(eTestArchiveError, eError);

void TestMemoryArchive()
{
    // cMemoryArchive
    cMemoryArchive memarch;

    memarch.WriteInt32(1);
    memarch.WriteInt32(2);
    memarch.WriteInt32(3);
    memarch.WriteInt32(4);

    TSTRING s = _T("Iridogorgia");
    memarch.WriteString(s);

    memarch.WriteInt64(1234567L);

    memarch.WriteInt16(42);

    memarch.Seek(0, cBidirArchive::BEGINNING);

    int32_t i;
    int64_t l;
    memarch.ReadInt32(i);
    TEST(i == 1);
    memarch.ReadInt32(i);
    TEST(i == 2);
    memarch.ReadInt32(i);
    TEST(i == 3);
    memarch.ReadInt32(i);
    TEST(i == 4);

    TSTRING s2;
    memarch.ReadString(s2);
    TEST(s2.compare(_T("Iridogorgia")) == 0);

    memarch.ReadInt64(l);
    TEST(l == 1234567L);

    TEST(memarch.ReadBlob(NULL, sizeof(int16_t)) == sizeof(int16_t));
    TEST(memarch.ReadBlob(NULL, 1024) == 0);

    try
    {
        memarch.ReadInt32(i);
        throw eTestArchiveError();
    }
    catch (eArchive& e)
    {
        // Cool we got the right exception
        (void)e;
    }
    catch (eError& e)
    {
        TEST(false);
        (void)e;
    }

    memarch.MapArchive(4 * sizeof(int32_t) + sizeof(int32_t) + 6, sizeof(int64_t));
    TEST(memarch.GetMappedOffset() == 4 * sizeof(int32_t) + sizeof(int32_t) + 6);
    TEST(memarch.GetMappedLength() == sizeof(int64_t));
    //    TEST(tw_ntohll(*(int64_t*)memarch.GetMap()) == 1234567L);
}

void TestLockedTemporaryArchive()
{
    TSTRING s = _T("Metallogorgia");

    bool threw = false;
    // cLockedTemporaryFileArchive
    TSTRING lockedFileName = TwTestPath("inaccessable_file.bin");

    cLockedTemporaryFileArchive lockedArch;

    try
    {
        // try to create an archive using a temp file
        lockedArch.OpenReadWrite();
        lockedArch.Close();
    }
    catch (eError& e)
    {
        threw = true;
        TCERR << "Error opening locked temp archive" << std::endl;
        cTWUtil::PrintErrorMsg(e);
    }
    catch (...)
    {
        threw = true;
    }

    try
    {
        // this should open and lock the file -- shouldn't be able to access it
        lockedArch.OpenReadWrite(lockedFileName.c_str());
        lockedArch.Seek(0, cBidirArchive::BEGINNING);

        // shouldn't be able to see these changes
        lockedArch.WriteInt32(1);
        lockedArch.WriteInt32(2);
        lockedArch.WriteInt32(3);
        lockedArch.WriteInt32(4);
        lockedArch.WriteString(s);
        lockedArch.WriteInt64(1234567L);
        lockedArch.WriteInt16(42);

        // this should delete the file
        lockedArch.Close();
    }
    catch (eError& e)
    {
        threw = true;
        TCERR << "Error writing locked temp archive" << std::endl;
        cTWUtil::PrintErrorMsg(e);
    }
    catch (...)
    {
        threw = true;
    }

    TEST(!threw);
}

void TestFileArchive()
{
    bool    threw = false;
    TSTRING s     = _T("Acanthogorgia");
    // cFileArchive
    TSTRING fileName = TwTestPath("archive_test.bin");

    cFileArchive filearch;
    filearch.OpenReadWrite(fileName.c_str());
    filearch.Seek(0, cBidirArchive::BEGINNING);

    filearch.WriteInt32(1);
    filearch.WriteInt32(2);
    filearch.WriteInt32(3);
    filearch.WriteInt32(4);

    filearch.WriteString(s);
    filearch.WriteInt64(1234567L);

    filearch.WriteInt16(42);
    filearch.Close();

    int32_t j;
    int64_t k;
    filearch.OpenRead(fileName.c_str());
    filearch.Seek(0, cBidirArchive::BEGINNING);
    filearch.ReadInt32(j);
    TEST(j == 1);
    filearch.ReadInt32(j);
    TEST(j == 2);
    filearch.ReadInt32(j);
    TEST(j == 3);
    filearch.ReadInt32(j);
    TEST(j == 4);

    TSTRING s3;
    filearch.ReadString(s3);
    TEST(s3.compare(_T("Acanthogorgia")) == 0);
    filearch.ReadInt64(k);
    TEST(k == 1234567L);

    TEST(filearch.ReadBlob(NULL, sizeof(int16_t)) == sizeof(int16_t));
    TEST(filearch.ReadBlob(NULL, 1024) == 0); // should be EOF

    try
    {
        filearch.ReadInt32(j);
        throw eTestArchiveError();
    }
    catch (eArchive& e)
    {
        // Cool we got the right exception
        (void)e;
    }
    catch (eError& e)
    {
        threw = true;
        (void)e;
    }

    TEST(!threw);
}

void RegisterSuite_Archive()
{
    RegisterTest("Archive", "MemoryArchive", TestMemoryArchive);
    RegisterTest("Archive", "LockedTemporaryArchive", TestLockedTemporaryArchive);
    RegisterTest("Archive", "FileArchive", TestFileArchive);
}
