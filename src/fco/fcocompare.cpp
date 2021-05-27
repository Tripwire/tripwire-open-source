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
// fcocompare.cpp
//

#include "stdfco.h"

#include "fcocompare.h"
#include "core/debug.h"
#include "fco.h"
#include "fcoprop.h"
#include "fcopropset.h"
#include "core/errorutil.h"

cFCOCompare::cFCOCompare()
{
}

cFCOCompare::cFCOCompare(const cFCOPropVector& propsToCompare) : mPropsToCmp(propsToCompare)
{
}

cFCOCompare::~cFCOCompare()
{
}

///////////////////////////////////////////////////////////////////////////////
// Compare
///////////////////////////////////////////////////////////////////////////////
uint32_t cFCOCompare::Compare(const iFCO* pFco1, const iFCO* pFco2)
{
    ASSERT(pFco1 != 0);
    ASSERT(pFco2 != 0);
    // first, make sure the fcos are of the same type...
    if (pFco1->GetType() != pFco2->GetType())
    {
        ASSERT(false);
        INTERNAL_ERROR("fcocompare.cpp");
    }


    const iFCOPropSet*    ps1 = pFco1->GetPropSet();
    const iFCOPropSet*    ps2 = pFco2->GetPropSet();
    const cFCOPropVector& v1  = pFco1->GetPropSet()->GetValidVector();
    const cFCOPropVector& v2  = pFco2->GetPropSet()->GetValidVector();

    uint32_t result = 0;

    mInvalidProps.SetSize(v1.GetSize());
    mUnequalProps.SetSize(v1.GetSize());
    mInvalidProps.Clear();
    mUnequalProps.Clear();

    // finally, comapre all the properties
    for (int i = 0; i < v1.GetSize(); i++)
    {
        if (mPropsToCmp.ContainsItem(i))
        {
            if ((!v1.ContainsItem(i)) || (!v2.ContainsItem(i)))
            {
                mInvalidProps.AddItem(i);
                result |= PROPS_NOT_ALL_VALID;
            }
            else
            {
                // comapre the properties
                if (ps1->GetPropAt(i)->Compare(ps2->GetPropAt(i), iFCOProp::OP_EQ) != iFCOProp::CMP_TRUE)
                {
                    // they are not equal!
                    mUnequalProps.AddItem(i);
                    result |= PROPS_UNEQUAL;
                }
            }
        }
    }

    if (!result)
        result = EQUAL;

    return result;
}
