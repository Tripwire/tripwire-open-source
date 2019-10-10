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
// Name....: charutil.cpp
// Date....: 10/20/99
// Creator.: Brian McFeely (bmcfeely)
//
// [Description]
//

#include "stdcore.h"
#include "charutil.h"
#include "ntmbs.h"

#if !HAVE_MBLEN && HAVE_MBTOWC
// Musl libc & Android NDK implement mblen just like this
int mblen(const char* s, size_t n)
{
    return mbtowc(0, s, n);
}
#endif

//
// finds the next whole character in string identified by ['cur'-'end')
// identifies beginning of char in 'first', then end of character in 'last'
// returns number of TCHARs that make up the next character
// if there are no more characters, will return 0 and first = last = end
// POSTCONDITIONS:
//
// RETURNS:
//
// THROWS:
//
// COMPLEXITY:
//
//

/* static */
bool cCharUtil::PeekNextChar(const TSTRING::const_iterator& cur,
                             const TSTRING::const_iterator& end,
                             TSTRING::const_iterator&       first,
                             TSTRING::const_iterator&       last)
{
    //
    // do we have a valid string here?
    //
    if (cur > end)
    {
        return false;
    }

    if (cur == end)
    {
        first = last = end;
        return false;
    }

    if (*cur == _T('\0'))
    {
        first = last = cur;
        return false;
    }

    first = cur;

    if (!(*cur))
    {
        last = cur;
    }
    else
    {
#if !IS_AROS
        mblen(NULL, 0);
        int len = mblen(&*cur, MB_CUR_MAX);
        if (len < 0) //invalid multibyte sequence, but let's not blow up.
            len = 1;

        last = cur + len;
#else // AROS mblen() seems broken (as of 6/2016) so don't use it.
        last = cur + 1;
#endif
    }

    return true;
}


//=============================================================================
//
// /* static */
// bool cCharUtil::PopNextChar(   TSTRING::const_iterator& cur,
//               const TSTRING::const_iterator& end,
//                     TSTRING::const_iterator& first,
//                     TSTRING::const_iterator& last )
//-----------------------------------------------------------------------------
// REQUIRES:
//
// EFFECTS:
//
// same as PeekNextChar but increments 'cur' to 'last'
//
// POSTCONDITIONS:
//
// RETURNS:
//
// THROWS:
//
// COMPLEXITY:
//
//

/* static */
bool cCharUtil::PopNextChar(TSTRING::const_iterator&       cur,
                            const TSTRING::const_iterator& end,
                            TSTRING::const_iterator&       first,
                            TSTRING::const_iterator&       last)
{
    bool f = PeekNextChar(cur, end, first, last);

    cur = last; // pop causes 'cur' to move to just beyond character ('last')

    return f;
}
