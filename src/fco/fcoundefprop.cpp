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
//
// Name....: fcoundefprop.cpp
// Date....: 05/07/99
// Creator.:
//
// The undefined FCO property
//

#include "stdfco.h"
#include "fcoundefprop.h"
#include "core/errorutil.h"
#include "fcostrings.h"

//=============================================================================
// cFCOUndefinedProp
//
// This class exists only as a singleton.  iFCOPropSet derived classes will return a
// pointer to the singleton for properties that are undefined.
//=============================================================================

IMPLEMENT_TYPEDSERIALIZABLE(cFCOUndefinedProp, _T( "cFCOUndefinedProp" ), 0, 1);

//////////////////////////////////////////////////////////////////////////////
// Singleton interface

struct cFCOUndefinedPropProxy
{
    cFCOUndefinedProp mUndef;
};


cFCOUndefinedProp* cFCOUndefinedProp::GetInstance()
{
    static cFCOUndefinedPropProxy _default;
    return &_default.mUndef;
}


//////////////////////////////////////////////////////////////////////////////
// Method definition

cFCOUndefinedProp::cFCOUndefinedProp()
{
}

cFCOUndefinedProp::~cFCOUndefinedProp()
{
}

TSTRING cFCOUndefinedProp::AsString() const
{
    return TSS_GetString(cFCO, fco::STR_UNDEFINED);
}

iFCOProp::CmpResult cFCOUndefinedProp::Compare(const iFCOProp* rhs, iFCOProp::Op op) const
{
    if (op == iFCOProp::OP_GT || op == iFCOProp::OP_LT || op == iFCOProp::OP_GE || op == iFCOProp::OP_LE)
    {
        // It is not very well-defined what we should do if you are testing inequality
        // on an undefined property.  In 2.1 we never compare inequality so this should
        // not happen. This changes some code will need to be re-written.
        ASSERTMSG(false, "cFCOUndefinedProp does not support >, <, >=, or <= compares!");

        return iFCOProp::CMP_UNSUPPORTED;
    }

    ASSERT(op == iFCOProp::OP_EQ || op == iFCOProp::OP_NE);


    if (rhs->GetType() != GetType())
    {
        // undefined prop is unequal to all other props
        return (op == iFCOProp::OP_EQ) ? iFCOProp::CMP_FALSE : iFCOProp::CMP_TRUE;
    }
    else if (op == iFCOProp::OP_EQ)
    {
        return iFCOProp::CMP_TRUE;
    }
    else
    {
        return iFCOProp::CMP_FALSE;
    }
}

void cFCOUndefinedProp::Read(iSerializer* pSerializer, int32_t version)
{
    ThrowAndAssert(INTERNAL_ERROR("fcoundefprop.cpp"));
}

void cFCOUndefinedProp::Write(iSerializer* pSerializer) const
{
    ThrowAndAssert(INTERNAL_ERROR("fcoundefprop.cpp"));
}

void cFCOUndefinedProp::Copy(const iFCOProp* rhs)
{
    ThrowAndAssert(INTERNAL_ERROR("fcoundefprop.cpp"));
}
