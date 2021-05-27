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
// generatedb.cpp

#include "stdtripwire.h"
#include "tripwirestrings.h"
#include "generatedb.h"
#include "core/debug.h"
#include "core/error.h"
#include "fco/twfactory.h"
#include "fco/fcospeclist.h"
#include "tw/dbdatasource.h"
#include "fco/genreswitcher.h"
#include "fco/fcopropcalc.h"
#include "fco/fcopropset.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropdisplayer.h"
#include "tripwireutil.h"
#include "fco/fcodatasourceiter.h"

// for verbose output
#include "core/usernotify.h"
#include "fco/fconametranslator.h"

///////////////////////////////////////////////////////////////////////////////
// util_ProcessDir
//
//      this returns true if at least one object was added to the directory.
///////////////////////////////////////////////////////////////////////////////
static void util_ProcessDir(
    cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, iFCOSpec* pSpec, iFCOPropCalc* pPC, iFCOPropDisplayer* pPD)
{
    ASSERT(!dbIter.Done());
    ASSERT(!pIter->Done());
    ASSERT(pIter->CanDescend());
    ASSERT(dbIter.CanDescend());
    if (!pIter->CanDescend())
        return;
    //
    // first, descend into the directory
    //
    pIter->Descend();
    dbIter.Descend();
    //
    // now, iterate through the data source, adding entries to the database ...
    //
    for (pIter->SeekBegin(); !pIter->Done(); pIter->Next())
    {
        //
        // don't continue if the spec says we shouldn't
        //
        if (pSpec->ShouldStopDescent(pIter->GetName()))
        {
            continue;
        }
        // TODO -- do I need to check the db iter to see if the given item already exists?
        //      this should never really happen unless the data source iter is screwed up.
        // TODO -- use a smart reference counted object pointer here to release the object when it
        //      goes out of scope.
        iFCO* pFCO = pIter->CreateFCO();
        if (pFCO)
        {
            cTripwireUtil::CalcProps(pFCO, pSpec, pPC, pPD);
            dbIter.AddFCO(pIter->GetShortName(), pFCO);
            pFCO->Release();
            //
            // descend into this directory if we can...
            //
            if (pIter->CanDescend())
            {
                if (!dbIter.CanDescend())
                {
                    dbIter.AddChildArray();
                }
                TW_UNIQUE_PTR<iFCODataSourceIter> pCopy(pIter->CreateCopy());
                util_ProcessDir(dbIter, pCopy.get(), pSpec, pPC, pPD);
                //
                // if no files were added, remove the child array...
                //
                if (dbIter.CanRemoveChildArray())
                {
                    dbIter.RemoveChildArray();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
void cGenerateDb::Execute(
    const cFCOSpecList& specList, cHierDatabase& db, iFCOPropDisplayer* pPD, cErrorBucket* pBucket, uint32_t flags)
{
    // TODO -- assert the db is empty or clear it out myself!

    TW_UNIQUE_PTR<iFCODataSourceIter> pDSIter(iTWFactory::GetInstance()->CreateDataSourceIter());

    //
    // set up the database's iterator...
    // I assume the current genre is correct...
    //
    cDbDataSourceIter dbIter(&db);

    //
    // set the iterators' error bucket...
    //
    pDSIter->SetErrorBucket(pBucket);
    dbIter.SetErrorBucket(pBucket);

    //
    // this is the object that will calculate all of the properties of the fcos.
    //
    TW_UNIQUE_PTR<iFCOPropCalc> pPC(iTWFactory::GetInstance()->CreatePropCalc());
    pPC->SetErrorBucket(pBucket);
    if (flags & FLAG_ERASE_FOOTPRINTS_GD)
    {
        pPC->SetCalcFlags(iFCOPropCalc::DO_NOT_MODIFY_PROPERTIES);
        dbIter.SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
        pDSIter->SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
    }

    if (flags & FLAG_DIRECT_IO)
    {
        pPC->SetCalcFlags(pPC->GetCalcFlags() | iFCOPropCalc::DIRECT_IO);
    }

    //
    // iterate over all of the specs...
    //
    cFCOSpecListCanonicalIter specIter(specList);
    for (specIter.SeekBegin(); !specIter.Done(); specIter.Next())
    {
        // verbose output
        TW_NOTIFY_VERBOSE(
            _T("%s %s\n"),
            TSS_GetString(cTripwire, tripwire::STR_NOTIFY_PROCESSING).c_str(),
            iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(specIter.Spec()->GetStartPoint()).c_str());
        //
        // have the iterators seek to the appropriate starting point
        //
        pDSIter->SeekToFCO(specIter.Spec()->GetStartPoint(), false); // false means don't generate my peers...
        if (!pDSIter->Done())
        {
            iFCO* pFCO = pDSIter->CreateFCO();
            if (pFCO)
            {
                dbIter.CreatePath(specIter.Spec()->GetStartPoint());
                //
                // ok, now process this directory
                // we need to add the start point here, since the first thing the ProcessDir function does is
                //      Descend().
                //
                cTripwireUtil::CalcProps(pFCO, specIter.Spec(), pPC.get(), pPD);
                dbIter.SetFCOData(pFCO);
                pFCO->Release();
                //
                // if this is a directory, process its children...
                //
                if (pDSIter->CanDescend() && (!specIter.Spec()->ShouldStopDescent(pDSIter->GetName())))
                {
                    // make the child array, if it doesn't exist...
                    //
                    if (!dbIter.CanDescend())
                    {
                        dbIter.AddChildArray();
                    }
                    TW_UNIQUE_PTR<iFCODataSourceIter> pCopy(pDSIter->CreateCopy());
                    util_ProcessDir(dbIter, pCopy.get(), specIter.Spec(), pPC.get(), pPD);
                    //
                    // if no files were added, remove the child array...
                    //
                    if (dbIter.CanRemoveChildArray())
                    {
                        dbIter.RemoveChildArray();
                    }
                }
            }
        }
    }
}
