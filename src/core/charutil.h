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
// Name....: charutil.h
// Date....: 10/20/99
// Creator.: Brian McFeely (bmcfeely)
//
// [Description]
//

#ifndef __CHARUTIL_H
#define __CHARUTIL_H

// TODO:BAM -- get rid of this class and just use tss::strinc() !!!!!
class cCharUtil
{
public:
    // finds the next whole character in string identified by ['cur'-'end')
    // identifies beginning of char in 'first', then end of character in 'last'
    // returns 'are there more characters in string?'
    // if there are no more characters, will return 0 and first = last = end
    static bool PeekNextChar(const TSTRING::const_iterator& cur,
                             const TSTRING::const_iterator& end,
                             TSTRING::const_iterator&       first,
                             TSTRING::const_iterator&       last);

    // same as PeekNextChar but increments 'cur' to 'last'
    static bool PopNextChar(TSTRING::const_iterator&       cur,
                            const TSTRING::const_iterator& end,
                            TSTRING::const_iterator&       first,
                            TSTRING::const_iterator&       last);
};

#endif //__CHARUTIL_H
