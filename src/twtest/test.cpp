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
#include "core/fsservices.h"
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
#include "config.h"

#include <unistd.h>
#include <sys/stat.h>

// Known test suites
void RegisterSuite_Archive();
void RegisterSuite_BlockFile();
void RegisterSuite_BlockRecordArray();
void RegisterSuite_CharUtil();
void RegisterSuite_CmdLineParser();
void RegisterSuite_CodeConvert();
void RegisterSuite_ConfigFile();
void RegisterSuite_CryptoArchive();
void RegisterSuite_Crypto();
void RegisterSuite_DbDataSource();
void RegisterSuite_Debug();
void RegisterSuite_DisplayEncoder();
void RegisterSuite_Error();
void RegisterSuite_ErrorBucketImpl();
void RegisterSuite_FCOCompare();
void RegisterSuite_FCODatabaseFile();
void RegisterSuite_FCOName();
void RegisterSuite_FCONameTbl();
void RegisterSuite_FCONameTranslator();
void RegisterSuite_FCOPropImpl();
void RegisterSuite_FCOPropVector();
void RegisterSuite_FCOReport();
void RegisterSuite_FCOSetImpl();
void RegisterSuite_FCOSpec();
void RegisterSuite_FCOSpecAttr();
void RegisterSuite_FCOSpecHelper();
void RegisterSuite_FCOSpecList();
void RegisterSuite_FcoSpecUtil();
void RegisterSuite_File();
void RegisterSuite_FileHeader();
void RegisterSuite_FileUtil();
void RegisterSuite_FSDataSourceIter();
void RegisterSuite_FSObject();
void RegisterSuite_FSPropCalc();
void RegisterSuite_FSPropDisplayer();
void RegisterSuite_FSPropSet();
void RegisterSuite_FCOSpecImpl();
void RegisterSuite_GenreSwitcher();
void RegisterSuite_GenreSpecList();
void RegisterSuite_Error();
void RegisterSuite_GrowHeap();
void RegisterSuite_HashTable();
void RegisterSuite_HierDatabase();
void RegisterSuite_KeyFile();
void RegisterSuite_Platform();
void RegisterSuite_PolicyParser();
void RegisterSuite_RefCountObj();
void RegisterSuite_Resources();
void RegisterSuite_Serializer();
void RegisterSuite_SerializerImpl();
void RegisterSuite_Signature();
void RegisterSuite_SerRefCountObj();
void RegisterSuite_StringEncoder();
void RegisterSuite_StringUtil();
void RegisterSuite_TaskTimer();
void RegisterSuite_TCHAR();
void RegisterSuite_TextReportViewer();
void RegisterSuite_TWLocale();
void RegisterSuite_TWUtil();
void RegisterSuite_Types();
void RegisterSuite_UnixFSServices();
void RegisterSuite_UserNotifyStdout();
void RegisterSuite_Wchar16();

/// This is easier than all the (cpp) files and declarations
#include "stringutil_t.h"

#if IS_AROS
#    define VERSION_PREFIX "$VER: "
#else
#    define VERSION_PREFIX "@(#)"
#endif

const char* STR_EMBEDDED_VERSION = _T(VERSION_PREFIX "twtest " PACKAGE_VERSION);


void Usage()
{
    TCERR << _T("Usage: twtest {all | list | help | version | testid [testid ...]}\n")
             _T("\n")
             _T("Ex: twtest foo bar/baz\n")
             _T("(runs suite foo and test bar/baz)\n\n");
}

void Version()
{
    TCOUT << "twtest " << PACKAGE_VERSION << std::endl;
}

static int ran_count     = 0;
static int failed_count  = 0;
static int skipped_count = 0;
static int macro_count   = 0;

static std::vector<std::string> error_strings;
static std::vector<std::string> skipped_strings;

class skip_exception : public std::runtime_error
{
public:
    skip_exception(const std::string& reason) : std::runtime_error(reason)
    {
    }
};

void skip(const std::string& reason)
{
    throw skip_exception(reason);
}

void fail(const std::string& reason)
{
    throw std::runtime_error(reason);
}

void CountMacro()
{
    macro_count++;
}

/////////////////////////

static TestMap tests;

void RegisterTest(const std::string& suite, const std::string testName, TestPtr testPtr)
{
    tests[suite][testName] = testPtr;
}


static void RunTest(const std::string& suiteName, const std::string& testName, TestPtr testPtr)
{
    try
    {
        if (testPtr)
        {
            ran_count++;
            int pre_count = macro_count;
            testPtr();
            if (macro_count > pre_count)
                TCERR << "PASSED" << std::endl;
            else
                skip("Test did not make any TEST assertions");
        }
        return;
    }
    catch (skip_exception& e)
    {
        TCERR << "SKIPPED: " << e.what() << std::endl;

        TSTRINGSTREAM sstr;
        sstr << "Test " << suiteName << "/" << testName << ": " << e.what();
        skipped_strings.push_back(sstr.str());

        skipped_count++;
    }
    catch (eError& error)
    {
        TCERR << "FAILED: ";
        cTWUtil::PrintErrorMsg(error);

        TSTRINGSTREAM sstr;
        sstr << "Test " << suiteName << "/" << testName << ": " << error.GetMsg();
        error_strings.push_back(sstr.str());

        failed_count++;
    }
    catch (std::exception& e)
    {
        TCERR << "FAILED: " << e.what() << std::endl;

        TSTRINGSTREAM sstr;
        sstr << "Test " << suiteName << "/" << testName << ": " << e.what();
        error_strings.push_back(sstr.str());

        failed_count++;
    }
    catch (...)
    {
        TCERR << "FAILED: <unknown>" << std::endl;

        TSTRINGSTREAM sstr;
        sstr << "Test " << suiteName << "/" << testName << ": <unknown>";
        error_strings.push_back(sstr.str());

        failed_count++;	
    }
}


static void RunTestSuite(const std::string& suiteName, SuiteMap suite)
{
    SuiteMap::const_iterator itr;
    for (itr = suite.begin(); itr != suite.end(); ++itr)
    {
        TCERR << "----- Running test: " << suiteName << "/" << itr->first << " -----" << std::endl << std::endl;
        RunTest(suiteName, itr->first, itr->second);
        TCERR << std::endl << "----- Finished test: " << suiteName << "/" << itr->first << " -----" << std::endl;
    }
}

static void RunAllTests()
{
    TestMap::const_iterator itr;
    for (itr = tests.begin(); itr != tests.end(); ++itr)
    {
        TCERR << std::endl << "===== Starting test suite: " << itr->first << " =====" << std::endl;
        RunTestSuite(itr->first, itr->second);
        TCERR << "===== Finished test suite: " << itr->first << " =====" << std::endl;
    }
}

static void ListTests()
{
    TestMap::const_iterator itr;
    for (itr = tests.begin(); itr != tests.end(); ++itr)
    {
        std::string suiteName = itr->first;
        SuiteMap    suite     = itr->second;

        TCERR << suiteName << std::endl;
        SuiteMap::const_iterator itr;
        for (itr = suite.begin(); itr != suite.end(); ++itr)
        {
            TCERR << "  " << suiteName << "/" << itr->first << std::endl;
        }
    }
}

static void RunTest(const std::string& to_run)
{
    std::string::size_type pos = to_run.find_first_of("/");
    if (pos == std::string::npos)
    {
        RunTestSuite(to_run, tests[to_run]);
    }
    else
    {
        std::string suite    = to_run.substr(0, pos);
        std::string testName = to_run.substr(pos + 1);
        RunTest(suite, testName, tests[suite][testName]);
    }
}

static void RegisterSuites()
{
    RegisterSuite_Archive();
    RegisterSuite_BlockFile();
    RegisterSuite_BlockRecordArray();
    RegisterSuite_CharUtil();
    RegisterSuite_CmdLineParser();
    RegisterSuite_CodeConvert();
    RegisterSuite_ConfigFile();
    RegisterSuite_CryptoArchive();
    RegisterSuite_Crypto();
    RegisterSuite_DbDataSource();
    RegisterSuite_Debug();
    RegisterSuite_DisplayEncoder();
    RegisterSuite_Error();
    RegisterSuite_ErrorBucketImpl();
    RegisterSuite_FCOCompare();
    RegisterSuite_FCODatabaseFile();
    RegisterSuite_FCOName();
    RegisterSuite_FCONameTbl();
    RegisterSuite_FCONameTranslator();
    RegisterSuite_FCOPropImpl();
    RegisterSuite_FCOPropVector();
    RegisterSuite_FCOReport();
    RegisterSuite_FCOSetImpl();
    RegisterSuite_FCOSpec();
    RegisterSuite_FCOSpecAttr();
    RegisterSuite_FCOSpecHelper();
    RegisterSuite_FCOSpecList();
    RegisterSuite_FcoSpecUtil();
    RegisterSuite_File();
    RegisterSuite_FileHeader();
    RegisterSuite_FileUtil();
    RegisterSuite_FSDataSourceIter();
    RegisterSuite_FSObject();
    RegisterSuite_FSPropCalc();
    RegisterSuite_FSPropDisplayer();
    RegisterSuite_FSPropSet();
    RegisterSuite_FCOSpecImpl();
    RegisterSuite_GenreSwitcher();
    RegisterSuite_GenreSpecList();
    RegisterSuite_Error();
    RegisterSuite_GrowHeap();
    RegisterSuite_HashTable();
    RegisterSuite_HierDatabase();
    RegisterSuite_KeyFile();
    RegisterSuite_Platform();
    RegisterSuite_PolicyParser();
    RegisterSuite_RefCountObj();
    RegisterSuite_Resources();
    RegisterSuite_Serializer();
    RegisterSuite_SerializerImpl();
    RegisterSuite_Signature();
    RegisterSuite_SerRefCountObj();
    RegisterSuite_StringEncoder();
    RegisterSuite_StringUtil();
    RegisterSuite_TaskTimer();
    RegisterSuite_TCHAR();
    RegisterSuite_TextReportViewer();
    RegisterSuite_TWLocale();
    RegisterSuite_TWUtil();
    RegisterSuite_Types();
    RegisterSuite_UnixFSServices();
    RegisterSuite_UserNotifyStdout();
    RegisterSuite_Wchar16();
}


std::string TwTestDir()
{
    static std::string dir;

    if (dir.empty())
    {
        iFSServices::GetInstance()->GetCurrentDir(dir);
        dir.append("/TWTestData");
        TCERR << "Using test directory: " << dir << std::endl;
        tw_mkdir(dir.c_str(), 0777);
    }

    return dir;
}

std::string TwTestPath(const std::string& child)
{
    TOSTRINGSTREAM sstr;
    sstr << TwTestDir();
    if (child[0] != '/')
        sstr << '/';
    sstr << child;

    tss_return_stream(sstr, out);
}

///////////////////////////////////////////////////////////////////////////////
// cTest
///////////////////////////////////////////////////////////////////////////////
TSS_ImplementPackage(cTest)

    cTest::cTest()
{
    TSS_Dependency(cCore);
    TSS_Dependency(cDb);
    TSS_Dependency(cTWCrypto);
    TSS_Dependency(cTWParser);
    TSS_Dependency(cTW);
    TSS_Dependency(cFCO);
    TSS_Dependency(cFS);
    TSS_Dependency(cUtil);

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

#if USE_UNEXPECTED
void tw_unexpected_handler()
{
    fputs("### Internal Error.\n### Unexpected Exception Handler called.\n### Exiting...\n", stderr);
    exit(1);
}
#endif

int _tmain(int argc, TCHAR** argv)
{
#ifdef _DEBUG
    std::cout << "Test: Init" << std::endl;
    std::cout << "Test: Setup" << std::endl;
    std::cout << "Test: argc  - " << argc << std::endl;
    std::cout << "Test: *argv - " << argv[0] << std::endl;
#endif

    try
    {
        EXCEPTION_NAMESPACE set_terminate(tw_terminate_handler);
#if USE_UNEXPECTED
        EXCEPTION_NAMESPACE set_unexpected(tw_unexpected_handler);
#endif
        if (argc < 2)
        {
            Usage();
            return 0;
        }

        cTWInit twInit;
        twInit.Init(argv[0]);

        // set up the debug output
        cDebug::AddOutTarget(cDebug::OUT_STDOUT);
        //cDebug::SetDebugLevel(cDebug::D_NEVER);
        cDebug::SetDebugLevel(cDebug::D_DETAIL);
        //cDebug::SetDebugLevel(cDebug::D_DEBUG);

        RegisterSuites();
        std::string arg1 = argv[1];

        if (arg1 == "all" || arg1 == "--all")
        {
            RunAllTests();
        }
        else if (arg1 == "list" || arg1 == "--list")
        {
            ListTests();
            return 0;
        }
        else if (arg1 == "help" || arg1 == "--help" || arg1 == "-?")
        {
            Usage();
            return 0;
        }
        else if (arg1 == "version" || arg1 == "--version")
        {
            Version();
            return 0;
        }
        else
        {
            for (int i = 1; i < argc; ++i)
                RunTest(argv[i]);
        }
    }
    catch (eError& error)
    {
        cTWUtil::PrintErrorMsg(error);
        failed_count++;
    }
    catch (std::exception& error)
    {
        TCERR << "Caught std::exception: " << error.what() << std::endl;
        failed_count++;	
    }
    catch (...)
    {
        TCERR << _T("Unhandled exception caught!");
        failed_count++;	
    }
    
    // make sure all the reference counted objects have been destroyed
    // this test always fails because of the static cFCONameTbl
    //TEST(cRefCountObj::AllRefCountObjDestoryed() == true);

    if (!ran_count)
    {
        std::cout << "Ran 0 unit tests. The specified group/test names may be incorrect (names are case sensitive).\n";
        return 1;
    }

    std::cout << std::endl
              << "Ran " << ran_count << " unit tests with " << failed_count << " failures, " << skipped_count
              << " skipped." << std::endl;
    std::cout << "(total test assertions: " << macro_count << ")" << std::endl;

    if (failed_count)
    {
        std::cout << std::endl << "Failures: " << std::endl;
        std::vector<std::string>::iterator itr;
        for (itr = error_strings.begin(); itr != error_strings.end(); ++itr)
        {
            std::cout << "\t" << *itr << std::endl;
        }
    }

    if (skipped_count)
    {
        std::cout << std::endl << "Skipped: " << std::endl;
        std::vector<std::string>::iterator itr;
        for (itr = skipped_strings.begin(); itr != skipped_strings.end(); ++itr)
        {
            std::cout << "\t" << *itr << std::endl;
        }
    }

    std::cout << std::endl;


    return failed_count ? -1 : 0;
}
