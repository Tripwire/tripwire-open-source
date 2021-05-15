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
// fcopropimpl_t.cpp
#include "fco/stdfco.h"
#include "fco/fcopropimpl.h"
#include "core/debug.h"
#include "twtest/test.h"

void TestNumeric()
{
    cDebug d("TestNumeric");
    d.TraceDebug("Entering...\n");

    // print the enum key:
    d.TraceDebug("Output key:%d True\n%d False\n%d Wrong Type\n%d Unsupported\n",
                 iFCOProp::CMP_TRUE,
                 iFCOProp::CMP_FALSE,
                 iFCOProp::CMP_WRONG_PROP_TYPE,
                 iFCOProp::CMP_UNSUPPORTED);


    // test string rep
    cFCOPropInt64  pi64;
    cFCOPropUint64 pui64, pui64b;
    pi64.SetValue(-456);
    pui64.SetValue(456);
    pui64b.SetValue(333);
    d.TraceDebug(_T("property int64_t = (should be -456) %s\n"), pi64.AsString().c_str());
    TEST(pi64.AsString() == "-456");

    // test a few operators
    d.TraceDebug("-456 < 456 (uint cmp to int should fail)= %d\n", pi64.Compare(&pui64, iFCOProp::OP_LT));
    TEST(iFCOProp::CMP_WRONG_PROP_TYPE == pi64.Compare(&pui64, iFCOProp::OP_LT));

    cFCOPropInt64 p2i64;
    p2i64.SetValue(4);

    d.TraceDebug("-456 <  4 = %d\n", pi64.Compare(&p2i64, iFCOProp::OP_LT));
    TEST(iFCOProp::CMP_TRUE == pi64.Compare(&p2i64, iFCOProp::OP_LT));

    d.TraceDebug("4 == 456 = %d\n", p2i64.Compare(&pi64, iFCOProp::OP_EQ));
    TEST(iFCOProp::CMP_FALSE == p2i64.Compare(&pi64, iFCOProp::OP_EQ));

    d.TraceDebug("333ui64 == 456ui64 = %d\n", pui64.Compare(&pui64b, iFCOProp::OP_EQ));
    TEST(iFCOProp::CMP_FALSE == p2i64.Compare(&pi64, iFCOProp::OP_EQ));
}

void TestStrings()
{
    cDebug          d("TestStrings");
    cFCOPropTSTRING pt1;
    cFCOPropTSTRING pt2;
    pt1.SetValue(TSTRING(_T("bar")));
    pt2.SetValue(TSTRING(_T("foo")));
    cFCOPropInt64 pi64;
    pi64.SetValue(8675309);

    d.TraceDebug(_T("property TSTRING = (should be \"bar\") %s\n"), pt1.AsString().c_str());
    TEST(pt1.AsString() == "bar");

    d.TraceDebug(_T("property TSTRING = (should be \"foo\") %s\n"), pt2.AsString().c_str());
    TEST(pt2.AsString() == "foo");

    d.TraceDebug("bar == foo = %d\n", pt1.Compare(&pt2, iFCOProp::OP_EQ));
    TEST(iFCOProp::CMP_FALSE == pt1.Compare(&pt2, iFCOProp::OP_EQ));

    d.TraceDebug("bar == bar = %d\n", pt1.Compare(&pt1, iFCOProp::OP_EQ));
    TEST(iFCOProp::CMP_TRUE == pt1.Compare(&pt1, iFCOProp::OP_EQ));

    d.TraceDebug("bar == 456 = %d\n", pt1.Compare(&pi64, iFCOProp::OP_EQ));
    TEST(iFCOProp::CMP_WRONG_PROP_TYPE == pt1.Compare(&pi64, iFCOProp::OP_EQ));

    d.TraceDebug("Leaving...\n");
    return;
}

void RegisterSuite_FCOPropImpl()
{
    RegisterTest("FCOPropImpl", "Numeric", TestNumeric);
    RegisterTest("FCOPropImpl", "Strings", TestStrings);
}
