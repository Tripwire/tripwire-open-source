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
/*
* Name....: tss.resources_t.cpp
* Date....: 05/04/99
* Creator.: Robert DiFalco (rdifalco)
*/

#include "core/stdcore.h"
#include "core/package.h"
#include "twtest/test.h"
#include <stdio.h>


TSS_BeginPackage(cTestResources)

    TSS_DECLARE_STRINGTABLE;

public:
cTestResources() : mnCount(10)
{
}

size_t Count() const
{
    return mnCount;
}

size_t Count(size_t nCount)
{
    return mnCount = nCount;
}

private:
size_t mnCount;

TSS_EndPackage(cTestResources)

    TSS_BeginStringIds(test) IDS_TEST1 = 1,
                             IDS_TEST2, IDS_TEST3,
                             IDS_INVALID TSS_EndStringIds(test)

                                 TSS_BeginStringtable(cTestResources)
                                     TSS_StringEntry(test::IDS_TEST1, _T("Test String 1")),
                             TSS_StringEntry(test::IDS_TEST2, _T("Test String 2")),
                             TSS_StringEntry(test::IDS_TEST3, _T("Test String 3")) TSS_EndStringtable(cTestResources)


                                 TSS_ImplementPackage(cTestResources)

                                     void TestResources()
{
    TSS_Package(cTestResources).Count(20);

    TEST(TSS_Package(cTestResources).Count() == 20);
    TEST(TSS_GetString(cTestResources, test::IDS_TEST1) == _T("Test String 1"));
    TEST(TSS_GetString(cTestResources, test::IDS_TEST2) == _T("Test String 2"));
    TEST(TSS_GetString(cTestResources, test::IDS_TEST3) == _T("Test String 3"));
    TEST(TSS_GetString(cTestResources, test::IDS_INVALID) == _T(""));
    TEST(TSS_GetString(cTestResources, 42) == _T(""));
}

void RegisterSuite_Resources()
{
    RegisterTest("Resources", "Basic", TestResources);
}
