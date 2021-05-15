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
// fconametbl_t.cpp
#include "fco/stdfco.h"
#include "fco/fconametbl.h"
#include "twtest/test.h"

void TestFCONameTbl()
{
    cDebug d("TestFCONameTbl()");

    cFCONameTbl nameTbl;

    cFCONameTblNode* pNode1 = nameTbl.CreateNode(_T("aBc"));
    cFCONameTblNode* pNode2 = nameTbl.CreateNode(_T("Abc"));
    cFCONameTblNode* pNode3 = nameTbl.CreateNode(_T("abc"));
    cFCONameTblNode* pNode4 = nameTbl.CreateNode(_T("def"));
    cFCONameTblNode* pNode5 = nameTbl.CreateNode(_T("dEf"));

    TEST(*pNode1 != *pNode2);
    TEST(*pNode5 != *pNode4);
    TEST(*pNode1->GetLowercaseNode() == *pNode3);
    TEST(*pNode2->GetLowercaseNode() == *pNode3);
    TEST(*pNode3->GetLowercaseNode() == *pNode3);
    TEST(*pNode4->GetLowercaseNode() == *pNode4);
    TEST(*pNode5->GetLowercaseNode() == *pNode4);


    pNode1->Release();
    pNode2->Release();
    pNode3->Release();
    pNode4->Release();
    pNode5->Release();
}

void RegisterSuite_FCONameTbl()
{
    RegisterTest("FCONameTbl", "Basic", TestFCONameTbl);
}
