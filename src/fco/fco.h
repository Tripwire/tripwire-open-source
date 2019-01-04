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
// Name....: fco.h
// Date....: 05/07/99
// Creator.: Robert DiFalco (rdifalco)
//
// package:  cFCO       -- FCO package data access
// interface iFCO       -- abstract first class object
// interface iFCOSet    -- abstract set of iFCOs
// interface iFCOIter   -- iterator over an iFCOSet
//

#ifndef __FCO_H
#define __FCO_H

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Dependencies
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "core/package.h" // for: TSS_DeclarePackage

#ifndef __SREFCOUNTOBJ_H
#include "core/srefcountobj.h" // for: iSerRefCountObj
#endif


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Forward Declarations
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class cFCO; // cFCO is the Package Representation
class iFCO;
class cFCOName;
class cFCOPropVector;
class iFCOPropSet;
class iFCOVisitor;
class iFCOSet;
class iFCOIter;
class iFCOPropDisplayer;


//=============================================================================
// cFCO -- First Class Object Package
//=============================================================================
// SYNOPSIS:
//
// CONSTRAINTS:
//
// INVARIANTS:
//
//
TSS_BeginPackage(cFCO)

    TSS_DECLARE_STRINGTABLE;

public:
cFCO(); // Used to construct dependencies

TSS_EndPackage(cFCO)


    //=============================================================================
    // iFCO -- First Class Object Interface
    //=============================================================================
    // SYNOPSIS:
    //
    // CONSTRAINTS:
    //
    // INVARIANTS:
    //
    //
    class iFCO : public iSerRefCountObj
{
public:
    virtual void            SetName(const cFCOName& name) = 0;
    virtual const cFCOName& GetName() const               = 0;
    // returns the name that uniquely identifies the fco in whatever system
    // it lives in. Reference is valid as long as the fco exists

    virtual const iFCOPropSet* GetPropSet() const = 0;
    virtual iFCOPropSet*       GetPropSet()       = 0;
    // returns a pointer to the FCO's property set. It is to be destroyed
    // by the FCO and is only guarenteed to be valid through the life of the
    // fco.

    virtual uint32_t GetCaps() const = 0;
    // returns a bitmask that indicates properties that this object has.
    // see the enum below for what the caps can contain

    virtual iFCO* Clone() const = 0;
    // return a copy of the fco, with a reference count of 1.
    // note that this will return an iFCO with _no_ children. If you want the
    // child information, it should be copied manually

    virtual void AcceptVisitor(iFCOVisitor* pVisitor) = 0;
    // visitor interface

    virtual void TraceContents(int debugLevel = -1) const
    {
    }
    // spit the contents of the fco to debug out. -1 means to use D_DEBUG

    //--------------------------------
    // GetCaps() enumeration
    //--------------------------------
    enum Caps
    {
        CAP_CAN_HAVE_CHILDREN = 0x1,
        CAP_CAN_HAVE_PARENT   = 0x2,
        CAP_GENRE_SPECIFIC    = 0x4 // start the genre-specific caps here.
    };

protected:
    virtual ~iFCO();
};

class iFCOIter
{
public:
    virtual void SeekBegin() const = 0;
    // seeks to the beginning of the set
    virtual bool Done() const = 0;
    // returns true if the iterator has gone past the end of the set
    virtual bool IsEmpty() const = 0;
    // is the set empty?
    virtual void Next() const = 0;
    // seek to the next element in the set

    virtual const iFCO* FCO() const = 0;
    virtual iFCO*       FCO()       = 0;
    // methods for getting the FCO that the iterator is currently pointing at

    virtual bool SeekToFCO(const cFCOName& name) const = 0;
    // seeks the iterator to the fco specified by "name". If an fco by that name doesn't exist,
    // false is returned and the iterator points at the end of the list (Done() will return true

    virtual void Remove()       = 0;
    virtual void Remove() const = 0;
    // removes the fco pointed to by the iter. Behavior is undefined if the set is empty.
    // after the erase, the iterator points to the next element in the list

    virtual void DestroyIter() const = 0;
    // call this instead of delete()ing it. Since the FCOSet created this iterator,
    // destruction with this method allows the object to "give" itself back to the
    // FCOSet.

protected:
    virtual ~iFCOIter(){};
};

class iFCOSet : public iTypedSerializable
{
public:
    virtual ~iFCOSet(){};

    virtual const iFCOIter* Lookup(const cFCOName& name) const = 0;
    virtual iFCOIter*       Lookup(const cFCOName& name)       = 0;
    // returns a pointer to the FCO specified by "name" if it exists
    // in the set, otherwise returns NULL. This will always map uniquely to a
    // single FCO, since it is an error to have multiple FCOs with the same name in the set.

    virtual const iFCOIter* GetIter() const = 0;
    virtual iFCOIter*       GetIter()       = 0;
    // returns an iterator on the set

    virtual void Insert(iFCO* pFCO) = 0;
    // inserts an fco into the set. The set will increment the reference count of the fco, and
    // release it when the fco is removed from the set of the set is destroyed.

    virtual void Clear() = 0;
    // clears out all the elements from the set

    virtual bool      IsEmpty() const = 0;
    virtual int       Size()    const = 0;

    virtual void TraceContents(int debugLevel = -1) const
    {
    }
    // traces the contents of the spec to wherever cDebug is pointing; not pure
    // virtual so that one is not forced to implement it.
    // -1 means to use D_DEBUG
};


#endif //__FCO_H
