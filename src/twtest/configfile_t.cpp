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
//configfile_t.cpp : a test driver for cConfigFile
//
//Don't see a test driver for this module in source safe -
//hopefully this hasn't been implemented already!  -DA

#include "tw/stdtw.h"
#include "tw/configfile.h"
#include "core/errorbucketimpl.h"
#include "twtest/test.h"
#include "core/debug.h"
#include <string>
#include "core/fsservices.h"
#include "fco/fconame.h"
#include "tw/twutil.h"

using namespace std;

static void assertParse(const std::string& configLineIn, bool expectValid)
{
    static const std::string sMandatory = "\nPOLFILE=foo"
                                          "\nDBFILE=foo"
                                          "\nREPORTFILE=foo"
                                          "\nSITEKEYFILE=foo"
                                          "\nLOCALKEYFILE=foo";

    bool        threw = false;
    cConfigFile cfg;

    std::string configLine = configLineIn + sMandatory;

    try
    {
        cfg.ReadString(configLine);
    }
    catch (eConfigFileMissReqKey& e)
    {
        TCERR << "Got a missing key exception, which should not happen" << std::endl;
        TEST(false);
    }
    catch (eConfigFile& e)
    {
        e.SetFatality(false);
        cTWUtil::PrintErrorMsg(e);

        threw = true;
    }

#ifdef _DEBUG
    TCERR << "LINE [" << configLineIn << "]" << std::endl
          << "Expected = " << expectValid << std::endl
          << "Threw = " << threw << std::endl;
#endif

    TEST(expectValid != threw);
}


void TestConfigFile(void)
{
    // should succeed
    assertParse(_T("BRIAN=foo"), true);                                        // 0 fine
    assertParse(_T("BRIAN=foo\nBILL=bar"), true);                              // 1 fine
    assertParse(_T("BRIAN=foo\r\nBILL=bar"), true);                            // 2 fine
    assertParse(_T("BRIAN=foo\n\n\rBILL=bar\n"), true);                        // 3 fine
    assertParse(_T("    WS=foo    \n\n\r     BILL=bar\n"), true);              // 4 fine
    assertParse(_T("    WS   =   foo    \n\n\r     BILL  =  bar   \n"), true); // 5 fine
    assertParse(_T("FOO=foo\nBAR=$(FOO)"), true);                              // 6 fine

    // should fail
    assertParse(_T("FOO=foo\nBAR=$(FO)"), false);    // 7 undefined var
    assertParse(_T("FOO=foo\nBAR=$(FOO"), false);    // 8 no r paren
    assertParse(_T("FOO=foo\nBAR=$(FOO   "), false); // 9 no r paren
    assertParse(_T("BAR=$(FOO\n"), false);           // 10 no r paren

    // should succeed
    assertParse(_T(" VAR    =foo    \nWS     = $(VAR)\n"), true); // 11 fine
    assertParse(_T(""), true);                                    // 12 fine
    assertParse(_T("\n"), true);                                  // 13 fine
    assertParse(_T("\r"), true);                                  // 14 fine
    assertParse(_T("\r\n"), true);                                // 15 fine

    // should fail
    assertParse(_T("B=POO\nA"), false);           // 16 no equals
    assertParse(_T(" B=POO \n   A   \r"), false); // 17 no equals

    /*  This next test asserts that you can't change a variable once you've defined it.
    However there's no actual code in cConfigFile to check for this, and 
    OST appears to work fine if you redefine a config variable, so I'm not going
    to change the current behavior.  Leaving this test in w/ a note for reference.

    assertParse( _T("B=POO\nB=CRAP"), false );   // 18 redefined var
*/
    assertParse(_T("DATE=CRAP"), false);        // 19 redefine predefine var
    assertParse(_T("B=POO\nDATE=CRAP"), false); // 20 redefine predefine var
    // should succeed
    assertParse(_T("A=1\nB=$(A)\nC=$(B)"), true);    // 21 fine -- checking var sub
    assertParse(_T("A=$(DATE)"), true);              // 22 fine -- checking predef var sub
    assertParse(_T("A=1\nB=$(A)\nC=$(DATE)"), true); // 23 fine -- checking predef var sub

    // should fail
    assertParse(_T("A=1\n=$(A)\nC=$(DATE)"), false); // 24 no key

    // should succeed
    assertParse(_T("A=$(DATE)-B"), true);       // 25 fine -- check that the '-' shows up
    assertParse(_T("A=$(DATE)-$(DATE)"), true); // 26 fine -- check that the '-' shows up
}

void TestConfigFile2(void)
{
    cDebug d("Testconfigfile");
    d.TraceDetail("Entering...\n");
    //iFSServices* pFSServices = iFSServices::GetInstance();

    //Define some test values for <name, value> pairs to be
    //stored in a test config. module. I'm going to use the
    //values specified in the install doc. -DA

    //TODO maybe also test read failure when mandatory config values aren't set


    //Begin tests of config. module parser:
    cConfigFile write_cfgmod;
    //Add all the mandatory config options.
    write_cfgmod.Insert(_T("POLFILE"), "test.pol");
    write_cfgmod.Insert(_T("DBFILE"), "test.twd");
    write_cfgmod.Insert(_T("REPORTFILE"), "test.twr");
    write_cfgmod.Insert(_T("SITEKEYFILE"), "site.key");
    write_cfgmod.Insert(_T("LOCALKEYFILE"), "local.key");

    //Filename for writing/reading some value pairs:
    const TSTRING testfile = TwTestPath("tripwire.cfg");

    //Store these values on disk.
    TSTRING configText;
    write_cfgmod.WriteString(configText);
    cTWUtil::WriteConfigText(testfile.c_str(), configText, false, NULL);

    //Instantiate another cConfigFile object for reading back
    //values:
    cConfigFile read_cfgmod;

    try
    {
        cTWUtil::ReadConfigText(testfile.c_str(), configText);
        read_cfgmod.ReadString(configText);
    }
    catch (eError& error)
    {
        TCERR << (int)error.GetID() << std::endl << error.GetMsg() << std::endl;
        TEST(false);
    }

    //These TSTRINGS will hold info. from .Lookup:
    TSTRING lookup1, lookup2;

    read_cfgmod.Lookup(_T("POLFILE"), lookup1);
    read_cfgmod.Lookup(_T("DBFILE"), lookup2);
    d.TraceDetail("First lookup's value: %s \n", lookup1.c_str());
    d.TraceDetail("Second lookup's value: %s \n", lookup2.c_str());
    TEST(lookup1 == "test.pol");
    TEST(lookup2 == "test.twd");

    d.TraceDetail("Tests Passed!\n");
}

void RegisterSuite_ConfigFile()
{
    RegisterTest("ConfigFile", "Basic1", TestConfigFile);
    RegisterTest("ConfigFile", "Basic2", TestConfigFile2);
}
