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
// twcmdline.h
//
// cTWCmdLine  -- holds all information on tripwire's command line
// iTWMode     -- abstract base class for a mode of operation in tripwire
#ifndef __TWCMDLINE_H
#define __TWCMDLINE_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __WCHAR16_H
#include "core/wchar16.h"
#endif

#ifndef __MAILMESSAGE_H
#include "mailmessage.h" // used for email reporting
#endif

#ifndef __TEXTREPORTVIEWER_H // for multiple levels of reporting
#include "tw/textreportviewer.h"
#endif

#ifndef __ERROR_H
#include "core/error.h"
#endif

class cConfigFile;
class cCmdLineParser;
class cErrorQueue;
class cErrorBucket;
class iTWMode;
class cFCODatabaseFile;
class cFCOReport;
class cFCODbHeader;
class cFCOReportHeader;

TSS_EXCEPTION(eTWHelpInvalidModeSwitch, eError);
TSS_EXCEPTION(eTWInitialization, eError);
TSS_EXCEPTION(eTWInvalidConfigFileKey, eError);
TSS_EXCEPTION(eTWInvalidReportLevel, eError);
TSS_EXCEPTION(eTWInvalidReportLevelCfg, eError);
TSS_EXCEPTION(eTWInvalidPortNumber, eError);
TSS_EXCEPTION(eTWPassForUnencryptedDb, eError);
TSS_EXCEPTION(eTWInvalidTempDirectory, eError);

TSS_EXCEPTION(eTWSyslogNotSupported, eError);
TSS_EXCEPTION(eTWDirectIONotSupported, eError);

///////////////////////////////////////////////////////////////////////////////
// cTWCmdLine -- class with a bunch of static member functions helpful in parsing
//    the tripwire command line
///////////////////////////////////////////////////////////////////////////////
class cTWCmdLine
{
public:
    //static void InitCmdLineParser(cCmdLineParser& parser);
    // call this to initialize the command line parser

    static iTWMode* GetMode(int argc, const TCHAR* const* argv);
    // this will look at the beginning of the command line, determine the appropriate
    // mode to create, and returns it. If the mode can't be determined, an error message
    // is printed and null is returned.

    // constants for all the command line arguments
    enum CmdLineArgs
    {
        HELP,

        // modes
        MODE, // generic -m
        MODE_INIT,
        MODE_CHECK,
        MODE_UPDATE_DB,
        MODE_UPDATE_POL,
        MODE_TEST,
        MODE_EXPLORE,
        MODE_DEBUG,
        MODE_HELP,
        MODE_HELP_ALL,
        MODE_VERSION,

        VERBOSE,
        SILENT,
        POL_FILE,
        CFG_FILE,
        SITE_KEY_FILE,
        LOCAL_KEY_FILE,
        DB_FILE,
        REPORT_FILE,
        LOOSE_DIRS,
        NO_BACKUP,
        NO_ENCRYPT,
        INTER_UPDATE, // interactive update after integrity check
        EDITOR,
        NO_TTY,
        MAIL_REPORT,
        IGNORE_PROP,
        ENCRYPT_REPORT,
        SEVERITY_LEVEL,
        //SEVERITY_NAME,
        RULE_NAME,
        GENRE_NAME,
        ACCEPT_ALL, // update db with entire report
        SECURE_MODE,
        TEXT_POL_FILE,
        LOCAL_PASSPHRASE,
        SITE_PASSPHRASE,
        TEST_EMAIL,
        REPORTLEVEL,
        HEXADECIMAL,
        PARAMS, // the final parameters

        NUM_CMDLINEARGS
    };
};

//#############################################################################
// cTWModeCommon -- contains data common to all modes; all cTWMode*_i will
//    derive from this class.
//#############################################################################
class cTWModeCommon
{
public:
    int         mVerbosity;       // must be 0 <= n <= 2
    wc16_string mLocalPassphrase; // pass phrase for private key
    bool        mLocalProvided;
    TSTRING     mPolFile;
    TSTRING     mSiteKeyFile;
    TSTRING     mLocalKeyFile;
    TSTRING     mDbFile;
    TSTRING     mReportFile;
    TSTRING     mEditor;
    TSTRING     mTempDirectory; // where we create our temp files...
    TSTRING     mGlobalEmail;   // a list of email addr's

    bool mbLatePassphrase;   // do we ask for the passphrase at the end of the operation or the beginning?
    bool mfLooseDirs;        // do we do loose directory checking?
    bool mbResetAccessTime;  // do we reset access time when calculating properties of files?
    bool mbLogToSyslog;      // log significant events and level 0 reports to SYSLOG
    bool mbCrossFileSystems; // automatically recurse across mount points on Unis FS genre
    bool mbDirectIO;         // Use direct i/o when scanning files, if platform supports it.

    cTextReportViewer::ReportingLevel mEmailReportLevel; // What level of email reporting we should use
    cMailMessage::MailMethod          mMailMethod;       // What mechanism should we use to send the report

    TSTRING        mSmtpHost;         // name of SMTP host to use
    unsigned short mSmtpPort;         // port number of that SMTP host
    TSTRING        mMailProgram;      // full path to executable to use for piped mail
    bool           mMailNoViolations; // Email out reports with no violations?
    TSTRING        mMailFrom;

    cTWModeCommon()
        : mVerbosity(1),
          mLocalProvided(false),
          mbLatePassphrase(false),
          mfLooseDirs(false),
          mbResetAccessTime(false),
          mbLogToSyslog(false),
          mbCrossFileSystems(false),
          mbDirectIO(false),
          mMailMethod(cMailMessage::NO_METHOD),
          mSmtpPort(25),
          mMailNoViolations(true)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// iTWMode -- base class that contains an interface for initializing tripwire
//    in a mode of operation and executing in that mode; a class is derived
//    from this for each mode of operation
///////////////////////////////////////////////////////////////////////////////
class iTWMode
{
public:
    virtual void InitCmdLineParser(cCmdLineParser& cmdLine) = 0;
    // fills out the command line parser with all the switches that are appropriate for
    // the given mode.
    virtual bool Init(const cConfigFile& cf, const cCmdLineParser& cmdLine) = 0;
    // call this to ask the mode object to initialize all of its data structures
    // with the passed in information. If it returns false, it can be assumed that an error
    // message was spit to stderr.
    virtual int Execute(cErrorQueue* pQueue) = 0;
    // go to work! TODO -- I don't know how errors (or return values) should be conveyed
    // I think that maybe this guy should return an int indicating the return value
    virtual TSTRING GetModeUsage(void) = 0;
    // Return a usage string particular to the concrete mode.
    virtual cTWCmdLine::CmdLineArgs GetModeID() = 0;
    // return the MODE_* enumeration for a given mode.
    void SetCmdLine(const TSTRING& str)
    {
        mstrCmdLine = str;
    };
    void SetConfigFile(const TSTRING& str)
    {
        mstrConfigFile = str;
    };

    virtual ~iTWMode(){};

protected:
    TSTRING mstrCmdLine;
    TSTRING mstrConfigFile;
};

///////////////////////////////////////////////////////////////////////////////
// Database Initialization
///////////////////////////////////////////////////////////////////////////////
class cTWModeDbInit_i;
class cTWModeDbInit : public iTWMode
{
public:
    cTWModeDbInit();
    virtual ~cTWModeDbInit();

    virtual void                    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool                    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int                     Execute(cErrorQueue* pQueue);
    virtual TSTRING                 GetModeUsage(void);
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_INIT;
    }

private:
    cTWModeDbInit_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Integrity Check
///////////////////////////////////////////////////////////////////////////////
class cTWModeIC_i;
class cTWModeIC : public iTWMode
{
public:
    enum
    {
        ERROR_IC = 8
    };

    cTWModeIC();
    virtual ~cTWModeIC();

    virtual void                    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool                    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int                     Execute(cErrorQueue* pQueue);
    virtual TSTRING                 GetModeUsage();
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_CHECK;
    }

private:
    cTWModeIC_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Database Update
///////////////////////////////////////////////////////////////////////////////
class cTWModeDbUpdate_i;
class cTWModeDbUpdate : public iTWMode
{
public:
    cTWModeDbUpdate();
    virtual ~cTWModeDbUpdate();

    virtual void InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    void         Init(const cTWModeIC_i* pICData,
                      cFCODatabaseFile*  dbFile,
                      cFCOReportHeader*  prh,
                      cFCOReport*        pReport,
                      bool               bEncryptDb);
    // this Init is used when you want to automatically launch into db update
    // mode from integrity checking mode. It is called by cTWModeIC after it
    // is totally done with the integrity check.
    // The database and report are passed in becuase they are already loaded
    // into memory when IC is done, so it doesn't make sense for this class to
    // load them again. If bEncrypt is true, then the database should be encrypted when
    // saved.

    virtual int                     Execute(cErrorQueue* pQueue);
    virtual TSTRING                 GetModeUsage();
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_UPDATE_DB;
    }

private:
    cTWModeDbUpdate_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Policy Update
///////////////////////////////////////////////////////////////////////////////
class cTWModePolUpdate_i;
class cTWModePolUpdate : public iTWMode
{
public:
    cTWModePolUpdate();
    virtual ~cTWModePolUpdate();

    virtual void                    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool                    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int                     Execute(cErrorQueue* pQueue);
    virtual TSTRING                 GetModeUsage();
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_UPDATE_POL;
    }

private:
    cTWModePolUpdate_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Tripwire test mode
///////////////////////////////////////////////////////////////////////////////
class cTWModeTest_i;
class cTWModeTest : public iTWMode
{
public:
    cTWModeTest();
    virtual ~cTWModeTest();

    virtual void                    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool                    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int                     Execute(cErrorQueue* pQueue);
    virtual TSTRING                 GetModeUsage();
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_TEST;
    }

private:
    cTWModeTest_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Tripwire help mode : Used to issue usage statements for a variable number
//    of specific modes.
///////////////////////////////////////////////////////////////////////////////
class cTWModeHelp_i;
class cTWModeHelp : public iTWMode
{
public:
    cTWModeHelp();
    virtual ~cTWModeHelp();

    virtual void    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return _T("");
    }
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_HELP;
    }

private:
    cTWModeHelp_i* mpData;
};

//These two modes are invisible if DEBUG is not defined.
#ifdef DEBUG

///////////////////////////////////////////////////////////////////////////////
// Explore Db
///////////////////////////////////////////////////////////////////////////////
class cTWModeExploreDb_i;
class cTWModeExploreDb : public iTWMode
{
public:
    cTWModeExploreDb();
    virtual ~cTWModeExploreDb();

    virtual void    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return _T("");
    }
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_EXPLORE;
    }

private:
    cTWModeExploreDb_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// DebugDb
///////////////////////////////////////////////////////////////////////////////
class cTWModeDebugDb_i;
class cTWModeDebugDb : public iTWMode
{
public:
    cTWModeDebugDb();
    virtual ~cTWModeDebugDb();

    virtual void    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return _T("");
    }
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_DEBUG;
    }

private:
    cTWModeDebugDb_i* mpData;
};

#endif //DEBUG

///////////////////////////////////////////////////////////////////////////////
// Version Mode
///////////////////////////////////////////////////////////////////////////////
class cTWModeVersion : public iTWMode
{
public:
    cTWModeVersion()
    {
    }
    virtual ~cTWModeVersion()
    {
    }

    virtual void InitCmdLineParser(cCmdLineParser& cmdLine)
    {
        ASSERT(false);
    }
    virtual bool Init(const cConfigFile& cf, const cCmdLineParser& cmdLine)
    {
        ASSERT(false);
        return true;
    }
    virtual int Execute(cErrorQueue* pQueue)
    {
        ASSERT(false);
        return 0;
    }
    virtual TSTRING GetModeUsage()
    {
        ASSERT(false);
        return _T("");
    }
    virtual cTWCmdLine::CmdLineArgs GetModeID()
    {
        return cTWCmdLine::MODE_VERSION;
    }

private:
};

#endif
