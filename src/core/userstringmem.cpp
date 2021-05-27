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
#include "stdcore.h"
#include "userstringmem.h"
#include "debug.h"

///////////////////////////////////////////////////////////////////////////////
// class cUserStringMemBased -- A implementation where the various string
//      sets are compiled into the executable (as opposed to loading them
//      from an external file at run time).  At startup someone needs to
//      construct a cUserStringMemBased and set any valid string sets.
///////////////////////////////////////////////////////////////////////////////

cUserStringMemBased::cUserStringMemBased()
{
    mCurrentStringSet = 0;
    mStringSets.clear();
}

cUserStringMemBased::~cUserStringMemBased()
{
    while (!mStringSets.empty())
    {
        mStringSets.erase(mStringSets.begin());
    }
}

// the abstract interface
bool cUserStringMemBased::SelectStringSet(int setID)
{
    if (mStringSets.find(setID) == mStringSets.end())
        return false;

    mCurrentStringSet = setID;
    return true;
}

const TCHAR* cUserStringMemBased::GetString(int stringID) const
{
    std::map<int, StringSet*>::const_iterator setItr;

    setItr = mStringSets.find(mCurrentStringSet);
    if (setItr == mStringSets.end())
    {
        // mCurrentStringSet is invallid
        ASSERT(false);
        return _T("<Bad String Set>");
    }

    StringSet::const_iterator stringItr;

    ASSERT(setItr->second);
    stringItr = setItr->second->find(stringID);
    if (stringItr == setItr->second->end())
    {
        // string not found
        ASSERT(false);
        return _T("<Missing String>");
    }

    return stringItr->second;
}

// Clear all strings in specified string set
void cUserStringMemBased::ClearStringSet(int id)
{
    std::map<int, StringSet*>::iterator setItr;

    setItr = mStringSets.find(id);
    if (setItr == mStringSets.end())
    {
        return;
    }

    delete setItr->second;
    mStringSets.erase(setItr);
}

// Add an array of string pairs to a string set.  The pair array passed in
// should terminate with an id of -1.  If this string set contains string IDs
// that already exist in the string set, they will "overwrite" the
// the previous strings.
// Note: We store only pointers to the string pairs so they must remain
// around for as long as this class uses them.  A static array defined at
// compile time is perfect for this.
void cUserStringMemBased::AddStringSet(int setID, const iUserString::tStringPair* pPairArray)
{
    std::map<int, StringSet*>::const_iterator setItr;

    setItr = mStringSets.find(setID);
    if (setItr == mStringSets.end())
    {
        // first time we have seen this setID
        setItr = mStringSets.insert(mStringSets.begin(), std::pair<int, StringSet*>(setID, new StringSet));
    }

    ASSERT(pPairArray);
    while (pPairArray->id >= 0)
    {
        StringSet::iterator stringItr;

        ASSERT(setItr->second);
        stringItr = setItr->second->find(pPairArray->id);
        if (stringItr != setItr->second->end())
        {
            // Already a string of for this ID,
            // so lets replace it!
            setItr->second->erase(stringItr);
        }

        setItr->second->insert(std::pair<int, TCHAR*>(pPairArray->id, pPairArray->string));

        pPairArray++;
    }
}

void cUserStringMemBased::AddString(int setID, int stringID, TCHAR* string)
{
    std::map<int, StringSet*>::const_iterator setItr;

    setItr = mStringSets.find(setID);
    if (setItr == mStringSets.end())
    {
        // first time we have seen this setID
        setItr = mStringSets.insert(mStringSets.begin(), std::pair<int, StringSet*>(setID, new StringSet));
    }

    StringSet::iterator stringItr;

    ASSERT(setItr->second);
    stringItr = setItr->second->find(stringID);
    if (stringItr != setItr->second->end())
    {
        // Already a string of for this ID,
        // so lets replace it!
        setItr->second->erase(stringItr);
    }

    setItr->second->insert(std::pair<int, TCHAR*>(stringID, string));
}
