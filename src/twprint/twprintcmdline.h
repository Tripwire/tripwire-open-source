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
// twprintcmdline.h
//
// cTWPrintCmdLine  -- holds all information on twprint's command line
// iTWMode      -- abstract base class for a mode of operation in tripwire
#ifndef __TWPRINTCMDLINE_H
#define __TWPRINTCMDLINE_H

#ifndef __TCHAR_H
#include "tchar.h"
#endif

class cConfigFile;
class cCmdLineParser;
class cErrorQueue;
class cErrorBucket;
class iTWMode;

TSS_EXCEPTION(eTWPrintInvalidParamHelp, eError)
TSS_EXCEPTION(eTWPrintInvalidReportLevel, eError)
TSS_EXCEPTION(eTWPrintInvalidReportLevelCfg, eError)
TSS_EXCEPTION(eTWPrintInvalidDbPrintLevel, eError)
TSS_EXCEPTION(eTWPrintInvalidDbPrintLevelCfg, eError)

// Help is requested for a non-existent mode.

///////////////////////////////////////////////////////////////////////////////
// cTWPrintCmdLine -- class with a bunch of static member functions helpful in parsing
//      the tripwire command line
///////////////////////////////////////////////////////////////////////////////
class cTWPrintCmdLine
{
public:
    static iTWMode* GetMode(int argc, const TCHAR* const* argv);
    // after the command line has been read, call this to extract the mode information
    // from the command line, create an iTWMode-derived class, and return it. If this
    // fails, it will print the error message to stderr and return NULL.

    // constants for all the command line arguments
    enum CmdLineArgs
    {
        HELP,

        // modes
        MODE, // generic -m
        MODE_DBPRINT,
        MODE_REPORTPRINT,
        MODE_HELP,
        MODE_HELP_ALL,

        VERBOSE,
        SILENT,

        CFG_FILE,
        LOCAL_KEY_FILE,
        SITE_KEY_FILE,

        DB_FILE,
        REPORT_FILE,

        PASSPHRASE,
        REPORTLEVEL,
        HEXADECIMAL,

        PARAMS, // the final parameters

        NUM_CMDLINEARGS
    };
};

///////////////////////////////////////////////////////////////////////////////
// iTWMode -- base class that contains an interface for initializing tripwire
//      in a mode of operation and executing in that mode; a class is derived
//      from this for each mode of operation
///////////////////////////////////////////////////////////////////////////////
class iTWMode
{
public:
    virtual void InitCmdLineParser(cCmdLineParser& parser) = 0;
    // call this to initialize the command line parser. All modes must override this
    // method.  A static function will be made to initialize the cmdline with parameters
    // common to all modes.
    virtual bool Init(const cConfigFile& cf, const cCmdLineParser& cmdLine) = 0;
    // call this to ask the mode object to initialize all of its data structures
    // with the passed in information. If it returns false, it can be assumed that an error
    // message was spit to stderr.
    virtual int Execute(cErrorQueue* pQueue) = 0;
    // go to work! TODO -- I don't know how errors (or return values) should be conveyed
    // I think that maybe this guy should return an int indicating the return value
    virtual TSTRING GetModeUsage(void) = 0;
    // Returns a TSTRING that describes specific usage for a derived mode.
    virtual cTWPrintCmdLine::CmdLineArgs GetModeID()                           = 0;
    virtual void                         SetConfigFile(TSTRING configFilePath) = 0;

    virtual ~iTWMode()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// Report Printing
///////////////////////////////////////////////////////////////////////////////
class cTWPrintReportMode_i;
class cTWPrintReportMode : public iTWMode
{
public:
    cTWPrintReportMode();
    virtual ~cTWPrintReportMode();


    void FillOutReportModeConfigInfo(cTWPrintReportMode_i* pModeInfo, const cConfigFile& cf);

    virtual void                         InitCmdLineParser(cCmdLineParser& parser);
    virtual bool                         Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int                          Execute(cErrorQueue* pQueue);
    virtual TSTRING                      GetModeUsage();
    virtual cTWPrintCmdLine::CmdLineArgs GetModeID()
    {
        return cTWPrintCmdLine::MODE_DBPRINT;
    }
    virtual void SetConfigFile(TSTRING configFilePath);

private:
    cTWPrintReportMode_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Database Printing
///////////////////////////////////////////////////////////////////////////////
class cTWPrintDBMode_i;
class cTWPrintDBMode : public iTWMode
{
public:
    cTWPrintDBMode();
    virtual ~cTWPrintDBMode();

    void FillOutDBModeConfigInfo(cTWPrintDBMode_i* pModeInfo, const cConfigFile& cf);

    virtual void                         InitCmdLineParser(cCmdLineParser& parser);
    virtual bool                         Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int                          Execute(cErrorQueue* pQueue);
    virtual TSTRING                      GetModeUsage();
    virtual cTWPrintCmdLine::CmdLineArgs GetModeID()
    {
        return cTWPrintCmdLine::MODE_REPORTPRINT;
    }
    virtual void SetConfigFile(TSTRING configFilePath);

private:
    cTWPrintDBMode_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// Help Mode - a means for emitting mode-specific help.
///////////////////////////////////////////////////////////////////////////////
class cTWPrintHelpMode_i;
class cTWPrintHelpMode : public iTWMode
{
public:
    cTWPrintHelpMode();
    virtual ~cTWPrintHelpMode();

    virtual void    InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool    Init(const cConfigFile& cf, const cCmdLineParser& cmdLine);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return _T("");
    }
    virtual cTWPrintCmdLine::CmdLineArgs GetModeID()
    {
        return cTWPrintCmdLine::MODE_HELP;
    }
    virtual void SetConfigFile(TSTRING configFilePath);

private:
    cTWPrintHelpMode_i* mpData;
};


#endif // TWPRINTCMDLINE
