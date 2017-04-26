//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2017 Tripwire,
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
// Name....: twlocale_t.h
// Date....: 07/26/99
// Creator.: Brian McFeely (bmcfeely)
//
// Tests the cTWLocale class
//

#include "core/stdcore.h"
#include "core/debug.h"
#include "core/twlocale.h"

void TestAtoi();
void TestItoa();
void TestFlags();
void TestRoundtrip();
void TestHex();

#define ASSERT_THAT_IT_THROWS( x, error ) \
    try \
    { \
        x; \
        TEST( false ); \
    } catch( error& ) {}

void TestTWLocale()
{
#ifdef DOESNTWORK
    TestHex();
    TestAtoi();
    TestItoa();    
    TestFlags();
    TestRoundtrip();
#endif//NOTDONE
}

#ifdef DOESNTWORK

void TestAtoi()
{
    //
    // setup
    //
    int32 n;
    TSTRING str = _T("123456");

    //
    // Try formatting with our default locale
    //
    cTWLocale::InitGlobalLocale();
    n = cTWLocale::FormatNumber( str );
    TEST( n == 123456 );

    //    
    // Try formatting with "" locale
    //
    std::locale::global( std::locale("") );
    n = cTWLocale::FormatNumber( str );
    TEST( n == 123456 );
    
    //    
    // Try formatting with "C" locale
    //
    std::locale::global( std::locale("") );
    n = cTWLocale::FormatNumber( str );
    TEST( n == 123456 );
}

void TestItoa()
{
    //
    // can't do ASSERT( str == _T("123456") )
    // because locale may turn it into "123,465" or whatever
    //

    //
    // setup
    //
    int32 n = 123456;
    TSTRING str;

    //
    // Try formatting with our default locale
    //
    cTWLocale::InitGlobalLocale();
    cTWLocale::FormatNumber( n, str );
    TCOUT << str << std::endl;

    //    
    // Try formatting with "" locale
    //
    std::locale::global( std::locale("") );
    cTWLocale::FormatNumber( n, str );
    TCOUT << str << std::endl;
    
    //    
    // Try formatting with "C" locale
    //
    std::locale::global( std::locale("") );
    cTWLocale::FormatNumber( n, str );
    TCOUT << str << std::endl;
}

void TestRoundtrip()
{
    //
    // init
    //
    cTWLocale::InitGlobalLocale();

    //
    // atoitoa
    // 
    TSTRING strIn  = _T("123456");
    TSTRING strOut;
    strOut = cTWLocale::FormatNumber( cTWLocale::FormatNumber( strIn ), strOut );
    // don't know if string will be the same due to possible changes in formatting from locale
    // ASSERT( strOut == strIn ); <---- can't do this ^^^
    TEST( 123456 == cTWLocale::FormatNumber( strOut ) );

    
    //
    // itoatoi
    // 
    int32 nIn  = 654321;
    int32 nOut;
    nOut = cTWLocale::FormatNumber( cTWLocale::FormatNumber( nIn, strIn ) );
    TEST( nOut == nIn );
}


void TestFlags()
{
    //
    // init
    //
    cTWLocale::InitGlobalLocale();

    // 
    // hex
    // 
    TSTRING str = _T("FF");
    int n = cTWLocale::FormatNumber( str, std::ios_base::hex );
    TEST( n == 0xFF );

    // 
    // bad number for dec
    //
    ASSERT_THAT_IT_THROWS( cTWLocale::FormatNumberAsHex( str ), eError );
    
    // 
    // oct
    // 
    TSTRING strOct = _T("0712");
    n = cTWLocale::FormatNumber( strOct, std::ios_base::oct );
    TEST( n == 0712 );
    
    // 
    // oct again
    // 
    strOct = _T("00712");
    n = cTWLocale::FormatNumber( strOct, std::ios_base::oct );
    TEST( n == 0712 );
    
    // 
    // oct again again
    // 
    strOct = _T("712");
    n = cTWLocale::FormatNumber( strOct, std::ios_base::oct );
    TEST( n == 0712 );

    // 
    // try bad oct
    //
    ASSERT_THAT_IT_THROWS( cTWLocale::FormatNumber( _T("99"), std::ios_base::oct ), eError );
}


void TestHex()
{
    TSTRING str;

    str = cTWLocale::FormatNumberAsHex( 0x1234 );
    TEST( str == _T("1234") );
    
    str = cTWLocale::FormatNumberAsHex( 16 );
    TEST( str == _T("10") );

    str = cTWLocale::FormatNumberAsHex( 0x12344321 );
    TEST( str == _T("12344321") );
    
    str = cTWLocale::FormatNumberAsHex( 0xFFFFFFFF );
    TEST( str == _T("FFFFFFFF") || str == _T("ffffffff") );
}
#endif//DOESNTWORK

