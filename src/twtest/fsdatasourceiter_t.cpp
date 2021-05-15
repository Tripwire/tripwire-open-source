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
// fsdatasourceiter_t
#include "fs/stdfs.h"
#include "fs/fsdatasourceiter.h"
#include "core/fsservices.h"
#include "core/debug.h"
#include "twtest/test.h"
#include "fco/fco.h"
#include "fco/twfactory.h"
#include "core/errorbucketimpl.h"

namespace
{
int num_processed = 0;

void util_ProcessDir(iFCODataSourceIter* pIter)
{
    TEST(!pIter->Done());
    TEST(pIter->CanDescend());
    if (!pIter->CanDescend())
        return;

    pIter->Descend();

    for (pIter->SeekBegin(); !pIter->Done(); pIter->Next())
    {
        iFCO* pFCO = pIter->CreateFCO();
        if (pFCO)
        {
            num_processed++;
            pFCO->Release();

            if (pIter->CanDescend())
            {
                TW_UNIQUE_PTR<iFCODataSourceIter> pCopy(pIter->CreateCopy());
                util_ProcessDir(pCopy.get());
            }
        }
        else
        {
            fail("CreateFCO failure");
        }
    }
}

void util_ProcessDir(const cFCOName& name)
{
    //Create a cFSDataSourceIter the same way we do in DB gen / IC
    TW_UNIQUE_PTR<iFCODataSourceIter> pDSIter(iTWFactory::GetInstance()->CreateDataSourceIter());

    cErrorQueue errorQueue;
    pDSIter->SetErrorBucket(&errorQueue);

    pDSIter->SeekToFCO(name, false); // false means don't generate my peers...
    if (!pDSIter->Done())
    {
        iFCO* pFCO = pDSIter->CreateFCO();
        if (pFCO)
        {
            num_processed++;
            pFCO->Release();

            if (pDSIter->CanDescend())
            {
                TW_UNIQUE_PTR<iFCODataSourceIter> pCopy(pDSIter->CreateCopy());
                util_ProcessDir(pCopy.get());
            }
        }
        else
        {
            fail("CreateFCO failure");
        }
    }

    TEST(0 == errorQueue.GetNumErrors());
    TEST(0 < num_processed);
}


} // namespace


//////////////////////////////////////

void TestFSDataSourceIter()
{
    cDebug d("TestFSDataSourceIter");


    cFCOName base(TwTestDir());
    util_ProcessDir(base);
}

void RegisterSuite_FSDataSourceIter()
{
    RegisterTest("FSDataSourceIter", "Basic", TestFSDataSourceIter);
}
