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
// Name....: codeconvert_t.cpp
// Date....: 9/8/99
// Creator.: Brian McFeely (bmcfeely)
//
// [Description]

#include "core/stdcore.h"
#include "core/codeconvert.h"
#include "core/wchar16.h"
#include "twtest/test.h"

#include <iomanip>

bool util_IsWideCharSameAsNarrow(char ch);
bool LowASCIILooksLikeUCS2InWchart();
void TestMbToDb();
void TestDbToMb();

void TestCodeConverter()
{
    cDebug d("TestCodeConverter()");

#if 0 //( !(HAVE_ICONV_H) && WCHAR_REP_IS_UCS2 )

    //
    // check that rep is really UCS2
    //
    TEST( LowASCIILooksLikeUCS2InWchart() );
#endif

    d.TraceDetail("Testing multi byte to double byte conversion.\n");
    TestMbToDb();
    d.TraceDetail("TestMbToDb() done.\n");

    // Took out this test as it currently throws and exception.
    // We expect not to be able to convert every UCS2 to a multi-byte char.
    d.TraceDetail("Testing double byte to multi byte conversion.\n");
    TestDbToMb();
}

// first last identify the lhs string
// x identifies the start of the second string
// start identifies the original start of the lhs string
template<class IterT> bool Compare(IterT first, IterT last, IterT x, IterT start)
{
    std::pair<IterT, IterT> p = std::mismatch(first, last, x);

    if (p.first != last)
    {
        // success !!
        std::cout << "*** mismatched value at: " << (int)(p.first - start)
                  << ".  Values are: " << (size_t)tss::util::char_to_size(*p.first) << " and "
                  << (size_t)tss::util::char_to_size(*p.second) << std::endl;

        return Compare(p.first + 1, last, p.second + 1, start);
    }

    return true;
}

void CompareStrings(const std::string& s1, const std::string& s2)
{
    if (s1.length() != s2.length())
    {
        std::cout << "*** string lengths didn't match. Lengths were: " << s1.length() << " and " << s2.length()
                  << std::endl;
    }

    if (Compare(s1.begin(), s1.end(), s2.begin(), s1.begin()))
    {
        std::cout << "** string matched." << std::endl;
    }
}


void DisplayString(const std::string& s)
{
    std::string::size_type i;
    for (i = 0; i < s.length(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (size_t)(unsigned char)s[i] << " ";
    }
}

size_t DistanceToOne(size_t n)
{
    size_t dist;
    for (dist = 0; n != 1; dist++)
    {
        n = (n >> 1);
    }

    TEST(n == 1); // n was not a power of 2!

    return dist;
}


void ConvertAndCompareString(const std::string& s)
{
    std::cout << "* Converting: ";
    DisplayString(s);
    std::cout << std::endl;

    // convert to dbchar_t string
    int         nWrote;
    wc16_string ws;
    ws.resize(s.length());
    nWrote = iCodeConverter::GetInstance()->Convert((ntdbs_t)ws.c_str(), ws.length(), s.c_str(), ws.length());
    TEST(nWrote != -1);
    ws.resize(nWrote);

    // convert back to mbchar_t string
    std::string s2;
    s2.resize(ws.length() * MB_CUR_MAX);
    nWrote = iCodeConverter::GetInstance()->Convert((ntmbs_t)s2.c_str(), s2.length(), ws.c_str(), ws.length());
    TEST(nWrote != -1);
    s2.resize(nWrote);

    std::cout << "* Result    : ";
    DisplayString(s2);
    std::cout << std::endl;

    CompareStrings(s, s2);
}

char NonZeroChar(char ch)
{
    return ch == 0 ? '0' : ch;
}

// mbchar_t to dbchar_t
//TestMbToDb in codeconvert_t.cpp seems to hit an infinite loop or runs verrrry long; ifdef'd"
void TestMbToDb()
{
    skip("This test is flaky & needs to be fixed/replaced; currently disabled.");

#if 0
    std::string s;
    s.resize( 0x10000 * 2 ); // two bytes for each combination

    for( size_t i = 0; i < 0x1000; i++ )
    {
        for( size_t j = 0; j < 0x10; j++ )
        {
            size_t first_byte = ( i & 0xFF00 ) >> 8;
            TEST( first_byte <= 0xFF );

            size_t second_byte = ( ( i & 0x00F0 ) >> 4 ) | j;
            TEST( second_byte <= 0xFF );

            s[ 2 * j ]          = NonZeroChar( (char)first_byte );
            s[ ( 2 * j ) + 1 ]  = NonZeroChar( (char)second_byte );
        }
        
        ConvertAndCompareString( s );
    }
#endif
    /*
    const std::string::size_type TOTAL_VALUE_COMBINATIONS = 0x10000; // 0x100 ^ 2 (256 possible per byte, and two bytes) (must always be this value)
    const std::string::size_type CHARS_AT_A_TIME = 0x10; // can change this, but needs to be a power of 2
    const std::string::size_type FIRST_BYTE_MASK = TOTAL_VALUE_COMBINATIONS - CHARS_AT_A_TIME;
    const std::string::size_type SECOND_BYTE_MASK = CHARS_AT_A_TIME - 1;

    std::string s;
    s.resize( CHARS_AT_A_TIME * 2 ); // two bytes for each combination

    for( size_t i = 0; i < TOTAL_VALUE_COMBINATIONS / CHARS_AT_A_TIME; i++ )
    {
        for( size_t j = 0; j < CHARS_AT_A_TIME; j++ )
        {
            size_t first_byte = ( ( i & FIRST_BYTE_MASK ) >> DistanceToOne( CHARS_AT_A_TIME ) );
            TEST( first_byte <= 0xFF );

            size_t second_byte = ( ( i & ( SECOND_BYTE_MASK << DistanceToOne( CHARS_AT_A_TIME ) ) | j );
            TEST( second_byte <= 0xFF );

            s[ 2 * j ]          = NonZeroChar( (char)first_byte );
            s[ ( 2 * j ) + 1 ]  = NonZeroChar( (char)second_byte );
        }
        
        ConvertAndCompareString( s );
    }

    */
}


// dbchar_t to mbchar_t
void TestDbToMb()
{
    skip("This test fails, most likely due to not speaking UTF-16. Should fix this.");

#if 0
    wc16_string ws;
    wc16_string::size_type n;
    const wc16_string::size_type max = 0x10000;
 
    // Setup string will all UCS2 characters
    ws.resize( max );
    for( n = 1; n < max; n++ )
    {
        TEST( n < std::numeric_limits< wc16_string::size_type >::max() );
        ws[ n - 1 ] = (WCHAR16)n;    
    }
 
    // convert to mbchar_t string
    std::string s;
    s.resize((max - 1) * MB_CUR_MAX);
    iCodeConverter::GetInstance()->Convert( (ntmbs_t)s.c_str(), s.length(), ws.c_str(), max - 1 );
    
    // convert back to dbchar_t string
    wc16_string ws2;
    ws2.resize(max - 1);
    iCodeConverter::GetInstance()->Convert( (ntdbs_t)ws2.c_str(), max - 1, s.c_str(), s.length() );
 
    TEST( ws.compare( ws2 ) == 0 );
#endif
}

#if 0
bool util_IsWideCharSameAsNarrow( char ch )
{
    cDebug d("LowASCIILooksLikeUCS2InWchart()");

    //
    // translate to a wide char
    //
    wchar_t wc;
    int i = mbtowc( &wc, &ch, 1 );

    //
    // assure that it has some representation in 
    // the wchar character set
    //
    if( i == -1 )
    {
        d.TraceDebug( "mbtowc failed on ch=0x%04X\n", (size_t)(unsigned char)ch );
        return false;
    }

    //
    // assure that the wide char representation looks like
    // UCS2 ( a 8859-1 char in UCS2 looks like 0x00XX, where
    // XX is the value of the char )
    //
    if( (char)wc != ch )
    {
        d.TraceDebug( "comparison failed on ch=0x%04X, wc=0x%04X\n", (size_t)(unsigned char)ch, (size_t)wc );
        return false;
    }

    return true;
}


bool LowASCIILooksLikeUCS2InWchart()
{
    cDebug d("LowASCIILooksLikeUCS2InWchart()");
    bool fOK = true;

#    if 0 // I hear this function isn't even correct... rjf

    //
    // save old locale
    //
    char* pOldLocale = 
        strdup( 
        setlocale( LC_CTYPE, NULL ) );
    d.TraceDebug( "Old locale: %s\n", pOldLocale );

    //
    // set to C locale
    //
    setlocale( LC_CTYPE, "C" );
    TEST( 0 == strcmp( "C", setlocale( LC_CTYPE, NULL ) ) );

    //
    // check each C locale char ( which is the ISO 8859-1 set )
    // against it's wchar_trepresentation
    //
    for( char ch = 0x00; fOK && (unsigned char)ch < 0x80U; ch++ )
        fOK &= util_IsWideCharSameAsNarrow( ch );

    //
    // reset locale
    //
    setlocale( LC_CTYPE, pOldLocale );
    TEST( 0 == strcmp( pOldLocale, setlocale( LC_CTYPE, NULL ) ) );
    free( pOldLocale );

#    endif
    return fOK;
}
#endif

void RegisterSuite_CodeConvert()
{
    RegisterTest("CodeConvert", "MbToDb", TestMbToDb);
    RegisterTest("CodeConvert", "DbToMb", TestDbToMb);
}
