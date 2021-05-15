//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
/*
* Name....: stringutil_t.h
* Date....: 09/03/99
* Creator.: rdifalco
*
* Tests String Utils
*
*/

#ifndef __STRINGUTIL_T_H
#    define __STRINGUTIL_T_H

#include "core/platform.h"
#include "core/tchar.h"

#include "core/ntmbs.h"
#include "core/ntdbs.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Char traits for WCHAR16 (aka dbchar_t) and NTMBCS (mctype_t)
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void TestStringUtil()
{
    cDebug db("Test std::char_traits<dbchar_t>");

    db.TraceAlways("Entering...\n");

    tss::dbstring a;
    tss::dbstring b;
    std::string   c; // Control String

    static char     NTMBS1[] = {65, 66, 67, 68, 0};
    static dbchar_t NTDBS1[] = {65, 66, 67, 68, 0};
    static dbchar_t NTDBS2[] = {40, 66, 67, 68, 0};
    //    static dbchar_t NTDBS0[] = { 65, 66, 67, 68, 0, 0 };

    c.assign(NTMBS1);
    a.assign(NTDBS1);

    TEST(c == NTMBS1);
    TEST(a == NTDBS1);

    a.resize(0);
    b.resize(0);

    TEST(a.empty());
    TEST(b.empty());

    a.assign(NTDBS1);
    TEST(a.size() == 4);
    TEST(std::equal(a.begin(), a.end(), NTDBS1));
    TEST(b.size() == 0);

    a.resize(3);
#if !ARCHAIC_STL    
    TEST(a.c_str() != b.c_str());
#endif    
    TEST(std::equal(a.begin(), a.end(), NTDBS1));
    TEST(std::equal(b.begin(), b.end(), NTDBS1));

    a = NTDBS2;
    b = a;
    TEST(a == b);
    TEST(a.size() == b.size());

    TEST(a[0] == b[0]);
    TEST(a[1] == b[1]);
    TEST(a[2] == b[2]);
    TEST(a[3] == b[3]);

    const tss::dbstring a1 = a;
    const tss::dbstring b1 = b;
    TEST(a1[0] == b1[0]);
    TEST(a1[1] == b1[1]);
    TEST(a1[2] == b1[2]);
    TEST(a1[3] == b1[3]);

#    if 0
    try 
    {
        TEST( a[4] > 0 );
        TEST( false );    // exception did not get thrown
    }
    catch ( std::out_of_range )
    {
        ;
    }
#    endif

    //--Test Reference Counting

    tss::dbstring x(NTDBS1);
    tss::dbstring ref = x;

    TEST(x == ref);
    TEST(std::equal(x.begin(), x.end(), ref.begin()));
    TEST(x.size() == ref.size());


    ///////////////////////////
    // now test the crazy convert functions

    std::string singleStr, singleStrNull;
    wc16_string wc16Str, wc16StrNull;
    TSTRING     tStr, tStrNull;

    // the four functions for reference
    //std::string  TstrToStr( const TSTRING& tstr );
    //TSTRING      StrToTstr( const std::string& str );
    //TSTRING      WstrToTstr( const wc16_string& src );
    //wc16_string  TstrToWstr( const TSTRING& tstr );

    // test Null assignments
    singleStr = cStringUtil::TstrToStr(tStrNull);
    TEST(singleStr.length() == 0);
    tStr = cStringUtil::StrToTstr(singleStrNull);
    TEST(tStr.length() == 0);
    tStr = cStringUtil::WstrToTstr(wc16StrNull);
    TEST(tStr.length() == 0);
    wc16StrNull = cStringUtil::TstrToWstr(tStrNull);
    TEST(wc16StrNull.length() == 0);

    // test copying some text around
    singleStr = "123456789";
    tStr.erase();
    tStr = cStringUtil::StrToTstr(singleStr);
    TEST(tStr.length() == 9);
    wc16Str.resize(0);
    wc16Str = cStringUtil::TstrToWstr(tStr);
    TEST(wc16Str.length() == 9);
    singleStr.erase();
    singleStr = cStringUtil::TstrToStr(tStr);
    TEST(singleStr.length() == 9);
    tStr.erase();
    tStr = cStringUtil::WstrToTstr(wc16Str);
    TEST(tStr.length() == 9);

    db.TraceAlways("Done...\n");
}


#endif //__STRINGUTIL_T_H

void RegisterSuite_StringUtil()
{
    RegisterTest("StringUtil", "Basic", TestStringUtil);
}
