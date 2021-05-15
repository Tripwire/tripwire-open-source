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
// cmdlineparser_t.cpp

#include "core/stdcore.h"
#include "core/cmdlineparser.h"
#include "twtest/test.h"
//#include "tw/twutil.h"
//#include "tw/twstrings.h"

const int    argc1   = 9;
const TCHAR* argv1[] = {
    _T("tripwire"), _T("-m"), _T("Init"), _T("-tp"), _T("one"), _T("two"), _T("--verbose"), _T("frog"), _T("cat")};

const int    argc2   = 3;
const TCHAR* argv2[] = {_T("tripwire"), _T("-m"), _T("-v")};

const int    argc3   = 3;
const TCHAR* argv3[] = {
    _T("tripwire"),
    _T("dog"),
    _T("-v"),
};

// test with the last param wanting 1 or 0 parameters :-)
const int    argc4   = 5;
const TCHAR* argv4[] = {_T("tripwire"), _T("-tp"), _T("-v"), _T("frog"), _T("cat")};

const int    argc5   = 4;
const TCHAR* argv5[] = {_T("tripwire"), _T("-tp"), _T("-v"), _T("frog")};

#ifdef DEBUG
static void PrintCmdLine(int argc, const TCHAR** argv, cDebug d)
{
    TSTRING str;
    d.TraceDebug("Testing command line:\n");
    for (int i = 0; i < argc; i++)
    {
        str += argv[i];
        str += _T(" ");
    }
    d.TraceDebug(_T(">>>%s\n"), str.c_str());
}
#endif

static void test_parse(cCmdLineParser& parser, const int argc, const TCHAR** argv, bool should_throw)
{
#ifdef DEBUG
    cDebug d("test_parse");
    PrintCmdLine(argc, argv, d);
#endif

    bool threw = false;

    try
    {
        parser.Parse(argc, argv);
    }
    catch (eError& e)
    {
        if (!should_throw)
            TCERR << e.GetMsg() << std::endl;
        threw = true;
    }

    TEST(threw == should_throw);

#ifdef DEBUG
    parser.TraceContents();
#endif
}


void TestCmdLineParser()
{
    enum ArgId
    {
        ID_M,
        ID_TP,
        ID_V,
        ID_UNNAMED
    };

    cCmdLineParser p;
    p.AddArg(ID_M, TSTRING(_T("m")), TSTRING(_T("mode")), cCmdLineParser::PARAM_ONE);
    p.AddArg(ID_TP, TSTRING(_T("tp")), TSTRING(_T("twoparam")), cCmdLineParser::PARAM_MANY);
    p.AddArg(ID_V, TSTRING(_T("v")), TSTRING(_T("verbose")), cCmdLineParser::PARAM_NONE);
    p.AddArg(ID_UNNAMED, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);

    cDebug d("TestCmdLineParser");

    test_parse(p, argc1, argv1, false);
    test_parse(p, argc2, argv2, true);
    test_parse(p, argc3, argv3, true);
    test_parse(p, argc4, argv4, false);

    // command line arg mutual exclusion
    d.TraceDebug("** Making -m and -v mutually exclusive, then running on first cmd line...\n");
    p.AddMutEx(ID_M, ID_V);
    test_parse(p, argc1, argv1, true); // should fail

    // make the command line want one parameter
    d.TraceDebug("** Changing cmd line to only want one last param...\n");
    p.AddArg(ID_UNNAMED, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    test_parse(p, argc4, argv4, true);

    test_parse(p, argc5, argv5, false);

    // TODO -- test a bunch more!!!
}

void RegisterSuite_CmdLineParser()
{
    RegisterTest("CmdLineParser", "Basic", TestCmdLineParser);
}
