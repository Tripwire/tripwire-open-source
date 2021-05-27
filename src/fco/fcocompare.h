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
// fcocompare.h
//
// iFCOCompare -- interface for object that compares FCO properties
#ifndef __FCOCOMPARE_H
#define __FCOCOMPARE_H

#ifndef __FCOPROPVECTOR_H
#include "fcopropvector.h"
#endif

class iFCO;

class cFCOCompare
{
public:
    enum Result
    {
        EQUAL               = 1,
        PROPS_UNEQUAL       = 2,
        PROPS_NOT_ALL_VALID = 4
    };

    cFCOCompare();
    explicit cFCOCompare(const cFCOPropVector& propsToCompare);
    virtual ~cFCOCompare();

    void                  SetPropsToCmp(const cFCOPropVector& pv);
    const cFCOPropVector& GetPropsToCmp() const;
    // gets and sets the property vector that indicates what properties the
    // object will consider in the comparisons.

    uint32_t Compare(const iFCO* pFco1, const iFCO* pFco2);
    // compares fco1 and fco2, only considering the properties specified set with
    // SetPropsToCmp(). The result of the comparison is a bitmask that is currently either
    // EQUAL or a combination of PROPS_NOT_ALL_VALID and PROPS_UNEQUAL. You can discover which
    // properties caused these return flags with the GetXXXProps() methods below.
    // NOTE -- in integrity checking, the old fco should be the first parameter and the new fco the
    // second (to support growing files properly)

    const cFCOPropVector& GetInvalidProps() const;
    // only meaningful if the last Compare() returned PROPS_NOT_ALL_VALID
    const cFCOPropVector& GetUnequalProps() const;
    // only meaningful if the last Compare() returned PROPS_UNEQUAL

private:
    cFCOPropVector mPropsToCmp;
    cFCOPropVector mInvalidProps;
    cFCOPropVector mUnequalProps;
};

/////////////////////////////////////////////////////////////
// inline implementation
inline void cFCOCompare::SetPropsToCmp(const cFCOPropVector& pv)
{
    mPropsToCmp = pv;
}

inline const cFCOPropVector& cFCOCompare::GetPropsToCmp() const
{
    return mPropsToCmp;
}

inline const cFCOPropVector& cFCOCompare::GetInvalidProps() const
{
    return mInvalidProps;
}

inline const cFCOPropVector& cFCOCompare::GetUnequalProps() const
{
    return mUnequalProps;
}


#endif //__FCOCOMPARE_H
