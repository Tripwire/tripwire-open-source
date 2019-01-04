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
// fcoprop.h
//
// iFCOProp -- abstract interface for all fco properties
#ifndef __FCOPROP_H
#define __FCOPROP_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __TYPED_H
#include "core/typed.h"
#endif
#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif

class iFCOProp : public iTypedSerializable
{
public:
    enum Op
    {
        OP_EQ = 1,  // ==
        OP_NE = 2,  // !=
        OP_GT = 4,  // >
        OP_LT = 8,  // <
        OP_GE = 16, // >=
        OP_LE = 32  // <=
    };
    enum CmpResult
    {
        CMP_TRUE            = 1,
        CMP_FALSE           = 2,
        CMP_WRONG_PROP_TYPE = 3,
        CMP_UNSUPPORTED     = 4
    };

    virtual TSTRING AsString() const = 0;
    // returns a string representation of the property
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const = 0;
    virtual void      Copy(const iFCOProp* rhs)                 = 0;
    // change my value so that it is the same as the passed in property.
    // this will assert() that the two properties are of the same type.

protected:
    virtual ~iFCOProp()
    {
    }
};

#endif //__FCOPROP_H
