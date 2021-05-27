//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
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
// twprintcmdline.cpp
//
#include "stdtwprint.h"
#include "twprintcmdline.h"
#include "twprintstrings.h" // strings specific to twprint
#include "twprintcmdline.h" // functions for the cmd line
#include "twprinterrors.h"

#include "core/cmdlineparser.h"
#include "core/errorgeneral.h"
#include "core/errorbucketimpl.h" // for the error table
#include "core/usernotify.h"      // for notifying the user of even
#include "core/serializerimpl.h"  // cSerializerImpl
#include "core/archive.h"         // cArchive and friends

#include "tw/configfile.h"
#include "tw/fcodatabasefile.h" // cFCODatabaseFile
#include "tw/fcoreport.h"       // for reports
#include "tw/textdbviewer.h"
#include "tw/textreportviewer.h" // text report viewer
#include "tw/twutil.h"
#include "tw/headerinfo.h"
#include "tw/systeminfo.h"
#include "tw/twerrors.h"
#include "tw/dbdatasource.h"
#include "tw/twstrings.h"

#include "twcrypto/keyfile.h" // cKeyFile -- used for encryption

#include "util/fileutil.h"

#include "fco/fcogenre.h"
#include "fco/genreswitcher.h"
#include "fco/twfactory.h"
#include "fco/fcospeclist.h" // cFCOSpecList
#include "fco/fcopropdisplayer.h"
#include "fco/signature.h"

#include <set>

///////////////////////////////////////////////////////////////////////////////
// Static functions --
///////////////////////////////////////////////////////////////////////////////
static void InitCmdLineCommon(cCmdLineParser& parser);
// A method for initializing the command line parser with arguments that
// are common to all modes of twprint.

//#############################################################################
// cTWPrintCmdLine
//#############################################################################


// this is used to make required condition checking in the Init() functions a little more compact
#define TEST_INIT_REQUIREMENT(t, n)                  \
    if (!t)                                          \
    {                                                \
        TCERR << TSS_GetString(cTW, n) << std::endl; \
        return false;                                \
    }

// TODO: get Matt to update this with changes he made to tripwire

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineCommon --
//          Initialize the cmdlineparser with the arguments that are common
//          to all modes.  Other parameters must be added in the
//          InitCmdLineParser method of the derived mode.
///////////////////////////////////////////////////////////////////////////////
static void InitCmdLineCommon(cCmdLineParser& parser)
{
    // help
    parser.AddArg(cTWPrintCmdLine::HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_NONE);

    // mode
    parser.AddArg(cTWPrintCmdLine::MODE, TSTRING(_T("m")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWPrintCmdLine::MODE_DBPRINT, TSTRING(_T("")), TSTRING(_T("print-dbfile")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWPrintCmdLine::MODE_REPORTPRINT, TSTRING(_T("")), TSTRING(_T("print-report")), cCmdLineParser::PARAM_NONE);

    // reporting
    parser.AddArg(cTWPrintCmdLine::VERBOSE, TSTRING(_T("v")), TSTRING(_T("verbose")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWPrintCmdLine::SILENT, TSTRING(_T("s")), TSTRING(_T("silent")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWPrintCmdLine::SILENT, TSTRING(_T("")), TSTRING(_T("quiet")), cCmdLineParser::PARAM_NONE);

    // config file overrides
    parser.AddArg(cTWPrintCmdLine::CFG_FILE, TSTRING(_T("c")), TSTRING(_T("cfgfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWPrintCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWPrintCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);

    // unattended operation
    parser.AddArg(cTWPrintCmdLine::PASSPHRASE, TSTRING(_T("P")), TSTRING(_T("passphrase")), cCmdLineParser::PARAM_ONE);


    parser.AddArg(
        cTWPrintCmdLine::HEXADECIMAL, TSTRING(_T("h")), TSTRING(_T("hexadecimal")), cCmdLineParser::PARAM_NONE);

    // the paramters to the command line ... for now, this will take "many", even though in some
    // modes, this is not valid to do...

    // NOTE -- print report doesn't take any naked parameters, and neither does print database (we took that
    //      functionality out) so I changed this to PARAM_NONE.

    // NOTE -- We __DO__ take "naked" parameters in print-database mode, contrary to the above comment!!! It is an error
    // to add this argument here, in the "common" code between the two modes of twprint.  I'm moving the PARAMS arg to
    // twprint --print-database's "personal" initialization (below). -DRA 8/9/99

    //parser.AddArg(cTWPrintCmdLine::PARAMS,            TSTRING(_T("")),    TSTRING(_T("")),                cCmdLineParser::/*PARAM_NONE*/PARAM_MANY);
    // Error for print-report!!!!

    parser.AddMutEx(cTWPrintCmdLine::VERBOSE, cTWPrintCmdLine::SILENT);
}

///////////////////////////////////////////////////////////////////////////////
// GetMode -- processes the command line arguments and creates an appropriate
//      structure for the selected mode, or NULL if an error occurs
///////////////////////////////////////////////////////////////////////////////
iTWMode* cTWPrintCmdLine::GetMode(int argc, const TCHAR* const* argv)
{
    // note -- it is assumed the executable name is the first parameter
    if (argc < 2)
    {
        TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_VERSION) << std::endl;
        TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
        TCOUT << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
        return NULL;
    }

    int          mode = MODE;
    const TCHAR* pcMode;
    if (_tcscmp(argv[1], _T("-m")) == 0)
    {
        if (argc < 3)
        {
            TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_ERR_NO_MODE) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
            return NULL;
        }
        pcMode = argv[2];
        if (_tcscmp(argv[2], _T("d")) == 0)
            mode = MODE_DBPRINT;
        else if (_tcscmp(argv[2], _T("r")) == 0)
            mode = MODE_REPORTPRINT;
    }
    else
    {
        pcMode = argv[1];
        if (_tcscmp(argv[1], _T("--print-dbfile")) == 0)
            mode = MODE_DBPRINT;
        else if (_tcscmp(argv[1], _T("--print-report")) == 0)
            mode = MODE_REPORTPRINT;
        else if (_tcscmp(argv[1], _T("--version")) == 0)
        {
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            return NULL;
        }
        else if (_tcscmp(argv[1], _T("--help")) == 0 || _tcscmp(argv[1], _T("-?")) == 0)
            mode = MODE_HELP;
    }

    if (mode == MODE)
    {
        // unknown mode switch
        cDebug d("cTWPrintCmdLine::GetMode");
        d.TraceError(_T("Error: Bad mode switch: %s\n"), pcMode);
        TCERR << TSS_GetString(cTW, tw::STR_UNKNOWN_MODE_SPECIFIED) << pcMode << std::endl;
        TCERR << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
        return NULL;
    }


    iTWMode* pRtn = 0;
    switch (mode)
    {
    case cTWPrintCmdLine::MODE_DBPRINT:
        pRtn = new cTWPrintDBMode;
        break;
    case cTWPrintCmdLine::MODE_REPORTPRINT:
        pRtn = new cTWPrintReportMode;
        break;
    case cTWPrintCmdLine::MODE_HELP:
        pRtn = new cTWPrintHelpMode;
        break;
    default:
        ASSERT(false);
    }

    return pRtn;
}

//#############################################################################
// cTWPrintModeCommon -- contains data common to all modes; all cTWMode*_i will
//      derive from this class.
//#############################################################################
class cTWPrintModeCommon
{
public:
    int         mVerbosity;  // must be 0 <= n <= 2
    std::string mPassPhrase; // pass phrase for private key
    TSTRING     mLocalKeyFile;
    TSTRING     mSiteKeyFile;
    TSTRING     mConfigFilePath;

    cTWPrintModeCommon() : mVerbosity(1)
    {
    }
    cTextReportViewer::ReportingLevel mReportLevel; // The level of reporting to use.
};

///////////////////////////////////////////////////////////////////////////////
// FillOutCommonConfigInfo -- fills out all the common info with config file information
///////////////////////////////////////////////////////////////////////////////
static void FillOutCommonConfigInfo(cTWPrintModeCommon* pModeInfo, const cConfigFile& cf)
{
    TSTRING str;
    if (cf.Lookup(TSTRING(_T("LOCALKEYFILE")), str))
        pModeInfo->mLocalKeyFile = str;

    if (cf.Lookup(TSTRING(_T("SITEKEYFILE")), str))
        pModeInfo->mSiteKeyFile = str;

    //
    // turn all of the file names into full paths (they're relative to the exe dir)
    //
    TSTRING fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mLocalKeyFile, cSystemInfo::GetExeDir()))
        pModeInfo->mLocalKeyFile = fullPath;
}

///////////////////////////////////////////////////////////////////////////////
// FillOutCmdLineInfo -- fills out info common to all modes that appears on the
//      command line.
///////////////////////////////////////////////////////////////////////////////
static void FillOutCmdLineInfo(cTWPrintModeCommon* pModeInfo, const cCmdLineParser& cmdLine)
{
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWPrintCmdLine::VERBOSE:
            pModeInfo->mVerbosity = 2;
            break;
        case cTWPrintCmdLine::SILENT:
            pModeInfo->mVerbosity = 0;
            break;
        case cTWPrintCmdLine::LOCAL_KEY_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mLocalKeyFile = iter.ParamAt(0);
            break;
        case cTWPrintCmdLine::SITE_KEY_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mSiteKeyFile = iter.ParamAt(0);
            break;
        case cTWPrintCmdLine::HEXADECIMAL:
            cArchiveSigGen::SetHex(true);
            break;
        case cTWPrintCmdLine::PASSPHRASE:
        {
            // this bites! I have to make sure it is a narrow char string
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mPassPhrase = iter.ParamAt(0);
            break;
        }
        default:
            break;
        }
    }

    // use the verbosity information
    ASSERT((pModeInfo->mVerbosity >= 0) && (pModeInfo->mVerbosity < 3));
    iUserNotify::GetInstance()->SetVerboseLevel(pModeInfo->mVerbosity);
}

//#############################################################################
// cTWPrintReportMode
//#############################################################################
class cTWPrintReportMode_i : public cTWPrintModeCommon
{
public:
    TSTRING                 mReportFile;
    std::set<TSTRING>       mFilesToCheck;

    // ctor can set up some default values
    cTWPrintReportMode_i() : cTWPrintModeCommon()
    {
    }
};

void cTWPrintReportMode::SetConfigFile(TSTRING configFilePath)
{
    mpData->mConfigFilePath = configFilePath;
}

///////////////////////////////////////////////////////////////////////////////
// FillOutReportModeConfigInfo -- fills out all the common info with config file information
///////////////////////////////////////////////////////////////////////////////
void cTWPrintReportMode::FillOutReportModeConfigInfo(cTWPrintReportMode_i* pModeInfo, const cConfigFile& cf)
{
    TSTRING str;
    if (cf.Lookup(TSTRING(_T("REPORTFILE")), str))
        pModeInfo->mReportFile = str;

    // Find out what level of reporting we should use, use default level if none
    // has been specified.
    if (cf.Lookup(TSTRING(_T("REPORTLEVEL")), str))
    {
        if (_tcsicmp(str.c_str(), _T("0")) == 0)
            /*throw eTWPrintReportLevelZeroCfg( TSS_GetString( cTWPrint, twprint::STR_ERR2_REPORT_LEVEL_ZERO_CFG ) );
        // We error if reporting-level 0 is specified in the config. file
        // Level zero must be specified on the command line. - changed, 7/14/99 dra */
            pModeInfo->mReportLevel = cTextReportViewer::SINGLE_LINE;
        else if (_tcsicmp(str.c_str(), _T("1")) == 0)
            pModeInfo->mReportLevel = cTextReportViewer::PARSEABLE;
        else if (_tcsicmp(str.c_str(), _T("2")) == 0)
            pModeInfo->mReportLevel = cTextReportViewer::SUMMARY_ONLY;
        else if (_tcsicmp(str.c_str(), _T("3")) == 0)
            pModeInfo->mReportLevel = cTextReportViewer::CONCISE_REPORT;
        else if (_tcsicmp(str.c_str(), _T("4")) == 0)
            pModeInfo->mReportLevel = cTextReportViewer::FULL_REPORT;
        else
        {
            // They specified an illegal level, error.
            TSTRING errStr = _T("Invalid Level: ");
            errStr += str;
            throw eTWPrintInvalidReportLevelCfg(errStr);
        }
    }
    else
        // Use the default level of reporting, they specified none in configuration file.
        pModeInfo->mReportLevel = cTextReportViewer::CONCISE_REPORT;

    //
    // turn all of the file names into full paths (they're relative to the exe dir)
    //
    TSTRING fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mReportFile, cSystemInfo::GetExeDir()))
        pModeInfo->mReportFile = fullPath;
}

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWPrintReportMode::cTWPrintReportMode()
{
    mpData = new cTWPrintReportMode_i;
}

cTWPrintReportMode::~cTWPrintReportMode()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWPrintReportMode::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWPrintCmdLine::REPORT_FILE, TSTRING(_T("r")), TSTRING(_T("twrfile")), cCmdLineParser::PARAM_ONE);

    // multiple levels of reporting
    parser.AddArg(
        cTWPrintCmdLine::REPORTLEVEL, TSTRING(_T("t")), TSTRING(_T("report-level")), cCmdLineParser::PARAM_ONE);

    // For the variable object list.
    parser.AddArg(cTWPrintCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWPrintReportMode::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutCommonConfigInfo(mpData, cf);
    FillOutReportModeConfigInfo(mpData, cf);

    // TODO -- should I error or warn for (1) mutual exclustion errors or (2) unneeded cmd line
    //      parameters? I think I should, but I won't right now.
    // TODO -- error at the end of Init() if I am missing any requires parameter values..

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    // now do the stuff specific to this mode...
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWPrintCmdLine::REPORT_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mpData->mReportFile = iter.ParamAt(0);
            break;
        case cTWPrintCmdLine::REPORTLEVEL:
        {
            if (iter.ParamAt(0) == _T("0"))
                mpData->mReportLevel = cTextReportViewer::SINGLE_LINE;
            else if (iter.ParamAt(0) == _T("1"))
                mpData->mReportLevel = cTextReportViewer::PARSEABLE;
            else if (iter.ParamAt(0) == _T("2"))
                mpData->mReportLevel = cTextReportViewer::SUMMARY_ONLY;
            else if (iter.ParamAt(0) == _T("3"))
                mpData->mReportLevel = cTextReportViewer::CONCISE_REPORT;
            else if (iter.ParamAt(0) == _T("4"))
                mpData->mReportLevel = cTextReportViewer::FULL_REPORT;
            else
            {
                // They specified an illegal level, error.
                TSTRING errStr = _T("Invalid Level: ");
                errStr += iter.ParamAt(0);
                throw eTWPrintInvalidReportLevel(errStr);
            }
            break;
        }
        case cTWPrintCmdLine::PARAMS:
        {
            // pack all of these onto the files to check list...
            mpData->mFilesToCheck.clear();
            for (int i = 0; i < iter.NumParams(); i++)
            {
                mpData->mFilesToCheck.insert(iter.ParamAt(i));
            }
        }
        //done with report-level stuff.
        break;
        default:
            // should I do anything, or just ignore this?
            ;
        }
    }

    //----------------------------------------
    // I require the following information:
    //
    // * local key file
    // * report file
    //-----------------------------------------
    TEST_INIT_REQUIREMENT((!mpData->mLocalKeyFile.empty()), tw::STR_ERR_MISSING_LOCAL_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mReportFile.empty()), tw::STR_ERR_MISSING_REPORT);

    // check that the config file and site key file are in sync...
    //
    if (!mpData->mConfigFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mpData->mConfigFilePath, mpData->mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWPrintReportMode::Execute(cErrorQueue* pQueue)
{
    try
    {
        ASSERT(!mpData->mReportFile.empty());
        // make sure the report file exists...
        cFileUtil::TestFileExists(mpData->mReportFile);

        cKeyFile                    localKeyfile;
        const cElGamalSigPublicKey* pKey;
        cFCOReport                  report;
        cFCOReportHeader            reportHeader;

        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);
        pKey = localKeyfile.GetPublicKey();

        if (!cTWUtil::IsObjectEncrypted(mpData->mReportFile.c_str(),
                                        cFCOReport::GetFileHeaderID(),
                                        TSS_GetString(cTW, tw::STR_ERR_REPORT_READ)))
        {
            // note (rather than warn) if the database is not encrytped
            iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                               TSS_GetString(cTW, tw::STR_REPORT_NOT_ENCRYPTED).c_str());
        }

        // open the report...
        bool bDummy;
        cTWUtil::ReadReport(mpData->mReportFile.c_str(), reportHeader, report, pKey, false, bDummy);

        // print it
        cTextReportViewer trv(reportHeader, report);
        if (!mpData->mFilesToCheck.empty())
        {
            trv.SetObjects(mpData->mFilesToCheck);
        }

        trv.PrintTextReport(_T("-"), mpData->mReportLevel);
    }
    catch (eError& e)
    {
        cTWUtil::PrintErrorMsg(e);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// GetModeUsage -- returns a mode-specific usage statement.
///////////////////////////////////////////////////////////////////////////////
TSTRING cTWPrintReportMode::GetModeUsage()
{
    return TSS_GetString(cTWPrint, twprint::STR_TWPRINT_HELP_PRINT_REPORT);
}

//#############################################################################
// cTWPrintDBMode
//#############################################################################
class cTWPrintDBMode_i : public cTWPrintModeCommon
{
public:
    TSTRING                    mDbFile;
    std::vector<TSTRING>       mFilesToCheck;
    cTextDBViewer::DbVerbosity mDbVerbosity;

    // ctor can set up some default values
    cTWPrintDBMode_i() : cTWPrintModeCommon(), mDbVerbosity(cTextDBViewer::VERBOSE)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWPrintDBMode::cTWPrintDBMode()
{
    mpData = new cTWPrintDBMode_i;
}

cTWPrintDBMode::~cTWPrintDBMode()
{
    delete mpData;
}

void cTWPrintDBMode::SetConfigFile(TSTRING configFilePath)
{
    mpData->mConfigFilePath = configFilePath;
}

///////////////////////////////////////////////////////////////////////////////
// FillOutDBModeConfigInfo -- fills out all the common info with config file information
///////////////////////////////////////////////////////////////////////////////
void cTWPrintDBMode::FillOutDBModeConfigInfo(cTWPrintDBMode_i* pModeInfo, const cConfigFile& cf)
{
    TSTRING str;
    if (cf.Lookup(TSTRING(_T("DBFILE")), str))
        pModeInfo->mDbFile = str;

    if (cf.Lookup(TSTRING(_T("DBPRINTLEVEL")), str))
    {
        if (_tcsicmp(str.c_str(), _T("0")) == 0)
            pModeInfo->mDbVerbosity = cTextDBViewer::SUMMARY;
        else if (_tcsicmp(str.c_str(), _T("1")) == 0)
            pModeInfo->mDbVerbosity = cTextDBViewer::CONCISE;
        else if (_tcsicmp(str.c_str(), _T("2")) == 0)
            pModeInfo->mDbVerbosity = cTextDBViewer::VERBOSE;
        else
        {
            // They specified an illegal level, error.
            TSTRING errStr = _T("Invalid Level: ");
            errStr += str;
            throw eTWPrintInvalidDbPrintLevelCfg(errStr);
        }
    }
    else
        // Use the default level of reporting, they specified none in configuration file.
        pModeInfo->mVerbosity = cTextDBViewer::VERBOSE;

    //
    // turn all of the file names into full paths (they're relative to the exe dir)
    //
    TSTRING fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mDbFile, cSystemInfo::GetExeDir()))
        pModeInfo->mDbFile = fullPath;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWPrintDBMode::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWPrintCmdLine::DB_FILE, TSTRING(_T("d")), TSTRING(_T("dbfile")), cCmdLineParser::PARAM_ONE);

    // multiple levels of reporting
    parser.AddArg(
        cTWPrintCmdLine::REPORTLEVEL, TSTRING(_T("t")), TSTRING(_T("output-level")), cCmdLineParser::PARAM_ONE);

    // For the variable object list.
    parser.AddArg(cTWPrintCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWPrintDBMode::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutCommonConfigInfo(mpData, cf);
    FillOutDBModeConfigInfo(mpData, cf);

    // TODO -- should I error or warn for (1) mutual exclustion errors or (2) unneeded cmd line
    //      parameters? I think I should, but I won't right now.
    // TODO -- error at the end of Init() if I am missing any requires parameter values..

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    // now do the stuff specific to this mode...
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {

        case cTWPrintCmdLine::DB_FILE:
        {
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mpData->mDbFile = iter.ParamAt(0);
            break;
        }
        case cTWPrintCmdLine::REPORTLEVEL:
        {
            if (iter.ParamAt(0) == _T("0"))
                mpData->mDbVerbosity = cTextDBViewer::SUMMARY;
            else if (iter.ParamAt(0) == _T("1"))
                mpData->mDbVerbosity = cTextDBViewer::CONCISE;
            else if (iter.ParamAt(0) == _T("2"))
                mpData->mDbVerbosity = cTextDBViewer::VERBOSE;
            else
            {
                // They specified an illegal level, error.
                TSTRING errStr = _T("Invalid Level: ");
                errStr += iter.ParamAt(0);
                throw eTWPrintInvalidDbPrintLevel(errStr);
            }
            break;
        }
        case cTWPrintCmdLine::PARAMS:
        {
            // pack all of these onto the files to check list...
            mpData->mFilesToCheck.clear();
            for (int i = 0; i < iter.NumParams(); i++)
            {
                mpData->mFilesToCheck.push_back(iter.ParamAt(i));
            }
        }
        default:
            // TODO: should I do anything, or just ignore this?
            ;
        }
    }

    //----------------------------------------
    // I require the following information:
    //
    // * local key file
    // * report file
    //-----------------------------------------
    TEST_INIT_REQUIREMENT((!mpData->mLocalKeyFile.empty()), tw::STR_ERR_MISSING_LOCAL_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mDbFile.empty()), tw::STR_ERR_MISSING_DB);

    // check that the config file and site key file are in sync...
    //
    if (!mpData->mConfigFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mpData->mConfigFilePath, mpData->mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWPrintDBMode::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWPrintDBMode::Execute");
    int    ret = 0;

    try
    {
        ASSERT(!mpData->mDbFile.empty());
        // make sure the database file exists...
        cFileUtil::TestFileExists(mpData->mDbFile);

        cFCODatabaseFile            db;
        cKeyFile                    localKeyfile;
        const cElGamalSigPublicKey* pKey = 0;

        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);
        pKey = localKeyfile.GetPublicKey();

        if (!cTWUtil::IsObjectEncrypted(
                mpData->mDbFile.c_str(), cFCODatabaseFile::GetFileHeaderID(), TSS_GetString(cTW, tw::STR_ERR_DB_READ)))
        {
            // warn if the database is not encrytped
            if (iUserNotify::GetInstance()->GetVerboseLevel() > iUserNotify::V_SILENT)
            {
                cTWUtil::PrintErrorMsg(eTWDbNotEncrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
            }
        }

        // open the database; note that ReadDatabase will set bEncrypted and open the
        // key file if necessary
        bool bDummy;
        cTWUtil::ReadDatabase(mpData->mDbFile.c_str(), db, pKey, bDummy);

        if (mpData->mFilesToCheck.size() > 0)
        {
            bool details = (mpData->mDbVerbosity == cTextDBViewer::VERBOSE);
            //------------------------------------------------
            // print specific FCOs from the database
            //------------------------------------------------
            cTWUtil::GenreObjList listOut;
            cTWUtil::ParseObjectList(listOut, mpData->mFilesToCheck);

            // now, iterate through the list of objects...
            //
            cFCODatabaseFile::iterator dbIter(db);
            for (cTWUtil::GenreObjList::iterator genreIter = listOut.begin(); genreIter != listOut.end(); ++genreIter)
            {
                dbIter.SeekToGenre(genreIter->first);
                if (!dbIter.Done())
                {
                    cGenreSwitcher::GetInstance()->SelectGenre((cGenre::Genre)dbIter.GetGenre());
                    cDbDataSourceIter dsIter(&dbIter.GetDb());
                    dsIter.SetErrorBucket(pQueue);
                    iFCONameTranslator* pNT = cGenreSwitcher::GetInstance()
                                                  ->GetFactoryForGenre((cGenre::Genre)dbIter.GetGenre())
                                                  ->GetNameTranslator();
                    //
                    // iterate over all the objects in this genre....
                    //
                    for (cTWUtil::ObjList::iterator it = genreIter->second.begin(); it != genreIter->second.end(); ++it)
                    {
                        try
                        {
                            cFCOName name = cTWUtil::ParseObjectName(*it);
                            dsIter.SeekToFCO(name, false);
                            if ((!dsIter.Done()) && (dsIter.HasFCOData()))
                            {
                                cTextDBViewer::OutputFCO(
                                    dsIter, dbIter.GetGenreHeader().GetPropDisplayer(), pNT, &TCOUT, details);
                            }
                            else
                            {
                                eTWUtilObjNotInDb e(*it);
                                e.SetFatality(false);
                                cTWUtil::PrintErrorMsg(e);
                            }
                        }
                        catch (eError& e)
                        {
                            e.SetFatality(false);
                            cTWUtil::PrintErrorMsg(e);
                        }
                    }
                }
                else
                {
                    eTWDbDoesntHaveGenre e(
                        cGenreSwitcher::GetInstance()->GenreToString((cGenre::Genre)genreIter->first, true));
                    e.SetFatality(false);
                    cTWUtil::PrintErrorMsg(e);
                }
            }
        }
        else
        {
            //------------------------------------------------
            // printing the entire db
            //------------------------------------------------
            cTextDBViewer::PrintDB(db, _T("-"), mpData->mDbVerbosity);
        }
    }
    catch (eError& e)
    {
        cTWUtil::PrintErrorMsg(e);
        return 1;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// GetModeUsage -- returns a mode-specific usage statement.
///////////////////////////////////////////////////////////////////////////////
TSTRING cTWPrintDBMode::GetModeUsage()
{
    return TSS_GetString(cTWPrint, twprint::STR_TWPRINT_HELP_PRINT_DATABASE);
}


//#############################################################################
// cTWPrintHelpMode
//#############################################################################

class cTWPrintHelpMode_i
{
public:
    cTWPrintHelpMode_i()
    {
    }
    ~cTWPrintHelpMode_i()
    {
    }

    // A list of modes to output usage statements for:
    std::set<TSTRING> mlModes;
    std::set<TSTRING> mPrinted;
};


///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWPrintHelpMode::cTWPrintHelpMode()
{
    mpData = new cTWPrintHelpMode_i();
}

cTWPrintHelpMode::~cTWPrintHelpMode()
{
    delete mpData;
}

void cTWPrintHelpMode::SetConfigFile(TSTRING configFilePath)
{
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWPrintHelpMode::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // We're only interested in one parameter, that being help.  Anything else
    // passed to this mode should be a cmdlineparser error.
    cmdLine.AddArg(cTWPrintCmdLine::MODE, TSTRING(_T("m")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY, true);
    cmdLine.AddArg(cTWPrintCmdLine::MODE_HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWPrintCmdLine::MODE_HELP_ALL, TSTRING(_T("")), TSTRING(_T("all")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(
        cTWPrintCmdLine::MODE_REPORTPRINT, TSTRING(_T("r")), TSTRING(_T("print-report")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(
        cTWPrintCmdLine::MODE_DBPRINT, TSTRING(_T("d")), TSTRING(_T("print-dbfile")), cCmdLineParser::PARAM_MANY);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWPrintHelpMode::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    cCmdLineIter iter(cmdLine);

    // Grab the arguments from the help parameter:
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWPrintCmdLine::MODE_HELP:
        {
            int i;
            for (i = 0; i < iter.NumParams(); ++i)
                mpData->mlModes.insert(iter.ParamAt(i));
        }
        break;
        case cTWPrintCmdLine::MODE:
        {
            int i;
            for (i = 0; i < iter.NumParams(); ++i)
            {
                mpData->mlModes.insert(iter.ParamAt(i));
            }
        }
        break;
        // Begin ugly hack so we can allow users to enter the mode
        // names with "--" prepended.  We have to do this, since
        // the cmdlineparser treats them as switches.
        case cTWPrintCmdLine::MODE_HELP_ALL:    // fall through
        case cTWPrintCmdLine::MODE_REPORTPRINT: // fall through
        case cTWPrintCmdLine::MODE_DBPRINT:
        {
            int     i;
            TSTRING str = iter.ActualParam();
            // Kill off the initial "--" or "-"
            str.erase(0, 1);
            if (str.length() != 1)
                str.erase(0, 1);

            // push back the parameter that was actually passed.
            mpData->mlModes.insert(str);
            // grab all the "parameters" following the mode/switch, since
            // they may be valid modes without the "--" prepended.
            for (i = 0; i < iter.NumParams(); ++i)
                mpData->mlModes.insert(iter.ParamAt(i));
        }
        break;
        default:
            // should I do anything, or just ignore this?
            ;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWPrintHelpMode::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWPrintHelpMode::Execute");

    // The iterator we will use to traverse the list of arguments:
    std::set<TSTRING>::iterator it = mpData->mlModes.begin();

    // We'll want to output the version information, regardless:
    TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_VERSION) << std::endl;
    TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;

    if (it == mpData->mlModes.end()) // all that was passed was --help
    {
        // Output a short usage statement for each mode
        TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_USAGE_SUMMARY);
        //
        // That's it.  return
        return 1;
    }

    for (; it != mpData->mlModes.end(); ++it)
    {
        if (_tcscmp((*it).c_str(), _T("all")) == 0)
        {
            //Since --help all was passed, emit all help messages and return.
            TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_HELP_PRINT_REPORT);
            TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_HELP_PRINT_DATABASE);

            //We're done, return
            return 1;
        }
    }

    //We need some subset of the usage statements.  Figure out which modes have
    //been specified:
    it = mpData->mlModes.begin();
    for (; it != mpData->mlModes.end(); ++it)
    {
        if (_tcscmp((*it).c_str(), _T("print-report")) == 0 || _tcscmp((*it).c_str(), _T("r")) == 0)
        {
            // make sure we don't print the same help twice...
            if (mpData->mPrinted.find(_T("print-report")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_HELP_PRINT_REPORT);
                mpData->mPrinted.insert(_T("print-report"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("print-dbfile")) == 0 || _tcscmp((*it).c_str(), _T("d")) == 0)
        {
            if (mpData->mPrinted.find(_T("print-dbfile")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWPrint, twprint::STR_TWPRINT_HELP_PRINT_DATABASE);
                mpData->mPrinted.insert(_T("print-dbfile"));
            }
        }
        else
        {
            cTWUtil::PrintErrorMsg(eTWPrintInvalidParamHelp((*it), eError::NON_FATAL));
            TCOUT << std::endl;
            // emit error/warning string, this mode does not exist
        }
    }

    //Everything went okay
    return 1;
}
