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
// policyupdate.h
//
#ifndef __POLICYUPDATE_H
#define __POLICYUPDATE_H

#ifndef __FCOSPECLIST_H
#include "fco/fcospeclist.h"
#endif
#ifndef __HIERDATABASE_H
#include "db/hierdatabase.h"
#endif
#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif

class cErrorBucket;


////////////////////////////////////////////////////////////
// Policy Update Exceptions
////////////////////////////////////////////////////////////
TSS_EXCEPTION(ePolicyUpdate, eError);
TSS_EXCEPTION(ePolicyUpdateAddedFCO, ePolicyUpdate);
TSS_EXCEPTION(ePolicyUpdateRemovedFCO, ePolicyUpdate);
TSS_EXCEPTION(ePolicyUpdateChangedFCO, ePolicyUpdate);

class cPolicyUpdate
{
public:
    cPolicyUpdate(cGenre::Genre       genreNum,
                  const cFCOSpecList& oldPolicy,
                  const cFCOSpecList& newPolicy,
                  cHierDatabase&      db,
                  cErrorBucket*       pBucket);

    bool Execute(uint32_t flags = 0); // throw (eError)
        // if false is returned, then there was at least one conflict that came up during the policy
        // update, and if tripwire was run in secure mode then the policy update should fail.

    enum Flags
    {
        FLAG_SECURE_MODE = 0x00000001, // if this is set, then we're in pedantic mode. This affects whether error
                                       // messages appear as "Error" or "Warning"
        FLAG_ERASE_FOOTPRINTS_PU = 0x00000002,
        // when this flag is set, cPolicyUpdate will attempt undo any inadvertant modifications
        // it may make when executing.

        FLAG_DIRECT_IO = 0x00000004
        // Use direct i/o when scanning files
    };

private:
    const cFCOSpecList& mOldPolicy;
    const cFCOSpecList& mNewPolicy;
    cHierDatabase&      mDb;
    cGenre::Genre       mGenre;
    cErrorBucket*       mpBucket;
};

#endif //__POLICYUPDATE_H
