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
// upperbound.h
//

#ifndef __UPPERBOUND_H
#define __UPPERBOUND_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Function name: template< class FwdIterT, class CmpObjT, class CmpFuncT >
//                UpperBound
//
// Description  : same as std::upper_bound except it allows 'less' to take
//                parameters of dissimilar types.  Therefore, *FwdIter need not be
//                the same type as CmpObjT.  Uses a binary search algorithm.
//
// Return type  : FwdIterT          : First element in the sequence [first, last)
//                                    that is equal to or greater than 'obj' or
//                                    'last' if there is no such element.
//
// Argument     : FwdIterT first    : First element in sequence.
// Argument     : FwdIterT last     : Last element in sequence.
// Argument     : const CmpObjT& obj: Object for which to search for upper bound.
// Argument     : CmpFuncT less     : Comparison function.  Must return as 'std::less'.
//                                    Will be called as: 'bool less( *FwdIterT, CmpObjT )'.
/////////////////////////////////////////////////////////////////////////////////////////////
template<class FwdIterT, class CmpObjT, class CmpFuncT>
FwdIterT UpperBound(FwdIterT first, FwdIterT last, const CmpObjT& obj, CmpFuncT less)
{
    // count elements in set
    int      nElemsInSet = 0;
    FwdIterT iCur        = first;
    for (; iCur != last; ++iCur)
        ++nElemsInSet;

    iCur = first;

    while (0 < nElemsInSet)
    {
        // go to halfway point
        int      iHalfWay = nElemsInSet / 2;
        FwdIterT iTemp    = iCur;
        for (int j = 0; j < iHalfWay; j++)
            iTemp++;

        if (less(*iTemp, obj))
        {
            // start next search set at next elem with half of the last search set's elements
            iCur = ++iTemp;
            nElemsInSet -= iHalfWay + 1; // already searched ( iHalfway + 1 ) elems
        }
        else
        {
            // start next search set beginning with half of the last search set's elements
            nElemsInSet = iHalfWay;
        }
    }

    return iCur;
}

#endif // __UPPERBOUND_H
