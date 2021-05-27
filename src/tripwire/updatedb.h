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
// updatedb.h
//
#ifndef __UPDATEDB_H
#define __UPDATEDB_H

#ifndef __ERROR_H
#include "core/error.h"
#endif

class cHierDatabase;
class cFCOReport;
class cErrorBucket;

////////////////////////////////////////////////////////////
// Update Db Exceptions
////////////////////////////////////////////////////////////
TSS_EXCEPTION(eUpdateDb, eError);
TSS_EXCEPTION(eUpdateDbAddedFCO, eUpdateDb);
TSS_EXCEPTION(eUpdateDbRemovedFCO, eUpdateDb);
TSS_EXCEPTION(eUpdateDbChangedFCO, eUpdateDb);

class cUpdateDb
{
public:
    cUpdateDb(cHierDatabase& db, cFCOReport& report, cErrorBucket* pBucket);

    bool Execute(uint32_t flags = 0);
    // returns false if there were any conflicts in updating
    // the database
    // TODO -- what kind of exceptions can come up from here?

    enum Flags
    {
        FLAG_REPLACE_PROPS = 0x00000001,
        // if this flag is passed to execute, then them
        // database's FCO's property set is completely
        // replaced with the reports property set.
        // (That means that invalid properties in the
        // report will be invalidated in the database.
        // This behavior is only desireable right now for
        // policy update, when we want to get rid of stale
        // information)
        FLAG_ERASE_FOOTPRINTS_UD = 0x00000002
        // when this flag is set, UpdateDb will attempt
        // undo any inadvertant modifications it may make
        // when executing.


    };

private:
    cHierDatabase& mDb;
    cFCOReport&    mReport;
    cErrorBucket*  mpBucket;
};

#endif
