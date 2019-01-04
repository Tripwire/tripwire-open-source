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
// propset.h -- Marcro definitions common to all property sets
//
// These macros were moved to a common file to provide a central place for
// their definitions.
//

#ifndef __PROPSET_H
#define __PROPSET_H

///////////////////////////////////////////////////////////////////////////////
// Convience macros for defining properties.
//
// PROPERTY() is used for properties whose values can be set by a passed in argument
// PROPERTY_OBJ() is used other properties whose value must be set using the object
//
// These macro's are different because it does not make sense to set a
// signature value (at least in the MD5 case where it takes a 128 bit number!).
///////////////////////////////////////////////////////////////////////////////
#define PROPERTY(TYPE, NAME, INDEX)              \
private:                                         \
    TYPE m##NAME;                                \
                                                 \
public:                                          \
    void Set##NAME(TYPE::ValueType value)        \
    {                                            \
        mValidProps.AddItem(INDEX);              \
        m##NAME.SetValue(value);                 \
    }                                            \
    TYPE::ValueType Get##NAME() const            \
    {                                            \
        ASSERT(mValidProps.ContainsItem(INDEX)); \
        return m##NAME.GetValue();               \
    }                                            \
    void SetUndefined##NAME()                    \
    {                                            \
        mValidProps.AddItem(INDEX);              \
        mUndefinedProps.AddItem(INDEX);          \
    }

#define PROPERTY_OBJ(TYPE, NAME, INDEX)        \
private:                                       \
    TYPE m##NAME;                              \
                                               \
public:                                        \
    TYPE* Get##NAME()                          \
    {                                          \
        return &m##NAME;                       \
    }                                          \
    void SetValid##NAME(bool valid)            \
    {                                          \
        if (valid)                             \
            mValidProps.AddItem(INDEX);        \
        else                                   \
            mValidProps.RemoveItem(INDEX);     \
    }                                          \
    void SetDefined##NAME(bool defined)        \
    {                                          \
        SetValid##NAME(true);                  \
        if (!defined)                          \
            mUndefinedProps.AddItem(INDEX);    \
        else                                   \
            mUndefinedProps.RemoveItem(INDEX); \
    }
///////////////////////////////////////////////////////////////////////////////

#endif
