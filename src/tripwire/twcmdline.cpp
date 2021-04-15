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
// twcmdline.cpp
#include "stdtripwire.h"
#include "twcmdline.h"
#include "core/cmdlineparser.h"
#include "tw/configfile.h"

#include "core/serializerimpl.h" // cSerializerImpl
#include "core/archive.h"        // cArchive and friends
#include "fco/fcospeclist.h"     // cFCOSpecList
#include "tripwirestrings.h"     // tripwire.exe specific strings
#include "twcrypto/keyfile.h"    // cKeyFile -- used for encryption
#include "tw/twutil.h"
#include "twcmdlineutil.h"       // utility functions for the cmd line
#include "tw/fcoreport.h"        // for reports
#include "tw/textreportviewer.h" // text report viewer
#include "core/usernotify.h"     // for notifying the user of events
#include "tw/fcoreportutil.h"    // for finalizing report
#include "tw/fcodatabaseutil.h"  // for finalizing database
#include "tw/headerinfo.h"       // for finalizing database
#include "fco/fcopropdisplayer.h"
#include "tw/systeminfo.h"
#include "tw/filemanipulator.h"
#include "fco/twfactory.h"
#include "mailmessage.h" // used for email reporting
#include "fco/genrespeclist.h"
#include "core/twlimits.h" // for severity limits
#include "core/errorgeneral.h"
#include "core/corestrings.h"
#include "tw/twerrors.h"
#include "core/stringutil.h"
#include "util/fileutil.h"
#include "tw/twstrings.h"
#include "syslog_trip.h"
#include <set>
#include "fco/parsergenreutil.h" // this is needed to figure out if a path is fully qualified for the current genre.
#include "tw/fcodatabasefile.h"
#include "fco/signature.h"
#include "fco/genreswitcher.h"
#include "generatedb.h"
#include "integritycheck.h"
#include "updatedb.h"
#include "policyupdate.h"
#include "core/platform.h"

#ifdef TW_PROFILE
#include "tasktimer.h"
#endif

#include "fs/fsdatasourceiter.h" // for cross file systems flag
#include <unistd.h>              // for _exit()

//-----------------------------------------------------------------------------
// #defines
//-----------------------------------------------------------------------------

// this is used to make required condition checking in the Init() functions a little more compact
#define TEST_INIT_REQUIREMENT(t, c, n)                                     \
    if (!t)                                                                \
    {                                                                      \
        cTWUtil::PrintErrorMsg(eTWInitialization(TSS_GetString(c, n), 0)); \
        return false;                                                      \
    }


static bool util_GetEditor(TSTRING& strEd);

//#############################################################################
// cTWCmdLine
//#############################################################################


///////////////////////////////////////////////////////////////////////////////
// GetMode -- processes the command line arguments and creates an appropriate
//    structure for the selected mode, or NULL if an error occurs
///////////////////////////////////////////////////////////////////////////////
iTWMode* cTWCmdLine::GetMode(int argc, const TCHAR* const* argv)
{
    // note -- it is assumed the executable name is the first parameter
    if (argc < 2)
    {
        TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_VERSION) << std::endl;
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
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_ERR_NO_MODE) << std::endl;
            TCOUT << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
            return NULL;
        }
        pcMode = argv[2];
        if (_tcscmp(argv[2], _T("i")) == 0)
            mode = MODE_INIT;
        else if (_tcscmp(argv[2], _T("c")) == 0)
            mode = MODE_CHECK;
        else if (_tcscmp(argv[2], _T("u")) == 0)
            mode = MODE_UPDATE_DB;
        else if (_tcscmp(argv[2], _T("p")) == 0)
            mode = MODE_UPDATE_POL;
        else if (_tcscmp(argv[2], _T("t")) == 0)
            mode = MODE_TEST;
    }
    else
    {
        pcMode = argv[1];
        if (_tcscmp(argv[1], _T("--init")) == 0)
            mode = MODE_INIT;
        else if (_tcscmp(argv[1], _T("--check")) == 0)
            mode = MODE_CHECK;
        else if (_tcscmp(argv[1], _T("--update")) == 0)
            mode = MODE_UPDATE_DB;
        else if (_tcscmp(argv[1], _T("--update-policy")) == 0)
            mode = MODE_UPDATE_POL;
        else if (_tcscmp(argv[1], _T("--version")) == 0)
            mode = MODE_VERSION;
        else if (_tcscmp(argv[1], _T("--help")) == 0 || _tcscmp(argv[1], _T("-?")) == 0)
            mode = MODE_HELP;
        else if (_tcscmp(argv[1], _T("--test")) == 0)
            mode = MODE_TEST;

#ifdef DEBUG
        if (_tcscmp(argv[1], _T("--explore")) == 0)
            mode = MODE_EXPLORE;
        if (_tcscmp(argv[1], _T("--verifydb")) == 0)
            mode = MODE_DEBUG;
#endif
    }

    if (mode == MODE)
    {
        // unknown mode switch
        cDebug d("cTWCmdLine::GetMode");
        d.TraceError(_T("Error: Bad mode switch: %s\n"), pcMode);
        TCERR << TSS_GetString(cTW, tw::STR_UNKNOWN_MODE_SPECIFIED) << pcMode << std::endl
              << TSS_GetString(cTW, tw::STR_GET_HELP) << std::endl;
        return NULL;
    }


    iTWMode* pRtn = 0;
    switch (mode)
    {
    case cTWCmdLine::MODE_INIT:
        pRtn = new cTWModeDbInit;
        break;
    case cTWCmdLine::MODE_CHECK:
        pRtn = new cTWModeIC;
        break;
    case cTWCmdLine::MODE_UPDATE_DB:
        pRtn = new cTWModeDbUpdate;
        break;
    case cTWCmdLine::MODE_UPDATE_POL:
        pRtn = new cTWModePolUpdate;
        break;
    case cTWCmdLine::MODE_TEST:
        pRtn = new cTWModeTest;
        break;
    case cTWCmdLine::MODE_HELP:
        pRtn = new cTWModeHelp;
        break;
    case cTWCmdLine::MODE_VERSION:
        pRtn = new cTWModeVersion;
        break;

//Explore and Debug modes are invisible unless DEBUG is defined.
#ifdef DEBUG
    case cTWCmdLine::MODE_EXPLORE:
        pRtn = new cTWModeExploreDb;
        break;
    case cTWCmdLine::MODE_DEBUG:
        pRtn = new cTWModeDebugDb;
        break;
#endif

    default:
        ASSERT(false);
    }

    return pRtn;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineCommon -- initializes the command line switches common to all
//    modes
///////////////////////////////////////////////////////////////////////////////
static void InitCmdLineCommon(cCmdLineParser& parser)
{
    parser.AddArg(cTWCmdLine::HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWCmdLine::MODE, TSTRING(_T("m")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWCmdLine::VERBOSE, TSTRING(_T("v")), TSTRING(_T("verbose")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWCmdLine::SILENT, TSTRING(_T("s")), TSTRING(_T("silent")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWCmdLine::SILENT, TSTRING(_T("")), TSTRING(_T("quiet")), cCmdLineParser::PARAM_NONE);
    //parser.AddArg(cTWCmdLine::NO_BACKUP,    TSTRING(_T("B")), TSTRING(_T("nobackup")),      cCmdLineParser::PARAM_NONE);
    // this was dropped from 1.5 -- mdb
    parser.AddArg(
        cTWCmdLine::LOCAL_PASSPHRASE, TSTRING(_T("P")), TSTRING(_T("local-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWCmdLine::POL_FILE, TSTRING(_T("p")), TSTRING(_T("polfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWCmdLine::CFG_FILE, TSTRING(_T("c")), TSTRING(_T("cfgfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWCmdLine::DB_FILE, TSTRING(_T("d")), TSTRING(_T("dbfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWCmdLine::REPORT_FILE, TSTRING(_T("r")), TSTRING(_T("twrfile")), cCmdLineParser::PARAM_ONE);

    //
    // mutual exclusions
    //
    parser.AddMutEx(cTWCmdLine::VERBOSE, cTWCmdLine::SILENT);
}


///////////////////////////////////////////////////////////////////////////////
// Set up temp directory
///////////////////////////////////////////////////////////////////////////////
static void util_InitTempDirectory(const cConfigFile& cf)
{
    TSTRING temp_directory;
    cf.Lookup(TSTRING(_T("TEMPDIRECTORY")), temp_directory);

    if (temp_directory.empty())
    {
#if IS_AROS
        temp_directory = "T:";
#elif IS_DOS_DJGPP
        temp_directory = "/dev/c/temp/";
#elif IS_RISCOS
        temp_directory = "/!BOOT/Resources/!Scrap/ScrapDirs/ScrapDir";
#elif IS_REDOX
        temp_directory = "/file/tmp/";
#else
        temp_directory = "/tmp/";
#endif
    }

#if !IS_RISCOS
    // make sure we have a trailing slash -- thanks Jarno...
    //
    if (*temp_directory.rbegin() != '/')
    {
#if !ARCHAIC_STL      
        temp_directory.push_back('/');
#else
        temp_directory.append("/");
#endif	
    }
#endif

    // make sure it exists...
    //

#if IS_AROS || IS_RISCOS
    temp_directory = cDevicePath::AsNative(temp_directory);
#elif IS_DOS_DJGPP
    temp_directory = cDevicePath::AsPosix(temp_directory);
#endif

    if (access(temp_directory.c_str(), F_OK) != 0)
    {
        TSTRING errStr = TSS_GetString(cCore, core::STR_BAD_TEMPDIRECTORY);
        TSTRING tmpStr = _T("Directory: ");
        tmpStr += (temp_directory + _T("\n"));
        tmpStr += errStr;
        throw eTWInvalidTempDirectory(tmpStr);
    }
    else
    {
#if IS_RISCOS
        if (*temp_directory.rbegin() != '.')
        {
            temp_directory.push_back('.');
        }
#endif

        iFSServices::GetInstance()->SetTempDirName(temp_directory);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Set up various email reporting options
///////////////////////////////////////////////////////////////////////////////
static void util_InitEmailOptions(cTWModeCommon* pModeInfo, const cConfigFile& cf)
{
    TSTRING str;
    if (cf.Lookup(TSTRING(_T("GLOBALEMAIL")), str))
    {
        if (str.length() != 0)
            pModeInfo->mGlobalEmail = str;
    }

    //
    // Set the report-viewing level if one has been specified, use
    // default level otherwise.
    //
    if (cf.Lookup(TSTRING(_T("EMAILREPORTLEVEL")), str))
    {
        if (_tcsicmp(str.c_str(), _T("0")) == 0)
            pModeInfo->mEmailReportLevel = cTextReportViewer::SINGLE_LINE;
        else if (_tcsicmp(str.c_str(), _T("1")) == 0)
            pModeInfo->mEmailReportLevel = cTextReportViewer::PARSEABLE;
        else if (_tcsicmp(str.c_str(), _T("2")) == 0)
            pModeInfo->mEmailReportLevel = cTextReportViewer::SUMMARY_ONLY;
        else if (_tcsicmp(str.c_str(), _T("3")) == 0)
            pModeInfo->mEmailReportLevel = cTextReportViewer::CONCISE_REPORT;
        else if (_tcsicmp(str.c_str(), _T("4")) == 0)
            pModeInfo->mEmailReportLevel = cTextReportViewer::FULL_REPORT;
        else
        {
            // They specified an illegal level, error.
            TSTRING errStr = _T("Invalid Level: ");
            errStr += str;
            throw eTWInvalidReportLevelCfg(errStr);
        }
    }
    else
    {
        // no level was specified in the configuration file, use default.
        pModeInfo->mEmailReportLevel = cTextReportViewer::CONCISE_REPORT;
    }

    // Decide what mail method should be used to email reports
    if (cf.Lookup(TSTRING(_T("MAILMETHOD")), str))
    {
        if (_tcsicmp(str.c_str(), _T("SENDMAIL")) == 0)
            pModeInfo->mMailMethod = cMailMessage::MAIL_BY_PIPE;
        else if (_tcsicmp(str.c_str(), _T("SMTP")) == 0)
            pModeInfo->mMailMethod = cMailMessage::MAIL_BY_SMTP;
        else
            pModeInfo->mMailMethod = cMailMessage::INVALID_METHOD;
    }
    else
    {
        pModeInfo->mMailMethod = cMailMessage::NO_METHOD;
    }

#if !SUPPORTS_NETWORKING
    if (pModeInfo->mMailMethod == cMailMessage::MAIL_BY_SMTP)
        throw eMailSMTPNotSupported();
#endif

    // Get the SMTP server
    if (cf.Lookup(TSTRING(_T("SMTPHOST")), str))
    {
        pModeInfo->mSmtpHost = str;
    }
    else
    {
        pModeInfo->mSmtpHost = _T("127.0.0.1"); // this is the default
    }

    // Get the SMTP port number
    if (cf.Lookup(TSTRING(_T("SMTPPORT")), str))
    {
        int i = _ttoi(str.c_str());
        if (i < 0 || i > SHRT_MAX)
            throw eTWInvalidPortNumber(str);
        pModeInfo->mSmtpPort = static_cast<unsigned short>(i);
    }
    else
    {
        pModeInfo->mSmtpPort = 25; // this is the default
    }

    // Get the mail program to use if we're piping our email
    if (cf.Lookup(TSTRING(_T("MAILPROGRAM")), str))
    {
        pModeInfo->mMailProgram = str;
    }
    else
    {
        pModeInfo->mMailProgram.erase(); // MAILPROGRAM is not required to be specified
    }

    // Get the mail program to use if we're piping our email
    if (cf.Lookup(TSTRING(_T("MAILNOVIOLATIONS")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            pModeInfo->mMailNoViolations = true;
        else
            pModeInfo->mMailNoViolations = false;
    }
    else
    {
        pModeInfo->mMailNoViolations = true; // MAILPROGRAM is not required to be specified
    }

    if (cf.Lookup(TSTRING(_T("MAILFROMADDRESS")), str))
    {
        pModeInfo->mMailFrom = str;
    }
}


///////////////////////////////////////////////////////////////////////////////
// FillOutConfigInfo -- fills out all the common info with config file information
///////////////////////////////////////////////////////////////////////////////
static void FillOutConfigInfo(cTWModeCommon* pModeInfo, const cConfigFile& cf)
{
    TSTRING str;
    if (cf.Lookup(TSTRING(_T("POLFILE")), str))
    {
        pModeInfo->mPolFile = str;
    }
    if (cf.Lookup(TSTRING(_T("DBFILE")), str))
    {
        pModeInfo->mDbFile = str;
    }
    if (cf.Lookup(TSTRING(_T("SITEKEYFILE")), str))
    {
        pModeInfo->mSiteKeyFile = str;
    }
    if (cf.Lookup(TSTRING(_T("LOCALKEYFILE")), str))
    {
        pModeInfo->mLocalKeyFile = str;
    }
    if (cf.Lookup(TSTRING(_T("REPORTFILE")), str))
    {
        pModeInfo->mReportFile = str;
    }
    if (cf.Lookup(TSTRING(_T("EDITOR")), str))
    {
        pModeInfo->mEditor = str;
    }
    if (cf.Lookup(TSTRING(_T("LATEPROMPTING")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            pModeInfo->mbLatePassphrase = true;
    }
    if (cf.Lookup(TSTRING(_T("RESETACCESSTIME")), str))
    {
        // We do not support reset access time on Unix, so we issue a warning.
        // This used to be a fatal error, however this prevents
        // cross platform config files.
        cTWUtil::PrintErrorMsg(eTWInvalidConfigFileKey(_T("RESETACCESSTIME"), eError::NON_FATAL));
    }
    if (cf.Lookup(TSTRING(_T("LOOSEDIRECTORYCHECKING")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            pModeInfo->mfLooseDirs = true;
    }

    util_InitTempDirectory(cf);

    util_InitEmailOptions(pModeInfo, cf);

    // SYSLOG reporting
    if (cf.Lookup(TSTRING(_T("SYSLOGREPORTING")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
        {
#if SUPPORTS_SYSLOG
            pModeInfo->mbLogToSyslog = true;
#else
            eTWSyslogNotSupported e;
            e.SetFatality(false);
            cErrorReporter::PrintErrorMsg(e);
            pModeInfo->mbLogToSyslog = false;
#endif
        }
        else
            pModeInfo->mbLogToSyslog = false;
    }
    else
    {
        pModeInfo->mbLogToSyslog = false;
    }

    // Crossing file systems
    if (cf.Lookup(TSTRING(_T("CROSSFILESYSTEMS")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            pModeInfo->mbCrossFileSystems = true;
        else
            pModeInfo->mbCrossFileSystems = false;
    }

    if (cf.Lookup(TSTRING(_T("HASH_DIRECT_IO")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
        {
            pModeInfo->mbDirectIO = true;
            cArchiveSigGen::SetUseDirectIO(true);
        }
    }

    if (cf.Lookup(TSTRING(_T("RESOLVE_IDS_TO_NAMES")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            iFSServices::GetInstance()->SetResolveNames(true);
        else
            iFSServices::GetInstance()->SetResolveNames(false);
    }

    //
    // turn all of the file names into full paths (they're relative to the exe dir)
    //
    TSTRING fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mPolFile, cSystemInfo::GetExeDir()))
        pModeInfo->mPolFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mDbFile, cSystemInfo::GetExeDir()))
        pModeInfo->mDbFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mSiteKeyFile, cSystemInfo::GetExeDir()))
        pModeInfo->mSiteKeyFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mLocalKeyFile, cSystemInfo::GetExeDir()))
        pModeInfo->mLocalKeyFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mReportFile, cSystemInfo::GetExeDir()))
        pModeInfo->mReportFile = fullPath;
}

///////////////////////////////////////////////////////////////////////////////
// FillOutCmdLineInfo -- fills out info common to all modes that appears on the
//    command line.
///////////////////////////////////////////////////////////////////////////////
static void FillOutCmdLineInfo(cTWModeCommon* pModeInfo, const cCmdLineParser& cmdLine)
{
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWCmdLine::VERBOSE:
            pModeInfo->mVerbosity = 2;
            break;
        case cTWCmdLine::SILENT:
            pModeInfo->mVerbosity = 0;
            break;
        case cTWCmdLine::POL_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mPolFile = iter.ParamAt(0);
            break;
        case cTWCmdLine::SITE_KEY_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mSiteKeyFile = iter.ParamAt(0);
            break;
        case cTWCmdLine::LOCAL_KEY_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mLocalKeyFile = iter.ParamAt(0);
            break;
        case cTWCmdLine::DB_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mDbFile = iter.ParamAt(0);
            break;
        case cTWCmdLine::REPORT_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mReportFile = iter.ParamAt(0);
            break;
        case cTWCmdLine::LOCAL_PASSPHRASE:
        {
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            pModeInfo->mLocalPassphrase = cStringUtil::TstrToWstr(iter.ParamAt(0));
            pModeInfo->mLocalProvided   = true;
        }
        break;
/*    case cTWCmdLine::NO_BACKUP:
         pModeInfo->mbBackup = false;
         break;
*/    }
    }

    //
    // turn all of the file names into full paths for nice presentation to the user.
    //
    TSTRING fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mPolFile))
        pModeInfo->mPolFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mSiteKeyFile))
        pModeInfo->mSiteKeyFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mLocalKeyFile))
        pModeInfo->mLocalKeyFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mDbFile))
        pModeInfo->mDbFile = fullPath;
    if (iFSServices::GetInstance()->FullPath(fullPath, pModeInfo->mReportFile))
        pModeInfo->mReportFile = fullPath;


    // use the verbosity information
    ASSERT((pModeInfo->mVerbosity >= 0) && (pModeInfo->mVerbosity < 3));
    iUserNotify::GetInstance()->SetVerboseLevel(pModeInfo->mVerbosity);

    // if a passphrase was passed on the command line, then late prompting should
    // be set to false so that wrong passwords are detected immediately
    if (pModeInfo->mLocalProvided && pModeInfo->mbLatePassphrase)
    {
        // TODO -- I don't know if we should display this or not -- mdb
        //iUserNotify::GetInstance()->Notify(  iUserNotify::V_VERBOSE, TSS_GetString( cTripwire, tripwire::STR_NOTIFY_CHANGE_PROMPT_TIME).c_str() );
        pModeInfo->mbLatePassphrase = false;
    }
}

//#############################################################################
// cTWModeDbInit
//#############################################################################
class cTWModeDbInit_i : public cTWModeCommon
{
public:
    bool mbEncryptDb;

    // ctor can set up some default values
    cTWModeDbInit_i() : cTWModeCommon(), mbEncryptDb(true)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeDbInit::cTWModeDbInit()
{
    mpData = new cTWModeDbInit_i;
}

cTWModeDbInit::~cTWModeDbInit()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeDbInit::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    cmdLine.AddArg(cTWCmdLine::MODE_INIT, TSTRING(_T("")), TSTRING(_T("init")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::NO_ENCRYPT, TSTRING(_T("e")), TSTRING(_T("no-encryption")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_NONE);

    // mutual exclusion...
    cmdLine.AddMutEx(cTWCmdLine::NO_ENCRYPT, cTWCmdLine::LOCAL_PASSPHRASE);
    cmdLine.AddMutEx(cTWCmdLine::NO_ENCRYPT, cTWCmdLine::LOCAL_KEY_FILE);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeDbInit::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    // now do the stuff specific to this mode...
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWCmdLine::NO_ENCRYPT:
            mpData->mbEncryptDb = false;
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
    // * site key file
    // * policy file
    // * db file
    //-----------------------------------------
    TEST_INIT_REQUIREMENT((!mpData->mLocalKeyFile.empty()), cTW, tw::STR_ERR_MISSING_LOCAL_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mSiteKeyFile.empty()), cTW, tw::STR_ERR_MISSING_SITE_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mPolFile.empty()), cTW, tw::STR_ERR_MISSING_POLICY);
    TEST_INIT_REQUIREMENT((!mpData->mDbFile.empty()), cTW, tw::STR_ERR_MISSING_DB);


    // make sure that the config file and site key file are in sync...
    //
    if (cTWUtil::VerifyCfgSiteKey(mstrConfigFile, mpData->mSiteKeyFile) == false)
        cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));

    // Set the cross file systems flag appropriately.
    cFSDataSourceIter::SetFileSystemCrossing(mpData->mbCrossFileSystems);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeDbInit::Execute(cErrorQueue* pQueue)
{
    cFCODatabaseFile           dbFile;
    cFCODatabaseFile::iterator dbIter(dbFile);

    dbFile.SetFileName(mpData->mDbFile);
    try
    {
        // don't go any further if we won't be able to write to the db file...
        cFileUtil::TestFileWritable(mpData->mDbFile);

        // open the keyfile for early passphrase...
        cKeyFile                     keyfile;
        const cElGamalSigPrivateKey* pPrivateKey = 0;
        if (mpData->mbEncryptDb)
        {
            cTWUtil::OpenKeyFile(keyfile, mpData->mLocalKeyFile);
            // open the private key for "early passphrase"
            if (!mpData->mbLatePassphrase)
                pPrivateKey = cTWUtil::CreatePrivateKey(
                    keyfile, mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0, cTWUtil::KEY_LOCAL);
        }

        //Parse the policy file
        cGenreSpecListVector genreSpecList;
        cTWCmdLineUtil::ParsePolicyFile(genreSpecList, mpData->mPolFile, mpData->mSiteKeyFile, pQueue);

#ifdef TW_PROFILE
        cTaskTimer timer("cTripwire::GenerateDatabase");
        timer.Start();
#endif

        iUserNotify::GetInstance()->Notify(1, TSS_GetString(cTripwire, tripwire::STR_GENERATING_DB).c_str());

        uint32_t gdbFlags = 0;
        gdbFlags |= (mpData->mbResetAccessTime ? cGenerateDb::FLAG_ERASE_FOOTPRINTS_GD : 0);
        gdbFlags |= (mpData->mbDirectIO ? cGenerateDb::FLAG_DIRECT_IO : 0);

        // loop through the genres
        cGenreSpecListVector::iterator genreIter;
        for (genreIter = genreSpecList.begin(); genreIter != genreSpecList.end(); ++genreIter)
        {
            // make this genre the current genre.
            //
            cGenreSwitcher::GetInstance()->SelectGenre(genreIter->GetGenre());

            // notify the user
            //
            iUserNotify::GetInstance()->Notify(
                iUserNotify::V_NORMAL,
                TSS_GetString(cTripwire, tripwire::STR_PROCESSING_GENRE).c_str(),
                cGenreSwitcher::GetInstance()->GenreToString(cGenreSwitcher::GetInstance()->CurrentGenre(), true));

            // add an entry to the database for each genre we are parsing
            dbFile.AddGenre(genreIter->GetGenre(), &dbIter);

            // copy the spec list to the database...
            dbIter.GetSpecList() = genreIter->GetSpecList();

            // generate the database...
            // TODO -- turn pQueue into an error bucket
            cGenerateDb::Execute(
                dbIter.GetSpecList(), dbIter.GetDb(), dbIter.GetGenreHeader().GetPropDisplayer(), pQueue, gdbFlags);
        }

        cFCODatabaseUtil::CalculateHeader(dbFile.GetHeader(),
                                          mpData->mPolFile,
                                          mstrConfigFile,
                                          mpData->mDbFile,
                                          mstrCmdLine,
                                          cSystemInfo::GetExeStartTime(),
                                          0);

#ifdef TW_PROFILE
        timer.Stop();
#endif

        // write the db to disk...
        if (mpData->mbEncryptDb)
        {
            // open the key file for late passphrase
            if (!pPrivateKey)
            {
                ASSERT(mpData->mbLatePassphrase);
                pPrivateKey = cTWUtil::CreatePrivateKey(
                    keyfile, mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0, cTWUtil::KEY_LOCAL);
            }

            // backup the file we are about to overwrite
            cFileUtil::BackupFile(mpData->mDbFile);

            cTWUtil::WriteDatabase(mpData->mDbFile.c_str(), dbFile, true, pPrivateKey);
            keyfile.ReleasePrivateKey();
        }
        else
        {
            cFileUtil::BackupFile(mpData->mDbFile); // backup the file we are about to overwrite

            cTWUtil::WriteDatabase(mpData->mDbFile.c_str(), dbFile, false, NULL); // false means no encryption
        }
    }
    catch (eError& e)
    {
        if (mpData->mbLogToSyslog)
        {
            TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_INIT_FAIL_MSG);
            msg.append(cErrorTable::GetInstance()->Get(e.GetID()));

            cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_ERROR, msg.c_str());
        }

        cTWUtil::PrintErrorMsg(e);
        return 8;
    }

    // everything went ok; return 0
    iUserNotify::GetInstance()->Notify(1, TSS_GetString(cTripwire, tripwire::STR_INIT_SUCCESS).c_str());

    if (mpData->mbLogToSyslog)
    {
        TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_INIT_MSG) + mpData->mDbFile;
        cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_SUCCESS, msg.c_str());
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// GetModeUsage
///////////////////////////////////////////////////////////////////////////////
TSTRING cTWModeDbInit::GetModeUsage(void)
{
    return (TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_INIT));
}

//#############################################################################
// cTWModeIC
//#############################################################################
class cTWModeIC_i : public cTWModeCommon
{
public:
    bool    mbUpdate;         // launch the editor after integrity checking is done
    bool    mbPrintToStdout;  // print a text version of the report to stdout?
    bool    mbEmail;          // email the report to the appropriate parties?
    TSTRING mIgnoreProps;     // symbols of properties to ignore
    bool    mbEncryptReport;  // should the report be encrypted?
    int     mSeverityLevel;   // only policy rules greater than or equal to the specified level will be checked.
    bool    mbTrimBySeverity; // do we use mSeverityLevel to trim the spec list?
    TSTRING mSeverityName;    // gets mapped to number, then treated like mSeverityLevel
    TSTRING mRuleName;        // only the named rule will be checked
    TSTRING mGenreName;       // if not empty, specifies the genre to check
    bool    mbSecureMode;     // are we in extra-pedantic mode? (only valid with mbUpdate == true)

    //TSTRING     mCmdLine;               // entire command line
    std::vector<TSTRING> mFilesToCheck;

    // ctor can set up some default values
    cTWModeIC_i()
        : cTWModeCommon(),
          mbUpdate(false),
          mbPrintToStdout(true),
          mbEmail(false),
          mbEncryptReport(false),
          mSeverityLevel(-1),
          mbTrimBySeverity(false),
          mbSecureMode(false)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeIC::cTWModeIC()
{
    mpData = new cTWModeIC_i;
}

cTWModeIC::~cTWModeIC()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeIC::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    cmdLine.AddArg(cTWCmdLine::MODE_CHECK, TSTRING(_T("")), TSTRING(_T("check")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::INTER_UPDATE, TSTRING(_T("I")), TSTRING(_T("interactive")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::EDITOR, TSTRING(_T("V")), TSTRING(_T("visual")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::NO_TTY, TSTRING(_T("n")), TSTRING(_T("no-tty-output")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::MAIL_REPORT, TSTRING(_T("M")), TSTRING(_T("email-report")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::IGNORE_PROP, TSTRING(_T("i")), TSTRING(_T("ignore")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(
        cTWCmdLine::ENCRYPT_REPORT, TSTRING(_T("E")), TSTRING(_T("signed-report")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::SEVERITY_LEVEL, TSTRING(_T("l")), TSTRING(_T("severity")), cCmdLineParser::PARAM_ONE);
    //cmdLine.AddArg(cTWCmdLine::SEVERITY_NAME,  TSTRING(_T("V")), TSTRING(_T("severityname")),  cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::RULE_NAME, TSTRING(_T("R")), TSTRING(_T("rule-name")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::GENRE_NAME, TSTRING(_T("x")), TSTRING(_T("section")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::HEXADECIMAL, TSTRING(_T("h")), TSTRING(_T("hexadecimal")), cCmdLineParser::PARAM_NONE);

    // multiple levels of reporting
    cmdLine.AddArg(
        cTWCmdLine::REPORTLEVEL, TSTRING(_T("t")), TSTRING(_T("email-report-level")), cCmdLineParser::PARAM_ONE);

    // mutual exclusion...
    // you can't specify any of these 3 things together...
    cmdLine.AddMutEx(cTWCmdLine::SEVERITY_LEVEL, cTWCmdLine::RULE_NAME);
    cmdLine.AddDependency(cTWCmdLine::REPORTLEVEL, cTWCmdLine::MAIL_REPORT);
    // Report error if the user has specified an editor and we're not in interactive update
    // mode:  This needs to happen here, so we don't nail someone for having the editor
    // specified in the config. file.  We should only be this picky if both parameters are
    // passed on the command line.
    cmdLine.AddDependency(cTWCmdLine::EDITOR, cTWCmdLine::INTER_UPDATE);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeIC::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // We will use this to access the parsed cmdLine.
    cCmdLineIter iter(cmdLine);

    // First, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // Now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    // now do the stuff specific to this mode..

    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWCmdLine::INTER_UPDATE:
            mpData->mbUpdate = true;
            break;
        case cTWCmdLine::EDITOR:
            ASSERT(iter.NumParams() > 0);
            mpData->mEditor = iter.ParamAt(0);
            break;
        case cTWCmdLine::NO_TTY:
            mpData->mbPrintToStdout = false;
            break;
        case cTWCmdLine::MAIL_REPORT:
            mpData->mbEmail = true;
            break;
        case cTWCmdLine::IGNORE_PROP:
            ASSERT(iter.NumParams() > 0);
            mpData->mIgnoreProps = iter.ParamAt(0);
            break;
        case cTWCmdLine::ENCRYPT_REPORT:
            mpData->mbEncryptReport = true;
            break;
        case cTWCmdLine::SEVERITY_LEVEL:
            ASSERT(iter.NumParams() > 0);
            if (_istdigit(iter.ParamAt(0).c_str()[0]))
            {
                // interpret severity value
                int             iSev;
                cSeverityLimits sl;
                if (!sl.InterpretInt(iter.ParamAt(0), &iSev))
                    throw eICBadSevVal(iter.ParamAt(0));

                mpData->mSeverityLevel   = iSev;
                mpData->mbTrimBySeverity = true;
            }
            else
            {
                mpData->mSeverityName = iter.ParamAt(0);
            }
            break;
        case cTWCmdLine::REPORTLEVEL:
        {
            if (iter.ParamAt(0) == _T("0"))
                mpData->mEmailReportLevel = cTextReportViewer::SINGLE_LINE;
            else if (iter.ParamAt(0) == _T("1"))
                mpData->mEmailReportLevel = cTextReportViewer::PARSEABLE;
            else if (iter.ParamAt(0) == _T("2"))
                mpData->mEmailReportLevel = cTextReportViewer::SUMMARY_ONLY;
            else if (iter.ParamAt(0) == _T("3"))
                mpData->mEmailReportLevel = cTextReportViewer::CONCISE_REPORT;
            else if (iter.ParamAt(0) == _T("4"))
                mpData->mEmailReportLevel = cTextReportViewer::FULL_REPORT;
            else
            {
                // They specified an illegal level, error.
                TSTRING errStr = _T("Invalid Level: ");
                errStr += iter.ParamAt(0);
                throw eTWInvalidReportLevel(errStr);
            }
        }
        //done with report-level stuff.
        break;
        case cTWCmdLine::RULE_NAME:
            ASSERT(iter.NumParams() > 0);
            mpData->mRuleName = iter.ParamAt(0);
            break;
        case cTWCmdLine::GENRE_NAME:
            ASSERT(iter.NumParams() > 0);
            mpData->mGenreName = iter.ParamAt(0);
            break;
        case cTWCmdLine::HEXADECIMAL:
            cArchiveSigGen::SetHex(true);
            break;

        case cTWCmdLine::PARAMS:
        {
            // pack all of these onto the files to check list...
            mpData->mFilesToCheck.clear();
            for (int i = 0; i < iter.NumParams(); i++)
            {
                mpData->mFilesToCheck.push_back(iter.ParamAt(i));
            }
        }

        default:
            // should I do anything, or just ignore this?
            ;
        }
    }


    // if an editor is not provided in cfg or on cmdline, try to get it from environment
    if (mpData->mbUpdate && mpData->mEditor.empty())
        util_GetEditor(mpData->mEditor);

    //----------------------------------------
    // I require the following information:
    //
    // * local key file
    // * site key file
    // * policy file
    // * db file
    // * report file
    // * editor (if interactive is specified)
    //-----------------------------------------
    TEST_INIT_REQUIREMENT((!mpData->mLocalKeyFile.empty()), cTW, tw::STR_ERR_MISSING_LOCAL_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mSiteKeyFile.empty()), cTW, tw::STR_ERR_MISSING_SITE_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mPolFile.empty()), cTW, tw::STR_ERR_MISSING_POLICY);
    TEST_INIT_REQUIREMENT((!mpData->mDbFile.empty()), cTW, tw::STR_ERR_MISSING_DB);
    TEST_INIT_REQUIREMENT((!mpData->mReportFile.empty()), cTW, tw::STR_ERR_MISSING_REPORT);
    TEST_INIT_REQUIREMENT((!(mpData->mEditor.empty() && mpData->mbUpdate)), cTW, tw::STR_ERR_MISSING_EDITOR);

    ///////////////////////////////////////////
    // do some email-related verifications
    ///////////////////////////////////////////
    if (mpData->mbEmail)
    {
        // make sure that email reporting and a file list were both not specified...
        TEST_INIT_REQUIREMENT(mpData->mFilesToCheck.empty(), cTripwire, tripwire::STR_ERR_IC_EMAIL_AND_FILES);

        // make sure that we have a valid mail method
        TEST_INIT_REQUIREMENT(
            (cMailMessage::NO_METHOD != mpData->mMailMethod), cTripwire, tripwire::STR_ERR_NO_MAIL_METHOD);
        TEST_INIT_REQUIREMENT(
            (cMailMessage::INVALID_METHOD != mpData->mMailMethod), cTripwire, tripwire::STR_ERR_INVALID_MAIL_METHOD);

        // error if method is SENDMAIL and no MAILPROGRAM
        TEST_INIT_REQUIREMENT((cMailMessage::MAIL_BY_PIPE != mpData->mMailMethod || !mpData->mMailProgram.empty()),
                              cTripwire,
                              tripwire::STR_ERR_MISSING_MAILPROGRAM);
    }

    // make sure that the config file and site key file are in sync...
    //
    if (cTWUtil::VerifyCfgSiteKey(mstrConfigFile, mpData->mSiteKeyFile) == false)
        cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));

    // Set the cross file systems flag appropriately.
    cFSDataSourceIter::SetFileSystemCrossing(mpData->mbCrossFileSystems);
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeIC::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWModeIC::Execute");

    cFCODatabaseFile           dbFile;
    cFCODatabaseFile::iterator dbIter(dbFile);
    cFCOReport                 report;
    cFCOReportHeader           reportHeader;
    bool                       bEncrypted, bCheckMade = false, bEmailError = false;
    try
    {

        // don't go any further if we won't be able to write the report to disk...
        cFileUtil::TestFileWritable(mpData->mReportFile);

        // open the database; note that ReadDatabase will set bEncrypted and open the
        // key file if necessary
        cKeyFile localKeyfile;
        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);

        cTWUtil::ReadDatabase(mpData->mDbFile.c_str(), dbFile, localKeyfile.GetPublicKey(), bEncrypted);

        //
        // give a warning if the user that created the database is not the same as the user running right now...
        //
        TSTRING userName;
        if (iFSServices::GetInstance()->GetCurrentUserName(userName))
        {
            if (dbFile.GetHeader().GetCreator().compare(userName.c_str()) != 0)
            {
                // TODO -- move these strings to the string table
                TOSTRINGSTREAM str;
                str << TSS_GetString(cTripwire, tripwire::STR_ERR2_DIFFERENT_USERS1) << dbFile.GetHeader().GetCreator()
                    << TSS_GetString(cTripwire, tripwire::STR_ERR2_DIFFERENT_USERS2) << userName;
		tss_mkstr(errStr, str);
		
                cTWUtil::PrintErrorMsg(eICDifferentUsers(errStr, eError::NON_FATAL));
            }
        }

        // give a stern warning if the database is not encrypted
        if (!bEncrypted && iUserNotify::GetInstance()->GetVerboseLevel() > iUserNotify::V_SILENT)
        {
            cTWUtil::PrintErrorMsg(eTWDbNotEncrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }

        if ((!bEncrypted) && (mpData->mLocalProvided) && (!mpData->mbEncryptReport))
        {
            //Issue a warning if a passphrase has been supplied, and the database is NOT encrypted.
            cTWUtil::PrintErrorMsg(eTWPassForUnencryptedDb(_T(""), eError::NON_FATAL));
        }


        // get the password here if they are saving an encrypted report
        const cElGamalSigPrivateKey* pPrivateKey = 0;
        if (mpData->mbEncryptReport)
        {
            // if the key file isn't open (ie -- the report wasn't encrypted) then we need to
            // open it now...
            cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);

            // open the private key for "early passphrase"
            if (!mpData->mbLatePassphrase)
            {
                // OK, here is the deal: if the user specified a password on the command line
                // and we are going to do an interactive update after the IC and both the report
                // and db are encrypted, then we need to save the passphrase so it doesn't get
                // scrambled by CreatePrivateKey
                if (mpData->mLocalProvided && mpData->mbUpdate && bEncrypted)
                {
                    wc16_string passPhrase = mpData->mLocalPassphrase.c_str();
                    pPrivateKey = cTWUtil::CreatePrivateKey(localKeyfile, passPhrase.c_str(), cTWUtil::KEY_LOCAL);
                }
                else
                    pPrivateKey =
                        cTWUtil::CreatePrivateKey(localKeyfile,
                                                  mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0,
                                                  cTWUtil::KEY_LOCAL);
            }
        }

        // either integrity check using the policy file or a list of files
        if (mpData->mFilesToCheck.size() != 0)
        {
            // we are checking files...
            iUserNotify::GetInstance()->Notify(1, TSS_GetString(cTripwire, tripwire::STR_IC_FILES).c_str());
            ////
            // tell the user we are ignoring severity level and rule name, if they specified them...
            //
            if (!mpData->mRuleName.empty())
                iUserNotify::GetInstance()->Notify(
                    iUserNotify::V_NORMAL, TSS_GetString(cTripwire, tripwire::STR_IC_IGNORING_RULE_NAME).c_str());
            if (mpData->mbTrimBySeverity)
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   TSS_GetString(cTripwire, tripwire::STR_IC_IGNORING_SEV_NUM).c_str());
            if (!mpData->mSeverityName.empty())
                iUserNotify::GetInstance()->Notify(
                    iUserNotify::V_NORMAL, TSS_GetString(cTripwire, tripwire::STR_IC_IGNORING_SEV_NAME).c_str());
            if (!mpData->mGenreName.empty())
                iUserNotify::GetInstance()->Notify(
                    iUserNotify::V_NORMAL, TSS_GetString(cTripwire, tripwire::STR_IC_IGNORING_GENRE_NAME).c_str());
            //
            ////

            //**********************************
            // new, experimental stuff.. trying out new command line interface
            //
            cTWUtil::GenreObjList listOut;
            cTWUtil::ParseObjectList(listOut, mpData->mFilesToCheck);
            //
            // now, we will iterate over the list, performing an integrity check for each genre
            //
            for (cTWUtil::GenreObjList::iterator genreIter = listOut.begin(); genreIter != listOut.end(); ++genreIter)
            {
                dbIter.SeekToGenre(genreIter->first);
                if (!dbIter.Done())
                {
                    cGenreSwitcher::GetInstance()->SelectGenre((cGenre::Genre)dbIter.GetGenre());
                    TW_UNIQUE_PTR<iParserGenreUtil> pParseUtil(iTWFactory::GetInstance()->CreateParserGenreUtil());
                    //
                    // I have to turn this into a list of cFCONames
                    //
                    std::list<cFCOName> fcoNames;
                    for (cTWUtil::ObjList::iterator it = genreIter->second.begin(); it != genreIter->second.end(); ++it)
                    {
                        // if this is not an absolute path, warn and continue...
                        //
                        try
                        {
                            fcoNames.push_back(cTWUtil::ParseObjectName(*it));
                        }
                        catch (eError& e)
                        {
                            e.SetFatality(false);
                            pQueue->AddError(e);
                            report.GetErrorQueue()->AddError(e);
                        }
                    }
                    // TODO -- emit "processing XXX"

                    cIntegrityCheck ic(
                        (cGenre::Genre)genreIter->first, dbIter.GetSpecList(), dbIter.GetDb(), report, pQueue);

                    //If any sort of exception escapes the IC, make sure it goes in the report.
                    try
                    {
                        uint32_t icFlags = 0;
                        icFlags |= (mpData->mfLooseDirs ? cIntegrityCheck::FLAG_LOOSE_DIR : 0);
                        icFlags |= (mpData->mbResetAccessTime ? cIntegrityCheck::FLAG_ERASE_FOOTPRINTS_IC : 0);
                        icFlags |= (mpData->mbDirectIO ? cIntegrityCheck::FLAG_DIRECT_IO : 0);

                        ic.ExecuteOnObjectList(fcoNames, icFlags);
                    }
                    catch (eError& e)
                    {
                        if (pQueue)
                            pQueue->AddError(e);
                    }
                    catch (std::exception& e)
                    {
                        if (pQueue)
                            pQueue->AddError(eIC(e.what()));
                    }
                    catch (...)
                    {
                        if (pQueue)
                            pQueue->AddError(eIC("Unknown"));
                    }
                    // put all info into report
                    cFCOReportGenreIter rgi(report);
                    rgi.SeekToGenre((cGenre::Genre)genreIter->first);
                    rgi.GetGenreHeader().SetPropDisplayer(iTWFactory::GetInstance()->CreatePropDisplayer());
                    rgi.GetGenreHeader().GetPropDisplayer()->Merge(dbIter.GetGenreHeader().GetPropDisplayer());
                    rgi.GetGenreHeader().SetObjectsScanned(ic.ObjectsScanned());
                }
                else
                {
                    //TODO -- what is the right thing to do here?
                    //    error and continue or exit?
                    //
                    eTWDbDoesntHaveGenre e(
                        cGenreSwitcher::GetInstance()->GenreToString((cGenre::Genre)genreIter->first, true));
                    e.SetFatality(false);
                    cTWUtil::PrintErrorMsg(e);
                }
            }
            //
            //**********************************
        }
        else
        {
            // we are checking a subset of the policy file

            // parse the policy file
            cGenreSpecListVector genreSpecList;
            cTWCmdLineUtil::ParsePolicyFile(genreSpecList, mpData->mPolFile, mpData->mSiteKeyFile, pQueue);

            //
            // make sure the parsed policy is equivelant to the one in the database...
            // TODO -- should this go in the report? should I make a report or just bail? I guess
            //    that for now, I will bail. (it throws an exception on failure
            //
            cTWCmdLineUtil::VerifyPolicy(genreSpecList, dbFile);

            //
            // if they are only checking one genre, figure out which one it is ...
            //
            cGenre::Genre genreToCheck = cGenre::GENRE_INVALID;
            if (!mpData->mGenreName.empty())
            {
                genreToCheck = cGenreSwitcher::GetInstance()->StringToGenre(mpData->mGenreName.c_str());
                if (genreToCheck == cGenre::GENRE_INVALID)
                {
                    throw eICUnknownGenre(mpData->mGenreName);
                }
                if (!cGenreSwitcher::GetInstance()->IsGenreAppropriate(genreToCheck))
                {
                    throw eICInappropriateGenre(mpData->mGenreName);
                }

                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   TSS_GetString(cTripwire, tripwire::STR_CHECKING_GENRE).c_str(),
                                                   cGenreSwitcher::GetInstance()->GenreToString(genreToCheck));
            }


            // iterate over the genres...
            //
            cGenreSpecListVector::iterator genreIter;
            for (genreIter = genreSpecList.begin(); genreIter != genreSpecList.end(); ++genreIter)
            {
                // if they are only checking one genre and it is not this one, then
                // skip everything else...
                //
                if ((genreToCheck != cGenre::GENRE_INVALID) && (genreToCheck != genreIter->GetGenre()))
                    continue;

                // make this genre the current genre.
                //
                cGenreSwitcher::GetInstance()->SelectGenre(genreIter->GetGenre());
                //
                // seek the database iter to this genre...
                //
                dbIter.SeekToGenre(genreIter->GetGenre());
                if (dbIter.Done())
                {
                    // TODO -- what is the right thing to do here? throw or warn and continue?
                    //    I think that I should add it to the report's error queue and spit it to
                    //    stderr and continue
                    //
                    TCERR << _T("Genre ") << cGenreSwitcher::GetInstance()->GenreToString(genreIter->GetGenre())
                          << _T(" not in database; skipping.") << std::endl;
                    continue;
                }
                // message to the user...
                //
                iUserNotify::GetInstance()->Notify(
                    iUserNotify::V_NORMAL,
                    TSS_GetString(cTripwire, tripwire::STR_PROCESSING_GENRE).c_str(),
                    cGenreSwitcher::GetInstance()->GenreToString(cGenreSwitcher::GetInstance()->CurrentGenre(), true));

                cFCOSpecList specList = genreIter->GetSpecList();

                // trim ignored properties from the specs...
                if (!mpData->mIgnoreProps.empty())
                    cTWCmdLineUtil::TrimPropsFromSpecs(specList, mpData->mIgnoreProps);

                // trim ignored rule names from the specs...
                if (!mpData->mRuleName.empty())
                    cTWCmdLineUtil::TrimSpecsByName(specList, mpData->mRuleName);

                // trim spec list based on severity
                if (mpData->mbTrimBySeverity)
                    cTWCmdLineUtil::TrimSpecsBySeverity(specList, mpData->mSeverityLevel);

                // trim specs by severity name
                if (!mpData->mSeverityName.empty())
                    cTWCmdLineUtil::TrimSpecsBySeverityName(specList, mpData->mSeverityName);

                //
                // only do the integrity check if there is something to check!
                //
                if (!specList.IsEmpty())
                {
                    bCheckMade = true; //we've checked at least one rule.

                    // do the integrity check...
                    iUserNotify::GetInstance()->Notify(1,
                                                       TSS_GetString(cTripwire, tripwire::STR_INTEGRITY_CHECK).c_str());

#ifdef TW_PROFILE
                    cTaskTimer timer("cTripwire::IntegrityCheck");
                    timer.Start();
#endif
                    cIntegrityCheck ic((cGenre::Genre)dbIter.GetGenre(), specList, dbIter.GetDb(), report, pQueue);

                    //If any sort of exception escapes the IC, make sure it goes in the report.
                    try
                    {
                        uint32_t icFlags = 0;
                        icFlags |= (mpData->mfLooseDirs ? cIntegrityCheck::FLAG_LOOSE_DIR : 0);
                        icFlags |= (mpData->mbResetAccessTime ? cIntegrityCheck::FLAG_ERASE_FOOTPRINTS_IC : 0);
                        icFlags |= (mpData->mbDirectIO ? cIntegrityCheck::FLAG_DIRECT_IO : 0);

                        ic.Execute(icFlags);
                    }
                    catch (eError& e)
                    {
                        if (pQueue)
                            pQueue->AddError(e);
                    }
                    catch (std::exception& e)
                    {
                        if (pQueue)
                            pQueue->AddError(eIC(e.what()));
                    }
                    catch (...)
                    {
                        if (pQueue)
                            pQueue->AddError(eIC("Unknown"));
                    }

                    // put all display info into report
                    cFCOReportGenreIter rgi(report);
                    rgi.SeekToGenre((cGenre::Genre)dbIter.GetGenre());
                    rgi.GetGenreHeader().SetPropDisplayer(iTWFactory::GetInstance()->CreatePropDisplayer());
                    rgi.GetGenreHeader().GetPropDisplayer()->Merge(dbIter.GetGenreHeader().GetPropDisplayer());
                    rgi.GetGenreHeader().SetObjectsScanned(ic.ObjectsScanned());

#ifdef TW_PROFILE
                    timer.Stop();
#endif
                }
                else
                {
                    iUserNotify::GetInstance()->Notify(
                        iUserNotify::V_NORMAL,
                        TSS_GetString(cTripwire, tripwire::STR_ERR_IC_NO_SPECS_LEFT).c_str(),
                        cGenreSwitcher::GetInstance()->GenreToString(genreIter->GetGenre(), true));
                }
            }
        }

        // save out the report...
        cFCOReportUtil::CalculateHeaderInfo(reportHeader,
                                            mpData->mPolFile,
                                            mstrConfigFile,
                                            mpData->mDbFile,
                                            mstrCmdLine,
                                            cSystemInfo::GetExeStartTime(),
                                            dbFile.GetHeader().GetLastDBUpdateTime());

        cFCOReportUtil::FinalizeReport(report);

        // email the report if that is desired...
        if (mpData->mbEmail)
        {
            bEmailError = cTWCmdLineUtil::EmailReport(reportHeader, report, mpData) == 0;

            // We should notify them that no email was sent should the following condition
            // apply.
            if (!bCheckMade)
            {
                iUserNotify::GetInstance()->Notify(
                    iUserNotify::V_NORMAL, TSS_GetString(cTripwire, tripwire::STR_IC_NOEMAIL_SENT).c_str(), _T(""));
            }
        }

        // write the report to disk...
        if (mpData->mbEncryptReport)
        {
            // get the private key for "late passphrase"
            if (!pPrivateKey)
            {
                ASSERT(mpData->mbLatePassphrase);
                // OK, here is the deal: if the user specified a password on the command line
                // and we are going to do an interactive update after the IC and both the report
                // and db are encrypted, then we need to save the passphrase so it doesn't get
                // scrambled by CreatePrivateKey
                if (mpData->mLocalProvided && mpData->mbUpdate && bEncrypted)
                {
                    wc16_string passPhrase = mpData->mLocalPassphrase.c_str();
                    pPrivateKey = cTWUtil::CreatePrivateKey(localKeyfile, passPhrase.c_str(), cTWUtil::KEY_LOCAL);
                }
                else
                    pPrivateKey =
                        cTWUtil::CreatePrivateKey(localKeyfile,
                                                  mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0,
                                                  cTWUtil::KEY_LOCAL);
            }

            // backup the file we are about to overwrite
            cFileUtil::BackupFile(mpData->mReportFile);

            // write the report
            cTWUtil::WriteReport(mpData->mReportFile.c_str(), reportHeader, report, true, pPrivateKey);
            localKeyfile.ReleasePrivateKey();
        }
        else
        {
            cFileUtil::BackupFile(mpData->mReportFile); // backup the file we are about to overwrite
            cTWUtil::WriteReport(
                mpData->mReportFile.c_str(), reportHeader, report, false, NULL); // false means no encryption
        }

        // print a text version of the report to stdout...
        if (mpData->mbPrintToStdout)
        {
            // print a couple of new lines so that the report isn't butted up against the rest of the output.
            iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL, _T("\n\n"));
            cTextReportViewer trv(reportHeader, report);
            trv.PrintTextReport(_T("-"), cTextReportViewer::SUMMARY_ONLY);
        }
    }
    catch (eError& e)
    {
        if (mpData->mbLogToSyslog)
        {
            TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IC_FAIL_MSG);
            msg.append(cErrorTable::GetInstance()->Get(e.GetID()));

            cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_ERROR, msg.c_str());
        }

        cTWUtil::PrintErrorMsg(e);
        return 8;
        //TODO -- send a sad email here if mbEmail == true
    }

    // tell the user the report was saved
    iUserNotify::GetInstance()->Notify(1, TSS_GetString(cTripwire, tripwire::STR_IC_SUCCESS).c_str());

    if (mpData->mbLogToSyslog)
    {
        cTextReportViewer trv(reportHeader, report);
        TSTRING           msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IC_MSG);
        msg.append(mpData->mDbFile);
        msg.append(_T(" "));
        msg.append(trv.SingleLineReport());
        cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_SUCCESS, msg.c_str());
    }

    // go directly into interactive update if that is what they want...
    //
    if (mpData->mbUpdate)
    {
        cTWModeDbUpdate dbUpdate;
        dbUpdate.Init(mpData, &dbFile, &reportHeader, &report, bEncrypted);
        int ret = dbUpdate.Execute(pQueue);

        if (ret != 0)
            return ret;
    }

    // the return level is based on the type of violations that were found:
    // 1 = added   files
    // 2 = removed files
    // 4 = changed files
    // 8 = ERROR_IC = Execution Error
    if (bEmailError)
        return (ERROR_IC | cTWCmdLineUtil::GetICReturnValue(report));
    else
        return cTWCmdLineUtil::GetICReturnValue(report);
}

///////////////////////////////////////////////////////////////////////////////
// GetModeUsage
///////////////////////////////////////////////////////////////////////////////
TSTRING cTWModeIC::GetModeUsage(void)
{
    return (TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_CHECK));
}

//#############################################################################
// cTWModeDbUpdate
//#############################################################################
class cTWModeDbUpdate_i : public cTWModeCommon
{
public:
    bool mbInteractive; // don't do interactive update; just integrate the report file
    bool mbSecureMode;  // are we in extra-pedantic mode?
    //std::string  mSitePassphrase;  // pass phrase for site key
    //bool    mSiteProvided;

    // these members are only used when this mode is entered after integrity checking.
    // they are set to variables that are on cTWModeIC's stack, so they should _not_ be deleted.
    // if they are non-null when Execute() is called, they should be used as the db and report.
    bool              mbEncryptDb;
    cFCOReport*       mpReport;
    cFCODatabaseFile* mpDbFile;
    cFCOReportHeader* mpReportHeader;

    // ctor can set up some default values
    cTWModeDbUpdate_i()
        : cTWModeCommon(),
          mbInteractive(true),
          mbSecureMode(true),
          mbEncryptDb(true),
          /*mSiteProvided(false),*/ mpReport(0),
          mpDbFile(0),
          mpReportHeader(0)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeDbUpdate::cTWModeDbUpdate()
{
    mpData = new cTWModeDbUpdate_i;
}


cTWModeDbUpdate::~cTWModeDbUpdate()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeDbUpdate::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    cmdLine.AddArg(cTWCmdLine::MODE_UPDATE_DB, TSTRING(_T("")), TSTRING(_T("update")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::ACCEPT_ALL, TSTRING(_T("a")), TSTRING(_T("accept-all")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::SECURE_MODE, TSTRING(_T("Z")), TSTRING(_T("secure-mode")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::EDITOR, TSTRING(_T("V")), TSTRING(_T("visual")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_NONE);

    // mutual exclusion...
    cmdLine.AddMutEx(cTWCmdLine::ACCEPT_ALL, cTWCmdLine::EDITOR);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeDbUpdate::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    // now do the stuff specific to this mode...
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWCmdLine::ACCEPT_ALL:
            mpData->mbInteractive = false;
            break;
        case cTWCmdLine::SECURE_MODE:
            ASSERT(iter.NumParams() > 0);
            if (iter.ParamAt(0).compare(_T("high")) == 0)
                mpData->mbSecureMode = true;
            else if (iter.ParamAt(0).compare(_T("low")) == 0)
                mpData->mbSecureMode = false;
            else
            {
                // invalid parameter to secure mode switch...
                // TODO -- print this to stderr; how do I display (1) the switch name
                // and (2) the possible values?
                // TODO -- move {high, low} somewhere else
                TCERR << TSS_GetString(cTripwire, tripwire::STR_ERR_BAD_PARAM) << _T(" : ") << iter.ParamAt(0).c_str()
                      << _T("; must be {high, low}") << std::endl;
                return false;
            }
            break;
        case cTWCmdLine::EDITOR:
            ASSERT(iter.NumParams() > 0);
            mpData->mEditor = iter.ParamAt(0);
            break;
        default:
            // should I do anything, or just ignore this?
            ;
        }
    }

    // if an editor is not provided in cfg or on cmdline, try to get it from environment
    if (mpData->mbInteractive && mpData->mEditor.empty())
        util_GetEditor(mpData->mEditor);

    //----------------------------------------
    // I require the following information:
    //
    // * local key file
    // * site key file
    // * policy file
    // * db file
    // * report file
    // * editor (if accept all is not specified)
    //-----------------------------------------
    TEST_INIT_REQUIREMENT((!mpData->mLocalKeyFile.empty()), cTW, tw::STR_ERR_MISSING_LOCAL_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mSiteKeyFile.empty()), cTW, tw::STR_ERR_MISSING_SITE_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mPolFile.empty()), cTW, tw::STR_ERR_MISSING_POLICY);
    TEST_INIT_REQUIREMENT((!mpData->mDbFile.empty()), cTW, tw::STR_ERR_MISSING_DB);
    TEST_INIT_REQUIREMENT((!mpData->mReportFile.empty()), cTW, tw::STR_ERR_MISSING_REPORT);
    TEST_INIT_REQUIREMENT(((!mpData->mEditor.empty()) || (!mpData->mbInteractive)), cTW, tw::STR_ERR_MISSING_EDITOR);

    // make sure that the config file and site key file are in sync...
    //
    if (cTWUtil::VerifyCfgSiteKey(mstrConfigFile, mpData->mSiteKeyFile) == false)
        cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));

    // Set the cross file systems flag appropriately.
    cFSDataSourceIter::SetFileSystemCrossing(mpData->mbCrossFileSystems);
    return true;
}

void cTWModeDbUpdate::Init(
    const cTWModeIC_i* pICData, cFCODatabaseFile* dbFile, cFCOReportHeader* prh, cFCOReport* pReport, bool bEncryptDb)
{
    mpData->mbInteractive = true; // always interactive
    mpData->mbSecureMode  = pICData->mbSecureMode;
    //mpData->mbBackup      = pICData->mbBackup;
    mpData->mDbFile          = pICData->mDbFile;
    mpData->mLocalKeyFile    = pICData->mLocalKeyFile;
    mpData->mLocalPassphrase = pICData->mLocalPassphrase;
    mpData->mLocalProvided   = pICData->mLocalProvided;
    mpData->mPolFile         = pICData->mPolFile;
    mpData->mReportFile      = pICData->mReportFile;
    mpData->mSiteKeyFile     = pICData->mSiteKeyFile;
    mpData->mVerbosity       = pICData->mVerbosity;
    mpData->mEditor          = pICData->mEditor;
    mpData->mbLatePassphrase = pICData->mbLatePassphrase;


    mpData->mbEncryptDb = bEncryptDb;
    mpData->mpDbFile    = dbFile;

    if (pReport)
    {
        ASSERT(prh != 0);
        mpData->mpReport       = pReport;
        mpData->mpReportHeader = prh;
    }
    else
    {
        mpData->mpReport       = 0;
        mpData->mpReportHeader = 0;
    }
}


///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeDbUpdate::Execute(cErrorQueue* pQueue)
{
    try
    {
        // don't go any further if we won't be able to write to the db file...
        cFileUtil::TestFileWritable(mpData->mDbFile);

        // open the database
        cFCODatabaseFile dbFile;
        cKeyFile         localKeyfile;
        bool             bDbEncrypted, bReportEncrypted;

        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);

        if (!mpData->mpDbFile)
        {
            // open the database; note that ReadDatabase will set bEncrypted and open the
            // key file if necessary
            cTWUtil::ReadDatabase(mpData->mDbFile.c_str(), dbFile, localKeyfile.GetPublicKey(), bDbEncrypted);
            mpData->mpDbFile = &dbFile;
        }
        else
        {
            // we were passed the database; we should open the key file now if the db needs
            // to be saved encrypted...
            if (mpData->mbEncryptDb)
                bDbEncrypted = true;
            else
                bDbEncrypted = false;
        }

        if ((!bDbEncrypted) && (mpData->mLocalProvided))
        {
            //Issue a warning if a passphrase has been supplied, and the database is NOT encrypted.
            cTWUtil::PrintErrorMsg(eTWPassForUnencryptedDb(_T(""), eError::NON_FATAL));
        }


        // open the report...
        cFCOReport       report;
        cFCOReportHeader reportHeader;
        if (!mpData->mpReport)
        {
            cTWUtil::ReadReport(mpData->mReportFile.c_str(),
                                reportHeader,
                                report,
                                localKeyfile.GetPublicKey(),
                                false,
                                bReportEncrypted);
            mpData->mpReport       = &report;
            mpData->mpReportHeader = &reportHeader;
        }
        else
            ASSERT(mpData->mpReportHeader != 0);

        // open the private key for "early passphrase"
        // I don't think that early passphrase should apply to db update; there is no reason the user
        // would want to enter the passphrase before editing the report .. mdb
        // TODO: Well ok, can we remove this commented out code then?
        const cElGamalSigPrivateKey* pPrivateKey = 0;
        /*
      if(bDbEncrypted && (! mpData->mbLatePassphrase))
      {
         pPrivateKey = cTWUtil::CreatePrivateKey(localKeyfile, mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0);
      }
      */

        // have the user modify the report, if they want to
        if (mpData->mbInteractive)
        {
            // put a try/catch here to allow ourselves to print a message
            // saying the editor launch failed
            try
            {
                cTextReportViewer trv(*mpData->mpReportHeader, *mpData->mpReport);
                trv.DisplayReportAndHaveUserUpdateIt(mpData->mEditor, cTextReportViewer::FULL_REPORT);
            }
            catch (...)
            {
                TCERR << TSS_GetString(cTripwire, tripwire::STR_ERR_UPDATE_ED_LAUNCH) << std::endl;
                throw;
            }
        }

        // if the report is empty, there is no need to do the update...
        if (!cTWCmdLineUtil::ReportContainsFCO(*mpData->mpReport))
        {
            // tell them ...
            iUserNotify::GetInstance()->Notify(1, TSS_GetString(cTripwire, tripwire::STR_REPORT_EMPTY).c_str());
            // keep those pesky ASSERTs from cKeyFile away!
            if (pPrivateKey)
                localKeyfile.ReleasePrivateKey();
            return 0; // is this the right return val?
        }

        // give a stern warning if the database is not encrypted
        if (!bDbEncrypted && iUserNotify::GetInstance()->GetVerboseLevel() > iUserNotify::V_SILENT)
        {
            // warn if the database is not encrytped
            cTWUtil::PrintErrorMsg(eTWDbNotEncrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }

        // iterate over all the genres in the report...
        //
        cFCOReportGenreIter genreIter(*mpData->mpReport);
        for (genreIter.SeekBegin(); !genreIter.Done(); genreIter.Next())
        {
            cGenreSwitcher::GetInstance()->SelectGenre(genreIter.GetGenre());
            //
            // seek the db iter to the current genre...
            //
            cFCODatabaseFile::iterator dbIter(*mpData->mpDbFile);
            dbIter.SeekToGenre(genreIter.GetGenre());
            if (dbIter.Done())
            {
                //TODO -- what is the right thing to do here?
                //
                ASSERT(false);
                throw eTWDbDoesntHaveGenre();
            }
            //
            // actually do the integrity check...
            //
            uint32_t udFlags = 0;
            udFlags |= (mpData->mbResetAccessTime ? cUpdateDb::FLAG_ERASE_FOOTPRINTS_UD : 0);

            cUpdateDb update(dbIter.GetDb(), *mpData->mpReport, pQueue);
            if ((!update.Execute(udFlags)) && mpData->mbSecureMode)
            {
                // we will not perform the update; simply exit.
                TCOUT << TSS_GetString(cTripwire, tripwire::STR_DB_NOT_UPDATED) << std::endl;
                return 8;
            }

            //
            // update the header info for the database...
            //
            // merge the prop displayers...
            dbIter.GetGenreHeader().GetPropDisplayer()->Merge(genreIter.GetGenreHeader().GetPropDisplayer());
        }

        // Update the header
        // Note: We DO NOT call cFCODatabaseUtil::CalculateHeader() here.  We only
        // want to update these entries.
        mpData->mpDbFile->GetHeader().SetCreator(cTWUtil::GetCurrentUser());
        mpData->mpDbFile->GetHeader().SetLastDBUpdateTime(cSystemInfo::GetExeStartTime());

        //
        // write the db to disk...
        //
        if (bDbEncrypted)
        {
            // open the private key for "late prompting"
            if (!pPrivateKey)
            {
                pPrivateKey = cTWUtil::CreatePrivateKey(
                    localKeyfile, mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0, cTWUtil::KEY_LOCAL);
            }

            // backup the file we are about to overwrite
            cFileUtil::BackupFile(mpData->mDbFile);

            cTWUtil::WriteDatabase(mpData->mDbFile.c_str(), *mpData->mpDbFile, true, pPrivateKey);
            localKeyfile.ReleasePrivateKey();
        }
        else
        {
            cFileUtil::BackupFile(mpData->mDbFile); // backup the file we are about to overwrite
            cTWUtil::WriteDatabase(
                mpData->mDbFile.c_str(), *mpData->mpDbFile, false, NULL); // false means no encryption
        }
    }
    catch (eError& e)
    {
        if (mpData->mbLogToSyslog)
        {
            TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_UP_FAIL_MSG);
            msg.append(cErrorTable::GetInstance()->Get(e.GetID()));

            cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_ERROR, msg.c_str());
        }

        cTWUtil::PrintErrorMsg(e);
        return 8;
    }

    if (mpData->mbLogToSyslog)
    {
        TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_UP_MSG);
        msg.append(mpData->mDbFile);
        cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_SUCCESS, msg.c_str());
    }

    // everything went ok; return 0
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// GetModeUsage
///////////////////////////////////////////////////////////////////////////////
TSTRING cTWModeDbUpdate::GetModeUsage(void)
{
    return (TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_UPDATE));
}

//#############################################################################
// cTWModePolUpdate
//#############################################################################
class cTWModePolUpdate_i : public cTWModeCommon
{
public:
    TSTRING     mTextPolFile;
    wc16_string mSitePassphrase;
    bool        mSiteProvided;
    bool        mbSecureMode;

    // ctor can set up some default values
    cTWModePolUpdate_i() : cTWModeCommon(), mSiteProvided(false), mbSecureMode(true)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModePolUpdate::cTWModePolUpdate()
{
    mpData = new cTWModePolUpdate_i;
}

cTWModePolUpdate::~cTWModePolUpdate()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModePolUpdate::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    cmdLine.AddArg(
        cTWCmdLine::MODE_UPDATE_POL, TSTRING(_T("")), TSTRING(_T("update-policy")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::TEXT_POL_FILE, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(
        cTWCmdLine::LOCAL_PASSPHRASE, TSTRING(_T("P")), TSTRING(_T("local-passphrase")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(
        cTWCmdLine::SITE_PASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    cmdLine.AddArg(cTWCmdLine::SECURE_MODE, TSTRING(_T("Z")), TSTRING(_T("secure-mode")), cCmdLineParser::PARAM_ONE);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModePolUpdate::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    // now do the stuff specific to this mode...
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        TSTRING fullPath;
        switch (iter.ArgId())
        {
        case cTWCmdLine::TEXT_POL_FILE:
            ASSERT(iter.NumParams() > 0);
            mpData->mTextPolFile = iter.ParamAt(0);
            //
            // convert it to a full path
            //
            if (iFSServices::GetInstance()->FullPath(fullPath, mpData->mTextPolFile))
                mpData->mTextPolFile = fullPath;
            break;
        case cTWCmdLine::SITE_PASSPHRASE:
            ASSERT(iter.NumParams() > 0);
            mpData->mSitePassphrase = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mpData->mSiteProvided   = true;
            break;
        case cTWCmdLine::SECURE_MODE:
            ASSERT(iter.NumParams() > 0);
            if (iter.ParamAt(0).compare(_T("high")) == 0)
                mpData->mbSecureMode = true;
            else if (iter.ParamAt(0).compare(_T("low")) == 0)
                mpData->mbSecureMode = false;
            else
            {
                // invalid parameter to secure mode  switch...
                // TODO -- print this to stderr; how do I display (1) the switch name
                // and (2) the possible values?
                // TODO -- move {high, low} somewhere else
                TCERR << TSS_GetString(cTripwire, tripwire::STR_ERR_BAD_PARAM) << _T(" : ") << iter.ParamAt(0).c_str()
                      << _T("; must be {high, low}") << std::endl;
                return false;
            }
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
    // * site key file
    // * policy file
    // * db file
    //-----------------------------------------
    TEST_INIT_REQUIREMENT((!mpData->mLocalKeyFile.empty()), cTW, tw::STR_ERR_MISSING_LOCAL_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mSiteKeyFile.empty()), cTW, tw::STR_ERR_MISSING_SITE_KEY);
    TEST_INIT_REQUIREMENT((!mpData->mPolFile.empty()), cTW, tw::STR_ERR_MISSING_POLICY);
    TEST_INIT_REQUIREMENT((!mpData->mDbFile.empty()), cTW, tw::STR_ERR_MISSING_DB);

    // if a passphrase was passed on the command line, then late prompting should
    // be set to false so that wrong passwords are detected immediately
    //
    // Note that the local passphrase was already checked against in FillOutCmdLineInfo(),
    // so I only need to test the site passphrase
    if (mpData->mSiteProvided && mpData->mbLatePassphrase)
    {
        // TODO -- I don't know if we should display this or not -- mdb
        //iUserNotify::GetInstance()->Notify(  iUserNotify::V_VERBOSE, TSS_GetString( cTripwire, tripwire::STR_NOTIFY_CHANGE_PROMPT_TIME).c_str() );
        mpData->mbLatePassphrase = false;
    }

    // make sure that the config file and site key file are in sync...
    //
    if (cTWUtil::VerifyCfgSiteKey(mstrConfigFile, mpData->mSiteKeyFile) == false)
        cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));

    // Set the cross file systems flag appropriately.
    cFSDataSourceIter::SetFileSystemCrossing(mpData->mbCrossFileSystems);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModePolUpdate::Execute(cErrorQueue* pQueue)
{
    try
    {
        // make sure all of the files are accessable
        cFileUtil::TestFileWritable(mpData->mPolFile);
        cFileUtil::TestFileWritable(mpData->mDbFile);

        // open and parse the new policy file...
        cGenreSpecListVector genreSpecList;
        cTWCmdLineUtil::ParseTextPolicyFile(genreSpecList, mpData->mTextPolFile, mpData->mSiteKeyFile, pQueue);

        cFCODatabaseFile dbFile;
        cKeyFile         localKeyfile, siteKeyfile;
        cPrivateKeyProxy privateLocal, privateSite;
        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);
        cTWUtil::OpenKeyFile(siteKeyfile, mpData->mSiteKeyFile);

        // open the old database; note that ReadDatabase will set bEncrypted and open the
        // key file if necessary
        bool bDbEncrypted;
        bool bPolEncrypted = true;

        cTWUtil::ReadDatabase(mpData->mDbFile.c_str(), dbFile, localKeyfile.GetPublicKey(), bDbEncrypted);

        //
        // give a warning if the user that created the database is not the same as the user running right now...
        //
        TSTRING userName;
        if (iFSServices::GetInstance()->GetCurrentUserName(userName))
        {
            if (dbFile.GetHeader().GetCreator().compare(userName.c_str()) != 0)
            {
                // TODO -- move these strings to the string table
                TOSTRINGSTREAM str;
                str << TSS_GetString(cTripwire, tripwire::STR_ERR2_DIFFERENT_USERS1) << dbFile.GetHeader().GetCreator()
                    << TSS_GetString(cTripwire, tripwire::STR_ERR2_DIFFERENT_USERS2) << userName;
		tss_mkstr(errStr, str);
		
                cTWUtil::PrintErrorMsg(eICDifferentUsers(errStr, eError::NON_FATAL));
            }
        }

        // warn if the database is not encrytped
        if (!bDbEncrypted && iUserNotify::GetInstance()->GetVerboseLevel() > iUserNotify::V_SILENT)
        {
            cTWUtil::PrintErrorMsg(eTWDbNotEncrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }

        if ((!bDbEncrypted) && (mpData->mLocalProvided))
        {
            //Issue a warning if a passphrase has been supplied, and the database is NOT encrypted.
            cTWUtil::PrintErrorMsg(eTWPassForUnencryptedDb(_T(""), eError::NON_FATAL));
        }


        try
        {
            cFileManipulator polManipulator(mpData->mPolFile.c_str());
            polManipulator.Init();
            bPolEncrypted = polManipulator.GetEncoding() == cFileHeader::ASYM_ENCRYPTION;
        }
        catch (eError&)
        {
        }

        //
        // get the passwords for early prompting
        //
        if (!mpData->mbLatePassphrase)
        {
            if (bDbEncrypted)
                cTWUtil::CreatePrivateKey(privateLocal,
                                          localKeyfile,
                                          mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0,
                                          cTWUtil::KEY_LOCAL);

            if (bPolEncrypted)
                cTWUtil::CreatePrivateKey(privateSite,
                                          siteKeyfile,
                                          mpData->mSiteProvided ? mpData->mSitePassphrase.c_str() : 0,
                                          cTWUtil::KEY_SITE);
        }


        /////////////////////////////////////////////////////////
        // iterate through all the genres in the new policy...
        /////////////////////////////////////////////////////////
        cFCODatabaseFile::iterator     dbIter(dbFile);
        cGenreSpecListVector::iterator genreIter;
        for (genreIter = genreSpecList.begin(); genreIter != genreSpecList.end(); ++genreIter)
        {
            // make this genre the current genre.
            //
            cGenreSwitcher::GetInstance()->SelectGenre(genreIter->GetGenre());
            //
            // seek the database iter to this genre...
            //
            dbIter.SeekToGenre(genreIter->GetGenre());
            if (!dbIter.Done())
            {
                // perform the policy update
                //
                cPolicyUpdate pu(
                    genreIter->GetGenre(), dbIter.GetSpecList(), genreIter->GetSpecList(), dbIter.GetDb(), pQueue);
                uint32_t puFlags = 0;
                puFlags |= mpData->mbSecureMode ? cPolicyUpdate::FLAG_SECURE_MODE : 0;
                puFlags |= (mpData->mbResetAccessTime ? cPolicyUpdate::FLAG_ERASE_FOOTPRINTS_PU : 0);
                puFlags |= (mpData->mbDirectIO ? cPolicyUpdate::FLAG_DIRECT_IO : 0);

                if ((!pu.Execute(puFlags)) && (mpData->mbSecureMode))
                {
                    // they were in secure mode and errors occured; an error condition
                    TCOUT << TSS_GetString(cTripwire, tripwire::STR_ERR_POL_UPDATE) << std::endl;
                    return 8;
                }
                // we need to update the database's prop set...
                //
                dbIter.GetSpecList() = genreIter->GetSpecList();
                //
                // TODO -- do some header stuff here?
                //
            }
            else
            {
                // do a database init for this genre; it did not appear in the old policy...
                //
                TW_NOTIFY_NORMAL(TSS_GetString(cTripwire, tripwire::STR_PU_ADDING_GENRE).c_str(),
                                 cGenreSwitcher::GetInstance()->GenreToString(genreIter->GetGenre(), true));
                dbFile.AddGenre(genreIter->GetGenre(), &dbIter);

                // copy the spec list to the database...
                dbIter.GetSpecList() = genreIter->GetSpecList();

                // generate the database...
                // TODO -- turn pQueue into an error bucket

                uint32_t gdbFlags = 0;
                gdbFlags |= (mpData->mbResetAccessTime ? cGenerateDb::FLAG_ERASE_FOOTPRINTS_GD : 0);
                gdbFlags |= (mpData->mbDirectIO ? cGenerateDb::FLAG_DIRECT_IO : 0);

                cGenerateDb::Execute(
                    dbIter.GetSpecList(), dbIter.GetDb(), dbIter.GetGenreHeader().GetPropDisplayer(), pQueue, gdbFlags);

                //TODO -- what other prop displayer stuff do I have to do here?
            }
        }
        //////////////////////////////////////////////////////////////////////////////////////
        // now, we need to prune genres that are in the database but not in the new policy...
        //////////////////////////////////////////////////////////////////////////////////////
        for (dbIter.SeekBegin(); !dbIter.Done();)
        {
            bool bFoundGenre = false;
            for (genreIter = genreSpecList.begin(); genreIter != genreSpecList.end(); ++genreIter)
            {
                if (genreIter->GetGenre() == dbIter.GetGenre())
                {
                    bFoundGenre = true;
                    break;
                }
            }
            if (!bFoundGenre)
            {
                // we need to remove this genre from the db...
                //
                dbIter.Remove();
            }
            else
                dbIter.Next();
        }


        // let's go ahead and update the database and policy file...

        //
        // open the site key file and get the site passphrase
        //
        if (!privateSite.Valid() && bPolEncrypted)
            cTWUtil::CreatePrivateKey(privateSite,
                                      siteKeyfile,
                                      mpData->mSiteProvided ? mpData->mSitePassphrase.c_str() : 0,
                                      cTWUtil::KEY_SITE);
        //
        // get the passphrase for the database, if necessary (it is important that we get both
        // private keys before saving anything
        //
        if (bDbEncrypted && !privateLocal.Valid())
        {
            ASSERT(mpData->mbLatePassphrase);
            cTWUtil::CreatePrivateKey(privateLocal,
                                      localKeyfile,
                                      mpData->mLocalProvided ? mpData->mLocalPassphrase.c_str() : 0,
                                      cTWUtil::KEY_LOCAL);
        }

        // backup the file we are about to overwrite
        cFileUtil::BackupFile(mpData->mPolFile);

        //
        // write the policy file
        //
        cTWUtil::UpdatePolicyFile(mpData->mPolFile.c_str(),
                                  mpData->mTextPolFile.c_str(),
                                  bPolEncrypted,
                                  bPolEncrypted ? privateSite.GetKey() : 0);

        //
        // update the header info
        //
        // Note: We DO NOT call cFCODatabaseUtil::CalculateHeader() here.  We only
        // want to update these entries.
        dbFile.GetHeader().SetCreator(cTWUtil::GetCurrentUser());
        dbFile.GetHeader().SetPolicyFilename(mpData->mPolFile);
        dbFile.GetHeader().SetConfigFilename(mstrConfigFile);
        dbFile.GetHeader().SetLastDBUpdateTime(cSystemInfo::GetExeStartTime());

        // backup the file we are about to overwrite
        cFileUtil::BackupFile(mpData->mDbFile);

        //
        // write the db to disk...
        //
        cTWUtil::WriteDatabase(mpData->mDbFile.c_str(), dbFile, bDbEncrypted, bDbEncrypted ? privateLocal.GetKey() : 0);
    }
    catch (eError& e)
    {
        if (mpData->mbLogToSyslog)
        {
            TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_POLUP_FAIL_MSG);
            msg.append(cErrorTable::GetInstance()->Get(e.GetID()));

            cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_ERROR, msg.c_str());
        }

        cTWUtil::PrintErrorMsg(e);
        return 8;
    }

    if (mpData->mbLogToSyslog)
    {
        TSTRING msg = TSS_GetString(cTripwire, tripwire::STR_SYSLOG_POLUP_MSG);
        msg.append(mpData->mDbFile);
        cSyslog::Log(TSS_GetString(cTripwire, tripwire::STR_SYSLOG_IDENT).c_str(), cSyslog::LOG_SUCCESS, msg.c_str());
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// GetModeUsage
///////////////////////////////////////////////////////////////////////////////
TSTRING cTWModePolUpdate::GetModeUsage(void)
{
    return (TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_UPDATE_POLICY));
}

//#############################################################################
// cTWModeTest
//#############################################################################

class cTWModeTest_i : public cTWModeCommon
{
public:
    cTWModeTest_i() : cTWModeCommon()
    {
        mMode = TEST_INVALID;
    }

    enum TestMode
    {
        TEST_INVALID,
        TEST_EMAIL
    };

    TestMode mMode;
    TSTRING  mEmailAddress;
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeTest::cTWModeTest()
{
    mpData = new cTWModeTest_i;
}

cTWModeTest::~cTWModeTest()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeTest::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    // The only allowed test mode syntax so far is
    // "tripwire --test --email name@domain.com" or "tripwire -t -e name@domain.com".
    cmdLine.AddArg(cTWCmdLine::MODE_TEST, TSTRING(_T("")), TSTRING(_T("test")), cCmdLineParser::PARAM_NONE);
    cmdLine.AddArg(cTWCmdLine::TEST_EMAIL, TSTRING(_T("e")), TSTRING(_T("email")), cCmdLineParser::PARAM_ONE);

    cmdLine.AddDependency(cTWCmdLine::MODE_TEST, cTWCmdLine::TEST_EMAIL);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeTest::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWCmdLine::TEST_EMAIL:
            mpData->mMode         = cTWModeTest_i::TEST_EMAIL;
            mpData->mEmailAddress = iter.ParamAt(0);
            break;
        default:
            // should I do anything, or just ignore this?
            ;
        }
    }

    // make sure that we have a valid operation
    TEST_INIT_REQUIREMENT((mpData->mMode != cTWModeTest_i::TEST_INVALID), cTripwire, tripwire::STR_ERR_NO_TEST_MODE);

    if (mpData->mMode == cTWModeTest_i::TEST_EMAIL)
    {
        // make sure that we have a valid mail method
        TEST_INIT_REQUIREMENT(
            (cMailMessage::NO_METHOD != mpData->mMailMethod), cTripwire, tripwire::STR_ERR_NO_MAIL_METHOD);
        TEST_INIT_REQUIREMENT(
            (cMailMessage::INVALID_METHOD != mpData->mMailMethod), cTripwire, tripwire::STR_ERR_INVALID_MAIL_METHOD);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeTest::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWModeTest::Execute");

    switch (mpData->mMode)
    {
    case cTWModeTest_i::TEST_EMAIL:
    {
        cTWCmdLineUtil::SendEmailTestMessage(mpData->mEmailAddress, mpData);
    }
    default:
        // should I do anything, or just ignore this?
        ;
    }

    return 0;
}

TSTRING cTWModeTest::GetModeUsage()
{
    return (TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_TEST));
}

//#############################################################################
// cTWModeHelp
//#############################################################################

class cTWModeHelp_i : public cTWModeCommon
{
public:
    cTWModeHelp_i()
    {
    }
    ~cTWModeHelp_i()
    {
    }

    std::set<TSTRING> mModes;
    // A set of modes to output usage statements for a specific mode.
    std::set<TSTRING> mPrinted;
    // A set for making sure we don't print the same help twice.
};


///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeHelp::cTWModeHelp()
{
    mpData = new cTWModeHelp_i();
}

cTWModeHelp::~cTWModeHelp()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeHelp::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // We're only interested in one parameter, that being help.  Anything else
    // passed to this mode should be a cmdlineparser error.
    cmdLine.AddArg(cTWCmdLine::MODE, TSTRING(_T("m")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY, true);
    cmdLine.AddArg(cTWCmdLine::MODE_HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_HELP_ALL, TSTRING(_T("")), TSTRING(_T("all")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_INIT, TSTRING(_T("i")), TSTRING(_T("init")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_CHECK, TSTRING(_T("c")), TSTRING(_T("check")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_UPDATE_DB, TSTRING(_T("u")), TSTRING(_T("update")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(
        cTWCmdLine::MODE_UPDATE_POL, TSTRING(_T("p")), TSTRING(_T("update-policy")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_TEST, TSTRING(_T("t")), TSTRING(_T("test")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_EXPLORE, TSTRING(_T("")), TSTRING(_T("explore")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWCmdLine::MODE_DEBUG, TSTRING(_T("")), TSTRING(_T("verify")), cCmdLineParser::PARAM_MANY);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeHelp::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    cCmdLineIter iter(cmdLine);

    // Grab the arguments from the help parameter:
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWCmdLine::MODE_HELP:
        {
            int i;
            for (i = 0; i < iter.NumParams(); ++i)
            {
                mpData->mModes.insert(iter.ParamAt(i));
            }
        }
        break;
        case cTWCmdLine::MODE:
        {
            int i;
            for (i = 0; i < iter.NumParams(); ++i)
            {
                mpData->mModes.insert(iter.ParamAt(i));
            }
        }
        break;
        // allow users to enter the mode
        // names with "--" prepended.  We have to do this, since
        // the cmdlineparser treats them as switches.
        case cTWCmdLine::MODE_HELP_ALL:   // fall through
        case cTWCmdLine::MODE_INIT:       // fall through
        case cTWCmdLine::MODE_CHECK:      // fall through
        case cTWCmdLine::MODE_UPDATE_DB:  // fall through
        case cTWCmdLine::MODE_UPDATE_POL: // fall through
        case cTWCmdLine::MODE_TEST:       // fall through
        case cTWCmdLine::MODE_EXPLORE:    // fall through
        case cTWCmdLine::MODE_DEBUG:
        {
            int i;
            // push back the parameter that was actually passed.
            TSTRING str = iter.ActualParam();
            // Kill off the initial "--" or "-"
            str.erase(0, 1);
            if (str.length() != 1)
                str.erase(0, 1);

            mpData->mModes.insert(str);
            for (i = 0; i < iter.NumParams(); ++i)
                mpData->mModes.insert(iter.ParamAt(i));
        }
        break;
        default:
            // should I do anything, or just ignore this?
            // cmdlineparser will catch the bad args.
            ;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeHelp::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWModeHelp::Execute");

    // The iterator we will use to traverse the list of arguments:
    std::set<TSTRING>::iterator it = mpData->mModes.begin();

    // We'll want to output the version information, regardless:
    TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_VERSION) << std::endl;
    TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;

    if (it == mpData->mModes.end()) // all that was passed was --help
    {
        // Output a short usage summary for each mode.
        TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_USAGE_SUMMARY);
        //
        // That's it, return.
        return 8;
    }

    for (; it != mpData->mModes.end(); ++it)
    {
        if (_tcscmp((*it).c_str(), _T("all")) == 0)
        {
            //Since --help all was passed, emit all help messages and return.
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_INIT);
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_CHECK);
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_UPDATE);
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_UPDATE_POLICY);
            TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_TEST);
#ifdef DEBUG
            // TODO: Do we need help messages for these modes? DRA
#endif
            //We're done, return
            return 8;
        }
    }

    //We need some subset of the usage statements.  Figure out which modes have
    //been specified:
    it = mpData->mModes.begin();
    for (; it != mpData->mModes.end(); ++it)
    {
        // This is going to be ugly code...
        if (_tcscmp((*it).c_str(), _T("init")) == 0 || _tcscmp((*it).c_str(), _T("i")) == 0)
        {
            //make sure we don't print the same help twice...
            if (mpData->mPrinted.find(_T("init")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_INIT);
                mpData->mPrinted.insert(_T("init"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("check")) == 0 || _tcscmp((*it).c_str(), _T("c")) == 0)
        {
            if (mpData->mPrinted.find(_T("check")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_CHECK);
                mpData->mPrinted.insert(_T("check"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("update")) == 0 || _tcscmp((*it).c_str(), _T("u")) == 0)
        {
            if (mpData->mPrinted.find(_T("update")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_UPDATE);
                mpData->mPrinted.insert(_T("update"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("update-policy")) == 0 || _tcscmp((*it).c_str(), _T("p")) == 0)
        {
            if (mpData->mPrinted.find(_T("update-policy")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_UPDATE_POLICY);
                mpData->mPrinted.insert(_T("update-policy"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("test")) == 0 || _tcscmp((*it).c_str(), _T("t")) == 0)
        {
            if (mpData->mPrinted.find(_T("test")) == mpData->mPrinted.end())
            {
                TCOUT << TSS_GetString(cTripwire, tripwire::STR_TRIPWIRE_HELP_TEST);
                mpData->mPrinted.insert(_T("test"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("explore")) == 0)
            TCOUT << _T(""); //we currently don't have usage statements for this mode.
        else if (_tcscmp((*it).c_str(), _T("verify")) == 0)
            TCOUT << _T(""); //we currently don't have usage statements for this mode.
        else
        {
            cTWUtil::PrintErrorMsg(eTWHelpInvalidModeSwitch((*it), eError::NON_FATAL));
            TCOUT << std::endl;
            // emit error string, this mode does not exist
        }
    }

    //Everything went okay
    return 8;
}

//================================================================
// UTIL FUNCTIONS
//================================================================

bool util_GetEditor(TSTRING& strEd)
{
    // see if VISUAL environment var is set
    TCHAR* pchEd = _tgetenv(_T("VISUAL"));
    if (pchEd != NULL && pchEd[0] != _T('\0'))
    {
        strEd = pchEd;
        return true;
    }

    // see if EDITOR environment var is set
    pchEd = _tgetenv(_T("EDITOR"));
    if (pchEd != NULL && pchEd[0] != _T('\0'))
    {
        strEd = pchEd;
        return true;
    }

    return false;
}

//Encase the explore and debug modes in this ifdef, since they are for internal
//use only.
#ifdef DEBUG

//#############################################################################
// cTWModeExploreDb
//#############################################################################
#include "tw/dbexplore.h"

class cTWModeExploreDb_i : public cTWModeCommon
{
public:
    cTWModeExploreDb_i() : cTWModeCommon()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeExploreDb::cTWModeExploreDb()
{
    mpData = new cTWModeExploreDb_i;
}

cTWModeExploreDb::~cTWModeExploreDb()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeExploreDb::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    cmdLine.AddArg(cTWCmdLine::MODE_EXPLORE, TSTRING(_T("")), TSTRING(_T("explore")), cCmdLineParser::PARAM_NONE);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeExploreDb::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeExploreDb::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWModeExploreDb::Execute");

    cFCODatabaseFile           dbFile;
    cFCODatabaseFile::iterator dbIter(dbFile);
    try
    {
        cKeyFile localKeyfile;
        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);

        bool bEncrypted;
        cTWUtil::ReadDatabase(mpData->mDbFile.c_str(), dbFile, localKeyfile.GetPublicKey(), bEncrypted);
        //
        // seek to the first genre in the db...
        //
        dbIter.SeekBegin();
        if (dbIter.Done())
        {
            //TODO -- what is the right thing to do here?
            //
            ASSERT(false);
            throw eTWDbHasNoGenres();
        }
        //
        // ok, time to explore the database!
        //
        cDbExplore::Execute(dbIter);
    }
    catch (eError& e)
    {
        cTWUtil::PrintErrorMsg(e);
        return 8;
    }

    return 0;
}

//#############################################################################
// cTWModeDebugDb
//#############################################################################
#include "tw/dbdebug.h"

class cTWModeDebugDb_i : public cTWModeCommon
{
public:
    cTWModeDebugDb_i() : cTWModeCommon()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cTWModeDebugDb::cTWModeDebugDb()
{
    mpData = new cTWModeDebugDb_i;
}

cTWModeDebugDb::~cTWModeDebugDb()
{
    delete mpData;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWModeDebugDb::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // initialize the switches common to all modes...
    InitCmdLineCommon(cmdLine);

    cmdLine.AddArg(cTWCmdLine::MODE_DEBUG, TSTRING(_T("")), TSTRING(_T("verifydb")), cCmdLineParser::PARAM_NONE);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWModeDebugDb::Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
{
    // first, fill out everything with the config file info...
    FillOutConfigInfo(mpData, cf);

    // now, parse the command line...
    // this takes care of the common stuff...
    FillOutCmdLineInfo(mpData, cmdLine);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
int cTWModeDebugDb::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWModeDebugDb::Execute");

    cFCODatabaseFile           dbFile;
    cFCODatabaseFile::iterator dbIter(dbFile);
    try
    {
        cKeyFile localKeyfile;
        cTWUtil::OpenKeyFile(localKeyfile, mpData->mLocalKeyFile);

        bool bEncrypted;
        cTWUtil::ReadDatabase(mpData->mDbFile.c_str(), dbFile, localKeyfile.GetPublicKey(), bEncrypted);
        //
        // seek to the first genre in the db...
        //
        dbIter.SeekBegin();
        if (dbIter.Done())
        {
            //TODO -- what is the right thing to do here?
            //
            ASSERT(false);
            throw eTWDbHasNoGenres();
        }
        //
        // debug the database.
        //
        cDbDebug::Execute(dbIter, mpData->mDbFile);
    }
    catch (eError& e)
    {
        cTWUtil::PrintErrorMsg(e);
        return 8;
    }

    return 0;
}


#endif //DEBUG
