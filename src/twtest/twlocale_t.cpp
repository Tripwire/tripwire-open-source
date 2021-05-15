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
// Name....: twlocale_t.h
// Date....: 07/26/99
// Creator.: Brian McFeely (bmcfeely)
//
// Tests the cTWLocale class
//

#include "core/stdcore.h"
#include "core/debug.h"
#include "core/twlocale.h"
#include "test.h"

void TestAtoi();
void TestItoa();
void TestFlags();
void TestRoundtrip();
void TestHex();

#define ASSERT_THAT_IT_THROWS(x, error) \
    try                                 \
    {                                   \
        x;                              \
        TEST(false);                    \
    }                                   \
    catch (error&)                      \
    {                                   \
    }

void TestItoa()
{
    try
    {
        //
        // can't do ASSERT( str == _T("123456") )
        // because locale may turn it into "123,465" or whatever
        //

        //
        // setup
        //
        int32_t n = 123456;
        TSTRING str;

        //
        // Try formatting with our default locale
        //
        cTWLocale::InitGlobalLocale();
        cTWLocale::FormatNumber(n, str);
        TEST(str == "123456");

#if !ARCHAIC_STL	
        //
        // Try formatting with "" locale
        //
        std::locale::global(std::locale(""));
        cTWLocale::FormatNumber(n, str);
        TEST(str == "123,456");

        //
        // Try formatting with "C" locale
        //
        std::locale::global(std::locale(""));
        cTWLocale::FormatNumber(n, str);
        TEST(str == "123,456");
#endif	
    }
    catch (const std::runtime_error& e)
    {
        skip("Skipping test due to configuration issue w/ 'C' locale");
    }
}

void RegisterSuite_TWLocale()
{
    RegisterTest("TWLocale", "Itoa", TestItoa);
}
