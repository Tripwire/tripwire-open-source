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
// policyupdate.cpp
#include "stdtripwire.h"
#include "policyupdate.h"
#include "tw/fcoreport.h"
#include "core/errorbucketimpl.h"
#include "integritycheck.h"
#include "updatedb.h"
#include "core/usernotify.h"
#include "fco/iterproxy.h"
#include "fco/fcopropset.h"
#include "tripwireutil.h"
#include "tripwirestrings.h"
#include "fco/twfactory.h"
#include "fco/fconametranslator.h"
#include "fco/genreswitcher.h"

///////////////////////////////////////////////////////////////////////////////
// util_FCOInSpecList -- given a spec list and an fco name, return true if the
//      name belongs in one of the specs
///////////////////////////////////////////////////////////////////////////////
static bool util_FCOInSpecList(const cFCOSpecList& specList, const cFCOName& name)
{
    cFCOSpecListCanonicalIter iter(specList);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        if (iter.Spec()->SpecContainsFCO(name))
            return true;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////
// util_PruneExtraObjects -- checks every object in the database against the
//      provided spec set and adds them to the vector if they don't fall under any specs.
///////////////////////////////////////////////////////////////////////////////
static void util_PruneExtraObjects(cDbDataSourceIter iter, cFCOSpecListCanonicalIter specIter, bool bFirst = true)
{
    if (!bFirst)
    {
        iter.Descend();
    }


    for (iter.SeekBegin(); !iter.Done(); /*iter.Next()*/)
    {
        // process our children first...
        //
        if (iter.CanDescend())
        {
            util_PruneExtraObjects(iter, specIter, false);
        }


        if (iter.HasFCOData())
        {
            // figure out if this fco fits in the new policy
            //
            bool bFoundSpec = false;
            for (specIter.SeekBegin(); !specIter.Done(); specIter.Next())
            {
                if (specIter.Spec()->SpecContainsFCO(iter.GetName()))
                {
                    // this doesn't belong in the new db; remove it!
                    //
                    bFoundSpec = true;
                    break;
                }
            }
            if (!bFoundSpec)
            {
                TW_NOTIFY_VERBOSE(
                    _T("--- %s%s\n"),
                    TSS_GetString(cTripwire, tripwire::STR_NOTIFY_DB_REMOVING).c_str(),
                    iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(iter.GetName()).c_str());
                iter.RemoveFCOData();
            }
        }
        //
        // remove the infrastructure, if appropriate...
        // this is also where the iterator incrementing occurs, so we need to
        // be careful with the Next()s
        //
        if (!iter.HasFCOData())
        {
            if (iter.CanDescend() && iter.CanRemoveChildArray())
                iter.RemoveChildArray();
            if (!iter.CanDescend())
                iter.RemoveFCO();
            else
                iter.Next();
        }
        else
            iter.Next();
    }
}


///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cPolicyUpdate::cPolicyUpdate(cGenre::Genre       genreNum,
                             const cFCOSpecList& oldPolicy,
                             const cFCOSpecList& newPolicy,
                             cHierDatabase&      db,
                             cErrorBucket*       pBucket)
    : mOldPolicy(oldPolicy), mNewPolicy(newPolicy), mDb(db), mGenre(genreNum), mpBucket(pBucket)
{
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
bool cPolicyUpdate::Execute(uint32_t flags) // throw (eError)
{
    // here is my current idea for the algorithm: first, do an integrity check with the new policy on the database and
    // a special flag passed to Execute() to modify what properties are checked. Then, take the resulting
    // report print to the user its contents, minus any violations that don't make sense to report (see my EPS for
    // details on what doesn't get displayed) Finally, do a database update with the full report, making sure that
    // properties that were from the policy are also removed from existing FCOs in the database ( I might need to
    // add a parameter to DbUpdate to accomodate that).
    //

    // first, do the integrity check with the new database...
    //
    TW_NOTIFY_NORMAL(TSS_GetString(cTripwire, tripwire::STR_PU_PROCESSING_GENRE).c_str(),
                     cGenreSwitcher::GetInstance()->GenreToString(cGenreSwitcher::GetInstance()->CurrentGenre(), true));
    TW_NOTIFY_NORMAL(TSS_GetString(cTripwire, tripwire::STR_PU_INTEGRITY_CHECK).c_str());
    //
    iFCONameTranslator* pTrans  = iTWFactory::GetInstance()->GetNameTranslator();
    bool                bResult = true; // begin by assuming success
    cFCOReport          report;
    cIntegrityCheck     ic(mGenre, mNewPolicy, mDb, report, mpBucket);

    //
    // set up flags for the property calculator and iterators
    //
    uint32_t icFlags = cIntegrityCheck::FLAG_COMPARE_VALID_PROPS_ONLY | cIntegrityCheck::FLAG_INVALIDATE_EXTRA_DB_PROPS |
                     cIntegrityCheck::FLAG_SET_NEW_PROPS;

    if (flags & FLAG_ERASE_FOOTPRINTS_PU)
    {
        icFlags |= cIntegrityCheck::FLAG_ERASE_FOOTPRINTS_IC;
    }

    if (flags & FLAG_DIRECT_IO)
    {
        icFlags |= cIntegrityCheck::FLAG_DIRECT_IO;
    }

    ic.Execute(icFlags);
    //TODO-- the second flag I just added probably makes the flag to cUpdateDb::Execute() unnecessary;
    //      I should probably remove it.


    //
    // Note that I will probably end up making two passes through the report (one for reporting to the user and one for
    //      the db update). I don't think there is a clean way to do this in a single report pass, other than rewriting
    //      db update, which I am not going to do.
    //
    cFCOReportSpecIter specIter(report, mGenre);
    for (specIter.SeekBegin(); !specIter.Done(); specIter.Next())
    {
        // notify the user of what we are doing...
        TW_NOTIFY_VERBOSE(_T("%s%s\n"),
                          TSS_GetString(cTripwire, tripwire::STR_NOTIFY_PROCESSING).c_str(),
                          pTrans->ToStringDisplay(specIter.GetSpec()->GetStartPoint()).c_str());

        //
        // report objects in the added set should only be reported if they fall in a spec
        // in the old policy
        //
        iFCOSet*                   pAddSet = specIter.GetAddedSet();
        const cIterProxy<iFCOIter> pIter   = pAddSet->GetIter();
        for (pIter->SeekBegin(); !pIter->Done(); pIter->Next())
        {
            if (util_FCOInSpecList(mOldPolicy, pIter->FCO()->GetName()))
            {
                // this is an error that should be reported to the user.
                ePolicyUpdateAddedFCO e(pTrans->ToStringDisplay(pIter->FCO()->GetName()));
                if ((flags & FLAG_SECURE_MODE) == 0)
                    e.SetFlags(eError::NON_FATAL);
                else
                    e.SetFlags(eError::SUPRESS_THIRD_MSG);
                mpBucket->AddError(e);
                bResult = false;
            }
        }
        //
        // objects in the removed set should only be reported if they fall in a spec in the
        // new policy..
        //
        iFCOSet*                   pRmSet  = specIter.GetRemovedSet();
        const cIterProxy<iFCOIter> pRmIter = pRmSet->GetIter();
        for (pRmIter->SeekBegin(); !pRmIter->Done(); pRmIter->Next())
        {
            if (util_FCOInSpecList(mNewPolicy, pRmIter->FCO()->GetName()))
            {
                // this is an error that should be reported to the user.
                ePolicyUpdateRemovedFCO e(pTrans->ToStringDisplay(pRmIter->FCO()->GetName()));

                if ((flags & FLAG_SECURE_MODE) == 0)
                    e.SetFlags(eError::NON_FATAL);
                else
                    e.SetFlags(eError::SUPRESS_THIRD_MSG);
                mpBucket->AddError(e);
                bResult = false;
            }
        }
        //
        // every object in the changed set should be document....
        //
        cFCOReportChangeIter changeIter(specIter);
        for (changeIter.SeekBegin(); !changeIter.Done(); changeIter.Next())
        {
            //----------------------------------------------------------------
            // I need to construct a string that contains all of the
            // property names that have changed.
            //----------------------------------------------------------------
            TSTRING badPropStr = TSS_GetString(cTripwire, tripwire::STR_PU_BAD_PROPS);

            badPropStr += pTrans->ToStringDisplay(changeIter.GetOld()->GetName());
            const cFCOPropVector& changeVector = changeIter.GetChangeVector();
            for (int i = 0; i < changeVector.GetSize(); i++)
            {
                if (changeVector.ContainsItem(i))
                {
                    badPropStr += _T("\n> ");
                    badPropStr += changeIter.GetOld()->GetPropSet()->GetPropName(i);
                }
            }

            // add this to the error bucket
            ePolicyUpdateChangedFCO e(badPropStr);
            if ((flags & FLAG_SECURE_MODE) == 0)
                e.SetFlags(eError::NON_FATAL);
            else
                e.SetFlags(eError::SUPRESS_THIRD_MSG);
            mpBucket->AddError(e);
            bResult = false;
        }
    }
    //
    // now, we will update the database with everything in the report...
    // TODO -- don't do this if the secure mode flag was passed in
    //
    TW_NOTIFY_NORMAL(TSS_GetString(cTripwire, tripwire::STR_PU_UPDATE_DB).c_str());
    //
    cUpdateDb update(mDb, report, mpBucket);

    uint32_t updateDBFlags = cUpdateDb::FLAG_REPLACE_PROPS;
    if (flags & FLAG_ERASE_FOOTPRINTS_PU)
    {
        updateDBFlags |= cUpdateDb::FLAG_ERASE_FOOTPRINTS_UD;
    }

    update.Execute(updateDBFlags);

    // the last thing that we have to do is to remove everything that is still
    // in the database that does not belong in the new database (ie -- does not fall under any
    // new rules)
    //
    // TODO -- is there any way to do this that does not involve iterating over the entire database?
    // TODO -- I should probably write a general-purpose database iterator class to do this...
    //

    TW_NOTIFY_NORMAL(TSS_GetString(cTripwire, tripwire::STR_PU_PRUNING).c_str());
    //
    cDbDataSourceIter i(&mDb);
    i.SetErrorBucket(mpBucket);

    if (flags & FLAG_ERASE_FOOTPRINTS_PU)
    {
        i.SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
    }

    const cFCOSpecListCanonicalIter newPolIter(mNewPolicy);
    util_PruneExtraObjects(i, newPolIter);

    return bResult;
}
