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
// fcospec.h
//
// iFCOSpec -- embodies a way to specify a set of FCOs
// iFCOSpecIter -- iterates over the FCOs that a spec maps to
// iFCOSpecMask -- classifies FCOs within an FCOSpec
#ifndef __FCOSPEC_H
#define __FCOSPEC_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __FCO_H
#include "fco.h"
#endif
#ifndef __TYPED_H
#include "typed.h"
#endif

class iFCOSpecIter;
class iFCOSpecMask;
class cFCOPropVector;
class iFCOSpecHelper;

class iFCOSpec : public iSerRefCountObj
{
public:
    virtual ~iFCOSpec()
    {
    }

    virtual const TSTRING& GetName() const = 0;
    // returns the name of this FCOSpec
    virtual void SetName(const TSTRING& name) = 0;
    // sets the spec's name

    virtual iFCOSpec* Clone() const = 0;
    // makes a copy of this FCO. This method is needed because it is sometimes necessary
    // for clients who only know about the base class interface to make copies of the object.

    virtual bool SpecContainsFCO(const cFCOName& name) const = 0;
    // returns true if the fco is below the spec's start point and above its terminator.

    virtual const cFCOName& GetStartPoint() const = 0;
    // Returns the name of the fco that is the "root node" for the FCO set that will
    // be produced by the spec.

    virtual void SetStartPoint(const cFCOName& name) = 0; // throw (eError);
        // sets the start point, as described in GetStartPoint() above. If a start point is added that is not
        // above all the existing stop points, an eError is thrown.

    virtual void                  SetHelper(iFCOSpecHelper* pHelper) = 0;
    virtual const iFCOSpecHelper* GetHelper() const                  = 0;
    // gets and sets the spec's helper class, which holds the start point and manages the spec's
    // terminating criterion (stop points, etc.)

    virtual const cFCOPropVector& GetPropVector(const iFCOSpecMask* pFCOSpecMask) const = 0;
    // returns the property vector associated with the given mask. Result is undefined
    // if the mask isn't a part of the FCOSpec
    virtual bool SetPropVector(const iFCOSpecMask* pMask, const cFCOPropVector& vector) = 0;
    // sets the property vector associated with a specific mask. The pointer to the mask must
    // be the same as one of the pointers that the spec contains (that is, the spec "owns" the
    // masks, and the pointer passed into this method was acquired from GetSpecMask() below.
    // Returns false if pMask isn't in the spec.

    virtual const iFCOSpecMask* GetSpecMask(const iFCO* pFCO) const = 0;
    // returns the FCOSpecMask associated with the FCO that the iterator is
    // pointing at

    const cFCOPropVector& GetPropVector(const iFCO* pFCO) const;
    // inline convenience function that calls GetPropVector(GetSpecMask(pFCO))

    // TODO -- add methods for manipulating the internal FCOSpecMask set

    ///////////////////////////////
    // New for 2.1
    //
    virtual bool ShouldStopDescent(const cFCOName& name) const = 0;
    // this method returns true if the named fco is not a part of the spec. More
    // specifically, it only returns true for all of the fcos on the "border"
    // between being in and out of the spec. This will assert that the name is
    // below the start point.

    //
    ///////////////////////////////

    virtual void TraceContents(int dl = -1) const
    {
    }
    // traces the contents of the spec to wherever cDebug is pointing; not pure
    // virtual so that one is not forced to implement it.
};

class iFCOSpecMask
{
public:
    virtual ~iFCOSpecMask()
    {
    }
    virtual const TSTRING& GetName() const = 0;
    // return the name of this mask

    virtual bool Accept(const iFCO* pFCO) const = 0;
    // returns true if the fco matches the criterion in the SpecMask

    static const iFCOSpecMask* GetDefaultMask();
    // returns the "default mask" that all iFCOSpecs must have. The default
    // mask matches every FCO and is called "Default". There is no need to
    // destroy the returned pointer
};

//#############################################################################
// inline implementation
inline const cFCOPropVector& iFCOSpec::GetPropVector(const iFCO* pFCO) const
{
    return GetPropVector(GetSpecMask(pFCO));
}


#endif //__FCOSPEC_H
