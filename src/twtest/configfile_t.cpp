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

void TestConfigFile(void)
{
    TSTRING asConfigFileText[] = 
    { 
        _T("BRIAN=foo"),                // 0 fine
        _T("BRIAN=foo\nBILL=bar"),      // 1 fine
        _T("BRIAN=foo\r\nBILL=bar"),    // 2 fine
        _T("BRIAN=foo\n\n\rBILL=bar\n"),// 3 fine
        _T("    WS=foo    \n\n\r     BILL=bar\n"), // 4 fine
        _T("    WS   =   foo    \n\n\r     BILL  =  bar   \n"), // 5 fine
        _T("FOO=foo\nBAR=$(FOO)"),      // 6 fine
        _T("FOO=foo\nBAR=$(FO)"),       // 7 undefined var
        _T("FOO=foo\nBAR=$(FOO"),       // 8 no r paren
        _T("FOO=foo\nBAR=$(FOO   "),    // 9 no r paren
        _T("BAR=$(FOO\n"),              // 10 no r paren
        _T(" VAR    =foo    \nWS     = $(VAR)\n"),  // 11 fine
        _T(""),  // 12 fine
        _T("\n"),  // 13 fine
        _T("\r"),  // 14 fine
        _T("\r\n"),  // 15 fine
        _T("B=POO\nA"),  // 16 no equals
        _T(" B=POO \n   A   \r"),  // 17 no equals
        _T("B=POO\nB=CRAP"),  // 18 redefined var
        _T("DATE=CRAP"),  // 19 redefine predefine var
        _T("B=POO\nDATE=CRAP"),  // 20 redefine predefine var
        _T("A=1\nB=$(A)\nC=$(B)"),  // 21 fine -- checking var sub
        _T("A=$(DATE)"),  // 22 fine -- checking predef var sub
        _T("A=1\nB=$(A)\nC=$(DATE)"),  // 23 fine -- checking predef var sub
        _T("A=1\n=$(A)\nC=$(DATE)"),  // 24 no key
        _T("A=$(DATE)-B"),  // 25 fine -- check that the '-' shows up
        _T("A=$(DATE)-$(DATE)"),  // 26 fine -- check that the '-' shows up
    };

    /*
    TSTRING sMandatory = \
        _T("\nPOLFILE=foo") \
        _T("\nDBFILE=foo") \
        _T("\nREPORTFILE=foo") \
        _T("\nSITEKEYFILE=foo") \
        _T("\nLOCALKEYFILE=foo");
    */


    for( TSTRING* at = &asConfigFileText[0];
         at != &asConfigFileText[countof(asConfigFileText)];
         at++ )
    {
        cConfigFile cfg;
        //*at += sMandatory;

        TCERR << _T("*** line:") << std::endl;
        TCERR << *at << std::endl;
        TCERR << _T("*** eol:") << std::endl;
        try
        {
            cfg.ReadString( *at );
        }
        catch( eConfigFileMissReqKey& )
        {
            // ignore....
        }
        catch( eConfigFile& e )
        {
            int offset = ( at - asConfigFileText );
            int itemSize = sizeof( asConfigFileText[0] );
            int num = offset / itemSize;
            TCERR << num << std::endl;
            cTWUtil::PrintErrorMsg( e );
        }
    }
}

void TestConfigFile2(void)
{
    cDebug d("Testconfigfile");
    d.TraceDetail("Entering...\n");
    iFSServices* pFSServices = iFSServices::GetInstance();

    //Define some test values for <name, value> pairs to be
    //stored in a test config. module. I'm going to use the
    //values specified in the install doc. -DA
    TSTRING currpath;
    pFSServices->GetCurrentDir(currpath);
    const TSTRING testTWROOT = currpath;

    //TODO maybe also test read failure when mandatory config values aren't set
    
    
    //Begin tests of config. module parser:
    cConfigFile write_cfgmod;
    //Add all the mandatory config options.
    write_cfgmod.Insert( _T("POLFILE"), "test.pol");
    write_cfgmod.Insert( _T("DBFILE"),  "test.twd");
    write_cfgmod.Insert( _T("REPORTFILE"), "test.twr");
    write_cfgmod.Insert( _T("SITEKEYFILE"), "site.key");
    write_cfgmod.Insert( _T("LOCALKEYFILE"), "local.key");

    //Filename for writing/reading some value pairs:
    const TSTRING testfile = testTWROOT + _T("/tripwire.cfg");

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

    read_cfgmod.Lookup( _T("POLFILE"), lookup1);
    read_cfgmod.Lookup( _T("DBFILE"), lookup2);
    d.TraceDetail("First lookup's value: %s \n", lookup1.c_str());
    d.TraceDetail("Second lookup's value: %s \n", lookup2.c_str());
    TEST( lookup1 == "test.pol" );
    TEST( lookup2 == "test.twd" );

    d.TraceDetail("Tests Passed!\n");
//#endif // NOT_BRIANS_TEST
}
