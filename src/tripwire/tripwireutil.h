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
// tripwireutil.h
//
// utility functions for the top-level tripwire functionality implemented in
// cTripwire
#ifndef __TRIPWIREUTIL_H
#define __TRIPWIREUTIL_H

class iFCO;
class iFCOSpec;
class iFCOPropCalc;
class iFCOPropDisplayer;
class cFCOName;
class cDbDataSourceIter;
class cHierDatabase;

class cTripwireUtil
{
public:
    static void CalcProps(iFCO* pFCO, const iFCOSpec* pSpec, iFCOPropCalc* pCalc, iFCOPropDisplayer* pPD);
    // this method calculates exactly the properties specified by pSpec (invalidating any extra ones)
    // for iFCO. It is OK to pass NULL for pPD if you don't want the property calculator run over the fco;
    // otherwise, InitForFCO() is called after the property calculation is done.
    // TODO -- I should determine what exceptions will come up from this and document it

    static bool RemoveFCOFromDb(cFCOName name, cDbDataSourceIter& dbIter);
    // removes the named fco from the passed in database. This asserts that the iterator is not done,
    // and gracefully handles the case where there is no fco data associated with the current node.
    // also, if there is an empty subtree below this node (ie -- hier nodes with no FCO data) this method
    // will remove them from the database.
    // this returns true if fco data was actually removed from the database.
};


#endif //__TRIPWIREUTIL_H
