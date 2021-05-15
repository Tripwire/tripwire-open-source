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
// fcopropvector_t.cpp -- class cFCOPropVector's test harness
#include "fco/stdfco.h"
#include "fco/fcopropvector.h"

#ifndef __TEST_H
#include "twtest/test.h"
#endif

static bool init(cFCOPropVector& testV);
static void addRemove(cFCOPropVector& test1, cFCOPropVector& test2, cDebug& d);
static void objManip(cFCOPropVector& testV, cDebug& d);

void TestFCOPropVector()
{
    //Local Variables for main test function

    bool   testout;
    cDebug d("TestFCOPropVector()");
    d.TraceDetail("Entering...\n");

    cFCOPropVector test1, test2, test3;

    test1.SetSize(32);
    test2.SetSize(64);
    test3.SetSize(45);

    d.TraceDetail("Empty Vector with no buffer:\n");
    test1.check(d);
    d.TraceDetail("\nEmpty Vector with buffer used: \n");
    test2.check(d);
    d.TraceDetail("\nEmpty Vector with buffer, mSize less buffer capacity: \n");
    test3.check(d);

    //First, test set initialization is correct.
    d.TraceDetail("\nTesting vector initialization...\n");
    TEST(testout = (init(test1)) & (init(test2)) & (init(test3)));
    if (testout)
        d.TraceDetail("success.\n");
    else
    {
        test1.check(d);
        test2.check(d);
        test3.check(d);
    }

    //Test ability to add and remove
    addRemove(test1, test2, d);
    d.TraceDetail("Add/Remove over all tests is %i \n", testout);

    // test clear.
    d.TraceDetail("Testing Clear()\n");
    d.TraceDetail("Filling with numbers...\n");
    cFCOPropVector v(44);
    v.AddItem(2);
    v.AddItem(3);
    v.AddItem(9);
    v.AddItem(40);
    v.check(d);
    d.TraceDetail("Clearing Numbers...\n");
    v.Clear();
    v.check(d);
    d.TraceDetail("Clear Test Done.\n");

    //test operators
    objManip(test1, d);

    return;
}

static bool init(cFCOPropVector& testV)
{
    int range = testV.GetSize() - 1;

    for (int i = 1; i <= range; ++i)
    {
        if (testV.ContainsItem(i))
            return false;
    } //end for
    return true;
}


static void addRemove(cFCOPropVector& test1, cFCOPropVector& test2, cDebug& d)
{
    int var1 = 0, var2 = 64, var3 = 2;

    test1.AddItem(var1);
    TEST(test1.ContainsItem(var1)); //hopefully this is true!
    TEST(!test1.ContainsItem(var3));

    test2.SetSize(var2);
    TEST((test2.GetSize() == ((var2 / 32) + 1) * 32));
    TEST((test1 != test2));

    test1.RemoveItem(var1);
    test2.SetSize(test1.GetSize());
    TEST(test1 == test2);

    test1.AddItem(var3);
    test2 |= test1;
    d.TraceDetail("\nmMask should be 4!\n");
    test2.check(d);
    test2.RemoveItem(var3);
    d.TraceDetail("\nmMask should be 0! \n");
    test2.check(d);
}

static void objManip(cFCOPropVector& testV, cDebug& d)
{
    cFCOPropVector test1, test2 = testV;

    /*testV.check(d);
    test2.check(d);*/

    // test operator = , ==
    TEST(testV == test2)
    test2.AddItem(1);
    TEST(testV != test2); // test operator !=

    /*testV.check(d);
    test2.check(d);*/

    testV &= test2; //test operator &=
    testV.check(d);
    testV |= test2; //test operator |=
    testV.check(d);

    test1 = testV | test2; //test operator |
    test1.check(d);
    TEST(test1 == testV);

    test2.RemoveItem(1);
    testV = (test2 & test1); //test operator &
    testV.check(d);
    TEST(!(test1 == testV));

    // test operator ^
    cFCOPropVector v1, v2, v3;
    v1.AddItem(1);
    v1.AddItem(3);
    v1.AddItem(4);
    v2.AddItem(3);
    // expected result
    v3.AddItem(1);
    v3.AddItem(4);
    TEST((v1 ^ v2) == v3);

    // try with larger sizes...
    v2.SetSize(40);
    v2.Clear();
    v2.AddItem(3);
    TEST((v1 ^ v2) == v3);

    v2.AddItem(38);
    v1.SetSize(40);
    v1.Clear();
    v1.AddItem(1);
    v1.AddItem(38);
    v3.SetSize(40);
    v3.Clear();
    v3.AddItem(1);
    v3.AddItem(3);
    TEST((v1 ^ v2) == v3);
}

void RegisterSuite_FCOPropVector()
{
    RegisterTest("FCOPropVector", "Basic", TestFCOPropVector);
}
