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
// twadmincl.h -- command line parsing for twadmin
//

#ifndef __TWADMINCL_H
#define __TWADMINCL_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#include "core/error.h"

TSS_EXCEPTION(eTWA, eError)
TSS_EXCEPTION(eTWASkippingDirectory, eTWA)
TSS_EXCEPTION(eTWAFileNotFound, eTWA)
TSS_EXCEPTION(eTWAFileAccess, eTWA)
TSS_EXCEPTION(eTWAFileTypeUnknown, eTWA)
TSS_EXCEPTION(eTWAInvalidHelpMode, eTWA)
TSS_EXCEPTION(eTWADecrypt, eTWA)
TSS_EXCEPTION(eTWADecryptCorrupt, eTWA)

TSS_BEGIN_EXCEPTION_NO_CTOR(eTWAEncryptionChange, eTWA)
eTWAEncryptionChange(const TSTRING& strMsg1, const TSTRING& strMsg2 = _T(""), uint32_t flags = 0)
    : eTWA(strMsg1 + strMsg2, flags)
{
}
TSS_END_EXCEPTION();

TSS_BEGIN_EXCEPTION_NO_CTOR(eTWACreateCfgMissingSitekey, eTWA)
eTWACreateCfgMissingSitekey(const TSTRING& msg, uint32_t flags = 0);
TSS_END_EXCEPTION();

TSS_BEGIN_EXCEPTION_NO_CTOR(eTWACreateCfgSitekeyMismatch, eTWA)
eTWACreateCfgSitekeyMismatch(const TSTRING& specifiedKeyfile, const TSTRING& configKeyfile, uint32_t flags = 0);
TSS_END_EXCEPTION();


///////////////////////////////////////////////////////////////////////////////
// cTWAdminCmdLine -- class with a bunch of static member functions helpful in parsing
//      the tripwire command line
///////////////////////////////////////////////////////////////////////////////

class iTWAMode;

class cTWAdminCmdLine
{
public:
    //static void InitCmdLineParser(cCmdLineParser& parser);
    // call this to initialize the command line parser

    static iTWAMode* GetMode(int argc, const TCHAR* const* argv);
    // this will look at the beginning of the command line, determine the appropriate
    // mode to create, and returns it. If the mode can't be determined, an error message
    // is printed and null is returned.

    // constants for all the command line arguments
    enum CmdLineArgs
    {
        HELP,

        // modes
        MODE, // generic -m
        MODE_CREATE_CONFIG,
        MODE_PRINT_CONFIG,
        MODE_CREATE_POLICY,
        MODE_PRINT_POLICY,
        MODE_REMOVE_ENCRYPTION,
        MODE_ENCRYPT,
        MODE_EXAMINE,
        MODE_GENERATE_KEYS,
        MODE_CHANGE_PASSPHRASES,
        MODE_HELP,
        MODE_HELP_ALL,
        MODE_VERSION,

        VERBOSE,
        SILENT,
        NO_BACKUP,
        NO_ENCRYPTION,

        SITE_KEY_FILE,
        LOCAL_KEY_FILE,
        KEY_FILE,
        POL_FILE,
        OUTPUT_FILE,
        PASSPHRASE,
        CFG_FILE,
        SITEPASSPHRASE,
        LOCALPASSPHRASE,
        SITEPASSPHRASEOLD,
        LOCALPASSPHRASEOLD,
        KEY_SIZE,

        PARAMS, // the final params

        NUM_CMDLINEARGS
    };
};

///////////////////////////////////////////////////////////////////////////////
// iTWAMode -- base class that contains an interface for initializing tripwire
//      in a mode of operation and executing in that mode; a class is derived
//      from this for each mode of operation
///////////////////////////////////////////////////////////////////////////////

class cCmdLineParser;
class cConfigFile;
class cErrorQueue;

class iTWAMode
{
public:
    virtual void InitCmdLineParser(cCmdLineParser& cmdLine) = 0;
    // fills out the command line parser with all the switches that are appropriate for
    // the given mode.
    virtual bool Init(const cConfigFile* cf, const cCmdLineParser& cmdLine) = 0;
    // call this to ask the mode object to initialize all of its data structures
    // with the passed in information. If it returns false, it can be assumed that an error
    // message was spit to stderr.
    virtual int Execute(cErrorQueue* pQueue) = 0;
    // go to work! Returns error level to exit with; 0 implies success
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const = 0;
    // return MODE_* enumeration coresponding to the current mode.

    virtual TSTRING GetModeUsage(void) = 0;
    // return a mode-specific usage string ( a subset of the entire usage statement for
    // twadmin

    void SetCfgFilePath(const TSTRING& path)
    {
        mCfgFilePath = path;
    }
    // if the config file is opened in main(), this should be called so the modes know what the path
    // to the config file is.
    virtual ~iTWAMode(){};

protected:
    TSTRING mCfgFilePath;
};

#endif
