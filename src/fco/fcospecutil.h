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
// fcospecutil.h
///////////////////////////////////////////////////////////////////////////////

#ifndef __FCOSPECUTIL_H
#define __FCOSPECUTIL_H

#ifndef __FCOSPEC_H
#include "fcospec.h"
#endif

class cFCOName;

class iFCOSpecUtil
{
public:
    static bool FCOSpecEqual(const iFCOSpec& lhs, const iFCOSpec& rhs);
    // Checks that the start point and all stop points are equal.
    // Note: This may not make sense if the FCO spacs are of differing types, but in
    // the name of simple implementation we define this as equality.
    static bool FCOSpecLessThan(const iFCOSpec& lhs, const iFCOSpec& rhs);
    // Defines an order of FCOSpecs.  A FCOSpec is less than another if and only if
    // the string concatination of start point and an ordered list of stop point
    // are less than the other.
    static bool SpecsOverlap(const iFCOSpec* pSpec1, const iFCOSpec* pSpec2);
    // returns true if the specs "overlap", meaning there exists an fco which is contained
    // in both specs. This is true if one specs's start point is contained in the other spec.
};

///////////////////////////////////////////////////////////////////////////////
// inline implementation

///////////////////////////////////////////////////////////////////////////////
// SpecsOverlap -- returns true if one of the specs contains the other's
//      start point
///////////////////////////////////////////////////////////////////////////////
inline bool iFCOSpecUtil::SpecsOverlap(const iFCOSpec* pSpec1, const iFCOSpec* pSpec2)
{
    return (pSpec1->SpecContainsFCO(pSpec2->GetStartPoint()) || pSpec2->SpecContainsFCO(pSpec1->GetStartPoint()));
}


#endif
