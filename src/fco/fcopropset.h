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
// fcopropertyset.h
//
// iFCOPropSet -- the interface to an fco's properties
#ifndef __FCOPROPSET_H
#define __FCOPROPSET_H

#ifndef __FCOPROPVECTOR_H
#include "fcopropvector.h"
#endif
#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __SERIALIZABLE_H
#include "serializable.h"
#endif

class iFCOProp;

class iFCOPropSet : public iTypedSerializable
{
public:
    enum
    {
        PROP_NOT_FOUND = -1
    };

    virtual const cFCOPropVector& GetValidVector() const = 0;
    // returns a vector that indicates which properties have valid values associated with them.

    virtual int GetNumProps() const = 0;
    // returns the number of properties this set has

    virtual int GetPropIndex(const TCHAR* name) const = 0;
    // maps a property name to an index number. If the property doesn't exist in the set, -1 is returned.
    // the return value is guarenteed to be in the range -1 <= x < GetNumProps()

    virtual TSTRING GetPropName(int index) const = 0;
    // returns the name associated with the property at the given index. Return result is
    // undefined if index is not in the range 0 <= index < GetNumProps()

    virtual const iFCOProp* GetPropAt(int index) const = 0;
    virtual iFCOProp*       GetPropAt(int index)       = 0;
    // returns a pointer to the property specified by index. Again,
    // the return result is undefined if 0 <= index < GetNumProps()

    virtual void InvalidateProp(int index)                                = 0;
    virtual void InvalidateProps(const cFCOPropVector& propsToInvalidate) = 0;
    virtual void InvalidateAll()                                          = 0;
    // invalidates one, some, or all of the properties in this set

    virtual void CopyProps(const iFCOPropSet* pSrc, const cFCOPropVector& propsToCopy) = 0;
    // copies the all the properties named in propsToCopy from pSrc to this prop set.
    // this method will assert that pSrc is of the same type as this. All the properties
    // in propsToCopy should be valid in pSrc.

    virtual void TraceContents(int dl = -1) const = 0;
    // sends the contents of this property set to wherever cDebug is pointing.

protected:
    virtual ~iFCOPropSet()
    {
    }
};

#endif //__FCOPROPSET_H
