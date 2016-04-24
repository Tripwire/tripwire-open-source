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
// Name....: displayencoder_t.cpp
// Date....: 10/18/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "core/stdcore.h"

#ifdef TSS_TEST

#include "test/utx.h"
#include "displayencoder.h"
#include "debug.h"
#include "twlocale.h"
#include "errorbucketimpl.h"


///////////////////////////////////////////////////////////////////////////////
// UTIL 
///////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
    #define TSS_TCHAR_MIN   WCHAR_MIN
    #define TSS_TCHAR_MAX   WCHAR_MAX
#else
    #define TSS_TCHAR_MIN   CHAR_MIN
    #define TSS_TCHAR_MAX   CHAR_MAX
#endif

template< class CharT > bool IsPrintable( const std::basic_string< CharT >& str )
{
    const std::ctype< CharT > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );
    for( std::basic_string< CharT >::const_iterator at = str.begin(); at != str.end(); at++ )
    {           
        if( ! ct.is( std::ctype_base::print, *at ) ) // if not printable
            return false;
    }

    return true;
}

static void util_TestUnprintable( const TSTRING& strCUnprintable )
{
    cDisplayEncoder e( cDisplayEncoder::ROUNDTRIP );
    TSTRING strEncoded = strCUnprintable;

    e.Encode( strEncoded );
    TCOUT << strEncoded << std::endl;
    TSTRING strOut = strEncoded;
    e.Decode( strOut );

    ASSERT( strOut == strCUnprintable );
}


///////////////////////////////////////////////////////////////////////////////
// cDisplayEncoderTest
///////////////////////////////////////////////////////////////////////////////
class cDisplayEncoderTest
{
public:

    ///////////////////////////////////////////////////////////////////////////
    // TestCharToHex
    ///////////////////////////////////////////////////////////////////////////    
    void TestCharToHex( tss::TestContext& ctx )
    {
        TCHAR ch;
        TSTRING str;
        const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );

        // only use lowercase strings with this define
        #define TSS_CHAR_TO_HEX_TEST( s ) \
            ch = 0x ## s; \
            str = cCharEncoderUtil::char_to_hex( ch ); \
            ct.tolower( str.begin(), str.end() ); \
            ASSERT( str == _T( #s ) );

        TSS_CHAR_TO_HEX_TEST( fefe );
        TSS_CHAR_TO_HEX_TEST( 0000 );
        TSS_CHAR_TO_HEX_TEST( 1234 );
        TSS_CHAR_TO_HEX_TEST( ffff );
        TSS_CHAR_TO_HEX_TEST( 0001 );
        TSS_CHAR_TO_HEX_TEST( 543c );
        TSS_CHAR_TO_HEX_TEST( cccc );
        TSS_CHAR_TO_HEX_TEST( 9999 );
        TSS_CHAR_TO_HEX_TEST( abcd );
    }

    ///////////////////////////////////////////////////////////////////////////
    // TestHexToChar
    ///////////////////////////////////////////////////////////////////////////    
    void TestHexToChar( tss::TestContext& ctx )
    {
        TCHAR ch;
        TSTRING str;

        // only use lowercase strings with this define
        #define TSS_HEX_TO_CHAR_TEST( s ) \
            str = _T( #s ); \
            ch = cCharEncoderUtil::hex_to_char( str.begin(), str.end() ); \
            ASSERT( ch == 0x ## s );

        TSS_HEX_TO_CHAR_TEST( fefe );
        TSS_HEX_TO_CHAR_TEST( 0000 );
        TSS_HEX_TO_CHAR_TEST( 1234 );
        TSS_HEX_TO_CHAR_TEST( ffff );
        TSS_HEX_TO_CHAR_TEST( 0001 );
        TSS_HEX_TO_CHAR_TEST( 543c );
        TSS_HEX_TO_CHAR_TEST( cccc );
        TSS_HEX_TO_CHAR_TEST( 9999 );
        TSS_HEX_TO_CHAR_TEST( abcd );
    }

    //////////////////////////////////////////////////////////////////////////
    // TestStringToHex -- locale specific test -- only works in ASCII
    ///////////////////////////////////////////////////////////////////////////    
    void TestStringToHex( tss::TestContext& ctx )
    {
        TSTRING str;
        const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );

        // only use lowercase strings with this define
        #define TSS_STRING_TO_HEX_TEST( s, n ) \
            str = cCharEncoderUtil::CharStringToHexValue( _T( #s ) ); \
            ct.tolower( str.begin(), str.end() ); \
            ASSERT( str == _T( #n ) );

        TSS_STRING_TO_HEX_TEST( \n, 000a );
        TSS_STRING_TO_HEX_TEST( \r, 000d );
        TSS_STRING_TO_HEX_TEST( \r\n, 000d000a );
        TSS_STRING_TO_HEX_TEST( a\r\nb, 0061000d000a0062 );
    }

    //////////////////////////////////////////////////////////////////////////
    // TestHexToString -- locale specific test -- only works in Unicode
    ///////////////////////////////////////////////////////////////////////////    
    void TestHexToString( tss::TestContext& ctx )
    {
        TSTRING str;
        const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );

        // only use lowercase strings with this define
        #define TSS_HEX_TO_STRING_TEST( s, n ) \
            str = cCharEncoderUtil::HexValueToCharString( _T( #n ) ); \
            ct.tolower( str.begin(), str.end() ); \
            ASSERT( str == _T( #s ) );

        TSS_HEX_TO_STRING_TEST( \n, 000a );
        TSS_HEX_TO_STRING_TEST( \r, 000d );
        TSS_HEX_TO_STRING_TEST( \r\n, 000d000a );
        TSS_HEX_TO_STRING_TEST( a\r\nb, 0061000d000a0062 );
    }
    
    //////////////////////////////////////////////////////////////////////////
    // TestUnconvertable -- locale specific test -- only works in Unicode
    ///////////////////////////////////////////////////////////////////////////    
    void TestUnconvertable( tss::TestContext& ctx )
    {
        cDisplayEncoder e( cDisplayEncoder::ROUNDTRIP );
        const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );
        TSTRING str;
        TCHAR ch;

        // only use lowercase strings with this define
        #define TSS_UNCONVERTABLE_TEST( n ) \
            ch = 0x ## n; \
            str = ch; \
            e.Encode( str ); \
            ct.tolower( str.begin(), str.end() ); \
            ASSERT( str == _T("\\x") _T( #n ) _T("x") );

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
    void TestUnprintable( tss::TestContext& ctx )
    {
        cDisplayEncoder e( cDisplayEncoder::ROUNDTRIP );
        const std::ctype< TCHAR > *pct = 0, &ct = tss::GetFacet( std::locale(), pct );
        TSTRING str;
        TCHAR ch;

        // only use lowercase strings with this define
        #define TSS_UNPRINTABLE_TEST( n ) \
            ch = 0x ## n; \
            str = ch; \
            e.Encode( str ); \
            ct.tolower( str.begin(), str.end() ); \
            ASSERT( str == _T("\\x") _T( #n ) _T("x") );

        TSS_UNPRINTABLE_TEST( 000a );
        TSS_UNPRINTABLE_TEST( 000d );
    }

    //////////////////////////////////////////////////////////////////////////
    // TestQuoteAndBackSlash
    ///////////////////////////////////////////////////////////////////////////    
    void TestQuoteAndBackSlash( tss::TestContext& ctx )
    {
        cDisplayEncoder e( cDisplayEncoder::ROUNDTRIP );
        TSTRING str;

        str = _T("\\");
        e.Encode( str );
        ASSERT( str == _T("\\\\") );
            
        str = _T("\"");
        e.Encode( str );
        ASSERT( str == _T("\\\"") );
    }

    // TODO:BAM -- try multibyte now.....


    ///////////////////////////////////////////////////////////////////////////
    // Basic
    ///////////////////////////////////////////////////////////////////////////    
    void Basic( tss::TestContext& ctx )
    {
        try
        { 
            //=============================================================
            // TEST UNPRINTABLE ENCODING/ROUNDTRIP
            //=============================================================
    
            util_TestUnprintable( _T("normal string") );

            util_TestUnprintable( _T("return\n") );
            util_TestUnprintable( _T("ret\rurn\n") );
            util_TestUnprintable( _T("ret\rnurn\n") );

            util_TestUnprintable( _T("bell\x08") );
            util_TestUnprintable( _T("\x08 bell") );
            util_TestUnprintable( _T("be\x08ll") );

            util_TestUnprintable( _T("\x1F\x1F\x1F") );

            util_TestUnprintable( _T("big\xFF") );
            util_TestUnprintable( _T("\xEE big") );
            util_TestUnprintable( _T("\xEE\xEEtwo big") );
            util_TestUnprintable( _T("small\x01") );
            util_TestUnprintable( _T("\x01\x01two small") );
            
            //=============================================================
            // TEST UNCONVERTABLE CHARS
            //=============================================================
            TSTRING strMessWithMe  = _T("Mess with me...");
            for( size_t c = TSS_TCHAR_MIN; 
                 c < TSS_TCHAR_MAX; 
#ifdef _UNICODE 
                 c += ( TSS_TCHAR_MAX / 150 ) )
#else
                 c++ )
#endif
            {
                if( ( c != '\0' ) )
                {
                    strMessWithMe += c;
                }
            }
            util_TestUnprintable( strMessWithMe );
        
            //=============================================================
            // TEST \\ and \x ENCODING/ROUNDTRIP
            //=============================================================

            util_TestUnprintable( _T("\\Other \\\\slashes") );
            util_TestUnprintable( _T("\\Other slashes\\\\") );
            util_TestUnprintable( _T("O\\ther slashes\\\\") );
            util_TestUnprintable( _T("\\\\\\") );
        
            util_TestUnprintable( _T("\\xTricky") );
            util_TestUnprintable( _T("Tri\\xcky") );
            util_TestUnprintable( _T("Tricky\\x") );
            util_TestUnprintable( _T("\\Tricky\\\\x") );

            
            //=============================================================
            // TEST UNCONVERTABLE, UNPRINTABLE, AND \\ and \" CHARS
            //=============================================================
            TSTRING strMessWithMe2  = _T("Mess with me...");
            for( size_t ch = TSS_TCHAR_MIN; 
                 ch < TSS_TCHAR_MAX; 
#ifdef _UNICODE 
                 ch += ( TSS_TCHAR_MAX / 150 ) )
#else
                 ch++ )
#endif
            {
                if( ( ch != '\0' ) )
                {
                    strMessWithMe2 += ch;
                }
            }
            
            strMessWithMe2 += _T("\r\n\t\b\\\"\\\\\\\"\v\"");
            util_TestUnprintable( strMessWithMe2 );

            // TODO:BAM -- create multibyte tests (create a mb string at random, then test it.  
            // make sure there are '\' and '"' in it )        
        }
        catch( eError& e )
        {
            cErrorReporter::PrintErrorMsg( e ); 
            ASSERT(false);
        }
    }
};

TSS_BeginTestSuiteFrom( cDisplayEncoderTest )

    TSS_AddTestCase( Basic );
    TSS_AddTestCase( TestHexToChar );
    TSS_AddTestCase( TestCharToHex );
    TSS_AddTestCase( TestStringToHex );
    TSS_AddTestCase( TestHexToString );
    TSS_AddTestCase( TestUnconvertable ); 
    TSS_AddTestCase( TestUnprintable ); 
    TSS_AddTestCase( TestQuoteAndBackSlash ); 
        
TSS_EndTestSuite( cDisplayEncoderTest )

#endif // TSS_TEST

// eof: displayencoder_t.cpp
