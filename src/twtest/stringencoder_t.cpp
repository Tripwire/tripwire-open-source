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
///////////////////////////////////////////////////////////////////////////////
// stringencoder_t.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "util/stdutil.h"
#include "util/stringencoder.h"
#include "twtest/test.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <iostream>

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

using namespace std;

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

void OutputString(TSTRING& str);

//=========================================================================
// FUNCTION CODE
//=========================================================================

void TestStringEncoder()
{
    TSTRING str      = _T("this is a non-quoted string");
    TSTRING strQ     = _T("here's a quote:\"and a slash:\\ ");
    TSTRING strJQ    = _T("\"");
    TSTRING strJS    = _T("\\");
    TSTRING strWeird = _T("\\\"\"\\\"\\\\\"\"\"\"");

    OutputString(str);
    OutputString(strQ);
    OutputString(strJQ);
    OutputString(strJS);
    OutputString(strWeird);
}

//=========================================================================
// UTIL FUNCTION CODE
//=========================================================================

void OutputString(TSTRING& str)
{
    cQuoteEncoder qe;

    /*    TCOUT << _T("Plain string:   <") << str << _T(">") << endl;
    TCOUT << _T("Encoded string: <") << qe.Encode( str ) << _T(">") << endl;
    TCOUT << _T("Decoded string: <") << qe.Unencode( str ) << _T(">") << endl << endl ; */

    TEST(str == qe.Unencode(qe.Encode(str)));
}

void RegisterSuite_StringEncoder()
{
    RegisterTest("StringEncoder", "Basic", TestStringEncoder);
}
