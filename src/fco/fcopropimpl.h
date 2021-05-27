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
// fcopropimpl.h -- implementation of some basic properties
//
#ifndef __FCOPROPIMPL_H
#define __FCOPROPIMPL_H

#ifndef __FCOPROP_H
#include "fcoprop.h"
#endif
#ifndef __TYPES_H
#include "types.h"
#endif


/////////////////////////////////////////////////////////////////
// Another in my collection of macros, this just saves
// some redundant typing on my part.
#define PROP_DATA(TYPE)           \
private:                          \
    TYPE mValue;                  \
                                  \
public:                           \
    typedef TYPE ValueType;       \
    TYPE         GetValue() const \
    {                             \
        return mValue;            \
    }                             \
    void SetValue(const TYPE& v)  \
    {                             \
        mValue = v;               \
    }
////////////////////////////////////////////////////////////////

class cFCOPropInt32 : public iFCOProp
{
public:
    PROP_DATA(int32_t)            // see macro above
    DECLARE_TYPEDSERIALIZABLE() // type information

    cFCOPropInt32() : mValue(0)
    {
    }
    virtual ~cFCOPropInt32()
    {
    }

    // from iFCOProp
    virtual TSTRING   AsString() const;
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;
    virtual void      Copy(const iFCOProp* rhs);

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
};

class cFCOPropInt64 : public iFCOProp
{
public:
    PROP_DATA(int64_t)            // see macro above
    DECLARE_TYPEDSERIALIZABLE() // type information

    cFCOPropInt64() : mValue(0)
    {
    }
    virtual ~cFCOPropInt64()
    {
    }

    // from iFCOProp
    virtual TSTRING   AsString() const;
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;
    virtual void      Copy(const iFCOProp* rhs);

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
};

class cFCOPropUint64 : public iFCOProp
{
public:
    PROP_DATA(uint64_t)           // see macro above
    DECLARE_TYPEDSERIALIZABLE() // type information

    cFCOPropUint64() : mValue(0)
    {
    }
    virtual ~cFCOPropUint64()
    {
    }

    // from iFCOProp
    virtual TSTRING   AsString() const;
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;
    virtual void      Copy(const iFCOProp* rhs);

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
};


class cFCOPropTSTRING : public iFCOProp
{
public:
    PROP_DATA(TSTRING)          // see macro above
    DECLARE_TYPEDSERIALIZABLE() // type information

    cFCOPropTSTRING() : mValue(_T(""))
    {
    }
    virtual ~cFCOPropTSTRING()
    {
    }

    // from iFCOProp
    virtual TSTRING   AsString() const;
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;
    virtual void      Copy(const iFCOProp* rhs);

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
};

///////////////////////////////////////////////////////////////////////////////
// cFCOPropGrowingFile -- a property that represents a growing file. It does
//      this by overriding Compare() and implementing it like less-than
//      it is important, then, to note that this requires the fcos to be passed
//      to the compare object in this order: old, new
///////////////////////////////////////////////////////////////////////////////
class cFCOPropGrowingFile : public cFCOPropInt64
{
public:
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;
};

#endif //__FCOPROPIMPL_H
