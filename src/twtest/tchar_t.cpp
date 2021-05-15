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
#include "core/stdcore.h"

#include <string>
#include <iostream>
#include <fstream>

#ifndef __DEBUG_H
#include "core/debug.h"
#endif

#include "test.h"

TSTRING test_wost(int, const TSTRING&);
void    test_wist(const TSTRING&, cDebug& d);

void TestTCHAR()
{
    TCERR
        << "TODO: Right now this test mostly verifies that STL string & file classes work, which is not overly useful."
        << std::endl;

    cDebug d("TestTCHAR()");

    d.TraceDetail("Entering...\n");

    //Testing TCOUT:
    TCOUT << _T("Simple test of TSTRING (and TCOUT) :\n\n");
    TCERR << _T("This should show up on cerr") << std::endl;

    TSTRING pString;
    pString = _T("Hi Mom!");

    d.TraceDetail("%s \n", pString.c_str());
    d.TraceDetail("Isn't this cool?\n\n");

    //Begin fun tests of string streams:

    TSTRINGSTREAM wst;
    //can I declare it?

    TSTRING str;
    str           = _T("Kiteman");
    TSTRING test1 = _T("word");

    d.TraceDetail("Testing TOSTRINGSTREAM with TSTRING:\n");

#if !ARCHAIC_STL
    TOSTRINGSTREAM ost(_T("test up"));
#else
    TOSTRINGSTREAM ost;
    ost << _T("test up");
#endif
    ost << test1;

    tss_mkstr(out, ost);
    
    d.TraceDetail("%s \n", out.c_str());

    //if this gives output, then I'm really baffled...
    //test gets overwritten, yielding "word up"

    TSTRING output;
    output = test_wost(3, str);
    d.TraceDetail("%s \n", output.c_str());
    //A true statement!

    d.TraceDetail("Testing TISTRINGSTREAM with TSTRING:\n");
    TSTRING send = _T("These should appear on separate lines");
    test_wist(send, d);
    //Did they?
}

void TestFstreams()
{
    //Testing file streams

    //explict constructors of 'TIFSTREAM' and "TOFSTREAM' take char*
    std::string inputfile  = TwTestPath("fun");
    std::string outputfile = TwTestPath("mo'fun");

    //Set up the input file.
    TOFSTREAM out;
    out.open(inputfile.c_str(), std::ios::out);
    out << "Unicode is fun\n";
    out.close();

    TIFSTREAM from;
    from.open(inputfile.c_str(), std::ios::in);
    TEST(from);


    TOFSTREAM to(outputfile.c_str(), std::ios::trunc);
    TEST(to);

    //Copy contents of input file to output file.
    TCHAR ch;
    while (from.get(ch))
        to.put(ch);

    TEST(from.eof() && to);

    return;
}


TSTRING test_wost(int n, const TSTRING& inject)
{
#if !ARCHAIC_STL  
    TOSTRINGSTREAM wost(_T("Weird how this doesn't show up! "));
#else
    TOSTRINGSTREAM wost;
    wost << _T("Weird how this doesn't show up! ");
#endif    
    //It's getting overwritten, why?
    wost << _T("One out of every ") << n << _T(" children loves ") << inject << _T("!\n");
    return wost.str();
}

void test_wist(const TSTRING& input, cDebug& d)
{
#if !ARCHAIC_STL  
    TISTRINGSTREAM wist(input);
#else
    TSTRINGSTREAM wist;
    wist << input;
#endif    
    TSTRING        parse;
    while (wist >> parse)
        d.TraceDetail("%s \n", parse.c_str());
}

void RegisterSuite_TCHAR()
{
    RegisterTest("TCHAR", "Basic", TestTCHAR);
#if !ARCHAIC_STL
    RegisterTest("TCHAR", "FStreams", TestFstreams);
#endif    
}
