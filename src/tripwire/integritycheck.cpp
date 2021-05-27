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
// integritycheck.cpp
#include "stdtripwire.h"
#include "tripwirestrings.h"
#include "integritycheck.h"
#include "fco/twfactory.h"
#include "tw/fcoreport.h"
#include "core/error.h"
#include "core/debug.h"
#include "fco/fcocompare.h"
#include "tripwireutil.h"
#include "fco/fcopropset.h"
#include "fco/fcospec.h"
#include "fco/fcopropcalc.h"
#include "fco/fcospeclist.h"
#include "core/errorbucket.h"
#include "fco/fcodatasourceiter.h"

// for verbose output
#include "core/usernotify.h"
#include "fco/fconametranslator.h"

//
// TODO -- change the report interface so that it takes an error bucket instead of an error queue and then
//      pass our error bucket to it.
//
// TODO -- we need to pass in the prop displayer and do ??? with it at the appropriate times...
//

///////////////////////////////////////////////////////////////////////////////
// ProcessAddedFCO
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::ProcessAddedFCO(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, bool bRecurse)
{
    ASSERT(pIter && (!pIter->Done()));
    //
    // don't do anything if this FCO is not a part of the spec...
    //
    if (mpCurSpec->ShouldStopDescent(pIter->GetName()))
        return;
    //
    // note that pIter points to the added fco
    //
    iFCO* pFCO = pIter->CreateFCO();
    mnObjectsScanned++;
    mReportIter.SetObjectsScanned(mReportIter.GetObjectsScanned() + 1);
    if (pFCO)
    {
        cTripwireUtil::CalcProps(
            pFCO, mpCurSpec, mpPropCalc, 0); //TODO -- a property displayer should be passed in here.
        mReportIter.GetAddedSet()->Insert(pFCO);
        pFCO->Release();
        //
        // descend here, if we can...
        //
        if (bRecurse)
        {
            if (pIter->CanDescend())
            {
                // note that we don't want to descend into the dbIter, so we will seek it
                // to done...
                //
                while (!dbIter.Done())
                    dbIter.Next();
                TW_UNIQUE_PTR<iFCODataSourceIter> pCopy(pIter->CreateCopy());
                ProcessDir(dbIter, pCopy.get());
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// ProcessRemovedFCO
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::ProcessRemovedFCO(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, bool bRecurse)
{
    ASSERT(!dbIter.Done());
    //
    // don't do anything if this FCO is not a part of the spec...
    //
    if (mpCurSpec->ShouldStopDescent(dbIter.GetName()))
        return;
    //
    // we have to be careful here, since not all db nodes are guarenteed to have fco data associated with them.
    //
    if (dbIter.HasFCOData())
    {
        // add this to the "removed" set
        //
        iFCO* pFCO = dbIter.CreateFCO();
        mReportIter.GetRemovedSet()->Insert(pFCO);
        pFCO->Release();
    }
    // descend if we can...
    //
    if (bRecurse)
    {
        if (dbIter.CanDescend())
        {
            // we don't want pIter to be descended into, so we will pass null as the second param...
            //
            ProcessDir(dbIter, 0);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// ProcessChangedFCO
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::ProcessChangedFCO(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, bool bRecurse)
{
    ASSERT(pIter && (!pIter->Done()));
    ASSERT(!dbIter.Done());
    cDebug d("cIntegrityCheck::ProcessChangedFCO");
    //
    // don't do anything if this FCO is not a part of the spec...
    //
    if (mpCurSpec->ShouldStopDescent(pIter->GetName()))
        return;

    // if the database doesn't have this data, then process this as an add
    //
    if (!dbIter.HasFCOData())
    {
        ProcessAddedFCO(dbIter, pIter, false); // false means not to recurse
    }
    else
    {
        // here we will actually compare the two FCOs
        //
        TW_NOTIFY_VERBOSE(_T("--- %s%s\n"),
                          TSS_GetString(cTripwire, tripwire::STR_NOTIFY_CHECKING).c_str(),
                          iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(pIter->GetName()).c_str());

        iFCO* pNewFCO = pIter->CreateFCO();
        iFCO* pOldFCO = dbIter.CreateFCO();

        mnObjectsScanned++;
        mReportIter.SetObjectsScanned(mReportIter.GetObjectsScanned() + 1);
        //
        // if we can't create the operating system object, treat this as a removal; we will
        // increment the os iterator...
        //
        if (!pNewFCO)
        {
            pIter->Next();
            ProcessRemovedFCO(dbIter, pIter, bRecurse);
            return;
        }

        CompareFCOs(pOldFCO, pNewFCO);

        //-------------------------------------------------------------------------
        // Begin functionality necessary for policy update
        //
        bool bDbFCODirty = false;
        // change the valid vector for the db's fco, if the appropriate flag is set
        //
        if (mFlags & FLAG_INVALIDATE_EXTRA_DB_PROPS)
        {
            cFCOPropVector propsToInvalidate =
                pOldFCO->GetPropSet()->GetValidVector() & mpCurSpec->GetPropVector(pOldFCO);
            propsToInvalidate ^= pOldFCO->GetPropSet()->GetValidVector();
            pOldFCO->GetPropSet()->InvalidateProps(propsToInvalidate);

            bDbFCODirty = true;
        }
        // update the old fco with the new properties...
        //
        if (mFlags & FLAG_SET_NEW_PROPS)
        {
            cFCOPropVector propsToCopy =
                pOldFCO->GetPropSet()->GetValidVector() & pNewFCO->GetPropSet()->GetValidVector();
            propsToCopy ^= pNewFCO->GetPropSet()->GetValidVector();

            pOldFCO->GetPropSet()->CopyProps(pNewFCO->GetPropSet(), propsToCopy);

            bDbFCODirty = true;
        }
        // rewrite the fco to the database, if necessary
        //
        if (bDbFCODirty)
        {
            dbIter.RemoveFCOData();
            dbIter.SetFCOData(pOldFCO);

            TW_NOTIFY_VERBOSE(
                _T("%s%s\n"),
                TSS_GetString(cTripwire, tripwire::STR_NOTIFY_DB_CHANGING).c_str(),
                iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(pOldFCO->GetName()).c_str());
        }
        //
        // end policy update specific code
        //-------------------------------------------------------------------------

        pNewFCO->Release();
        pOldFCO->Release();
    }
    //
    // finally, we need to recurse into our child directories...
    //
    if (bRecurse)
    {
        if (pIter->CanDescend() || dbIter.CanDescend())
        {
            TW_UNIQUE_PTR<iFCODataSourceIter> pCopy(pIter->CreateCopy());
            ProcessDir(dbIter, pCopy.get());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// CompareFCOs
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::CompareFCOs(iFCO* pOldFCO, iFCO* pNewFCO)
{
    cTripwireUtil::CalcProps(
        pNewFCO, mpCurSpec, mpPropCalc, 0); //TODO -- a property displayer should be passed in here.
    //
    // figure out what properties we are comparing...for an explanation of the propsToCheck derivation, see tripwire.cpp line 250.
    //
    cFCOPropVector propsToCheck;
    if (mFlags & FLAG_COMPARE_VALID_PROPS_ONLY)
    {
        propsToCheck = pOldFCO->GetPropSet()->GetValidVector() & pNewFCO->GetPropSet()->GetValidVector();
    }
    else
    {
        propsToCheck = pOldFCO->GetPropSet()->GetValidVector() | pNewFCO->GetPropSet()->GetValidVector();
    }
    propsToCheck &= mpCurSpec->GetPropVector(pNewFCO);
    //
    // trim off the loose dir stuff...
    //
    if ((pOldFCO->GetCaps() & iFCO::CAP_CAN_HAVE_CHILDREN) && (pNewFCO->GetCaps() & iFCO::CAP_CAN_HAVE_CHILDREN))
    {
        cFCOPropVector tmp = (propsToCheck ^ mLooseDirProps);
        propsToCheck &= tmp;
    }
    //
    // construct the compare object and actually do the compare
    //
    cFCOCompare compareObj(propsToCheck);
    uint32_t    result = compareObj.Compare(pOldFCO, pNewFCO);

    if ((result & cFCOCompare::PROPS_UNEQUAL) || (result & cFCOCompare::PROPS_NOT_ALL_VALID))
    {
        // this is a violation...
        //
        mReport.AddChangedFCO(
            mReportIter, pOldFCO, pNewFCO, compareObj.GetUnequalProps() | compareObj.GetInvalidProps());
    }
}

///////////////////////////////////////////////////////////////////////////////
// ProcessDir
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::ProcessDir(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter)
{
    cDebug d("cIntegrityCheck::ProcessDir");

    // if either iterator is done, or if the data source iter is NULL, that indicates that the tree
    // has stopped for that iterator, and all FCOs encountered by the other iter are all adds or removals,
    // as appropriate.
    //
    // the first thing we will do is descend, if possible, and if not possible, set them to done().
    //
    if (pIter)
    {
        if (pIter->CanDescend())
        {
            pIter->Descend();
        }
        else
        {
            pIter = 0;
        }
    }
    if (!dbIter.Done())
    {
        if (dbIter.CanDescend())
        {
            dbIter.Descend();
        }
        else
        {
            while (!dbIter.Done())
                dbIter.Next();
        }
    }
    // if they are both done, then we are done dealin'!
    //
    if (((!pIter) || (pIter->Done())) && (dbIter.Done()))
    {
        //TODO -- what else do I need to do here?
        return;
    }

#ifdef DEBUG
    if (dbIter.Done())
    {
        d.TraceDebug("Processing directory %s\n", (pIter ? pIter->GetParentName().AsString().c_str() : _T("[none]")));
    }
    else
    {
        d.TraceDebug("Processing directory %s\n", dbIter.GetParentName().AsString().c_str());
    }
#endif

    //
    // now, process the entries in this directory...
    //
    while (true)
    {
        // this just cleans up some logic statements below...
        //
        if (pIter && pIter->Done())
            pIter = NULL;

        // if they are both done, then we are finished...
        //
        if (dbIter.Done() && (!pIter))
            break;

        if (dbIter.Done())
        {
            ASSERT(pIter != 0);
            if (pIter)
            {
                d.TraceDetail("Examining <done> and %s\n", pIter->GetName().AsString().c_str());
                //
                // these are all new entries, add them to the "Added" set...
                //
                ProcessAddedFCO(dbIter, pIter);
                pIter->Next();
            }
        }
        else if (!pIter)
        {
            ASSERT(!dbIter.Done());
            d.TraceDetail(_T("--Examining %s and <done>\n"), dbIter.GetName().AsString().c_str());
            //
            // these are all removed objects.
            //
            ProcessRemovedFCO(dbIter, pIter);
            dbIter.Next();
        }
        else
        {
            d.TraceDetail(_T("--Examining %s and %s\n"),
                          dbIter.GetName().AsString().c_str(),
                          pIter->GetName().AsString().c_str());

            ASSERT(pIter->GetParentName() == dbIter.GetParentName());
            //
            // get the relationship between the current nodes of the two iterators...
            //
            iFCODataSourceIter::Relationship rel = dbIter.GetRelationship(*pIter);
            //
            // if rel == LT, then the old has something the new doesn't (ie -- a removal!)
            // if rel == GT, then there is an addition
            //
            if (rel == iFCODataSourceIter::REL_LT)
            {
                ProcessRemovedFCO(dbIter, pIter);
                dbIter.Next();
            }
            else if (rel == iFCODataSourceIter::REL_GT)
            {
                ProcessAddedFCO(dbIter, pIter);
                pIter->Next();
            }
            else
            {
                // we actually have to compare the FCOs at this point...
                // NOTE -- if the db iter has no data, then this is an add again.
                //
                ProcessChangedFCO(dbIter, pIter);

                dbIter.Next();
                pIter->Next();
            }

            ASSERT(pIter->GetParentName() == dbIter.GetParentName());
        }
    }
    d.TraceDebug("Done Processing Directory\n");
}

///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
cIntegrityCheck::cIntegrityCheck(
    cGenre::Genre genreNum, const cFCOSpecList& specList, cHierDatabase& db, cFCOReport& report, cErrorBucket* pBucket)
    : mGenre(genreNum),
      mSpecList(specList),
      mDb(db),
      mReport(report),
      mpPropCalc(iTWFactory::GetInstance()->CreatePropCalc()),
      mpCurSpec(0),
      mReportIter(report, genreNum),
      mFlags(0),
      mnObjectsScanned(0)
{
    // mBucket is a pass-thru bucket; its only job is to pass the errors onto its child.
    //
    mBucket.SetChild(pBucket);
    mpPropCalc->SetErrorBucket(&mBucket);
}


///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
cIntegrityCheck::~cIntegrityCheck()
{
    delete mpPropCalc;
}


///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::Execute(uint32_t flags)
{
    mFlags = flags;
    // create the data source iterator
    //
    TW_UNIQUE_PTR<iFCODataSourceIter> pDSIter(iTWFactory::GetInstance()->CreateDataSourceIter());
    //
    // set up the database's iterator...
    // I assume the current genre is correct...
    //
    cDbDataSourceIter dbIter(&mDb);

    //
    // set the iterator's error bucket...
    //
    pDSIter->SetErrorBucket(&mBucket);
    dbIter.SetErrorBucket(&mBucket);

    //
    // set up loose directories; note that mLooseDirProps represents properties that will
    // be removed from the ones used to compare, so if loose dirs is not specified, we can set this
    // to the empty vector.
    //
    if (flags & FLAG_LOOSE_DIR)
    {
        mLooseDirProps = iTWFactory::GetInstance()->GetLooseDirMask();
    }

    //
    // set up flags for the property calculator and iterators
    //
    if (flags & FLAG_ERASE_FOOTPRINTS_IC)
    {
        mpPropCalc->SetCalcFlags(iFCOPropCalc::DO_NOT_MODIFY_PROPERTIES);
        dbIter.SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
        pDSIter->SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
    }

    if (flags & FLAG_DIRECT_IO)
    {
        mpPropCalc->SetCalcFlags(mpPropCalc->GetCalcFlags() | iFCOPropCalc::DIRECT_IO);
    }

    //
    // iterate over all of the specs...
    //
    cFCOSpecListCanonicalIter specIter(mSpecList);
    for (specIter.SeekBegin(); !specIter.Done(); specIter.Next())
    {
        mpCurSpec = specIter.Spec();

        // verbose output
        TW_NOTIFY_VERBOSE(
            _T("%s%s\n"),
            TSS_GetString(cTripwire, tripwire::STR_NOTIFY_CHECKING_RULE).c_str(),
            iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(mpCurSpec->GetStartPoint()).c_str());
        //
        // add the spec to the report...
        //
        mReport.AddSpec(mGenre, mpCurSpec, specIter.Attr(), &mReportIter);
        ASSERT(!mReportIter.Done());
        //
        // associate the error bucket to this current spec in the report
        //
        mReportIter.GetErrorQueue()->SetChild(mBucket.GetChild());
        mBucket.SetChild(mReportIter.GetErrorQueue());
        //
        // seek each iterator to the appropriate starting point...
        //
        dbIter.SeekToFCO(mpCurSpec->GetStartPoint(), false); // false means not to create my peers
        pDSIter->SeekToFCO(mpCurSpec->GetStartPoint(), false);
        //
        // integrity check the start point; note that the ProcessXXX functions will
        //      handle recursing into the subdirectories and stopping when the spec says to
        //
        if (dbIter.Done() && pDSIter->Done())
        {
            // nothing to do here...
            ;
        }
        else if (pDSIter->Done())
        {
            // removed object...
            ProcessRemovedFCO(dbIter, pDSIter.get());
        }
        else if (dbIter.Done())
        {
            // added object...
            ProcessAddedFCO(dbIter, pDSIter.get());
        }
        else
        {
            // possible changed fco
            ProcessChangedFCO(dbIter, pDSIter.get());
        }

        // dissociate the report error bucket and mine...
        //
        mBucket.SetChild(mReportIter.GetErrorQueue()->GetChild());
        mReportIter.GetErrorQueue()->SetChild(0);
    }
}

///////////////////////////////////////////////////////////////////////////////
// ExecuteOnObjectList
///////////////////////////////////////////////////////////////////////////////
void cIntegrityCheck::ExecuteOnObjectList(const std::list<cFCOName>& fcoNames, uint32_t flags)
{
    iFCONameTranslator* pTrans = iTWFactory::GetInstance()->GetNameTranslator();
    //
    // create the data source iterator
    //
    TW_UNIQUE_PTR<iFCODataSourceIter> pDSIter(iTWFactory::GetInstance()->CreateDataSourceIter());
    //
    // set up the database's iterator...
    // I assume the current genre is correct...
    //
    cDbDataSourceIter         dbIter(&mDb);
    cFCOSpecListCanonicalIter specIter(mSpecList);

    //
    // set the iterators' error bucket...
    //
    pDSIter->SetErrorBucket(&mBucket);
    dbIter.SetErrorBucket(&mBucket);

    //
    // set up flags for the property calculator and iterators
    //
    if (flags & FLAG_ERASE_FOOTPRINTS_IC)
    {
        mpPropCalc->SetCalcFlags(iFCOPropCalc::DO_NOT_MODIFY_PROPERTIES);
        dbIter.SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
        pDSIter->SetIterFlags(iFCODataSourceIter::DO_NOT_MODIFY_OBJECTS);
    }

    if (flags & FLAG_DIRECT_IO)
    {
        mpPropCalc->SetCalcFlags(mpPropCalc->GetCalcFlags() | iFCOPropCalc::DIRECT_IO);
    }

    //
    // iterate over all the objects to integrity check..
    //
    std::list<cFCOName>::const_iterator it;
    for (it = fcoNames.begin(); it != fcoNames.end(); ++it)
    {
        TW_NOTIFY_VERBOSE(_T("%s%s\n"),
                          TSS_GetString(cTripwire, tripwire::STR_NOTIFY_CHECKING).c_str(),
                          pTrans->ToStringDisplay(*it).c_str());
        //
        // figure out which spec this fco belongs in...
        //
        for (specIter.SeekBegin(); !specIter.Done(); specIter.Next())
        {
            if (specIter.Spec()->SpecContainsFCO(*it))
                break;
        }
        // error if we found no spec.
        //
        if (specIter.Done())
        {
            mBucket.AddError(eICFCONotInSpec(pTrans->ToStringDisplay(*it)));
            mReport.GetErrorQueue()->cErrorBucket::AddError(eICFCONotInSpec(pTrans->ToStringDisplay(*it)));
            continue;
        }
        mpCurSpec = specIter.Spec();
        //
        // add this spec to the report if it is not there
        // and seek to the spec
        //
        if (!mReportIter.SeekToSpec(mpCurSpec))
        {
            mReport.AddSpec(mGenre, mpCurSpec, specIter.Attr(), &mReportIter);
        }
        // insert the report error bucket between mpErrorBucket and its children...
        //
        mReportIter.GetErrorQueue()->SetChild(mBucket.GetChild());
        mBucket.SetChild(mReportIter.GetErrorQueue());
        //
        // create the fco in the database...
        //
        dbIter.SeekToFCO(*it, false);
        if (dbIter.Done() || (!dbIter.HasFCOData()))
        {
            // fco not found in the db!
            //
            mBucket.AddError(eICFCONotInDb(pTrans->ToStringDisplay(*it)));
        }
        else
        {
            iFCO* pOldFCO = dbIter.CreateFCO();
            //
            // create the fco from the data source
            //
            pDSIter->SeekToFCO(*it, false);
            if (!pDSIter->Done())
            {
                iFCO* pNewFCO = pDSIter->CreateFCO();
                mnObjectsScanned++;
                mReportIter.SetObjectsScanned(mReportIter.GetObjectsScanned() + 1);

                if (pNewFCO)
                {
                    CompareFCOs(pOldFCO, pNewFCO);
                    pNewFCO->Release();
                }
                else
                {
                    mBucket.AddError(eICFCOCreate(pTrans->ToStringDisplay(*it)));
                }
            }
            pOldFCO->Release();
        }
        // dissociate the report error bucket and mine...
        //
        mBucket.SetChild(mReportIter.GetErrorQueue()->GetChild());
        mReportIter.GetErrorQueue()->SetChild(0);
    }
}
