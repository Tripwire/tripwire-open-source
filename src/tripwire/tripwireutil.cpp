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
// tripwireutil.cpp
#include "stdtripwire.h"
#include "tripwireutil.h"
#include "tw/fcodatabasefile.h"
#include "fco/fcospeclist.h"
#include "fco/fcopropcalc.h"
#include "fco/iterproxy.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"
#include "fco/fcopropdisplayer.h"
#include "fco/fconame.h"
#include "tw/dbdatasource.h"
#include "tripwirestrings.h"

// for verbose output
#include "core/usernotify.h"
#include "fco/twfactory.h"
#include "fco/fconametranslator.h"

///////////////////////////////////////////////////////////////////////////////
// CalcProps
///////////////////////////////////////////////////////////////////////////////
void cTripwireUtil::CalcProps(iFCO* pFCO, const iFCOSpec* pSpec, iFCOPropCalc* pCalc, iFCOPropDisplayer* pPD)
{
    // verbose output...
    TW_NOTIFY_VERBOSE(_T("--- %s%s\n"),
                      TSS_GetString(cTripwire, tripwire::STR_NOTIFY_GEN_SIG).c_str(),
                      iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(pFCO->GetName()).c_str());


    const cFCOPropVector& propsToCalc = pSpec->GetPropVector(pSpec->GetSpecMask(pFCO));
    // set the appropriate property mask for this fco
    pCalc->SetPropVector(propsToCalc);
    // do the calculation
    pFCO->AcceptVisitor(pCalc->GetVisitor());
    //
    // invalidate unneeded properties....
    // I want to invalidate everything that is in the fco but not in the spec...
    //
    cFCOPropVector propsToInvalidate = pFCO->GetPropSet()->GetValidVector();
    propsToInvalidate ^= propsToCalc;
    pFCO->GetPropSet()->InvalidateProps(propsToInvalidate);
    //
    // load this fco's data into the prop displayer
    //
    if (pPD)
        pPD->InitForFCO(pFCO);
}


///////////////////////////////////////////////////////////////////////////////
// RemoveFCOFromDb
///////////////////////////////////////////////////////////////////////////////
bool cTripwireUtil::RemoveFCOFromDb(cFCOName name, cDbDataSourceIter& dbIter)
{
    cDebug d("cTripwireUtil::RemoveFCOFromDb");
    //cDbDataSourceIter dbIter( &db );

    // seek to the fco to be removed..
    //
    dbIter.SeekToFCO(name, false);
    //
    // error if the fco doesn't exist...
    //
    if (dbIter.Done() || (!dbIter.HasFCOData()))
    {
        return false;
    }
    else
    {
        // remove the fco data...
        //
        d.TraceDebug(_T(">>> Removing FCO %s\n"), dbIter.GetName().AsString().c_str());
        dbIter.RemoveFCOData();
        if (!dbIter.CanDescend())
        {
            // note that this is not sufficient to remove all unused nodes from the database...
            //
            d.TraceDebug(_T(">>> Removing Database Node %s\n"), dbIter.GetName().AsString().c_str());
            dbIter.RemoveFCO();
        }
        //
        // get rid of all the empty parents above me...
        // TODO -- is this the right thing to do all the time?
        //
        while (!dbIter.AtRoot())
        {
            cFCOName parentName = dbIter.GetParentName();
            dbIter.Ascend();
            dbIter.SeekTo(parentName.GetShortName());
            ASSERT(!dbIter.Done());
            if ((!dbIter.Done()) && (dbIter.CanRemoveChildArray()))
            {
                dbIter.RemoveChildArray();
                //
                // and, remove this node if there is no fco data...
                //
                if (!dbIter.HasFCOData())
                {
                    d.TraceDebug(_T(">>> Removing Database Node %s\n"), dbIter.GetName().AsString().c_str());
                    dbIter.RemoveFCO();
                }
                else
                    break;
            }
            else
                break;
        }
    }

    return true;
}
