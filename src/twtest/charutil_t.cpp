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
// Name....: charutil_t.cpp
// Date....: 10/22/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "core/stdcore.h"

#include "core/charutil.h"
#include "core/debug.h"
#include "core/errorbucketimpl.h"
#include "twtest/test.h"

#include <algorithm>
#include <locale.h>

bool localeIsUtf8()
{
    std::string locale(setlocale(LC_CTYPE, 0));
    std::transform(locale.begin(), locale.end(), locale.begin(), ::tolower);

    if (locale.find("utf-8") != std::string::npos)
        return true;

    if (locale.find("utf8") != std::string::npos)
        return true;

    return false;
}

void CheckChars(const TSTRING& str, int length_expected = 1)
{
    TSTRING::const_iterator cur = str.begin();
    TSTRING::const_iterator end = str.end();
    TSTRING::const_iterator first, last;

    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        int length = (int)(last - first);
        if (length != length_expected)
            TCERR << "CheckChars on '" << str << "' : expected = " << length_expected << " | observed = " << length
                  << std::endl;
        TEST(length == length_expected);
    }
}

///////////////////////////////////////////////////////////////////////////
// Basic
///////////////////////////////////////////////////////////////////////////
void TestCharUtilBasic()
{
    CheckChars("foo");
    CheckChars("fo\x23 54");

    // Test case requires support for Unicode escape sequences
#if USE_UNICODE_ESCAPES
    if (localeIsUtf8())
        CheckChars("\U0001F408", 4); //Cat emoji, if UTF-8
    else
        CheckChars("\U0001F408", 1); // just a bag of bytes otherwise
#endif
}

void RegisterSuite_CharUtil()
{
    RegisterTest("CharUtil", "Basic", TestCharUtilBasic);
}
