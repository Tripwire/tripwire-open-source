//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
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
// updatedb.cpp
#include "stdtripwire.h"
#include "tripwirestrings.h"
#include "updatedb.h"
#include "db/hierdatabase.h"
#include "tw/dbdatasource.h"
#include "core/errorbucket.h"
#include "tw/fcoreport.h"
#include "core/debug.h"
#include "fco/genreswitcher.h"
#include "fco/iterproxy.h"
#include "fco/fcocompare.h"
#include "fco/fcopropset.h"
#include "fco/fco.h"
#include "core/usernotify.h"
#include "tripwireutil.h"
#include "fco/twfactory.h"
#include "fco/fconametranslator.h"

//=============================================================================
// class cUpdateDb
//=============================================================================

cUpdateDb::cUpdateDb(cHierDatabase& db, cFCOReport& report, cErrorBucket* pBucket)
    : mDb(db), mReport(report), mpBucket(pBucket)
{
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
bool cUpdateDb::Execute(uint32_t flags)
{
    cDebug d("cUpdateDb::Execute");
    bool   bResult = true;

    // I will assume that the current genre is correct...
    //
    cFCOReportSpecIter specIter(mReport, cGenreSwitcher::GetInstance()->CurrentGenre());
    cDbDataSourceIter  dbIter(&mDb);
    dbIter.SetErrorBucket(mpBucket);
    iFCONameTranslator* pTrans = iTWFactory::GetInstance()->GetNameTranslator();

    //
    // set flags
    //
    if (flags & FLAG_ERASE_FOOTPRINTS_UD)
    {
        dbIter.SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
    }

    //
    // iterate through all the specs...
    //
    for (specIter.SeekBegin(); !specIter.Done(); specIter.Next())
    {
        // skip this spec if there is nothing to update...
        //
        if ((specIter.GetAddedSet()->IsEmpty()) && (specIter.GetRemovedSet()->IsEmpty()) &&
            (specIter.GetNumChanged() == 0))
        {
            continue;
        }
        //
        // Add all of the "Added" files...
        //
        cIterProxy<iFCOIter> fcoIter(specIter.GetAddedSet()->GetIter());
        for (fcoIter->SeekBegin(); !fcoIter->Done(); fcoIter->Next())
        {
            TW_NOTIFY_VERBOSE(_T("%s%s\n"),
                              TSS_GetString(cTripwire, tripwire::STR_NOTIFY_DB_ADDING).c_str(),
                              pTrans->ToStringDisplay(fcoIter->FCO()->GetName()).c_str());
            //
            // seek to the new FCO, creating the path if necessary..
            //
            dbIter.CreatePath(fcoIter->FCO()->GetName());
            //
            // report an error if this already exists in the db
            //
            if (dbIter.HasFCOData())
            {
                d.TraceError(_T("Report says to add fco %s that already exists in the db!\n"),
                             fcoIter->FCO()->GetName().AsString().c_str());
                if (mpBucket)
                    mpBucket->AddError(eUpdateDbAddedFCO(pTrans->ToStringDisplay(fcoIter->FCO()->GetName()),
                                                         eError::SUPRESS_THIRD_MSG));
                bResult = false;
            }
            else
            {
                // add the fco to the database...
                //
                d.TraceDebug(_T(">>> Adding FCO %s\n"), fcoIter->FCO()->GetName().AsString().c_str());
                dbIter.SetFCOData(fcoIter->FCO());
            }
        }
        //
        // remove all of the "Removed" files
        //
        cIterProxy<iFCOIter> rmIter(specIter.GetRemovedSet()->GetIter());
        for (rmIter->SeekBegin(); !rmIter->Done(); rmIter->Next())
        {
            TW_NOTIFY_VERBOSE(_T("%s%s\n"),
                              TSS_GetString(cTripwire, tripwire::STR_NOTIFY_DB_REMOVING).c_str(),
                              pTrans->ToStringDisplay(rmIter->FCO()->GetName()).c_str());


            if (!cTripwireUtil::RemoveFCOFromDb(rmIter->FCO()->GetName(), dbIter))
            {
                d.TraceError(_T("Report says to remove fco %s that doesn't exist in the db!\n"),
                             rmIter->FCO()->GetName().AsString().c_str());
                if (mpBucket)
                    mpBucket->AddError(eUpdateDbRemovedFCO(pTrans->ToStringDisplay(rmIter->FCO()->GetName()),
                                                           eError::SUPRESS_THIRD_MSG));
                bResult = false;
            }
        }
        //
        // change all the "Changed" fcos....
        //
        cFCOCompare          compareObj;
        cFCOReportChangeIter changeIter(specIter);
        for (changeIter.SeekBegin(); !changeIter.Done(); changeIter.Next())
        {
            TW_NOTIFY_VERBOSE(_T("%s%s\n"),
                              TSS_GetString(cTripwire, tripwire::STR_NOTIFY_DB_CHANGING).c_str(),
                              pTrans->ToStringDisplay(changeIter.GetOld()->GetName()).c_str());

            d.TraceDebug(">>> Changing FCO %s\n", changeIter.GetOld()->GetName().AsString().c_str());
            //
            // error if the fco isn't in the database
            //
            dbIter.SeekToFCO(changeIter.GetOld()->GetName());
            if (dbIter.Done() || (!dbIter.HasFCOData()))
            {
                d.TraceError("Report says to change fco %s but it doesn't exist in the db!\n",
                             changeIter.GetOld()->GetName().AsString().c_str());
                if (mpBucket)
                    mpBucket->AddError(eUpdateDbRemovedFCO(pTrans->ToStringDisplay(changeIter.GetOld()->GetName()),
                                                           eError::SUPRESS_THIRD_MSG));
                bResult = false;
                continue;
            }
            // make sure that the fco properties match the "old" value in the report
            //
            iFCO* pDbFCO = dbIter.CreateFCO();
            compareObj.SetPropsToCmp(changeIter.GetOld()->GetPropSet()->GetValidVector());
            if ((compareObj.Compare(changeIter.GetOld(), pDbFCO) & cFCOCompare::EQUAL) == 0)
            {
                d.TraceError("FCO %s in report doesn't match current db values for properties!\n",
                             changeIter.GetOld()->GetName().AsString().c_str());
                changeIter.GetOld()->TraceContents(cDebug::D_ERROR);
                pDbFCO->TraceContents(cDebug::D_ERROR);
                if (mpBucket)
                    mpBucket->AddError(
                        eUpdateDbChangedFCO(pTrans->ToStringDisplay(pDbFCO->GetName()), eError::SUPRESS_THIRD_MSG));
                pDbFCO->Release();
                bResult = false;
                continue;
            }
            //
            // ok, we can finally update the database...
            // TODO -- I only think that we should be copying all of the properties if FLAG_REPLACE_PROPS
            //      is set. For example, what if they ignore a property? This bears some investigation...
            //dbFcoIter->FCO()->GetPropSet()->CopyProps(changeIter.GetNew()->GetPropSet(), changeIter.GetNew()->GetPropSet()->GetValidVector());
            //
            dbIter.RemoveFCOData();
            if (flags & FLAG_REPLACE_PROPS)
            {
                // replace the old fco's data with the new data
                //
                dbIter.SetFCOData(changeIter.GetNew());
            }
            else
            {
                iFCO* pNewFCO = changeIter.GetNew()->Clone();
                //
                // we need to fold in all properties that are in the old FCO but not valid in the new one
                // (in case an integrity check ignored certain properties, we need to keep the old values in the
                // database) -- 5 feb 99 mdb
                //
                cFCOPropVector propsToCopy =
                    pDbFCO->GetPropSet()->GetValidVector() ^ pNewFCO->GetPropSet()->GetValidVector();
                propsToCopy &= pDbFCO->GetPropSet()->GetValidVector();
                pNewFCO->GetPropSet()->CopyProps(pDbFCO->GetPropSet(), propsToCopy);

                dbIter.SetFCOData(pNewFCO);

                pNewFCO->Release();
            }
            pDbFCO->Release();
        }
    }


    return bResult;
}
