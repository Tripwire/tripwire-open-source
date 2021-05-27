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
// integritycheck.h
//
#ifndef __INTEGRITYCHECK_H
#define __INTEGRITYCHECK_H

#ifndef __DBDATASOURCE_H
#include "tw/dbdatasource.h"
#endif
#ifndef __FCOREPORT_H
#include "tw/fcoreport.h"
#endif
#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif
#ifndef __ERRORBUCKETIMPL_H
#include "core/errorbucketimpl.h"
#endif
#ifndef __FCOPROPVECTOR_H
#include "fco/fcopropvector.h"
#endif

class cFCOSpecList;
class cHierDatabase;
class cFCOReport;
class cErrorBucket;
class iFCODataSourceIter;
class iFCOSpec;
class cFCOReportSpecIter;
class iFCOPropCalc;

TSS_EXCEPTION(eIC, eError);
TSS_EXCEPTION(eICFCONotInSpec, eIC);
TSS_EXCEPTION(eICFCONotInDb, eIC);
TSS_EXCEPTION(eICFCOCreate, eIC);
TSS_EXCEPTION(eICBadPol, eIC);
TSS_EXCEPTION(eICUnknownGenre, eIC);
TSS_EXCEPTION(eICBadSevVal, eIC);
TSS_EXCEPTION(eICInappropriateGenre, eIC);
TSS_EXCEPTION(eICDifferentUsers, eIC);
TSS_EXCEPTION(eICMismatchingParams, eIC);


class cIntegrityCheck
{
public:
    cIntegrityCheck(cGenre::Genre       genreNum,
                    const cFCOSpecList& specList,
                    cHierDatabase&      db,
                    cFCOReport&         report,
                    cErrorBucket*       pBucket);
    // if this is an integrity check of a set of files (instead of a policy file based check)
    // then specList refers to the db's spec list. Otherwise, it represents the spec list being
    // checked. NOTE -- this class has no way to decide if the specList is appropriate for the database
    // or not, so it is the client's responsibility to do this.

    ~cIntegrityCheck();

    void Execute(uint32_t flags = 0);
    // flags should be 0, or some combination of the below enumeration
    // TODO -- specify what kinds of exception can come up from here....
    void ExecuteOnObjectList(const std::list<cFCOName>& fcoNames, uint32_t flags = 0);
    // executes an integrity check on the objects named in the list. The specList passed in
    // as the first parameter to the ctor is interprited as the db's spec list.
    int ObjectsScanned()
    {
        return mnObjectsScanned;
    };

    enum Flags
    {
        FLAG_LOOSE_DIR = 0x00000001,
        // when this is set, fcos whose caps return CAP_CAN_HAVE_CHILDREN will not
        // compare any properties that are returned from iTWFactory::GetLooseDirMask()
        FLAG_COMPARE_VALID_PROPS_ONLY = 0x00000002,
        // when this is set, only the properties that are valid in both FCOs being
        // compared are compared.
        FLAG_INVALIDATE_EXTRA_DB_PROPS = 0x00000004,
        // used by policy update; this flag indicates that we should invalidate properties
        // of objects in the database that are not a part of the spec used to compare the object.
        // Note: it is kind of a hack that this is here; in a perfect design, integrity check
        // would never modify the database -- 8 feb 99 mdb
        FLAG_SET_NEW_PROPS = 0x00000008,
        // also used exclusively in policy update, this is an even bigger and even sicker hack than the
        // previous enumeration. This flag indicates that any valid properties in the new FCO during
        // an integrity check that are not valid in the database FCO should be copied to the db's fco.
        // Yuck!
        FLAG_ERASE_FOOTPRINTS_IC = 0x00000010,
        // when this flag is set, IC will attempt to leave no footprints when doing an integrity check.
        // for instance, IC will tell the property calculator to reset access times.
        FLAG_DIRECT_IO = 0x00000020
        // Use direct i/o when scanning files
    };

private:
    cGenre::Genre        mGenre;
    const cFCOSpecList&  mSpecList;
    cHierDatabase&       mDb;
    cFCOReport&          mReport;
    cErrorBucketPassThru mBucket;
    iFCOPropCalc*        mpPropCalc;
    iFCOSpec*            mpCurSpec;        // the spec we are currently operating on
    cFCOReportSpecIter   mReportIter;      // the current iterator into the report
    cFCOPropVector       mLooseDirProps;   // properties that should be ignored in loose directories
    uint32_t             mFlags;           // flags passed in to execute()
    int                  mnObjectsScanned; // number of objects scanned in system ( scanning includes
                                           // discovering that an FCO does not exist )

    void ProcessDir(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter);
    void ProcessAddedFCO(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, bool bRecurse = true);
    void ProcessRemovedFCO(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, bool bRecurse = true);
    void ProcessChangedFCO(cDbDataSourceIter dbIter, iFCODataSourceIter* pIter, bool bRecurse = true);
    void CompareFCOs(iFCO* pOldFCO, iFCO* pNewFCO);
};


#endif //__INTEGRITYCHECK_H
