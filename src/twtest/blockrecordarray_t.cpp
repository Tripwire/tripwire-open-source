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
// blockrecordarray_t.cpp

#include "db/stddb.h"
#include "db/blockrecordarray.h"
#include "test.h"
#include "core/debug.h"
#include "core/error.h"

//TODO: This test needs more comprehensive validity checks
void TestBlockRecordArray()
{
    cDebug d("TestBlockRecordArray");

    std::string fileName = TwTestPath("test2.bf");

    cBlockFile bf;
    bf.Open(fileName, 2, true); // opened up with two pages

    // make sure the file is large enough...
    //
    while (bf.GetNumBlocks() < 2)
    {
        bf.CreateBlock();
    }

    // create the record arrays and associate them with the two blocks...
    //
    cBlockRecordArray ra1(&bf, 0);
    ra1.InitNewBlock();
    cBlockRecordArray ra2(&bf, 1);
    ra2.InitNewBlock();

    TEST(ra1.IsClassValid());
    TEST(ra2.IsClassValid());

    TEST(ra1.Initialized());

    //
    // now, start adding and removing things from the arrays...
    //
    static const char data1[] = "This is data 1";
    static const char data2[] = "And here we have data 2";
    static const char data3[] = "Here is d a t a 3!";
    static const char data4[] = "Three cheers for data 4!";
    ra1.AddItem((int8_t*)data1, sizeof(data1), 1);
    //    TEST(ra1.IsItemValid(1));
    ra1.AddItem((int8_t*)data2, sizeof(data2), 2);
    //    TEST(ra1.IsItemValid(2));
    ra1.AddItem((int8_t*)data3, sizeof(data3), 3);
    //    TEST(ra1.IsItemValid(3));
    ra1.AddItem((int8_t*)data4, sizeof(data4), 4);
    //    TEST(ra1.IsItemValid(4));
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif

    // TODO -- try deleting the second to last and then the last thing from the array to
    //      see if we clean up properly.

    // delete item 2...
    ra1.DeleteItem(1);
    TEST(!ra1.IsItemValid(1));
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif

    // add a new item...
    static const char data5[] = "fffiiivvveee!";
    ra1.AddItem((int8_t*)data5, sizeof(data5), 5);
    //    TEST(ra1.IsItemValid(5));
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif

    // delete the second to last and last items to see if we clean up properly...
    // note that there are four things here at this point.
    ra1.DeleteItem(2);
    TEST(!ra1.IsItemValid(2));
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif
    ra1.DeleteItem(3);
    TEST(!ra1.IsItemValid(3));
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif

    // delete the first item to see if that works ok....
    ra1.DeleteItem(0);
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif

    // add a couple more just for kicks :-)
    static const char data6[] = "We're looking for six";
    static const char data7[] = "All 7s go to heaven";
    ra1.AddItem((int8_t*)data6, sizeof(data6), 6);
    ra1.AddItem((int8_t*)data7, sizeof(data7), 7);
#ifdef _BLOCKFILE_DEBUG
    ra1.TraceContents();
#endif

    TEST(ra1.IsClassValid());

    // Now, we will close the file, reopen it and see if we can read it ok.
    //
    bf.Close();
    bf.Open(fileName, 2, false);
    cBlockRecordArray ra3(&bf, 0);
    ra3.InitForExistingBlock();
    d.TraceDebug("\n------ Tracing out the contents of the first block after being read back in from disk...\n\n");
#ifdef _BLOCKFILE_DEBUG
    ra3.TraceContents();
#endif

    TEST(ra3.Initialized());
    TEST(ra3.IsClassValid());
}

void RegisterSuite_BlockRecordArray()
{
    RegisterTest("BlockRecordArray", "Basic", TestBlockRecordArray);
}
