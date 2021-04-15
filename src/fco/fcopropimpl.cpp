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
// fcopropimpl.cpp
//

#include "stdfco.h"
#include "fcopropimpl.h"
#include "core/debug.h"
#include "fcoprop.h"
#include "core/serializer.h"
#include "fcoundefprop.h"
#include "core/errorutil.h"

///////////////////////////////////////////////////////////////////////////////
// DefaultCompare -- template function that implements compare in a way suitable
//      for most fundamental types
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
/*static*/
iFCOProp::CmpResult DefaultCompare(const TYPE* lhs, const iFCOProp* rhs, iFCOProp::Op op)
{
    // compares with undefined props are not equal
    if (rhs->GetType() == cFCOUndefinedProp::GetInstance()->GetType())
    {
        return (op == iFCOProp::OP_EQ) ? iFCOProp::CMP_FALSE :
                                         (op == iFCOProp::OP_NE) ? iFCOProp::CMP_TRUE : iFCOProp::CMP_WRONG_PROP_TYPE;
    }

    // first, make sure we are the right type...
    if (rhs->GetType() != lhs->GetType())
    {
        return iFCOProp::CMP_WRONG_PROP_TYPE;
    }

    // do the down cast
    const TYPE* newRhs = static_cast<const TYPE*>(rhs);
    ASSERT(newRhs != 0);

    // finally, do the comparison...
    bool bResult;
    switch (op)
    {
    case iFCOProp::OP_EQ:
        bResult = (lhs->GetValue() == newRhs->GetValue());
        break;
    case iFCOProp::OP_NE:
        bResult = (lhs->GetValue() != newRhs->GetValue());
        break;
    case iFCOProp::OP_GT:
        bResult = (lhs->GetValue() > newRhs->GetValue());
        break;
    case iFCOProp::OP_LT:
        bResult = (lhs->GetValue() < newRhs->GetValue());
        break;
    case iFCOProp::OP_GE:
        bResult = (lhs->GetValue() >= newRhs->GetValue());
        break;
    case iFCOProp::OP_LE:
        bResult = (lhs->GetValue() <= newRhs->GetValue());
        break;
    default:
        // we have exhausted all the possibilities
        ASSERT(false);
        throw INTERNAL_ERROR("fcopropimpl.cpp");
    }

    return bResult ? iFCOProp::CMP_TRUE : iFCOProp::CMP_FALSE;
}

template<class TYPE>
/*static*/
void DefaultCopy(TYPE* lhs, const iFCOProp* rhs)
{
    // first, make sure we are the right type...
    if (rhs->GetType() != lhs->GetType())
    {
        ASSERT(false);
        return;
    }

    // do the down cast
    const TYPE* newRhs = static_cast<const TYPE*>(rhs);
    ASSERT(newRhs != 0);
    lhs->SetValue(newRhs->GetValue());
}

//#####################################
// cFCOPropInt32
//#####################################

IMPLEMENT_TYPEDSERIALIZABLE(cFCOPropInt32, _T("cFCOPropInt32"), 0, 1);

TSTRING cFCOPropInt32::AsString() const
{
    //TODO:mdb -- implement this through twlocale!
    //
    TOSTRINGSTREAM ostr;
    tss_classic_locale(ostr);
    ostr << mValue;
    tss_mkstr(out, ostr);
    
    return out;
}

iFCOProp::CmpResult cFCOPropInt32::Compare(const iFCOProp* rhs, iFCOProp::Op op) const
{
    return DefaultCompare(this, rhs, op);
}

void cFCOPropInt32::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > 0)
        ThrowAndAssert(eSerializerVersionMismatch(_T("int32_t Property Read")));

    pSerializer->ReadInt32(mValue);
}

void cFCOPropInt32::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(mValue);
}

void cFCOPropInt32::Copy(const iFCOProp* rhs)
{
    DefaultCopy(this, rhs);
}


//#####################################
// cFCOPropInt64
//#####################################

IMPLEMENT_TYPEDSERIALIZABLE(cFCOPropInt64, _T("cFCOPropInt64"), 0, 1);

TSTRING cFCOPropInt64::AsString() const
{
    //TODO:mdb -- implement this through twlocale!
    //
    TOSTRINGSTREAM ostr;
    tss_classic_locale(ostr);
    ostr << (int32_t)mValue;  // TODO: remove this cast where possible
    tss_mkstr(out, ostr);
    return out;
}

iFCOProp::CmpResult cFCOPropInt64::Compare(const iFCOProp* rhs, iFCOProp::Op op) const
{
    return DefaultCompare(this, rhs, op);
}

void cFCOPropInt64::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > 0)
        ThrowAndAssert(eSerializerVersionMismatch(_T("int64_t Property Read")));

    pSerializer->ReadInt64(mValue);
}

void cFCOPropInt64::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt64(mValue);
}

void cFCOPropInt64::Copy(const iFCOProp* rhs)
{
    DefaultCopy(this, rhs);
}


//#####################################
// cFCOPropUint64
//#####################################

IMPLEMENT_TYPEDSERIALIZABLE(cFCOPropUint64, _T("cFCOPropUint64"), 0, 1);

TSTRING cFCOPropUint64::AsString() const
{
    //TODO:mdb -- implement this through twlocale!
    //
    TOSTRINGSTREAM ostr;
    tss_classic_locale(ostr);
    ostr << (int32_t)mValue; // TODO: remove this cast where possible
    tss_return_stream(ostr, out);
}

iFCOProp::CmpResult cFCOPropUint64::Compare(const iFCOProp* rhs, iFCOProp::Op op) const
{
    return DefaultCompare(this, rhs, op);
}

void cFCOPropUint64::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > 0)
        ThrowAndAssert(eSerializerVersionMismatch(_T("uint64_t Property Read")));

    pSerializer->ReadInt64((int64_t&)mValue);
}

void cFCOPropUint64::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt64(mValue);
}

void cFCOPropUint64::Copy(const iFCOProp* rhs)
{
    DefaultCopy(this, rhs);
}


//#####################################
// cFCOPropTSTRING
//#####################################

IMPLEMENT_TYPEDSERIALIZABLE(cFCOPropTSTRING, _T("cFCOPropTSTRING"), 0, 1);

TSTRING cFCOPropTSTRING::AsString() const
{
    return mValue;
}

iFCOProp::CmpResult cFCOPropTSTRING::Compare(const iFCOProp* rhs, iFCOProp::Op op) const
{
    return DefaultCompare(this, rhs, op);
}

void cFCOPropTSTRING::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > 0)
        ThrowAndAssert(eSerializerVersionMismatch(_T("String Property Read")));

    pSerializer->ReadString(mValue);
}

void cFCOPropTSTRING::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteString(mValue);
}

void cFCOPropTSTRING::Copy(const iFCOProp* rhs)
{
    DefaultCopy(this, rhs);
}

//#############################################################################
// cFCOPropGrowingFile
//#############################################################################
iFCOProp::CmpResult cFCOPropGrowingFile::Compare(const iFCOProp* rhs, Op op) const
{
    // here is how the hack works: we want OP_EQ to return true if the file has grown (if rhs's
    // size is bigger than or equal to mine) Therefore, equality is defined as my size being
    // smaller than or equal to the rhs's size
    if (op == OP_EQ)
        return cFCOPropInt64::Compare(rhs, OP_LE);
    else
        return cFCOPropInt64::Compare(rhs, op);
}
