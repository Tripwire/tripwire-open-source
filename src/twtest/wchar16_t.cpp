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
// wchar16_t.h
//
// Function and classes dealing with the WCHAR16 type
//

#include "core/stdcore.h"
#include "core/wchar16.h"
#include "twtest/test.h"

void TestWchar16()
{
    cDebug db("TestWchar16()");

    db.TraceAlways("Entering...\n");

    wc16_string a;
    TEST(a.length() == 0);

    wc16_string b(a);
    TEST(b.empty());

    //#if !IS_UNIX // need to get the L"" stuff working

    static WCHAR16 STRING1[] = {65, 66, 67, 68, 0};
    static WCHAR16 STRING2[] = {40, 66, 67, 68, 0};
    static WCHAR16 STRING0[] = {65, 66, 67, 68, 0, 0};

    a = STRING1;
    TEST(a.length() == 4);
    TEST(memcmp(a.data(), STRING1, 4 * sizeof(WCHAR16)) == 0);
    TEST(b.length() == 0);

    b = a;
    TEST(b.length() == 4);
    TEST(memcmp(b.data(), STRING1, 4 * sizeof(WCHAR16)) == 0);

    a.resize(3);
    TEST(a.length() == 3);
    TEST(memcmp(a.data(), STRING1, 3 * sizeof(WCHAR16)) == 0);
    TEST(b.length() == 4);
    TEST(memcmp(b.data(), STRING1, 4 * sizeof(WCHAR16)) == 0);

    b.resize(5);
    TEST(memcmp(b.data(), STRING0, 5 * sizeof(WCHAR16)) == 0);

    a    = STRING1;
    b    = a;
    a[0] = 40;
    TEST(a.length() == 4);
    TEST(memcmp(a.data(), STRING2, 4 * sizeof(WCHAR16)) == 0);
    TEST(b.length() == 4);
    TEST(memcmp(b.data(), STRING1, 4 * sizeof(WCHAR16)) == 0);

    a    = STRING1;
    b    = a;
    a[0] = 40;
    TEST(a.length() == 4);
    TEST(memcmp(a.data(), STRING2, 4 * sizeof(WCHAR16)) == 0);
    TEST(b.length() == 4);
    TEST(memcmp(b.data(), STRING1, 4 * sizeof(WCHAR16)) == 0);

    a = b;
    TEST(a[0] == b[0]);
    TEST(a[1] == b[1]);
    TEST(a[2] == b[2]);
    TEST(a[3] == b[3]);

    const wc16_string c = a;
    const wc16_string d = b;
    TEST(c[0] == d[0]);
    TEST(c[1] == d[1]);
    TEST(c[2] == d[2]);
    TEST(c[3] == d[3]);

#if 0 // this is anoying due to ASSERT() in wchar16.cpp
    try 
    {
        TEST(a[4] > 0);
        TEST(false);    // exception did not get thrown
    }
    catch (std::out_of_range)
    {
        ;
    }
#endif

    tss::swapbytes(b);
    TEST(memcmp(a.data(), STRING1, 4) == 0);
    tss::swapbytes(b);
    TEST(memcmp(b.data(), STRING1, 4) == 0);
    //#endif // IS_UNIX

    db.TraceAlways("Done...\n");
}

void RegisterSuite_Wchar16()
{
    RegisterTest("Wchar16", "Basic", TestWchar16);
}

