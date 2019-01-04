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
// UserString.h -- A single point of access for all strings that will be
//      displayed to the end user.  This implementation is intended to ease
//      any future localization efforts.
///////////////////////////////////////////////////////////////////////////////

#ifndef __DEBUG_H
#include "debug.h"
#endif

#ifndef __USERSTRINGMEM_H
#    define __USERSTRINGMEM_H

#    ifdef _MSC_VER
#        pragma warning(disable : 4786) /* disable unwanted C++ /W4 warning */
/* #pragma warning(default:4786) */     /* use this to reenable, if necessary */
#    endif                              /* _MSC_VER */

#    ifndef __USERSTRING_H
#        include "userstring.h"
#    endif

///////////////////////////////////////////////////////////////////////////////
// class cUserStringMemBased -- A implementation where the various string
//      sets are compiled into the executable (as opposed to loading them
//      from an external file at run time).  At startup someone needs to
//      construct a cUserStringMemBased and set any valid string sets.
///////////////////////////////////////////////////////////////////////////////

// Note: If we ever want to use an external text file for User Strings, we
// could derive another class from iUserString called cUserStringFileBased
// that retrieved strings from files.

class cUserStringMemBased : public iUserString
{
public:
    cUserStringMemBased();
    ~cUserStringMemBased();

    // the abstract interface
    virtual bool         SelectStringSet(int setID);
    virtual const TCHAR* GetString(int stringID) const;
    virtual void         ClearStringSet(int id);
    virtual void         AddStringSet(int setID, const iUserString::tStringPair* pPairArray);
    virtual void         AddString(int setID, int stringID, TCHAR* string);

private:
    typedef std::map<int, TCHAR*> StringSet;

    int                       mCurrentStringSet;
    std::map<int, StringSet*> mStringSets;
};


#    ifdef _MSC_VER
//#pragma warning(default:4786)
#    endif /* _MSC_VER */

#endif // __USERSTRINGMEM_H
