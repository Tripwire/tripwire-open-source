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
// policyparser_t -- test policy files
//                  TODO:
//                  we should put lots of files in ./testfiles, and verify:
//                  1.  correct files parse correctly
//                  2.  incorrect files cause errors
//                  3.  slightly incorrect files cause errors

#include "twparser/stdtwparser.h"
#include "core/debug.h"
#include "fco/fcospecimpl.h"
#include "twparser/parserhelper.h"
#include "twparser/policyparser.h"
#include "fs/fspropset.h"
#include "fco/fcospeclist.h"
#include "twtest/test.h"
#include "util/fileutil.h"

#include <fstream>
#include <unistd.h>

// helper class that checks output of each fcospec
class cPolicyParserTester
{
public:
    static bool VerifyNameAndStartPoint(iFCOSpec* pfsspec, TSTRING& name);
};

TSTRING get_test_file_dir()
{
    if (cFileUtil::IsDir("../src/parser/testfiles"))
        return "../src/parser/testfiles";

    if (cFileUtil::IsDir("src/parser/testfiles"))
        return "src/parser/testfiles";

    return "";
}

void test_policy_file(const std::string& polfile)
{
    cDebug::AddOutTarget(cDebug::OUT_STDOUT);

    TSTRING pol_path = get_test_file_dir();
    pol_path.append("/");
    pol_path.append(polfile);

    if (-1 == access(pol_path.c_str(), F_OK))
        skip("policy parser test file not found/accessible");

    std::ifstream in;
    in.open(pol_path.c_str());
    if (!in.good())
        throw eParserHelper(_T("couldn't open test file"));

    cPolicyParser parser(in);

    cGenreSpecListVector policy;
    cErrorQueue          errorQ;
    cErrorReporter       errorR;
    cErrorTracer         errorT;

    // set up an error bucket that will spit things to stderr
    errorT.SetChild(&errorR);
    errorQ.SetChild(&errorT);

    parser.Execute(policy, &errorQ);
    TEST("No exceptions thrown in cPolicyParser::Execute");

    TCERR << "Parsed policy test file " << polfile << std::endl;
    return;
}


void TestPolicyParser()
{
    cDebug d("TestPolicyParser()");

    test_policy_file("pol.txt");

    TCERR << "TestPolicyParser: Parser needs work to be able to test more than one policy" << std::endl;

    //    test_policy_file("directives.txt");  //fails unless you substitute your hostname for 'your_host' in this file

    // TODO: test currently segfaults if you create more than one policy parser in a process. (Not a real world scenario).

    /*    test_policy_file("poleasy.txt");
    test_policy_file("polhard.txt");
    test_policy_file("polruleattr.txt"); */
}

void RegisterSuite_PolicyParser()
{
    RegisterTest("PolicyParser", "Basic", TestPolicyParser);
}
