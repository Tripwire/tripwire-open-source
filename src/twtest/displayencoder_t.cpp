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
//
// Name....: displayencoder_t.cpp
// Date....: 10/18/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "core/stdcore.h"


#include "core/displayencoder.h"
#include "core/debug.h"
#include "core/twlocale.h"
#include "core/errorbucketimpl.h"
#include "twtest/test.h"

///////////////////////////////////////////////////////////////////////////////
// UTIL
///////////////////////////////////////////////////////////////////////////////

#define TSS_TCHAR_MIN CHAR_MIN
#define TSS_TCHAR_MAX CHAR_MAX

/*
template< class CharT > bool IsPrintable( const std::basic_string< CharT >& str )
{
    const std::ctype< CharT > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );
    for( std::basic_string< CharT >::const_iterator at = str.begin(); at != str.end(); ++at )
    {           
        if( ! ct.is( std::ctype_base::print, *at ) ) // if not printable
            return false;
    }

    return true;
}
*/

static void util_TestUnprintable(const TSTRING& strCUnprintable)
{
    cDisplayEncoder e(cDisplayEncoder::ROUNDTRIP);
    TSTRING         strEncoded = strCUnprintable;

    e.Encode(strEncoded);
    //    TCOUT << strEncoded << std::endl;
    TSTRING strOut = strEncoded;
    e.Decode(strOut);

    TEST(strOut == strCUnprintable);
}


///////////////////////////////////////////////////////////////////////////
// TestCharToHex
///////////////////////////////////////////////////////////////////////////
void test_char_to_hex(char ch, const TSTRING& expected)
{
    TSTRING observed = cCharEncoderUtil::char_to_hex(ch);
    TEST(expected == observed);
}


void TestCharToHex()
{
    test_char_to_hex(0xfe, "fe");
    test_char_to_hex(0xff, "ff");
    test_char_to_hex(0x00, "00");
    test_char_to_hex(0x01, "01");
    test_char_to_hex(0x7f, "7f");
    test_char_to_hex(0x80, "80");
}


///////////////////////////////////////////////////////////////////////////
// TestHexToChar
///////////////////////////////////////////////////////////////////////////
void test_hex_to_char(const TSTRING& str, char expected, bool should_throw = false)
{
    bool threw = false;
    try
    {
        char observed = cCharEncoderUtil::hex_to_char(str.begin(), str.end());
        TEST(expected == observed);
    }
    catch (eError& e)
    {
        threw = true;
    }

    TEST(should_throw == threw);
}


void TestHexToChar()
{
    test_hex_to_char("fe", 0xfe);
    test_hex_to_char("ff", 0xff);
    test_hex_to_char("00", 0x00);
    test_hex_to_char("01", 0x01);
    test_hex_to_char("7f", 0x7f);
    test_hex_to_char("80", 0x80);

    test_hex_to_char("100", 0, true); // should throw
    test_hex_to_char("-01", 0, true); // should throw
}


//////////////////////////////////////////////////////////////////////////
// TestStringToHex -- locale specific test -- only works in ASCII
///////////////////////////////////////////////////////////////////////////
void test_string_to_hex(const std::string& str, const std::string& expected)
{
    std::string observed = cCharEncoderUtil::CharStringToHexValue(str);
    TEST(expected == observed);
}

void TestStringToHex()
{
    test_string_to_hex("\n", "0a");
    test_string_to_hex("\r", "0d");
    test_string_to_hex("\r\n", "0d0a");
    test_string_to_hex("a\r\nb", "610d0a62");
}


//////////////////////////////////////////////////////////////////////////
// TestHexToString -- locale specific test -- only works in Unicode
///////////////////////////////////////////////////////////////////////////
void test_hex_to_string(const std::string& str, const std::string& expected)
{
    std::string observed = cCharEncoderUtil::HexValueToCharString(str);
    TEST(expected == observed);
}

void TestHexToString()
{
    test_hex_to_string("0a", "\n");
    test_hex_to_string("0d", "\r");
    test_hex_to_string("0d0a", "\r\n");
    test_hex_to_string("610d0a62", "a\r\nb");
}

#if 0
//////////////////////////////////////////////////////////////////////////
// TestUnconvertable -- locale specific test -- only works in Unicode
///////////////////////////////////////////////////////////////////////////    
void TestUnconvertable()
{
    cDisplayEncoder e( cDisplayEncoder::ROUNDTRIP );
    const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );
    TSTRING str;
    TCHAR ch;

    // only use lowercase strings with this define
#    define TSS_UNCONVERTABLE_TEST(n)       \
        ch  = 0x##n;                        \
        str = ch;                           \
        e.Encode(str);                      \
        ct.tolower(str.begin(), str.end()); \
        TEST(str == _T("\\x") _T(#n) _T("x"));

    TSS_UNCONVERTABLE_TEST( fefe );
    TSS_UNCONVERTABLE_TEST( 1234 );
    TSS_UNCONVERTABLE_TEST( ffff );
    TSS_UNCONVERTABLE_TEST( 1000 );
    TSS_UNCONVERTABLE_TEST( 543c );
    TSS_UNCONVERTABLE_TEST( cccc );
    TSS_UNCONVERTABLE_TEST( 9999 );
    TSS_UNCONVERTABLE_TEST( 0123 );
    TSS_UNCONVERTABLE_TEST( 0100 );
}

//////////////////////////////////////////////////////////////////////////
// TestUnprintable -- locale specific test -- only works in Unicode
///////////////////////////////////////////////////////////////////////////    
void TestUnprintable()
{
    cDisplayEncoder e( cDisplayEncoder::ROUNDTRIP );
    const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );
    TSTRING str;
    TCHAR ch;

    // only use lowercase strings with this define
#    define TSS_UNPRINTABLE_TEST(n)         \
        ch  = 0x##n;                        \
        str = ch;                           \
        e.Encode(str);                      \
        ct.tolower(str.begin(), str.end()); \
        TEST(str == _T("\\x") _T(#n) _T("x"));

    TSS_UNPRINTABLE_TEST( 000a );
    TSS_UNPRINTABLE_TEST( 000d );
}
#endif

//////////////////////////////////////////////////////////////////////////
// TestQuoteAndBackSlash
///////////////////////////////////////////////////////////////////////////
void TestQuoteAndBackSlash()
{
    cDisplayEncoder e(cDisplayEncoder::ROUNDTRIP);
    TSTRING         str;

    str = _T("\\");
    e.Encode(str);
    TEST(str == _T("\\\\"));

    str = _T("\"");
    e.Encode(str);
    TEST(str == _T("\\\""));
}

// TODO:BAM -- try multibyte now.....


///////////////////////////////////////////////////////////////////////////
// Basic
///////////////////////////////////////////////////////////////////////////
void TestDisplayEncoderBasic1()
{
    //=============================================================
    // TEST UNPRINTABLE ENCODING/ROUNDTRIP
    //=============================================================

    util_TestUnprintable(_T("normal string"));

    util_TestUnprintable(_T("return\n"));
    util_TestUnprintable(_T("ret\rurn\n"));
    util_TestUnprintable(_T("ret\rnurn\n"));

    util_TestUnprintable(_T("bell\x08"));
    util_TestUnprintable(_T("\x08 bell"));
    util_TestUnprintable(_T("be\x08ll"));

    util_TestUnprintable(_T("\x1F\x1F\x1F"));

    util_TestUnprintable(_T("big\xFF"));
    util_TestUnprintable(_T("\xEE big"));
    util_TestUnprintable(_T("\xEE\xEEtwo big"));
    util_TestUnprintable(_T("small\x01"));
    util_TestUnprintable(_T("\x01\x01two small"));
}

void TestDisplayEncoderBasic2()
{
    //=============================================================
    // TEST UNCONVERTABLE CHARS
    //=============================================================
    TSTRING strMessWithMe = _T("Mess with me...");
    for (size_t c = TSS_TCHAR_MIN; c < TSS_TCHAR_MAX; c++)
    {
        if ((c != '\0'))
        {
            strMessWithMe += c;
        }
    }
    util_TestUnprintable(strMessWithMe);
}

void TestDisplayEncoderBasic3()
{
    //=============================================================
    // TEST \\ and \x ENCODING/ROUNDTRIP
    //=============================================================

    util_TestUnprintable(_T("\\Other \\\\slashes"));
    util_TestUnprintable(_T("\\Other slashes\\\\"));
    util_TestUnprintable(_T("O\\ther slashes\\\\"));
    util_TestUnprintable(_T("\\\\\\"));

    util_TestUnprintable(_T("\\xTricky"));
    util_TestUnprintable(_T("Tri\\xcky"));
    util_TestUnprintable(_T("Tricky\\x"));
    util_TestUnprintable(_T("\\Tricky\\\\x"));
}

void TestDisplayEncoderBasic4()
{
    //=============================================================
    // TEST UNCONVERTABLE, UNPRINTABLE, AND \\ and \" CHARS
    //=============================================================
    TSTRING strMessWithMe2 = _T("Mess with me...");
    for (size_t ch = TSS_TCHAR_MIN; ch < TSS_TCHAR_MAX; ch++)
    {
        if ((ch != '\0'))
        {
            strMessWithMe2 += ch;
        }
    }

    strMessWithMe2 += _T("\r\n\t\b\\\"\\\\\\\"\v\"");
    util_TestUnprintable(strMessWithMe2);

    // TODO:BAM -- create multibyte tests (create a mb string at random, then test it.
    // make sure there are '\' and '"' in it )
}

void RegisterSuite_DisplayEncoder()
{
    RegisterTest("DisplayEncoder", "Basic1", TestDisplayEncoderBasic1);
    RegisterTest("DisplayEncoder", "Basic2", TestDisplayEncoderBasic2);
    RegisterTest("DisplayEncoder", "Basic3", TestDisplayEncoderBasic3);
    RegisterTest("DisplayEncoder", "Basic4", TestDisplayEncoderBasic4);
    RegisterTest("DisplayEncoder", "CharToHex", TestCharToHex);
    RegisterTest("DisplayEncoder", "HexToChar", TestHexToChar);
    RegisterTest("DisplayEncoder", "StringToHex", TestStringToHex);
    RegisterTest("DisplayEncoder", "HexToString", TestHexToString);
    //RegisterTest("DisplayEncoder", "Unconvertable", TestUnconvertable);
    //RegisterTest("DisplayEncoder", "Unprintable", TestUnprintable);
    RegisterTest("DisplayEncoder", "QuoteAndBackSlash", TestQuoteAndBackSlash);
}
