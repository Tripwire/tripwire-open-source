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

/* NOW WE USE tss::strinc
// like mblen but also for wchar_t
int util_tlen( const TCHAR* cur, size_t count )
{    
    int nch = -2; // 'unused' value

    ASSERT( count >= 0 );
    #ifdef _UNICODE   
        if( count > 0 )
            nch = 1;   // next char is always one TCHAR long
        else
            nch = 0;   // no more chars
    #else
        nch = ::mblen( cur, count ); // here sizeof(TCHAR) == 1
    #endif

    ASSERT( nch != -2 ); // make sure nch was set
    return nch;
}
*/


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
bool cCharUtil::PeekNextChar(  const TSTRING::const_iterator& cur, 
                               const TSTRING::const_iterator& end, 
                                     TSTRING::const_iterator& first, 
                                     TSTRING::const_iterator& last )
{
    //
    // do we have a valid string here?
    //
    if( cur > end )
    {
        return false;
    }

    if( cur == end )
    {
        first = last = end;
        return false;
    }

    if( *cur == _T('\0') )
    {
        first = last = cur;
        return false;
    }

    first = cur;
//    last = tss::strinc( cur );
    last = *cur ? cur + 1 : cur;
    return true;
}


/*  OLD way of doing this...
    NOW WE USE tss::strinc
int cCharUtil::PeekNextChar(   const TSTRING::const_iterator& cur, 
                               const TSTRING::const_iterator& end, 
                                     TSTRING::const_iterator& first, 
                                     TSTRING::const_iterator& last, 
                                     bool fThrowOnError )
{
    //
    // do we have a valid string here?
    //
    if( cur > end )
        return -1;

    //
    // determine length of character in TCHARs
    //
    int charSizeInTCHARs = util_tlen( cur, (size_t)end - (size_t)cur );
    if( charSizeInTCHARs == -1 ) // TODO:BAM -- what if size is zero?  does that make sense?
    {
        if( fThrowOnError )
            throw eCharUtilUnrecognizedChar();
        else
            return -1;
    }

    //
    // denote beginning and end of character
    //
    first   = cur;          // first char always starts at 'cur'
    last    = first + charSizeInTCHARs;
    
    //
    // there exist more characters
    //
    return charSizeInTCHARs;
}
*/

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
bool cCharUtil::PopNextChar(    TSTRING::const_iterator& cur, 
                          const TSTRING::const_iterator& end, 
                                TSTRING::const_iterator& first, 
                                TSTRING::const_iterator& last )
{
    bool f = PeekNextChar( cur, end, first, last );

    cur     = last;         // pop causes 'cur' to move to just beyond character ('last')

    return f;
}

// eof: charutil.cpp
