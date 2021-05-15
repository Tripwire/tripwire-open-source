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
//hashtable_t.cpp : Test suite for cHashTable.

#include "core/stdcore.h"
#include "core/hashtable.h"
#include <iostream>

#ifndef __TEST_H
#include "twtest/test.h"
#endif

//#include "dummy.h"

using namespace std;

void HashTest1()
{
    //Test the Hash table with Key = TSTRING

    cHashTable<TSTRING, void*> htable;
    cDebug                     d("TestHashTable()::Test1");
    d.TraceDetail("Entering ...\n");

     //test data
    TSTRING string      = _T("test string");
    TSTRING string2     = _T("another test string");
    TSTRING string3     = _T("yet another test string");
    void*   data_ptr    = NULL;
    void*   data_ptr2   = NULL;
    void*   data_ptr3   = NULL;
    void*   test_lookup = NULL;

    int  var   = 32;
    int  var2  = 33;
    int* test  = &var;
    int* test2 = &var2;

    data_ptr  = test;
    data_ptr2 = test2;

    //Test insert and lookup.
    htable.Insert(string, data_ptr);
    TEST(htable.Lookup(string, test_lookup));

    //Make sure value is being stored and returned correctly
    d.TraceDetail("Value returned from table is %i, and should be %i.\n", *((int*)test_lookup), var);
    TEST(*((int*)test_lookup) == var);

    //Check remove and lookup (lookup should fail)
    TEST(htable.Remove(string));
    TEST(!htable.Lookup(string, test_lookup));

    //Has test_lookup's value changed?  It shouldn't have...
    TEST(*((int*)test_lookup) == var);

    //Insert and Remove different string/key combo.
    htable.Insert(string2, data_ptr2);
    htable.Insert(string3, data_ptr3);

    // test iteration
    cHashTableIter<TSTRING, void*> iter(htable);
    d.TraceDebug("Testing the iterator:\n");
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        d.TraceDebug(_T("Key=%s\tValue=%d\n"), iter.Key().c_str(), iter.Val());
    }

    // get statistics
#ifdef DEBUG
    htable.TraceDiagnostics();
#endif

    //Test IsEmpty()
    TEST(!htable.IsEmpty());

    //Test Clear(), IsEmpty()
    TEST(htable.Clear());
    TEST(htable.IsEmpty());
}


// TODO: Turn this into a working, useful test, or remove it.
void HashTest3()
{
    /*
    //Test the Hash table with arbitrary key
    

    //Won't work with int!! (oops).  I'll need to make one of our data types 
    // const TCHAR*() capable.  Casting an int to a TCHAR* just returns an address,
    // so there's no way to properly hash (no length, etc).
    cHashTable<cDummy, void*> htable2;
    cDummy key1, key2;
    key1.SetInt(40);
    key2.SetInt(50);
    test_lookup = NULL;

    //Test insert and lookup.
    htable2.Insert(key1, data_ptr);
    TEST(ret &= htable2.Lookup(key1, test_lookup));

    //Make sure value is being stored and returned correctly
    d.TraceDetail("Value returned from table is %i, and should be %i.\n", *((int*)test_lookup), var);
    TEST(*((int*)test_lookup) == var);

    //Check remove and lookup (lookup should fail)
    TEST(ret &= htable2.Remove(key1));
    TEST(ret &= !htable2.Lookup(key1, test_lookup));
    //Has test_lookup's value changed?  It shouldn't have...
    TEST(*((int*)test_lookup) == var);

    //Insert and different key/val combo.
    htable2.Insert(key2, data_ptr2);

    //Test IsEmpty()
    ret &= !htable2.IsEmpty();
    TEST(!htable2.IsEmpty());

    //Test Clear(), IsEmpty()
    ret &= htable2.Clear();
    TEST(htable2.Clear());
    ret &= htable2.IsEmpty();
    TEST(htable2.IsEmpty());
    
*/
    return;
}


void HashTest2()
{
    cDebug d("TestHashTable()::Test2");
    d.TraceDebug("entering...\n");

    {
        cHashTable<TSTRING, TSTRING> tbl;

        // test insert and lookup
        TEST(tbl.Insert(_T("foo"), _T("foo")) == false);

        TSTRING val;
        TEST(tbl.Lookup(_T("foo"), val));
        TEST(val.compare(_T("foo")) == 0);

        // check Empty() on non-empty list
        TEST(tbl.IsEmpty() == false);

        // test insertion with collision
        TEST(tbl.Insert(_T("foo"), _T("bar")) == true);
        TEST(tbl.Lookup(_T("foo"), val));
        TEST(val.compare(_T("bar")) == 0);

        // test removal
        TEST(tbl.Remove(_T("foo")));

        // make sure it's totally empty (confirms that hash table insertion worked!)
        TEST(tbl.IsEmpty());

        // test another insertion
        TEST(tbl.Insert(_T("a"), _T("bcd")) == false);
        TEST(tbl.Insert(_T("b"), _T("def")) == false);

        TSTRING v1, v2;
        TEST(tbl.Lookup(_T("a"), v1));
        TEST(tbl.Lookup(_T("b"), v2));

        TEST(v1.compare(_T("bcd")) == 0);
        TEST(v2.compare(_T("def")) == 0);

        // remove and test IsEmpty()
        TEST(tbl.Remove(_T("a")));
        TEST(tbl.IsEmpty() == false);

        TEST(tbl.Remove(_T("b")));
        TEST(tbl.IsEmpty() == true);
    }

    d.TraceDebug("PASSED!\n");
}

void RegisterSuite_HashTable()
{
    RegisterTest("HashTable", "Basic 1", HashTest1);
    RegisterTest("HashTable", "Basic 2", HashTest2);
}
