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
// File:    errorutil.h
// Date:    30 April 99
// Creator: mdb
// Company: TSS
// Desc:    contains useful eError derived classes
//
// eInternal        -- internal logic errors ( ie -- programming mistakes )
// ePoly            -- "polymorphic" error that takes its ID as input instead
//                      of from its class name
// ThrowAndAssert   -- asserts false and throws the specified exception
//
#ifndef __ERRORUTIL_H
#define __ERRORUTIL_H

#ifndef __ERROR_H
#include "error.h"
#endif


//-----------------------------------------------------------------------------
// ePoly
//-----------------------------------------------------------------------------
class ePoly : public eError
{
public:
    //-------------------------------------------------------------------------
    // Construction and Assignment
    //-------------------------------------------------------------------------
    ePoly(uint32_t id, const TSTRING& msg, uint32_t flags = 0);
    explicit ePoly(const eError& rhs);
    explicit ePoly();
    void operator=(const eError& rhs);

    //-------------------------------------------------------------------------
    // ID manipulation
    //-------------------------------------------------------------------------
    virtual uint32_t GetID() const;
    void             SetID(uint32_t id);

private:
    uint32_t mID;
};

//-----------------------------------------------------------------------------
// eInternal
//-----------------------------------------------------------------------------
TSS_BEGIN_EXCEPTION(eInternal, eError)
public:
eInternal(TCHAR* file, int lineNum);
TSS_END_EXCEPTION()

//-----------------------------------------------------------------------------
// cErrorUtil
//-----------------------------------------------------------------------------
class cErrorUtil
{
public:
    static TSTRING MakeFileError(const TSTRING& msg, const TSTRING& fileName);
    // constructs an error message of the form:
    // File: <fileName> \n <msg>
    // This is useful for constructing strings to pass as the msg parameter
    // to eError constructors.
};

//-----------------------------------------------------------------------------
// Convenience Macros
//-----------------------------------------------------------------------------
//
// NOTE -- we require the developer to supply the file name instead of using
//      __FILE__ because that includes the full path to the file, which we
//      would not like to display to the user.
//
#define INTERNAL_ERROR(filename) eInternal((TCHAR*)_T(filename), __LINE__)
#define THROW_INTERNAL(filename) throw eInternal((TCHAR*)_T(filename), __LINE__)


// TODO: ASSERT is always fatal in Unix, perhaps we could #ifdef the ASSERT
//      to echo to cout the line number the exception occured at?
#define ThrowAndAssert(exception) \
    {                             \
        ASSERT(false);            \
        throw exception;          \
    }


//-----------------------------------------------------------------------------
// Inline Implementation
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// ePoly
///////////////////////////////////////////////////////////////////////////////
inline ePoly::ePoly(uint32_t id, const TSTRING& msg, uint32_t flags) : eError(msg, flags), mID(id)
{
}

///////////////////////////////////////////////////////////////////////////////
// ePoly
///////////////////////////////////////////////////////////////////////////////
inline ePoly::ePoly(const eError& rhs)
{
    *this = rhs;
}

///////////////////////////////////////////////////////////////////////////////
// ePoly
///////////////////////////////////////////////////////////////////////////////
inline ePoly::ePoly() : eError(_T("")), mID(0)
{
}

///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
inline void ePoly::operator=(const eError& rhs)
{
    mMsg   = rhs.GetMsg();
    mFlags = rhs.GetFlags();
    mID    = rhs.GetID();
}

///////////////////////////////////////////////////////////////////////////////
// GetID
///////////////////////////////////////////////////////////////////////////////
inline uint32_t ePoly::GetID() const
{
    return mID;
}

///////////////////////////////////////////////////////////////////////////////
// SetID
///////////////////////////////////////////////////////////////////////////////
inline void ePoly::SetID(uint32_t id)
{
    mID = id;
}

#endif //__ERRORUTIL_H
