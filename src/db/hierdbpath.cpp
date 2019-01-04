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
// hierdbpath.cpp :  Implements cHierDbPath.
//

#include "stddb.h"

#include "hierdbpath.h"
#include "core/debug.h"

//Ctor:
cHierDbPath::cHierDbPath(void) : mbCaseSensitive(false), mDelimitingChar(_T('/'))
{
    ASSERT(mCurrPath.empty());
}

cHierDbPath::cHierDbPath(TCHAR cDelChar, bool bCaseSensitive)
    : mbCaseSensitive(bCaseSensitive), mDelimitingChar(cDelChar)
{
    ASSERT(mCurrPath.empty());
}

//Copy Ctor:
cHierDbPath::cHierDbPath(const cHierDbPath& rhs)
{
    mbCaseSensitive = rhs.mbCaseSensitive;
    mDelimitingChar = rhs.mDelimitingChar;

    if (!rhs.mCurrPath.empty())
    {
        DbPath::const_iterator it = rhs.mCurrPath.begin();
        for (; it != rhs.mCurrPath.end(); ++it)
            mCurrPath.push_back(*it);
    }
}

///////////////////////////////////////////////////////////////////////////////
// operator=
cHierDbPath& cHierDbPath::operator=(const cHierDbPath& rhs)
{
    if (this != &rhs)
    {
        mbCaseSensitive = rhs.mbCaseSensitive;
        mDelimitingChar = rhs.mDelimitingChar;

        if (!rhs.mCurrPath.empty())
        {
            DbPath::const_iterator it = rhs.mCurrPath.begin();
            for (; it != (rhs.mCurrPath.end()); ++it)
                mCurrPath.push_back(*it);
        }
    }
    return *this;
}

//Dtor:
cHierDbPath::~cHierDbPath(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Push: Adds a substring to the end of the sequence(path)
void cHierDbPath::Push(const TSTRING& str)
{
    mCurrPath.push_back(str);
}

///////////////////////////////////////////////////////////////////////////////
// Pop: Removes a substring from the end of the path and returns it as a
// TSTRING.
TSTRING cHierDbPath::Pop(void)
{
    TSTRING ret;

    if (mCurrPath.empty())
        return _T("");

    ret = mCurrPath.back();

    mCurrPath.pop_back();

    return ret;
}

void cHierDbPath::SetCaseSensitive(bool bCaseSensitive)
{
    mbCaseSensitive = bCaseSensitive;
}

bool cHierDbPath::IsCaseSensitive(void) const
{
    return mbCaseSensitive;
}

void cHierDbPath::SetDelimiter(TCHAR Delimiter)
{
    mDelimitingChar = Delimiter;
}

TCHAR cHierDbPath::GetDelimiter(void) const
{
    return mDelimitingChar;
}

void cHierDbPath::Clear(void)
{
    mCurrPath.clear();

    ASSERT(mCurrPath.empty());
}

///////////////////////////////////////////////////////////////////////////////
// AsString -- Returns a TSTRING representation of the current path
TSTRING cHierDbPath::AsString(void) const
{
    TSTRING ret = _T("");

    if (mCurrPath.size() == 1)
    {
        ret = *(mCurrPath.begin());
        ret += mDelimitingChar;
        return ret;
    }

    DbPath::const_iterator it = mCurrPath.begin();

    // The loop adds delimiting characters until the last element in the
    // sequence is reached, so we don't have a trailing character.
    while (it != mCurrPath.end())
    {
        // the loop is constructed in this odd fashion because I don't want a trailing mDelimiter
        ret += (*it);
        ++it;
        if (it != mCurrPath.end())
            ret += mDelimitingChar;
    }

    //return the completed TSTRING
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// GetSize: Returns the number of substrings in the sequence
int cHierDbPath::GetSize(void) const
{
    return mCurrPath.size();
}
