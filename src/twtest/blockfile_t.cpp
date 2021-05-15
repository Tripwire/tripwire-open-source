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
// blockfile_t
#include "db/stddb.h"
#include "db/blockfile.h"
#include "db/block.h"
#include "core/archive.h"
#include "test.h"
#include "core/debug.h"

void TestBlockFile()
{
    cDebug d("TestBlockFile");

    std::string fileName = TwTestPath("test.bf");
    // truncate the file I am going to use...
    //
    cFileArchive a;
    a.OpenReadWrite(fileName.c_str());
    a.Close();
    //
    // open up the block file...
    //
    cBlockFile bf;
    bf.Open(fileName, 2); // opened up with two pages
#ifdef _BLOCKFILE_DEBUG
    bf.TraceContents();
#endif

    // get a block and write something to it...
    //
    cBlockFile::Block* pB = bf.GetBlock(0);
    TEST(pB);
    static const TCHAR str1[] = _T("Block 1");
    memcpy(pB->GetData(), str1, sizeof(str1));
    pB->SetDirty();
#ifdef _BLOCKFILE_DEBUG
    bf.TraceContents();
#endif

    // get another block...
    //
    pB = bf.CreateBlock();
    TEST(pB);
    static const TCHAR str2[] = _T("Block 2");
    memcpy(pB->GetData(), str2, sizeof(str2));
    pB->SetDirty();
#ifdef _BLOCKFILE_DEBUG
    bf.TraceContents();
#endif

    // get the first block we wrote...
    //
    pB = bf.GetBlock(0);
    TEST(pB);
    *pB->GetData() = _T('F');
#ifdef _BLOCKFILE_DEBUG
    bf.TraceContents();
#endif

    //
    // create a third block -- someone will have to be paged out in order for this to happen
    //
    pB = bf.CreateBlock();
    TEST(pB);
    static const TCHAR str3[] = _T("Block 3");
    memcpy(pB->GetData(), str3, sizeof(str3));
    pB->SetDirty();
#ifdef _BLOCKFILE_DEBUG
    bf.TraceContents();
#endif

    //
    // test the guard bytes...
    /*
    memcpy( pB->GetData() + (cBlockFile::BLOCK_SIZE - 4),   str3, sizeof(str3) );
    memcpy( pB->GetData() - 1,                              str3, sizeof(str3) );
    pB->AssertValid();
    */

    bf.Close();
}

void RegisterSuite_BlockFile()
{
    RegisterTest("BlockFile", "Basic", TestBlockFile);
}
