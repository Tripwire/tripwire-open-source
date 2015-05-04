//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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

#ifndef __USERSTRING_H
#define __USERSTRING_H

#ifndef __DEBUG_H
#include "core/debug.h"
#endif

#ifndef _MAP_
#include <map>
#endif

///////////////////////////////////////////////////////////////////////////////
// class iUserString -- Abstract interface for retrieving User Strings
///////////////////////////////////////////////////////////////////////////////

class iUserString
{
public:
    virtual ~iUserString() {}

    struct tStringPair
    {
        int id;
        TCHAR* string;
    };
    
	// Select between the different localized string sets 
    // for this product.  Returns false if string not defined.
    virtual bool            SelectStringSet(int setID) = 0;

    // Get the string from the localized string set
    virtual const TCHAR*    GetString(int stringID) const = 0;

    // Clear all strings in specified string set
    virtual void ClearStringSet(int id) = 0;

    // Add an array of string pairs to a string set.  The pair array passed in
    // should terminate with an id of -1.  If this string set contains string IDs 
    // that already exist in the string set, they will "overwrite" the
    // the previous strings.  
    // Note: We store only pointers to the string pairs so they must remain 
    // around for as long as this class uses them.  A static array defined at
    // compile time is perfect for this.
    virtual void AddStringSet(int setID, const tStringPair* pPairArray) = 0;

    // Add a single string.  The above rules apply.
    virtual void AddString(int setID, int stringID, TCHAR* string) = 0;

	// singleton manipulation
	static iUserString* GetInstance();
	static void			SetInstance(iUserString* pInst);
private:
	static iUserString* mpInstance;
};

///////////////////////////////////////////////////////////////////////////////
// Macros useful for building UserString tables. 
///////////////////////////////////////////////////////////////////////////////

/*  
Example Use:

    static struct iUserString::tStringPair englishStrings[] =
    {
        USERSTRING(STR_HELLO_WORLD, "Hello World\n")
        USERSTRING(STR_OUT_OF_MEM, "Fatal Error: Out of memory.\n")
        USERSTRING_END()
    };

    pUSMB->AddStringSet(LANG_USENGLISH, englishStrings);
*/

#define USERSTRING(id, str) { (id), _T(str) },

#define USERSTRING_END()    { -1, _T("") }

///////////////////////////////////////////////////////////////////////////////////////////////////
// inline implementation
///////////////////////////////////////////////////////////////////////////////////////////////////
inline iUserString* iUserString::GetInstance()
{
	ASSERT(mpInstance);

	return mpInstance;
}

inline void iUserString::SetInstance(iUserString* pInst)
{
	mpInstance = pInst;
}


#ifdef  _MSC_VER
//#pragma warning(default:4786)
#endif  /* _MSC_VER */

#endif // __USERSTRING_H
