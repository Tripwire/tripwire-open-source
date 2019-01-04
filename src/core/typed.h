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
// typed.h
//
// cType -- object that represents a class's type
// iTyped -- interface to implement to have a cType
#ifndef __TYPED_H
#define __TYPED_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

class cType
{
public:
    cType(const TCHAR* name);

    const TCHAR* AsString() const;
    bool         operator==(const cType& rhs) const;
    bool         operator!=(const cType& rhs) const;

private:
    TSTRING mString;
};

class iTyped
{
public:
    virtual const cType& GetType() const = 0;
    // the type of an FCO; classes that implement this interface need to
    // (a) declare a public static const cType member mType and
    // (b) returning that object in their implementation of GetType()
    // You can use the macros below to simplify the process

    virtual ~iTyped()
    {
    }
};

//////////////////////////////////////////////
// convenience macros for implementing iTyped
//////////////////////////////////////////////
#define DECLARE_TYPED()         \
public:                         \
    static const cType   mType; \
    virtual const cType& GetType() const;
// put DECLARE_TYPED in the class definition

#define IMPLEMENT_TYPED(CLASS, STRING)  \
    const cType  CLASS::mType(STRING);  \
    const cType& CLASS::GetType() const \
    {                                   \
        return mType;                   \
    }
// put IMPLEMENT_TYPED in the .cpp file where the class is implemented

#define CLASS_TYPE(CLASS) CLASS::mType
// a convienent way to specify a class's type

///////////////////////////////////////////////
// iTyped Example
///////////////////////////////////////////////
/*
    (foo.h)
    class cFoo : public iTyped
    {
    public:
        DECLARE_TYPED()
    }

    (foo.cpp)
    DECLARE_TYPED(cFoo, "Foo");

    (main.cpp)
    int main()
    {
        iTyped* pi = Bar(); // returned a cFoo
        cout << "Encountered class " << pi->GetType().AsString() << endl;
            // prints "Encountered class Foo"
        if(pi->GetType() == CLASS_TYPE(cFoo))
        {
            cFoo* pFoo = static_cast<cFoo*>(pi);
            // cast is always safe
        }
    }
*/

///////////////////////////////////////////////////////////////////////////////
// inline implementation
///////////////////////////////////////////////////////////////////////////////
inline cType::cType(const TCHAR* name) : mString(name)
{
    ASSERT(!mString.empty());
}


inline const TCHAR* cType::AsString() const
{
    return mString.c_str();
}

inline bool cType::operator==(const cType& rhs) const
{
    return (this == &rhs);
}

inline bool cType::operator!=(const cType& rhs) const
{
    return (this != &rhs);
}


#endif //__TYPED_H
