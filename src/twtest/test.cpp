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
// test.cpp
//
// main() for all the component test driver functions

#include "stdtest.h"

#include "core/core.h"
#include "db/db.h"
#include "twcrypto/twcrypto.h"
#include "twparser/twparser.h"
#include "tw/tw.h"
#include "fco/fco.h"


#include "fs/fs.h"
#include "util/util.h"

#include <stdlib.h>
#include "core/debug.h"
#include "core/error.h"
#include "core/twlocale.h"
#include "test.h"
#include "core/errorbucketimpl.h"
#include "tw/twinit.h"
#include "tw/twutil.h"
#include "fco/fcogenre.h"
#include "fco/genreswitcher.h"
#include "core/growheap.h"
#include "db/blockfile.h"
#include "db/blockrecordarray.h"
#include "db/hierdatabase.h"


// the test routines
void TestFCOName();
void TestFCOTest();
void TestFCOSetImpl();
void TestFCOSpec();
void TestFCOPropVector();
void TestFileHeader();
void TestFSPropSet();
void TestFCOSpecImpl();
void TestFSPropCalc();
void TestFCOPropImpl();
void TestFCOCompare();
//void TestTripwire();
void TestWin32FSServices();
void TestFCOSpecList();
void TestFCOReport();
void TestArchive();
void TestSerializer();
void TestSerializerImpl();
void TestRefCountObj();
void TestSignature();
void TestSerRefCountObj();
void TestUnixFSServices();
void TestError();
void TestDebug();
void TestFcoSpecUtil();
void TestTypes();
void TestTCHAR();
void TestErrorBucketImpl();
void TestHashTable();
void TestTextReportViewer();
void TestFCONameTbl();
void TestConfigFile();

void TestPolicyParser();

void TestFCOSpecHelper();
void TestCrypto();
void TestCryptoArchive();
void TestFCOSpecAttr();
void TestCmdLineParser();
void TestTaskTimer();
void TestKeyFile();
void TestTWUtil();
void TestFSPropDisplayer();
void TestFSPropDisplayer();
void TestGenre();
void TestFSDataSourceIter();
void TestGenerateDb();
void TestHierDatabaseInteractive();
void TestGenreSwitcher();
void TestDbDataSource();
void TestGenreSpecList();
void TestIntegrityCheck();
void TestFCODatabaseFile();
void TestWchar16();
void TestStringEncoder();

void TestGrowHeap();
void TestPlatform();
void TestBlockFile();
void TestBlockRecordArray();
void TestTWLocale();
void TestFileUtil();
void TestFCONameTranslator();
void TestCodeConverter();

void TestCharToHex();
void TestHexToChar();
void TestStringToHex();
void TestHexToString();
//void TestUnconvertable();
//void TestUnprintable();
void TestQuoteAndBackSlash();
void TestDisplayEncoderBasic();
void TestCharUtilBasic();
void TestConfigFile2();

/// This is easier than all the (cpp) files and declarations
#include "stringutil_t.h"

void Usage()
{
    TCERR << _T("Usage: test {all | testid [testid ...]}\n")
             _T("\n")
             _T("Ex: test 1 2 3 12\n")
             _T("(runs test id's 1, 2, 3, and 12)\n\n");
}

const int MAX_TEST_ID = 88;

static void Test(int testID)
{
    TCERR << std::endl << "=== Running test ID #" << testID << " ===" << std::endl;
    
    switch (testID)
    {
    case 1: TestArchive(); break;
    case 2: TestCmdLineParser(); break;
    case 3: TestCrypto(); break;
    case 4: TestCryptoArchive(); break;
    case 5: TestDebug(); break;
    case 6: TestError(); break;
    case 7: TestErrorBucketImpl(); break;
    case 8: TestFCOCompare(); break;
            
    case 12: TestFCOName(); break;
    case 13: TestFCONameTbl(); break;
    case 14: TestFCOPropVector(); break;
    case 15: TestFCOPropImpl(); break;
    case 16: TestFCOReport(); break;
            
    case 18: TestFCOSetImpl(); break;

    case 20: TestFCOSpecAttr(); break;
    case 21: TestFCOSpecHelper(); break;
    case 22: TestFCOSpecList(); break;
    case 23: TestFcoSpecUtil(); break;
    case 24: TestFileHeader(); break;

    case 26: TestFSPropSet(); break;
    case 27: TestFSPropCalc(); break;
    case 28: TestFCOSpecImpl(); break;
    case 29: TestHashTable(); break;

    case 31: TestRefCountObj(); break;
    case 32: TestSerializerImpl(); break;
    case 33: TestSerRefCountObj(); break;
    case 34: TestSignature(); break;
    case 35: TestTaskTimer(); break;
    //case 36: TestTripwire(); break;
    case 37: TestTextReportViewer(); break;
    case 39: TestSerRefCountObj(); break;
    case 40: TestError(); break;
    case 41: TestFCODatabaseFile(); break;
    case 42: TestHashTable(); break;
    case 43: TestTCHAR(); break;
    case 44: TestUnixFSServices(); break;
    case 46: TestConfigFile(); break;
    case 47: TestPolicyParser(); break;
    case 48: TestKeyFile(); break;
    case 49: TestTWUtil(); break;
    case 50: TestFSPropDisplayer(); break;
    case 52: TestGenre(); break;        
    case 53: TestFSDataSourceIter(); break;
    //case 54: TestGenerateDb(); break;
    case 55: TestHierDatabaseInteractive(); break;
    case 56: TestGenreSwitcher(); break;
    case 57: TestDbDataSource(); break;
    case 58: TestGenreSpecList(); break;
    //case 59: TestIntegrityCheck(); break;
            
    case 65: TestWchar16(); break;        
    case 66: TestStringEncoder(); break;

    case 69: TestGrowHeap(); break;
    case 70: TestPlatform(); break;
    case 71: TestBlockFile(); break;
    case 72: TestBlockRecordArray(); break;
    case 74: TestFileUtil(); break;
    case 75: TestTWLocale(); break; 
    case 76: TestFCONameTranslator(); break; 
    case 77: TestStringUtil(); break; 
    case 78: TestCodeConverter(); break;
            
    case 79: TestCharToHex(); break;
    case 80: TestHexToChar(); break;
    case 81: TestStringToHex(); break;
    case 82: TestHexToString(); break;
        //    case 83: TestUnconvertable(); break;
        //    case 84: TestUnprintable(); break;
    case 85: TestQuoteAndBackSlash(); break;
    case 86: TestDisplayEncoderBasic(); break;
    case 87: TestCharUtilBasic(); break;
    case 88: TestConfigFile2(); break;
    }
    
    TCERR << std::endl << "=== test ID #" << testID << " completed ===" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// cTest
///////////////////////////////////////////////////////////////////////////////
TSS_ImplementPackage( cTest )

cTest::cTest()
{
    TSS_Dependency( cCore );
    TSS_Dependency( cDb );
    TSS_Dependency( cTWCrypto );
    TSS_Dependency( cTWParser );
    TSS_Dependency( cTW );
    TSS_Dependency( cFCO );
    TSS_Dependency( cFS );
    TSS_Dependency( cUtil );

// no erros excluivly for test package
//  TSS_REGISTER_PKG_ERRORS( test )
}

///////////////////////////////////////////////////////////////////////////////
// terminate and unexpected handlers
// TODO: move these to a common library
///////////////////////////////////////////////////////////////////////////////
void tw_terminate_handler()
{
    fputs("### Internal Error.\n### Terminate Handler called.\n### Exiting...\n", stderr);
    exit(1);
}

void tw_unexpected_handler()
{
    fputs("### Internal Error.\n### Unexpected Exception Handler called.\n### Exiting...\n", stderr);
    exit(1);
}

int _tmain(int argc, TCHAR** argv)
{
    std::cout << "Test: Init" << std::endl;

    try 
    {
        std::cout << "Test: Setup" << std::endl;
        std::cout << "Test: argc  - " << argc << std::endl;
        std::cout << "Test: *argv - " << argv[0] << std::endl;

        EXCEPTION_NAMESPACE set_terminate(tw_terminate_handler);
        EXCEPTION_NAMESPACE set_unexpected(tw_unexpected_handler);

        cTWInit twInit;
        twInit.Init( argv[0] );

        // set up the debug output
        cDebug::AddOutTarget(cDebug::OUT_STDOUT);
        //cDebug::SetDebugLevel(cDebug::D_NEVER);
        cDebug::SetDebugLevel(cDebug::D_DETAIL);
        //cDebug::SetDebugLevel(cDebug::D_DEBUG);
        
        int i;

        if (argc < 2)
            Usage();
        else if (_tcsicmp(argv[1], _T("all")) == 0) 
            // run all the tests
            for (i = 1; i <= MAX_TEST_ID; ++i)
                Test(i);
        else
            for (i = 1; i < argc; ++i)
                Test(_ttoi(argv[i]));    // Note: if atoi returns 0, Test() will handle it fine.
            
    } 
    catch (eError& error)
    {        
        cTWUtil::PrintErrorMsg(error);
        ASSERT(false);
        return 1;
    }
    catch (...)
    {        
        TCERR << _T("Unhandled exception caught!");
        ASSERT(false);
        return 1;
    }

    // make sure all the refrence counted objects have been destroyed
    // this test always fails because of the static cFCONameTbl
    //TEST(cRefCountObj::AllRefCountObjDestoryed() == true);

    // force user to hit <CR>
    
    std::cout << std::endl << "Tests completed" << std::endl;
    
    return 0;
}



