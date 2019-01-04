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
///////////////////////////////////////////////////////////////////////////////
// fcoreportutil.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdtw.h"
#include <time.h>

#include "fcoreportutil.h"
#include "fcoreport.h"
#include "core/debug.h"
#include "core/fsservices.h"
#include "tw/fcoreport.h"
#include "fco/fcodatasource.h"
#include "fco/iterproxy.h"
#include "fco/fco.h"
#include "headerinfo.h"
#include "core/timeconvert.h"
#include "fco/twfactory.h"
#include "fco/fcopropdisplayer.h"
#include "twutil.h"

//=============================================================================
// class cFCOReportUtil
//=============================================================================

void cFCOReportUtil::CalculateHeaderInfo(cFCOReportHeader& reportHeader,
                                         const TSTRING&    policyFilename,
                                         const TSTRING&    configFilename,
                                         const TSTRING&    dbFilename,
                                         const TSTRING&    commandLineParams,
                                         int64_t           createTime,
                                         int64_t           lastDBUpdateTime)
{
    reportHeader.SetPolicyFilename(policyFilename);
    reportHeader.SetConfigFilename(configFilename);
    reportHeader.SetDBFilename(dbFilename);
    reportHeader.SetSystemName(cTWUtil::GetSystemName());
    reportHeader.SetCommandLineParams(commandLineParams);
    reportHeader.SetIPAddress(cTWUtil::GetIPAddress());
    reportHeader.SetCreator(cTWUtil::GetCurrentUser());
    reportHeader.SetHostID(cTWUtil::GetHostID());
    reportHeader.SetCreationTime(createTime);
    reportHeader.SetLastDBUpdateTime(lastDBUpdateTime);
}

void cFCOReportUtil::FinalizeReport(cFCOReport& rr)
{
    cFCOReportGenreIter genreIter(rr);
    for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
    {
        InitPropDisplayer(genreIter);
    }
}

void cFCOReportUtil::InitPropDisplayer(cFCOReportGenreIter& gi)
{
    if (gi.GetGenreHeader().GetPropDisplayer() == NULL)
        gi.GetGenreHeader().SetPropDisplayer(iTWFactory::GetInstance()->CreatePropDisplayer());

    iFCOPropDisplayer* pPropDisplayer = gi.GetGenreHeader().GetPropDisplayer();
    ASSERT(pPropDisplayer != 0);

    // cycle through all FCOs in report
    // TODO: make a FCO iter class in fcoreport.cpp
    cFCOReportSpecIter ri(gi);
    for (ri.SeekBegin(); !ri.Done(); ri.Next())
    {
        //
        // added
        //

        // get added set
        const iFCOSet* pAddedSet = ri.GetAddedSet();
        ASSERT(pAddedSet != 0);

        // get fco iterator + proxy iter
        const cIterProxy<iFCOIter> pSetIterAdded = pAddedSet->GetIter();
        ASSERT(pSetIterAdded != 0);

        // iterate over all fcos
        for (pSetIterAdded->SeekBegin(); !pSetIterAdded->Done(); pSetIterAdded->Next())
        {
            pPropDisplayer->InitForFCO(pSetIterAdded->FCO());
        }

        //
        // removed
        //

        // get removed set
        const iFCOSet* pRemovedSet = ri.GetRemovedSet();
        ASSERT(pRemovedSet != 0);

        // get fco iterator + proxy iter
        const cIterProxy<iFCOIter> pSetIterRemoved = pRemovedSet->GetIter();
        ASSERT(pSetIterRemoved != 0);

        // iterate over all fcos
        for (pSetIterRemoved->SeekBegin(); !pSetIterRemoved->Done(); pSetIterRemoved->Next())
        {
            pPropDisplayer->InitForFCO(pSetIterRemoved->FCO());
        }

        //
        // changed
        //

        // get changed set iterator (don't need pointer to changed set, just need spec iter)
        cFCOReportChangeIter changedIter(ri);

        // iterate over all changed fcos
        for (changedIter.SeekBegin(); !changedIter.Done(); changedIter.Next())
        {
            // we've already have old FCO info from db
            pPropDisplayer->InitForFCO(changedIter.GetNew());
        }
    }
}
