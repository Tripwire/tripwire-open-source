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
// hierdbpath.h :
//

#ifndef __HIERDBPATH_H
#define __HIERDBPATH_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#include <vector>

///////////////////////////////////////////////////////////////////////////////
// cHierDbPath:
//      A class for maintaining a pathname object that can be added to and
//      removed from using push and pop operations.  A toggling of case-
//      sensitivitity/insensitivity is also supplied, as well as the ability
//      to set/change the delimiting character in the path.
class cHierDbPath
{
public:
    // Ctor, Dtor:
    cHierDbPath(); //empty constructor:
        // Default delimiter = '/', default case-sensitivity = false

    cHierDbPath(TCHAR delChar, bool bCaseSensitive);
    ~cHierDbPath();

    // Copy Ctor:
    cHierDbPath(const cHierDbPath& rhs);

    cHierDbPath& operator=(const cHierDbPath& rhs);

    void  SetDelimiter(TCHAR cDelimiter);
    TCHAR GetDelimiter(void) const;
    void  SetCaseSensitive(bool bCaseSensitive);
    bool  IsCaseSensitive(void) const;

    void Clear(void);
    void Push(const TSTRING& str);
    // Pushes a substring onto the end of the path( using str );
    TSTRING Pop(void);
    // Pops a substring off of the end of the path, and returns it.
    // mCurrPath is altered by removing the substring.

    TSTRING AsString(void) const;
    // Returns a TSTRING representation of mCurrPath.

    int GetSize(void) const;
    // Returns the number of substrings in the path.

    typedef std::vector<TSTRING> DbPath;

private:
    bool  mbCaseSensitive;
    TCHAR mDelimitingChar;

    DbPath mCurrPath;
    // Data structure for holding the path.
};

#endif //__HIERDBPATH_H
