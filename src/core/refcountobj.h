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
// refcountobj.h
///////////////////////////////////////////////////////////////////////////////

#ifndef __REFCOUNTOBJ_H
#define __REFCOUNTOBJ_H

///////////////////////////////////////////////////////////////////////////////
// class cRefCountObj
//
// Base class for objects that are to be reference counted. Derive from this
// class to create an object that will keep track of how many owners it has
// and will delete itself when it runs out of owners.
//
// The convention for using this class it to call AddRef() when you pass
// a pointer to the object to a new owner.  When the new owner is done with
// the object it
//
// Note: In the current implementation we do not support a reference counted
// object being an owner of itself, either directly or indirectly.  For instance
// if you make a cRefCountObj capable of being an owner of a cRefCountObj* (say
// through some AddChild(cRefCountObj*) function) and then call AddRef() followed
// by AddChild(), then call Release(), you will end up with an object of reference
// count 1 but no owners outside itself.
//
// This is not foreseen to be a problem with the tripwire usage of this class.
//
// See refcountobj_t.cpp for examples on use.
//

class cRefCountObj
{
public:
    cRefCountObj();

    virtual void AddRef() const;
    virtual void Release() const;

    int GetRefCount() const
    {
        return mRefCount;
    }
    // sometimes it is useful to know an object's refrence
protected:
    virtual ~cRefCountObj();

    virtual void Delete() const;
    // override this if you don't want to be destroyed by "delete this"!
private:
    mutable int mRefCount;

#ifdef DEBUG
private:
    static int  objectCounter;
    static int  referenceCounter;
    friend void TestRefCountObj();

public:
    static bool AllRefCountObjDestoryed()
    {
        return objectCounter == 0 && referenceCounter == 0;
    }
#endif
};

#endif
