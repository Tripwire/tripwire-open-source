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
// twadmincl.cpp -- command line parsing for twadmin
//

#include "stdtwadmin.h"
#include "twadmincl.h"
#include "twadminstrings.h"
#include "keygeneration.h"
#include "core/errorgeneral.h"
#include "core/errorbucketimpl.h" // needed for the error table
#include "core/archive.h"
#include "core/usernotify.h"
#include "core/cmdlineparser.h"
#include "core/usernotify.h"
#include "core/corestrings.h"
#include "fco/fconame.h"
#include "tw/configfile.h"
#include "tw/twutil.h"
#include "tw/filemanipulator.h"
#include "tw/fcodatabasefile.h"
#include "tw/fcoreport.h"
#include "tw/policyfile.h"
#include "tw/systeminfo.h"
#include "tw/twerrors.h"
#include "tw/twstrings.h"
#include "twparser/policyparser.h"
#include "twcrypto/keyfile.h"
#include "core/stringutil.h"
#include "util/fileutil.h"
#include "twcrypto/crypto.h"
#include "core/displayencoder.h"

#if HAVE_SWAB && (__cplusplus <= 1)
  #ifndef __USE_XOPEN
    #define _USE_XOPEN 1
  #endif
#endif

#include <unistd.h>

//Provide a swab() impl. from glibc, for platforms that don't have one
#if !HAVE_SWAB || NEEDS_SWAB_IMPL
void swab(const void* bfrom, void* bto, ssize_t n)
{
    const char* from = (const char*)bfrom;
    char*       to   = (char*)bto;

    n &= ~((ssize_t)1);
    while (n > 1)
    {
        const char b0 = from[--n], b1 = from[--n];
        to[n]     = b0;
        to[n + 1] = b1;
    }
}
#endif

// forwards
static bool NotifyFileType(const cFileHeaderID& id, uint32_t version, iUserNotify::VerboseLevel vl);
// Calls UserNotify(V_VERBOSE, ...) to print out type of file specified in cFileHeaderID.
// Returns false if cFileHeaderID not recognized.
// Used in changing and removing encryption algorithms
static bool NotifyEncryptionType(cFileHeader::Encoding encoding, iUserNotify::VerboseLevel vl);
// Calls UserNotify(V_VERBOSE, ...) to print out type of file specified in cFileHeaderID.
// Returns false if encoding not recognized.
// Used in changing and removing encryption algorithms


// error implementations

eTWACreateCfgMissingSitekey::eTWACreateCfgMissingSitekey(const TSTRING& msg, uint32_t flags) : eTWA(TSTRING(), flags)
{
    mMsg = TSS_GetString(cTWAdmin, twadmin::STR_ERR2_CREATE_CFG_MISSING_KEYFILE) + msg;
}

eTWACreateCfgSitekeyMismatch::eTWACreateCfgSitekeyMismatch(const TSTRING& specifiedKeyfile,
                                                           const TSTRING& configKeyfile,
                                                           uint32_t       flags)
    : eTWA(TSTRING(), flags)
{
    mMsg = TSS_GetString(cTWAdmin, twadmin::STR_ERR2_CREATE_CFG_SITEKEY_MISMATCH1);
    mMsg.append(specifiedKeyfile);
    mMsg.append(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_CREATE_CFG_SITEKEY_MISMATCH2));
    mMsg.append(configKeyfile);
    mMsg.append(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_CREATE_CFG_SITEKEY_MISMATCH3));
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeCommon -- common modes that all iTWAMode implementations will
//      derive from.

class cTWAModeCommon : public iTWAMode
{
protected:
    int     mVerbosity; // must be 0 <= n <= 2
    TSTRING mPolFile;
    TSTRING mSiteKeyFile;
    bool    mSiteKeyFileProvieded;
    TSTRING mLocalKeyFile;
    bool    mLocalKeyFileProvieded;
    bool    mLatePassphrase;

    cTWAModeCommon()
        : mVerbosity(1), mSiteKeyFileProvieded(false), mLocalKeyFileProvieded(false), mLatePassphrase(false)
    {
    }

    void InitCmdLineCommon(cCmdLineParser& parser);
    void FillOutConfigInfo(const cConfigFile* cf);
    void FillOutCmdLineInfo(const cCmdLineParser& parser);
};

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineCommon -- initializes the command line switches common to all
//      modes

void cTWAModeCommon::InitCmdLineCommon(cCmdLineParser& parser)
{
    parser.AddArg(cTWAdminCmdLine::HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWAdminCmdLine::MODE, TSTRING(_T("m")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::VERBOSE, TSTRING(_T("v")), TSTRING(_T("verbose")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWAdminCmdLine::SILENT, TSTRING(_T("s")), TSTRING(_T("silent")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWAdminCmdLine::SILENT, TSTRING(_T("")), TSTRING(_T("quiet")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWAdminCmdLine::CFG_FILE, TSTRING(_T("c")), TSTRING(_T("cfgfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::POL_FILE, TSTRING(_T("p")), TSTRING(_T("polfile")), cCmdLineParser::PARAM_ONE);
    parser.AddMutEx(cTWAdminCmdLine::VERBOSE, cTWAdminCmdLine::SILENT);
}

///////////////////////////////////////////////////////////////////////////////
// FillOutConfigInfo -- fills out all the common info with config file information
///////////////////////////////////////////////////////////////////////////////
void cTWAModeCommon::FillOutConfigInfo(const cConfigFile* cf)
{
    if (cf == 0)
        return;

    TSTRING str;
    if (cf->Lookup(TSTRING(_T("POLFILE")), str))
        mPolFile = str;
    if (cf->Lookup(TSTRING(_T("SITEKEYFILE")), str))
        mSiteKeyFile = str;
    if (cf->Lookup(TSTRING(_T("LOCALKEYFILE")), str))
        mLocalKeyFile = str;
    if (cf->Lookup(TSTRING(_T("LATEPROMPTING")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            mLatePassphrase = true;
    }

    //
    // turn all of the file names into full paths (they're relative to the exe dir)
    //
    TSTRING fullPath;
    if (!mPolFile.empty() && iFSServices::GetInstance()->FullPath(fullPath, mPolFile, cSystemInfo::GetExeDir()))
        mPolFile = fullPath;
    if (!mSiteKeyFile.empty() && iFSServices::GetInstance()->FullPath(fullPath, mSiteKeyFile, cSystemInfo::GetExeDir()))
        mSiteKeyFile = fullPath;
    if (!mLocalKeyFile.empty() &&
        iFSServices::GetInstance()->FullPath(fullPath, mLocalKeyFile, cSystemInfo::GetExeDir()))
        mLocalKeyFile = fullPath;
}

///////////////////////////////////////////////////////////////////////////////
// FillOutCmdLineInfo -- fills out info common to all modes that appears on the
//      command line.
///////////////////////////////////////////////////////////////////////////////
void cTWAModeCommon::FillOutCmdLineInfo(const cCmdLineParser& parser)
{
    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::VERBOSE:
            mVerbosity = 2;
            break;
        case cTWAdminCmdLine::SILENT:
            mVerbosity = 0;
            break;
        case cTWAdminCmdLine::POL_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mPolFile = iter.ParamAt(0);
            break;
        case cTWAdminCmdLine::SITE_KEY_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mSiteKeyFile          = iter.ParamAt(0);
            mSiteKeyFileProvieded = true;
            break;
        case cTWAdminCmdLine::LOCAL_KEY_FILE:
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mLocalKeyFile          = iter.ParamAt(0);
            mLocalKeyFileProvieded = true;
            break;
        }
    }

    // Turn all of the file names into full paths for nice presentation to the user.
    // Note: We depend later on the if one of these files is not specified, the string
    // is empty.  Currently FullPath() expands "" to the CWD.
    TSTRING fullPath;
    if (!mPolFile.empty() && iFSServices::GetInstance()->FullPath(fullPath, mPolFile))
        mPolFile = fullPath;
    if (!mSiteKeyFile.empty() && iFSServices::GetInstance()->FullPath(fullPath, mSiteKeyFile))
        mSiteKeyFile = fullPath;
    if (!mLocalKeyFile.empty() && iFSServices::GetInstance()->FullPath(fullPath, mLocalKeyFile))
        mLocalKeyFile = fullPath;

    // use the verbosity information
    ASSERT((mVerbosity >= 0) && (mVerbosity < 3));
    iUserNotify::GetInstance()->SetVerboseLevel(mVerbosity);
}

///////////////////////////////////////////////////////////////////////////////
// TWAdmin modes
//
// No one else should need to see these classes, so we will keep them
// localized to this file.
//

///////////////////////////////////////////////////////////////////////////////
// cTWAModeCreateCfg

class cTWAModeCreateCfg : public cTWAModeCommon
{
public:
    cTWAModeCreateCfg();
    virtual ~cTWAModeCreateCfg();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CREATE_CFGFILE);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_CREATE_CONFIG;
    }

private:
    TSTRING     mPlaintextConfig;
    TSTRING     mEncryptedConfig;
    wc16_string mPassPhrase; // pass phrase for private key
    bool        mPassPhraseProvided;
    bool        mNoEncryption;
};

cTWAModeCreateCfg::cTWAModeCreateCfg()
{
    mNoEncryption       = false;
    mPassPhraseProvided = false;
}

cTWAModeCreateCfg::~cTWAModeCreateCfg()
{
}

void cTWAModeCreateCfg::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWAdminCmdLine::MODE_CREATE_CONFIG,
                  TSTRING(_T("")),
                  TSTRING(_T("create-cfgfile")),
                  cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::PASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::NO_ENCRYPTION, TSTRING(_T("e")), TSTRING(_T("no-encryption")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWAdminCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    parser.AddMutEx(cTWAdminCmdLine::NO_ENCRYPTION, cTWAdminCmdLine::SITE_KEY_FILE);
    parser.AddMutEx(cTWAdminCmdLine::NO_ENCRYPTION, cTWAdminCmdLine::PASSPHRASE);
}

bool cTWAModeCreateCfg::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    TSTRING str;

    FillOutConfigInfo(cf);

    FillOutCmdLineInfo(parser);

    bool         fConfigOnCmdLine = false;
    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::NO_ENCRYPTION:
            mNoEncryption = true;
            break;
        case cTWAdminCmdLine::PASSPHRASE:
        {
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mPassPhrase         = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mPassPhraseProvided = true;
        }
        break;
        case cTWAdminCmdLine::CFG_FILE:
            if (iter.NumParams() < 1)
            {
                cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_CONFIG)));
                return false;
            }
            mEncryptedConfig = iter.ParamAt(0);
            fConfigOnCmdLine = true;
            break;
        case cTWAdminCmdLine::PARAMS:
            ASSERT(iter.NumParams() == 1);
            mPlaintextConfig = iter.ParamAt(0);

            break;
        }
    }

    // get full path to files
    TSTRING strPlaintextConfigPath;
    if (iFSServices::GetInstance()->FullPath(strPlaintextConfigPath, mPlaintextConfig))
        mPlaintextConfig = strPlaintextConfigPath;

    // OK, now we need to figure out where to put the new config file.
    // If the location was specified on the command line, get the full path to it.
    // otherwise, the location is the dir that this exe is in.
    if (fConfigOnCmdLine)
    {
        TSTRING strEncryptedConfigPath;
        if (iFSServices::GetInstance()->FullPath(strEncryptedConfigPath, mEncryptedConfig))
            mEncryptedConfig = strEncryptedConfigPath;
    }
    else
    {
        iFSServices::GetInstance()->FullPath(
            mEncryptedConfig, TSS_GetString(cTW, tw::STR_DEF_CFG_FILENAME), cSystemInfo::GetExeDir());
    }

    return true;
}

int cTWAModeCreateCfg::Execute(cErrorQueue* pQueue)
{
    if (mPlaintextConfig.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_PT_CONFIG)));
        return 1;
    }

    if (mEncryptedConfig.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_CONFIG)));
        return 1;
    }

    if (mSiteKeyFile.empty() && !mNoEncryption)
    {
        cTWUtil::PrintErrorMsg(
            eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_SITE_KEY_NOENCRYPT_NOT_SPECIFIED)));
        return 1;
    }

    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                       TSS_GetString(cTWAdmin, twadmin::STR_UPCONFIG_VERBOSE_PT_CONFIG).c_str(),
                                       cDisplayEncoder::EncodeInline(mPlaintextConfig).c_str());


    cFileArchive arch;
    arch.OpenRead(mPlaintextConfig.c_str(), cFileArchive::FA_OPEN_TEXT); // Note: eArchive may be thrown

    // read in config text
    std::string narrowPlaintext;
    narrowPlaintext.resize(arch.Length());
    // NOTE: archive may write fewer than arch.Length() bytes since
    // it is converting CR/LF to CR.
    int cbRead = arch.ReadBlob((void*)narrowPlaintext.data(), arch.Length());
    narrowPlaintext.resize(cbRead); // may have garbage after plaintext[cbRead] due to text mode conversion

    TSTRING plaintext = cStringUtil::StrToTstr(narrowPlaintext);

    // Try reading it to see if there are any problems.
    // Also open the keyfile so we can verify the keyfile specified in the plaintext
    // matches the key we intend to encrypt the config file with.
    cKeyFile plaintextKeyFile;
    TSTRING  plaintextKeyFileName;
    bool     plaintextKeyFileLoaded = false;
    try
    {
        cConfigFile cf;
        cf.ReadString(plaintext);

        if ((plaintextKeyFileLoaded = cf.Lookup(_T("SITEKEYFILE"), plaintextKeyFileName)) == true &&
            (plaintextKeyFileLoaded = !plaintextKeyFileName.empty()) == true)
        {
            TSTRING fullPath;
            if (iFSServices::GetInstance()->FullPath(fullPath, plaintextKeyFileName, cSystemInfo::GetExeDir()))
                plaintextKeyFileName = fullPath;

            cTWUtil::OpenKeyFile(plaintextKeyFile, plaintextKeyFileName);
            plaintextKeyFileLoaded = true;
        }
    }
    catch (eConfigFile& e)
    {
        pQueue->AddError(e);
        return 1;
    }

    cKeyFile                     specifiedKeyFile;
    const cElGamalSigPrivateKey* pPrivateKey;

    if (!mNoEncryption)
    {
        cTWUtil::OpenKeyFile(specifiedKeyFile, mSiteKeyFile);
        if (!plaintextKeyFileLoaded)
        {
            // User wishes to create a config file using a key, but has not specified the key in the config file.
            // For safty and sanity reasons we do not let them do this.
            pQueue->AddError(eTWACreateCfgMissingSitekey(TSTRING()));
            return false;
        }
        else if (!specifiedKeyFile.GetPublicKey()->IsEqual(*plaintextKeyFile.GetPublicKey()))
        {
            // The site key specified in this config file does not match the sitekey they are encrypting the
            // config file with.  This is likely to lead to problems so we try to catch it here.
            pQueue->AddError(eTWACreateCfgSitekeyMismatch(mSiteKeyFile, plaintextKeyFileName));
            return false;
        }

        pPrivateKey = cTWUtil::CreatePrivateKey(
            specifiedKeyFile, mPassPhraseProvided ? mPassPhrase.c_str() : NULL, cTWUtil::KEY_SITE);
    }
    else
        pPrivateKey = 0;

    // Make sure we can create the file
    cFileUtil::TestFileWritable(mEncryptedConfig);

    // backup current file if it exists
    cFileUtil::BackupFile(mEncryptedConfig);

    if (!mNoEncryption)
    {
        cTWUtil::WriteConfigText(mEncryptedConfig.c_str(), plaintext, true, pPrivateKey);
        specifiedKeyFile.ReleasePrivateKey();
    }
    else
        cTWUtil::WriteConfigText(mEncryptedConfig.c_str(), plaintext, false, 0);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeCreatePol

class cTWAModeCreatePol : public cTWAModeCommon
{
public:
    cTWAModeCreatePol();
    virtual ~cTWAModeCreatePol();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CREATE_POLFILE);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_CREATE_POLICY;
    }

private:
    TSTRING     mPlaintextPolicy;
    wc16_string mPassPhrase; // pass phrase for private key
    bool        mPassPhraseProvided;
    bool        mNoEncryption;
};

cTWAModeCreatePol::cTWAModeCreatePol()
{
    mNoEncryption       = false;
    mPassPhraseProvided = false;
}

cTWAModeCreatePol::~cTWAModeCreatePol()
{
}

void cTWAModeCreatePol::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWAdminCmdLine::MODE_CREATE_POLICY,
                  TSTRING(_T("")),
                  TSTRING(_T("create-polfile")),
                  cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::PASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::NO_ENCRYPTION, TSTRING(_T("e")), TSTRING(_T("no-encryption")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(cTWAdminCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_ONE);
    parser.AddMutEx(cTWAdminCmdLine::NO_ENCRYPTION, cTWAdminCmdLine::SITE_KEY_FILE);
    parser.AddMutEx(cTWAdminCmdLine::NO_ENCRYPTION, cTWAdminCmdLine::PASSPHRASE);
}

bool cTWAModeCreatePol::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    TSTRING str;

    FillOutConfigInfo(cf);
    FillOutCmdLineInfo(parser);

    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::NO_ENCRYPTION:
            mNoEncryption = true;
            break;
        case cTWAdminCmdLine::PASSPHRASE:
        {
            ASSERT(iter.NumParams() > 0); // should be caught by cmd line parser
            mPassPhrase         = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mPassPhraseProvided = true;
        }
        break;
        case cTWAdminCmdLine::PARAMS:
            ASSERT(iter.NumParams() == 1);
            mPlaintextPolicy = iter.ParamAt(0);
            break;
        }
    }

    // get full path to files
    TSTRING strFullPath;
    if (iFSServices::GetInstance()->FullPath(strFullPath, mSiteKeyFile))
        mSiteKeyFile = strFullPath;
    if (iFSServices::GetInstance()->FullPath(strFullPath, mPlaintextPolicy))
        mPlaintextPolicy = strFullPath;

    if (mPlaintextPolicy.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_PT_POLICY)));
        return false;
    }

    // check that the config file and site key file are in sync...
    //
    if (!mCfgFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mCfgFilePath, mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

int cTWAModeCreatePol::Execute(cErrorQueue* pQueue)
{
    if (mPlaintextPolicy.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_PT_POLICY)));
        return 1;
    }

    if (mPolFile.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_POLICY)));
        return 1;
    }

    if (mSiteKeyFile.empty() && !mNoEncryption)
    {
        cTWUtil::PrintErrorMsg(
            eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_SITE_KEY_NOENCRYPT_NOT_SPECIFIED)));
        return 1;
    }


    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                       TSS_GetString(cTWAdmin, twadmin::STR_UPCONFIG_VERBOSE_PT_POLICY).c_str(),
                                       cDisplayEncoder::EncodeInline(mPlaintextPolicy).c_str());

    // open policy file as text
    cFileArchive arch;
    arch.OpenRead(mPlaintextPolicy.c_str(), cFileArchive::FA_OPEN_TEXT); // Note: eArchive may be thrown

    // read in policy text
    std::string plaintext;
    plaintext.resize(arch.Length());
    // NOTE: archive may write fewer than arch.Length() bytes since
    // it is converting CR/LF to CR.
    int cbRead = arch.ReadBlob((void*)plaintext.data(), arch.Length());
    plaintext.resize(cbRead); // may have garbage after plaintext[cbRead] due to text mode conversion

    //
    // make sure the policy file parses correctly before we update the old one
    //
#if !ARCHAIC_STL    
    std::istringstream in(plaintext);
#else
    strstream in;
    in << plaintext;
#endif
    
    cPolicyParser      parser(in);
    try
    {
        parser.Check(pQueue);
    }
    catch (eError& e)
    {
        cTWUtil::PrintErrorMsg(e);
        iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                           TSS_GetString(cTWAdmin, twadmin::STR_POL_NOT_UPDATED).c_str());
        return 1;
    }

    //
    // open the key file...
    //
    cKeyFile keyfile;

    const cElGamalSigPrivateKey* pPrivateKey;

    if (!mNoEncryption)
    {
        cTWUtil::OpenKeyFile(keyfile, mSiteKeyFile);
        pPrivateKey =
            cTWUtil::CreatePrivateKey(keyfile, mPassPhraseProvided ? mPassPhrase.c_str() : NULL, cTWUtil::KEY_SITE);
    }
    else
        pPrivateKey = 0;

    // Make sure we can create the file
    cFileUtil::TestFileWritable(mPolFile);

    // backup current file if it exists
    cFileUtil::BackupFile(mPolFile);

    if (!mNoEncryption)
    {
        cTWUtil::WritePolicyText(mPolFile.c_str(), plaintext, true, pPrivateKey);
        keyfile.ReleasePrivateKey();
    }
    else
        cTWUtil::WritePolicyText(mPolFile.c_str(), plaintext, false, 0);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModePrintCfg

class cTWAModePrintCfg : public cTWAModeCommon
{
public:
    cTWAModePrintCfg();
    virtual ~cTWAModePrintCfg();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_PRINT_CFGFILE);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_PRINT_CONFIG;
    }

private:
    TSTRING mEncryptedConfig;
};

cTWAModePrintCfg::cTWAModePrintCfg()
{
}

cTWAModePrintCfg::~cTWAModePrintCfg()
{
}

void cTWAModePrintCfg::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(
        cTWAdminCmdLine::MODE_PRINT_CONFIG, TSTRING(_T("")), TSTRING(_T("print-cfgfile")), cCmdLineParser::PARAM_NONE);
}

bool cTWAModePrintCfg::Init(const cConfigFile* cf, const cCmdLineParser& parser) //throw(eTWUTil)
{
    FillOutConfigInfo(cf);
    FillOutCmdLineInfo(parser);

    mEncryptedConfig = cTWUtil::GetCfgFilePath(parser, cTWAdminCmdLine::CFG_FILE);

    // check that the config file and site key file are in sync...
    //
    if (!mCfgFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mCfgFilePath, mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

int cTWAModePrintCfg::Execute(cErrorQueue* pQueue)
{
    try
    {
        TSTRING configText;
        cTWUtil::ReadConfigText(mEncryptedConfig.c_str(), configText);

        // Note: I believe print config and print policy are the only time we should
        // print to standard out directly.
        TCOUT << cDisplayEncoder::EncodeInlineAllowWS(configText);
        TCOUT << std::endl;
    }
    catch (eError& e)
    {
        TSTRING extra;
        extra += TSS_GetString(cTW, tw::STR_NEWLINE);
        extra += TSS_GetString(cTW, tw::STR_ERR_TWCFG_CANT_READ);
        cTWUtil::PrintErrorMsg(e, extra);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModePrintPol

class cTWAModePrintPol : public cTWAModeCommon
{
public:
    cTWAModePrintPol();
    virtual ~cTWAModePrintPol();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_PRINT_POLFILE);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_PRINT_POLICY;
    }

private:
};

cTWAModePrintPol::cTWAModePrintPol()
{
}

cTWAModePrintPol::~cTWAModePrintPol()
{
}

void cTWAModePrintPol::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(
        cTWAdminCmdLine::MODE_PRINT_POLICY, TSTRING(_T("")), TSTRING(_T("print-polfile")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
}

bool cTWAModePrintPol::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    FillOutConfigInfo(cf);
    FillOutCmdLineInfo(parser);

    return true;
}

int cTWAModePrintPol::Execute(cErrorQueue* pQueue)
{
    try
    {
        cKeyFile sitekey;

        bool policyEncrypted =
            cTWUtil::IsObjectEncrypted(mPolFile.c_str(),
                                       cPolicyFile::GetFileHeaderID(),
                                       TSS_GetString(cTWAdmin, twadmin::STR_ERR2_UNABLE_TO_PRINT_POLICY));

        if (policyEncrypted)
        {
            // check that the config file and site key file are in sync...
            //
            if (!mCfgFilePath.empty())
                try
                {
                    if (cTWUtil::VerifyCfgSiteKey(mCfgFilePath, mSiteKeyFile) == false)
                        cTWUtil::PrintErrorMsg(
                            eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
                }
                catch (eTWUtilCfgKeyMismatch& e)
                {
                    e.SetFatality(false);
                    cTWUtil::PrintErrorMsg(e);
                }

            cTWUtil::OpenKeyFile(sitekey, mSiteKeyFile);
        }
        else
            cTWUtil::PrintErrorMsg(eTWUtilPolUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));

        std::string policyText;

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           _T("%s%s\n"),
                                           TSS_GetString(cTW, tw::STR_OPEN_POLICY_FILE).c_str(),
                                           cDisplayEncoder::EncodeInline(mPolFile).c_str());

        cTWUtil::ReadPolicyText(mPolFile.c_str(), policyText, policyEncrypted ? sitekey.GetPublicKey() : 0);

        // Note: I believe print config and print policy are the only time we should
        // print to standard out directly.
        TCOUT << cDisplayEncoder::EncodeInlineAllowWS(cStringUtil::StrToTstr(policyText));
        TCOUT << std::endl;
    }
    catch (eError& e)
    {
        cTWUtil::PrintErrorMsg(e);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeRemoveEncryption

class cTWAModeRemoveEncryption : public cTWAModeCommon
{
public:
    cTWAModeRemoveEncryption();
    virtual ~cTWAModeRemoveEncryption();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_REMOVE_ENCRYPTION);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_REMOVE_ENCRYPTION;
    }

private:
    std::list<TSTRING> mFileList;
    wc16_string        mSitePassphrase;
    wc16_string        mLocalPassphrase;
    bool               mSitePassphraseProvided;
    bool               mLocalPassphraseProvided;
};

cTWAModeRemoveEncryption::cTWAModeRemoveEncryption()
{
    mSitePassphraseProvided  = false;
    mLocalPassphraseProvided = false;
}

cTWAModeRemoveEncryption::~cTWAModeRemoveEncryption()
{
}

void cTWAModeRemoveEncryption::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWAdminCmdLine::MODE_REMOVE_ENCRYPTION,
                  TSTRING(_T("")),
                  TSTRING(_T("remove-encryption")),
                  cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::SITEPASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCALPASSPHRASE, TSTRING(_T("P")), TSTRING(_T("local-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);

    parser.AddMutEx(cTWAdminCmdLine::KEY_FILE, cTWAdminCmdLine::LOCAL_KEY_FILE);
    parser.AddMutEx(cTWAdminCmdLine::KEY_FILE, cTWAdminCmdLine::LOCAL_KEY_FILE);
    parser.AddMutEx(cTWAdminCmdLine::KEY_FILE, cTWAdminCmdLine::SITEPASSPHRASE);
}

bool cTWAModeRemoveEncryption::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    int i;

    FillOutConfigInfo(cf);
    FillOutCmdLineInfo(parser);

    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::LOCALPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mLocalPassphraseProvided = true;
            mLocalPassphrase         = cStringUtil::TstrToWstr(iter.ParamAt(0));
            break;
        case cTWAdminCmdLine::SITEPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mSitePassphraseProvided = true;
            mSitePassphrase         = cStringUtil::TstrToWstr(iter.ParamAt(0));
            break;
        case cTWAdminCmdLine::PARAMS:
            for (i = 0; i < iter.NumParams(); ++i)
            {
                // get full path to files
                TSTRING strFullPath;
                if (iFSServices::GetInstance()->FullPath(strFullPath, iter.ParamAt(i)))
                    mFileList.push_back(strFullPath);
                else
                    mFileList.push_back(iter.ParamAt(i));
            }
            break;
        }
    }

    // check that the config file and site key file are in sync...
    //
    if (!mCfgFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mCfgFilePath, mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

int cTWAModeRemoveEncryption::Execute(cErrorQueue* pQueue)
{
    TSTRING          keyfile;
    cTWUtil::KeyType keyType;
    bool             userKnowsLocalPassphrase = false;
    bool             userKnowsSitePassphrase  = false;

    bool bResult = true;

    bool firstFile       = true;
    bool warningGiven    = false;
    bool lastMsgWasError = false; // used to send the separating newline to stderr instead of stdout

    if (mFileList.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_FILES_SPECIFIED)));
        return 1;
    }

    std::list<TSTRING>::iterator i;
    for (i = mFileList.begin(); i != mFileList.end(); ++i)
    {
        // These lines print out a blank line if necessary
        if (!firstFile)
            if (!lastMsgWasError)
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
            else
            {
                TCERR << std::endl;
                lastMsgWasError = false;
            }
        else
            firstFile = false;

        if (cFileUtil::IsDir(i->c_str()))
        {
            // Ignore directories for this particular operation.
            cTWUtil::PrintErrorMsg(eTWASkippingDirectory(*i, eError::NON_FATAL));
            lastMsgWasError = true;
        }
        else
            try
            {
                cFileManipulator manip(i->c_str());
                manip.Init();

                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   _T("%s %s\n"),
                                                   TSS_GetString(cTWAdmin, twadmin::STR_EXAMINING_FILE).c_str(),
                                                   cDisplayEncoder::EncodeInline(manip.GetFileName()).c_str());

                if (NotifyFileType(*manip.GetHeaderID(), manip.GetFileVersion(), iUserNotify::V_VERBOSE) == false)
                {
                    throw eTWAFileTypeUnknown(manip.GetFileName());
                }

                // can't decrypt keyfiles
                if (*manip.GetHeaderID() == cKeyFile::GetFileHeaderID())
                {
                    cTWUtil::PrintErrorMsg(
                        eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_CAN_NOT_DECRYPT_KEYFILE),
                                             manip.GetFileName(),
                                             eError::NON_FATAL));
                    lastMsgWasError = true;
                    bResult         = false;
                    continue;
                }

                // if this is a config file, make sure its embedded key is the
                // same as the provided keyfile
                if (*manip.GetHeaderID() == cConfigFile::GetFileHeaderID())
                {
                    try
                    {
                        cTWUtil::VerifyCfgSiteKey(*i, mSiteKeyFile);
                    }
                    catch (eTWUtil& e)
                    {
                        e.SetSupressThird(true);
                        e.SetFatality(false);
                        cTWUtil::PrintErrorMsg(e);

                        if (e.GetID() == eError::CalcHash("eTWUtilCorruptedFile"))
                            cTWUtil::PrintErrorMsg(eTWADecryptCorrupt(manip.GetFileName(), eError::NON_FATAL));
                        else
                            cTWUtil::PrintErrorMsg(eTWADecrypt(manip.GetFileName(), eError::NON_FATAL));

                        //cTWUtil::PrintErrorMsg(eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_REMOVE_ENCRYPTION_FAILED), manip.GetFileName(), eError::NON_FATAL));
                        lastMsgWasError = true;
                        bResult         = false;
                        continue;
                    }
                }

                if (cFileManipulator::UseSiteKey(*manip.GetHeaderID()))
                {
                    keyType = cTWUtil::KEY_SITE;
                    keyfile = mSiteKeyFile;
                }
                else
                {
                    keyType = cTWUtil::KEY_LOCAL;
                    keyfile = mLocalKeyFile;
                }

                if (NotifyEncryptionType(manip.GetEncoding(), iUserNotify::V_VERBOSE) == false)
                {
                    cTWUtil::PrintErrorMsg(
                        eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_ENCODING_TYPE_UNKNOWN),
                                             manip.GetFileName(),
                                             eError::NON_FATAL));
                    lastMsgWasError = true;
                    bResult         = false;
                    continue;
                }
                else if (manip.GetEncoding() != cFileHeader::ASYM_ENCRYPTION)
                {
                    cTWUtil::PrintErrorMsg(
                        eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_FILE_NOT_ENCRYPED),
                                             manip.GetFileName(),
                                             eError::NON_FATAL));
                    lastMsgWasError = true;
                    bResult         = false;
                    continue;
                }
                else
                {
                    // If we got here we have an Asymmetrically encrypted file

                    // warn user about removing encryption
                    if (!warningGiven)
                    {
                        iUserNotify::GetInstance()->Notify(
                            iUserNotify::V_SILENT,
                            TSS_GetString(cTWAdmin, twadmin::STR_REMOVE_ENCRYPTION_WARNING).c_str());
                        TCERR << std::endl;
                        warningGiven = true;
                    }

                    cKeyFile key;
                    key.ReadFile(keyfile.c_str());

                    if (keyType == cTWUtil::KEY_LOCAL)
                    {
                        if (!userKnowsLocalPassphrase)
                        {
                            cTWUtil::CreatePrivateKey(key,
                                                      mLocalPassphraseProvided ? mLocalPassphrase.c_str() : 0,
                                                      keyType); // note: this throws an exception on failure
                            key.ReleasePrivateKey();
                            if (!mLatePassphrase) // force user to enter passphrase each time
                                userKnowsLocalPassphrase = true;
                        }
                    }
                    else if (keyType == cTWUtil::KEY_SITE)
                    {
                        if (!userKnowsSitePassphrase)
                        {
                            // Check to see if we can even use the site key to decrypt this file.
                            cTWUtil::CreatePrivateKey(key,
                                                      mSitePassphraseProvided ? mSitePassphrase.c_str() : 0,
                                                      keyType); // note: this throws an exception on failure
                            key.ReleasePrivateKey();
                            if (!mLatePassphrase) // force user to enter passphrase each time
                                userKnowsSitePassphrase = true;
                        }
                    }
                    else
                        ASSERT(false);

                    try
                    {
                        manip.ChangeEncryption(key.GetPublicKey(), NULL, false);
                    }
                    catch (eError& e)
                    {
                        // Let the user know the decryption failed
                        e.SetSupressThird(true);
                        e.SetFatality(false);
                        cTWUtil::PrintErrorMsg(e);

                        if (e.GetID() == eError::CalcHash("eArchiveCrypto"))
                            cTWUtil::PrintErrorMsg(eTWADecryptCorrupt(manip.GetFileName(), eError::NON_FATAL));
                        else
                            cTWUtil::PrintErrorMsg(eTWADecrypt(manip.GetFileName(), eError::NON_FATAL));

                        lastMsgWasError = true;
                        bResult         = false;
                        continue;
                    }

                    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                                       TSS_GetString(cTWAdmin, twadmin::STR_ENCRYPTION_REMOVED).c_str(),
                                                       cDisplayEncoder::EncodeInline(manip.GetFileName()).c_str());
                }
            }
            catch (eFileManip& e)
            {
                e.SetFatality(false);
                cTWUtil::PrintErrorMsg(e);
                lastMsgWasError = true;
                bResult         = false;
            }
            catch (eArchive& e)
            {
                e.SetFatality(false);
                cTWUtil::PrintErrorMsg(e);
                lastMsgWasError = true;
                bResult         = false;
            }
    }

    return bResult == false;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeEncrypt

class cTWAModeEncrypt : public cTWAModeCommon
{
public:
    cTWAModeEncrypt();
    virtual ~cTWAModeEncrypt();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_ENCRYPT);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_REMOVE_ENCRYPTION;
    }

private:
    std::list<TSTRING> mFileList;
    wc16_string        mSitePassphrase;
    wc16_string        mLocalPassphrase;
    bool               mSitePassphraseProvided;
    bool               mLocalPassphraseProvided;
    bool               mbLatePassphrase;
};

cTWAModeEncrypt::cTWAModeEncrypt()
{
    mSitePassphraseProvided  = false;
    mLocalPassphraseProvided = false;
    mbLatePassphrase         = false;
}

cTWAModeEncrypt::~cTWAModeEncrypt()
{
}

void cTWAModeEncrypt::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWAdminCmdLine::MODE_ENCRYPT, TSTRING(_T("")), TSTRING(_T("encrypt")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCALPASSPHRASE, TSTRING(_T("P")), TSTRING(_T("local-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::SITEPASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);
}

bool cTWAModeEncrypt::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    int i;

    FillOutConfigInfo(cf);

    TSTRING str;
    if (cf->Lookup(TSTRING(_T("LATEPROMPTING")), str))
    {
        if (_tcsicmp(str.c_str(), _T("true")) == 0)
            mbLatePassphrase = true;
    }

    FillOutCmdLineInfo(parser);

    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::PARAMS:
            for (i = 0; i < iter.NumParams(); ++i)
            {
                TSTRING strFullPath;
                if (iFSServices::GetInstance()->FullPath(strFullPath, iter.ParamAt(i)))
                    mFileList.push_back(strFullPath);
                else
                    mFileList.push_back(iter.ParamAt(i));
            }
            break;
        case cTWAdminCmdLine::LOCALPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mLocalPassphraseProvided = true;
            mLocalPassphrase         = cStringUtil::TstrToWstr(iter.ParamAt(0));
            break;
        case cTWAdminCmdLine::SITEPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mSitePassphraseProvided = true;
            mSitePassphrase         = cStringUtil::TstrToWstr(iter.ParamAt(0));
            break;
        }
    }


    // check that the config file and site key file are in sync...
    //
    if (!mCfgFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mCfgFilePath, mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

int cTWAModeEncrypt::Execute(cErrorQueue* pQueue)
{
    cKeyFile         siteKeyFile, localKeyFile;
    cPrivateKeyProxy siteKey, localKey;

    if (mFileList.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_FILES_SPECIFIED)));
        return 1;
    }

    bool                         bResult = true;
    std::list<TSTRING>::iterator i;

    // We cycle through all the files twice.  This first time we
    // check for file validity and open any keyfiles needed.
    for (i = mFileList.begin(); i != mFileList.end(); /* incrementing done below...*/)
    {
        try
        {
            if (cFileUtil::IsDir(i->c_str()))
            {
                // Ignore directories for this particular operation.
                // Note: We don't throw here because we don't want to set bResult to false
                cTWUtil::PrintErrorMsg(eTWASkippingDirectory(*i, eError::NON_FATAL));
                TCERR << std::endl; // extra newline to separate filenames
                i = mFileList.erase(i);
                continue;
            }
            else if (!cFileUtil::FileExists(i->c_str()))
            {
                // tell user we could not open *i
                throw eTWAFileNotFound(*i, eError::NON_FATAL);
            }

            cFileManipulator manip(i->c_str());
            manip.Init();

            // can't encrypt keyfiles
            if (*manip.GetHeaderID() == cKeyFile::GetFileHeaderID())
            {
                throw eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_CAN_NOT_DECRYPT_KEYFILE),
                                           manip.GetFileName(),
                                           eError::NON_FATAL);
            }

            if (cFileManipulator::UseSiteKey(*manip.GetHeaderID()))
            {
                if (!siteKeyFile.KeysLoaded())
                    cTWUtil::OpenKeyFile(siteKeyFile, mSiteKeyFile);
            }
            else
            {
                if (!localKeyFile.KeysLoaded())
                    cTWUtil::OpenKeyFile(localKeyFile, mLocalKeyFile);
            }

            // increment the iterator
            ++i;
        }
        catch (eError& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
            TCERR << std::endl; // extra newline to separate filenames

            // erase this file so that it is skipped in the next round
            i       = mFileList.erase(i);
            bResult = false;
            continue;
        }
    }

    // On this second pass we go through and convert all files
    for (i = mFileList.begin(); i != mFileList.end(); ++i)
    {
        if (cFileUtil::IsDir(i->c_str()))
        {
            // Ignore directories for this particular operation.
        }
        else if (!cFileUtil::FileExists(i->c_str()))
        {
            // tell user we could not open *i
            bResult = false;
        }
        else
            try
            {
                cFileManipulator manip(i->c_str());
                manip.Init();

                if (*manip.GetHeaderID() == cKeyFile::GetFileHeaderID())
                {
                    continue;
                }

                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   _T("%s %s\n"),
                                                   TSS_GetString(cTWAdmin, twadmin::STR_EXAMINING_FILE).c_str(),
                                                   cDisplayEncoder::EncodeInline(manip.GetFileName()).c_str());

                if (NotifyFileType(*manip.GetHeaderID(), manip.GetFileVersion(), iUserNotify::V_VERBOSE) == false)
                {
                    cTWUtil::PrintErrorMsg(eTWAFileTypeUnknown(manip.GetFileName(), eError::NON_FATAL));
                    TCERR << std::endl; // extra newline to separate filenames
                    bResult = false;
                }

                if (NotifyEncryptionType(manip.GetEncoding(), iUserNotify::V_VERBOSE) == false)
                {
                    cTWUtil::PrintErrorMsg(
                        eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_ENCODING_TYPE_UNKNOWN),
                                             manip.GetFileName(),
                                             eError::NON_FATAL));
                    TCERR << std::endl; // extra newline to separate filenames
                    bResult = false;
                }
                else if (manip.GetEncoding() == cFileHeader::ASYM_ENCRYPTION)
                {
                    cTWUtil::PrintErrorMsg(
                        eTWAEncryptionChange(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_FILE_ALREADY_ENCRYPTED),
                                             manip.GetFileName(),
                                             eError::NON_FATAL));
                    TCERR << std::endl; // extra newline to separate filenames
                    bResult = false;
                }
                else
                {
                    cPrivateKeyProxy* pPrivateKey;

                    if (cFileManipulator::UseSiteKey(*manip.GetHeaderID()))
                    {
                        if (mbLatePassphrase || !siteKey.Valid())
                            cTWUtil::CreatePrivateKey(siteKey,
                                                      siteKeyFile,
                                                      mSitePassphraseProvided ? mSitePassphrase.c_str() : 0,
                                                      cTWUtil::KEY_SITE);

                        pPrivateKey = &siteKey;
                    }
                    else
                    {
                        if (mbLatePassphrase || !localKey.Valid())
                            cTWUtil::CreatePrivateKey(localKey,
                                                      localKeyFile,
                                                      mLocalPassphraseProvided ? mLocalPassphrase.c_str() : 0,
                                                      cTWUtil::KEY_LOCAL);

                        pPrivateKey = &localKey;
                    }

                    try
                    {
                        manip.ChangeEncryption(NULL, pPrivateKey->GetKey(), false);

                        iUserNotify::GetInstance()->Notify(
                            iUserNotify::V_VERBOSE,
                            TSS_GetString(cTWAdmin, twadmin::STR_ENCRYPTION_SUCCEEDED).c_str(),
                            cDisplayEncoder::EncodeInline(manip.GetFileName()).c_str());
                        iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                           TSS_GetString(cTW, tw::STR_NEWLINE).c_str(),
                                                           cDisplayEncoder::EncodeInline(manip.GetFileName()).c_str());
                    }
                    catch (eFileManip& e)
                    {
                        cTWUtil::PrintErrorMsg(e);
                        TCERR << std::endl; // extra newline to separate filenames
                        bResult = false;
                    }
                }
            }
            catch (eFileManip&)
            {
                continue;
            }
    }

    return bResult == false;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeExamine

class cTWAModeExamine : public cTWAModeCommon
{
public:
    cTWAModeExamine();
    virtual ~cTWAModeExamine();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_EXAMINE);
    }
    virtual bool LoadConfigFile()
    {
        return true;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_REMOVE_ENCRYPTION;
    }

private:
    std::list<TSTRING> mFileList;
};

cTWAModeExamine::cTWAModeExamine()
{
}

cTWAModeExamine::~cTWAModeExamine()
{
}

void cTWAModeExamine::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWAdminCmdLine::MODE_EXAMINE, TSTRING(_T("")), TSTRING(_T("examine")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);
}

bool cTWAModeExamine::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    int i;

    FillOutConfigInfo(cf);
    FillOutCmdLineInfo(parser);

    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::PARAMS:
            for (i = 0; i < iter.NumParams(); ++i)
            {
                TSTRING strFullPath;
                if (iFSServices::GetInstance()->FullPath(strFullPath, iter.ParamAt(i)))
                    mFileList.push_back(strFullPath);
                else
                    mFileList.push_back(iter.ParamAt(i));
            }
            break;
        }
    }

    // check that the config file and site key file are in sync...
    //
    if (!mCfgFilePath.empty())
        try
        {
            if (cTWUtil::VerifyCfgSiteKey(mCfgFilePath, mSiteKeyFile) == false)
                cTWUtil::PrintErrorMsg(eTWCfgUnencrypted(_T(""), eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
        catch (eTWUtilCfgKeyMismatch& e)
        {
            e.SetFatality(false);
            cTWUtil::PrintErrorMsg(e);
        }

    return true;
}

int cTWAModeExamine::Execute(cErrorQueue* pQueue)
{
    if (mFileList.empty())
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_NO_FILES_SPECIFIED)));
        return 1;
    }

    bool bResult = true;

    // Open site and local key up front so we can warn if they fail opening
    cKeyFile siteKey, localKey;
    try
    {
        siteKey.ReadFile(mSiteKeyFile.c_str());
    }
    catch (eKeyFile& e)
    {
        e.SetFatality(false);
        cTWUtil::PrintErrorMsg(e);
        bResult = false;
    }
    catch (eError&)
    {
    }

    try
    {
        localKey.ReadFile(mLocalKeyFile.c_str());
    }
    catch (eKeyFile& e)
    {
        e.SetFatality(false);
        cTWUtil::PrintErrorMsg(e);
        bResult = false;
    }
    catch (eError&)
    {
    }

    // now iteratate over files to examine
    std::list<TSTRING>::iterator i;
    for (i = mFileList.begin(); i != mFileList.end(); ++i)
    {
        if (cFileUtil::IsDir(i->c_str()))
        {
            //Ignore directories in examine encryption mode.
            cTWUtil::PrintErrorMsg(eTWASkippingDirectory(*i, eError::NON_FATAL));
            TCERR << std::endl; // extra newline to separate filenames
        }
        else if (!cFileUtil::FileExists(i->c_str()))
        {
            // tell user we could not open *i
            cTWUtil::PrintErrorMsg(eTWAFileNotFound(*i, eError::NON_FATAL));
            TCERR << std::endl; // extra newline to separate filenames
            bResult = false;
        }
        else
            try
            {
                cFileManipulator manip(i->c_str());
                manip.Init();

                // print out: "File: filename.ext"
                iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                   TSS_GetString(cTWAdmin, twadmin::STR_FILE).c_str());
                iUserNotify::GetInstance()->Notify(
                    iUserNotify::V_SILENT, _T("%s"), cDisplayEncoder::EncodeInline(manip.GetFileName()).c_str());
                iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                   TSS_GetString(cTWAdmin, twadmin::STR_ENDQUOTE_NEWLINE).c_str());

                NotifyFileType(*manip.GetHeaderID(), manip.GetFileVersion(), iUserNotify::V_NORMAL);
                NotifyEncryptionType(manip.GetEncoding(), iUserNotify::V_NORMAL);

                // Try different keys to see if they decrypt this file
                if (manip.GetEncoding() == cFileHeader::ASYM_ENCRYPTION)
                {
                    bool bFound = false;

                    // Output the keys that decrypt the file.
                    iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                       TSS_GetString(cTWAdmin, twadmin::STR_KEYS_DECRYPT).c_str());
                    iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                       TSS_GetString(cTW, tw::STR_NEWLINE).c_str());

                    if (siteKey.KeysLoaded())
                        try
                        {
                            if (manip.TestDecryption(*siteKey.GetPublicKey(), false) != false)
                            {
                                bFound = true;

                                iUserNotify::GetInstance()->Notify(
                                    iUserNotify::V_SILENT, TSS_GetString(cTWAdmin, twadmin::STR_SITEKEYFILE).c_str());

                                iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                                   cDisplayEncoder::EncodeInline(mSiteKeyFile).c_str());
                                iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                                   TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
                            }
                        }
                        catch (eError&)
                        {
                        }

                    if (localKey.KeysLoaded())
                        try
                        {
                            if (manip.TestDecryption(*localKey.GetPublicKey(), false) != false)
                            {
                                bFound = true;

                                iUserNotify::GetInstance()->Notify(
                                    iUserNotify::V_SILENT, TSS_GetString(cTWAdmin, twadmin::STR_LOCALKEYFILE).c_str());

                                iUserNotify::GetInstance()->Notify(
                                    iUserNotify::V_SILENT, cDisplayEncoder::EncodeInline(mLocalKeyFile).c_str());
                                iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                                   TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
                            }
                        }
                        catch (eError&)
                        {
                        }

                    if (!bFound)
                    {
                        bResult = false;
                        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, "\t");
                        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                           TSS_GetString(cCore, core::STR_UNKNOWN).c_str());
                        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT,
                                                           TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
                    }
                }
                TCOUT << std::endl;
            }
            catch (eFileManip& e)
            {
                e.SetFatality(false);
                cTWUtil::PrintErrorMsg(e);
                TCERR << std::endl;
                bResult = false;
            }
    }

    return bResult == false;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeGenerateKeys

class cTWAModeGenerateKeys : public cTWAModeCommon
{
public:
    cTWAModeGenerateKeys();
    virtual ~cTWAModeGenerateKeys();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_GENERATE_KEYS);
    }
    virtual bool LoadConfigFile()
    {
        return false;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_GENERATE_KEYS;
    }

private:
    bool                 mSiteProvided; // A site passphrase has been passed.
    wc16_string          mSitePassphrase;
    bool                 mLocalProvided; // A local passphrase has been passed.
    wc16_string          mLocalPassphrase;
    bool                 mGenerateSite;  // A Site key has been specified.
    bool                 mGenerateLocal; // A Local key has been specified.
    cElGamalSig::KeySize mKeySize;
};

cTWAModeGenerateKeys::cTWAModeGenerateKeys()
{
    mLocalProvided = false;
    mSiteProvided  = false;
    mGenerateSite  = false;
    mGenerateLocal = false;
    mKeySize       = cElGamalSig::KEY1024;
}

cTWAModeGenerateKeys::~cTWAModeGenerateKeys()
{
}

void cTWAModeGenerateKeys::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(
        cTWAdminCmdLine::MODE_GENERATE_KEYS, TSTRING(_T("")), TSTRING(_T("generate-keys")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::SITEPASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCALPASSPHRASE, TSTRING(_T("P")), TSTRING(_T("local-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::KEY_SIZE, TSTRING(_T("K")), TSTRING(_T("key-size")), cCmdLineParser::PARAM_ONE);
}

bool cTWAModeGenerateKeys::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    TSTRING str;

    // Note: we don't use the config file for anything any more.

    FillOutCmdLineInfo(parser);

    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::SITEPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mSitePassphrase = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mSiteProvided   = true;
            break;
        case cTWAdminCmdLine::LOCALPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mLocalPassphrase = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mLocalProvided   = true;
            break;
        case cTWAdminCmdLine::SITE_KEY_FILE:
            mGenerateSite = true;
            break;
        case cTWAdminCmdLine::LOCAL_KEY_FILE:
            mGenerateLocal = true;
            break;
        case cTWAdminCmdLine::KEY_SIZE:
            if (iter.ParamAt(0) == "2048")
                mKeySize = cElGamalSig::KEY2048;
            else if (iter.ParamAt(0) == "1024")
                mKeySize = cElGamalSig::KEY1024;
            else
                throw eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_INVALID_KEY_SIZE));
            break;
        }
    }

    if (mSiteProvided || mLocalProvided)
        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           TSS_GetString(cTWAdmin, twadmin::STR_KEYGEN_VERBOSE_PASSPHRASES).c_str());

    return true;
}

int cTWAModeGenerateKeys::Execute(cErrorQueue* pQueue)
{
    int i;

    if (!mGenerateSite && !mGenerateLocal)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_KEYS_NOT_SPECIFIED)));
        return 1;
    }

    if ((mGenerateSite && mSiteKeyFile.empty()) || (mGenerateLocal && mLocalKeyFile.empty()))
    {
        // This case should not occur (it should be caught by Init() logic), but in case it does we will spit out
        // a generic error string.
        ASSERT(false);
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_KEYS_NOT_SPECIFIED)));
        return 1;
    }

    if (!mGenerateSite && mSiteProvided)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_LONE_SITE_PASSPHRASE)));
        return 1;
    }

    if (!mGenerateLocal && mLocalProvided)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_LONE_LOCAL_PASSPHRASE)));
        return 1;
    }

    if ((mSiteProvided && !mGenerateSite) || (mLocalProvided && !mGenerateLocal))
    {
        //This is an error, a passphrase should not be specified if the corresponding key has not been passed.
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_PASSPHRASE_NOKEY)));
        return 1;
    }

    if (mSiteKeyFile == mLocalKeyFile)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_KEY_FILENAMES_IDENTICAL)));
        return 1;
    }

    if (mGenerateSite && cFileUtil::FileExists(mSiteKeyFile))
    {
        TSTRING prompt;
        prompt += TSS_GetString(cTWAdmin, twadmin::STR_SITEKEY_EXISTS_1);
        prompt += cDisplayEncoder::EncodeInline(mSiteKeyFile);
        prompt += TSS_GetString(cTWAdmin, twadmin::STR_SITEKEY_EXISTS_2);

        if (cTWUtil::ConfirmYN(prompt.c_str()) == false)
            return 1;
    }

    if (mGenerateLocal && cFileUtil::FileExists(mLocalKeyFile))
    {
        TSTRING prompt;
        prompt += TSS_GetString(cTWAdmin, twadmin::STR_LOCALKEY_EXISTS_1);
        prompt += cDisplayEncoder::EncodeInline(mLocalKeyFile);
        prompt += TSS_GetString(cTWAdmin, twadmin::STR_LOCALKEY_EXISTS_2);

        if (cTWUtil::ConfirmYN(prompt.c_str()) == false)
            return 1;
    }

    //
    // make sure that we can write to both of the files before we continue
    //
    if (mGenerateLocal)
        cFileUtil::TestFileWritable(mLocalKeyFile);
    if (mGenerateSite)
        cFileUtil::TestFileWritable(mSiteKeyFile);

    wc16_string verify;

    bool bPrintedPassphraseHint = false;

    if (mGenerateSite)
    {
        if (!mSiteProvided)
        {
            // give the user a passphrase hint...
            //
            if (!bPrintedPassphraseHint)
            {
                bPrintedPassphraseHint = true;
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   TSS_GetString(cTWAdmin, twadmin::STR_PASSPHRASE_HINT).c_str());
            }

            for (i = 1;; i++)
            {
                cTWUtil::NoEcho noEcho;
                // Note: Since we are interacting directly with the user here,
                // I think it is o.k. to use cout directly.
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_ENTER_SITE_PASS);
                cTWUtil::GetString(mSitePassphrase);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_VERIFY_SITE_PASS);
                cTWUtil::GetString(verify);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                if (0 == verify.compare(mSitePassphrase))
                    break;

                TCOUT << TSS_GetString(cTW, tw::STR_ERR_WRONG_PASSPHRASE_SITE) << std::endl;

                if (i == 3)
                    return 1;
            }
        }

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           TSS_GetString(cTWAdmin, twadmin::STR_KEYGEN_VERBOSE_SITEKEY).c_str(),
                                           cDisplayEncoder::EncodeInline(mSiteKeyFile).c_str());

        // backup current file if it exists
        cFileUtil::BackupFile(mSiteKeyFile);

        if (GenerateKey(mSiteKeyFile.c_str(), mSitePassphrase, mKeySize) == false)
            return 1;
    }

    if (mGenerateLocal)
    {
        if (!mLocalProvided)
        {
            // give the user a passphrase hint...
            //
            if (!bPrintedPassphraseHint)
            {
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   TSS_GetString(cTWAdmin, twadmin::STR_PASSPHRASE_HINT).c_str());
            }

            for (i = 1;; i++)
            {
                cTWUtil::NoEcho noEcho;
                // Note: Since we are interacting directly with the user here,
                // I think it is o.k. to use cout directly.
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_ENTER_LOCAL_PASS);
                cTWUtil::GetString(mLocalPassphrase);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_VERIFY_LOCAL_PASS);
                cTWUtil::GetString(verify);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                if (0 == verify.compare(mLocalPassphrase))
                    break;

                TCOUT << TSS_GetString(cTW, tw::STR_ERR_WRONG_PASSPHRASE_LOCAL) << std::endl;

                if (i == 3)
                    return 1;
            }
        }

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           TSS_GetString(cTWAdmin, twadmin::STR_KEYGEN_VERBOSE_LOCALKEY).c_str(),
                                           cDisplayEncoder::EncodeInline(mLocalKeyFile).c_str());

        // backup current file if it exists
        cFileUtil::BackupFile(mLocalKeyFile);

        if (GenerateKey(mLocalKeyFile.c_str(), mLocalPassphrase, mKeySize) == false)
            return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cTWAModeChangePassphrases

static bool ChangePassphrase(const TCHAR* keyPath, wc16_string passphraseOld, wc16_string passphrase)
{
    // in order to leave original password data intact upon return,
    // we must allocate a BIGENDIAN copy
    // and delete it before we return.
    // auto_ptr does not help (won't work with arrays).
    size_t  passphraseLenOld  = passphraseOld.length() * sizeof(WCHAR16);
    size_t  passphraseLen     = passphrase.length() * sizeof(WCHAR16);
    int8_t* passphraseCopyOld = new int8_t[passphraseLenOld];
    int8_t* passphraseCopy    = new int8_t[passphraseLen];
#ifdef WORDS_BIGENDIAN
    memcpy(passphraseCopyOld, passphraseOld.data(), passphraseLenOld);
    memcpy(passphraseCopy, passphrase.data(), passphraseLen);
#else

 #if SWAB_TAKES_CHAR_PTRS   
    swab((char*)passphraseOld.data(), (char*)passphraseCopyOld, passphraseLenOld);
    swab((char*)passphrase.data(), (char*)passphraseCopy, passphraseLen);
#else
    swab(passphraseOld.data(), passphraseCopyOld, passphraseLenOld);
    swab(passphrase.data(), passphraseCopy, passphraseLen);
#endif
    
#endif

    bool result;
    try
    {
        cKeyFile keyfile;
        keyfile.ReadFile(keyPath);
        keyfile.ChangePassphrase(passphraseCopyOld, passphraseLenOld, passphraseCopy, passphraseLen);
        keyfile.WriteFile(keyPath);
        result = true;
    }
    catch (eKeyFile& e)
    {
        cTWUtil::PrintErrorMsg(e);
        result = false;
    }
    delete [] passphraseCopyOld;
    delete [] passphraseCopy;
    return result;
}

class cTWAModeChangePassphrases : public cTWAModeCommon
{
public:
    cTWAModeChangePassphrases();
    virtual ~cTWAModeChangePassphrases();

    virtual void    InitCmdLineParser(cCmdLineParser& parser);
    virtual bool    Init(const cConfigFile* cf, const cCmdLineParser& parser);
    virtual int     Execute(cErrorQueue* pQueue);
    virtual TSTRING GetModeUsage()
    {
        return TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CHANGE_PASSPHRASES);
    }
    virtual bool LoadConfigFile()
    {
        return false;
    }
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_CHANGE_PASSPHRASES;
    }

private:
    bool        mSiteProvidedOld; // A site passphrase-old has been passed.
    wc16_string mSitePassphraseOld;
    bool        mLocalProvidedOld; // A local passphrase-old has been passed.
    wc16_string mLocalPassphraseOld;
    bool        mSiteProvided; // A site passphrase has been passed.
    wc16_string mSitePassphrase;
    bool        mLocalProvided; // A local passphrase has been passed.
    wc16_string mLocalPassphrase;
    bool        mChangeSite;  // A Site key has been specified.
    bool        mChangeLocal; // A Local key has been specified.
};

cTWAModeChangePassphrases::cTWAModeChangePassphrases()
{
    mLocalProvidedOld = false;
    mSiteProvidedOld  = false;
    mLocalProvided    = false;
    mSiteProvided     = false;
    mChangeSite       = false;
    mChangeLocal      = false;
}

cTWAModeChangePassphrases::~cTWAModeChangePassphrases()
{
}

void cTWAModeChangePassphrases::InitCmdLineParser(cCmdLineParser& parser)
{
    InitCmdLineCommon(parser);

    parser.AddArg(cTWAdminCmdLine::MODE_CHANGE_PASSPHRASES,
                  TSTRING(_T("")),
                  TSTRING(_T("change-passphrases")),
                  cCmdLineParser::PARAM_NONE);
    parser.AddArg(
        cTWAdminCmdLine::SITE_KEY_FILE, TSTRING(_T("S")), TSTRING(_T("site-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCAL_KEY_FILE, TSTRING(_T("L")), TSTRING(_T("local-keyfile")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::SITEPASSPHRASE, TSTRING(_T("Q")), TSTRING(_T("site-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(
        cTWAdminCmdLine::LOCALPASSPHRASE, TSTRING(_T("P")), TSTRING(_T("local-passphrase")), cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::SITEPASSPHRASEOLD,
                  TSTRING(_T("")),
                  TSTRING(_T("site-passphrase-old")),
                  cCmdLineParser::PARAM_ONE);
    parser.AddArg(cTWAdminCmdLine::LOCALPASSPHRASEOLD,
                  TSTRING(_T("")),
                  TSTRING(_T("local-passphrase-old")),
                  cCmdLineParser::PARAM_ONE);
}

bool cTWAModeChangePassphrases::Init(const cConfigFile* cf, const cCmdLineParser& parser)
{
    TSTRING str;

    // Note: we don't use the config file for anything any more.

    FillOutCmdLineInfo(parser);

    cCmdLineIter iter(parser);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::SITEPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mSitePassphrase = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mSiteProvided   = true;
            break;
        case cTWAdminCmdLine::LOCALPASSPHRASE:
            ASSERT(iter.NumParams() == 1);
            mLocalPassphrase = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mLocalProvided   = true;
            break;
        case cTWAdminCmdLine::SITEPASSPHRASEOLD:
            ASSERT(iter.NumParams() == 1);
            mSitePassphraseOld = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mSiteProvidedOld   = true;
            break;
        case cTWAdminCmdLine::LOCALPASSPHRASEOLD:
            ASSERT(iter.NumParams() == 1);
            mLocalPassphraseOld = cStringUtil::TstrToWstr(iter.ParamAt(0));
            mLocalProvidedOld   = true;
            break;
        case cTWAdminCmdLine::SITE_KEY_FILE:
            mChangeSite = true;
            break;
        case cTWAdminCmdLine::LOCAL_KEY_FILE:
            mChangeLocal = true;
            break;
        }
    }

    if ((mSiteProvidedOld && mSiteProvided) || (mLocalProvidedOld && mLocalProvided))
        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           TSS_GetString(cTWAdmin, twadmin::STR_KEYGEN_VERBOSE_PASSPHRASES).c_str());

    return true;
}

int cTWAModeChangePassphrases::Execute(cErrorQueue* pQueue)
{
    int  i;
    bool bSiteProvided  = mSiteProvided || mSiteProvidedOld;
    bool bLocalProvided = mLocalProvided || mLocalProvidedOld;

    if (!mChangeSite && !mChangeLocal)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_KEYS_NOT_SPECIFIED)));
        return 1;
    }

    if ((mChangeSite && mSiteKeyFile.empty()) || (mChangeLocal && mLocalKeyFile.empty()))
    {
        // This case should not occur (it should be caught by Init() logic), but in case it does we will spit out
        // a generic error string.
        ASSERT(false);
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_KEYS_NOT_SPECIFIED)));
        return 1;
    }

    if (!mChangeSite && bSiteProvided)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_LONE_SITE_PASSPHRASE)));
        return 1;
    }

    if (!mChangeLocal && bLocalProvided)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_LONE_LOCAL_PASSPHRASE)));
        return 1;
    }

    if ((bSiteProvided && !mChangeSite) || (bLocalProvided && !mChangeLocal))
    {
        //This is an error, a passphrase should not be specified if the corresponding key has not been passed.
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_PASSPHRASE_NOKEY)));
        return 1;
    }

    if (mSiteKeyFile == mLocalKeyFile)
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_KEY_FILENAMES_IDENTICAL)));
        return 1;
    }

    if (mChangeSite && !cFileUtil::FileExists(mSiteKeyFile))
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_SITE_KEY_DOESNT_EXIST)));
        return 1;
    }

    if (mChangeLocal && !cFileUtil::FileExists(mLocalKeyFile))
    {
        cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTWAdmin, twadmin::STR_ERR2_LOCAL_KEY_DOESNT_EXIST)));
        return 1;
    }

    //
    // make sure that we can write to both of the files before we continue
    //
    if (mChangeLocal)
        cFileUtil::TestFileWritable(mLocalKeyFile);
    if (mChangeSite)
        cFileUtil::TestFileWritable(mSiteKeyFile);

    wc16_string verify;

    bool bPrintedPassphraseHint = false;

    if (mChangeSite)
    {
        if (!mSiteProvidedOld)
        {
            cTWUtil::NoEcho noEcho;
            // Note: Since we are interacting directly with the user here,
            // I think it is o.k. to use cout directly.
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_ENTER_SITE_PASS_OLD);
            cTWUtil::GetString(mSitePassphraseOld);
            TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);
        }
        if (!mSiteProvided)
        {
            // give the user a passphrase hint...
            //
            if (!bPrintedPassphraseHint)
            {
                bPrintedPassphraseHint = true;
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   TSS_GetString(cTWAdmin, twadmin::STR_PASSPHRASE_HINT).c_str());
            }

            for (i = 1;; i++)
            {
                cTWUtil::NoEcho noEcho;
                // Note: Since we are interacting directly with the user here,
                // I think it is o.k. to use cout directly.
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_ENTER_SITE_PASS);
                cTWUtil::GetString(mSitePassphrase);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_VERIFY_SITE_PASS);
                cTWUtil::GetString(verify);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                if (0 == verify.compare(mSitePassphrase))
                    break;

                TCOUT << TSS_GetString(cTW, tw::STR_ERR_WRONG_PASSPHRASE_SITE) << std::endl;

                if (i == 3)
                    return 1;
            }
        }

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           TSS_GetString(cTWAdmin, twadmin::STR_KEYGEN_VERBOSE_SITEKEY).c_str(),
                                           cDisplayEncoder::EncodeInline(mSiteKeyFile).c_str());

        // backup current file if it exists
        cFileUtil::BackupFile(mSiteKeyFile);

        if (ChangePassphrase(mSiteKeyFile.c_str(), mSitePassphraseOld, mSitePassphrase) == false)
            return 1;
    }

    if (mChangeLocal)
    {
        if (!mLocalProvidedOld)
        {
            cTWUtil::NoEcho noEcho;
            // Note: Since we are interacting directly with the user here,
            // I think it is o.k. to use cout directly.
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_ENTER_LOCAL_PASS_OLD);
            cTWUtil::GetString(mLocalPassphraseOld);
            TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);
        }
        if (!mLocalProvided)
        {
            // give the user a passphrase hint...
            //
            if (!bPrintedPassphraseHint)
            {
                iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                                   TSS_GetString(cTWAdmin, twadmin::STR_PASSPHRASE_HINT).c_str());
            }

            for (i = 1;; i++)
            {
                cTWUtil::NoEcho noEcho;
                // Note: Since we are interacting directly with the user here,
                // I think it is o.k. to use cout directly.
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_ENTER_LOCAL_PASS);
                cTWUtil::GetString(mLocalPassphrase);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_VERIFY_LOCAL_PASS);
                cTWUtil::GetString(verify);
                TCOUT << TSS_GetString(cTW, tw::STR_NEWLINE);

                if (0 == verify.compare(mLocalPassphrase))
                    break;

                TCOUT << TSS_GetString(cTW, tw::STR_ERR_WRONG_PASSPHRASE_LOCAL) << std::endl;

                if (i == 3)
                    return 1;
            }
        }

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           TSS_GetString(cTWAdmin, twadmin::STR_KEYGEN_VERBOSE_LOCALKEY).c_str(),
                                           cDisplayEncoder::EncodeInline(mLocalKeyFile).c_str());

        // backup current file if it exists
        cFileUtil::BackupFile(mLocalKeyFile);

        if (ChangePassphrase(mLocalKeyFile.c_str(), mLocalPassphraseOld, mLocalPassphrase) == false)
            return 1;
    }

    return 0;
}


//#############################################################################
// cTWAModeHelp : A mode for supplying mode specific usage statements
//#############################################################################

class cTWAModeHelp : public iTWAMode
{
public:
    cTWAModeHelp()
    {
    }
    virtual ~cTWAModeHelp()
    {
    }

    virtual void                         InitCmdLineParser(cCmdLineParser& cmdLine);
    virtual bool                         Init(const cConfigFile* cf, const cCmdLineParser& cmdLine);
    virtual int                          Execute(cErrorQueue* pQueue);
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID(void) const;
    virtual TSTRING                      GetModeUsage()
    {
        return _T("");
    } //TODO: dra Output something here?

private:
    // A list of modes to output usage statements for:
    std::set<TSTRING> mModes;
    std::set<TSTRING> mPrinted;
};

///////////////////////////////////////////////////////////////////////////////
// GetModeID
///////////////////////////////////////////////////////////////////////////////
cTWAdminCmdLine::CmdLineArgs cTWAModeHelp::GetModeID(void) const
{
    return cTWAdminCmdLine::MODE_HELP;
}

///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser
///////////////////////////////////////////////////////////////////////////////
void cTWAModeHelp::InitCmdLineParser(cCmdLineParser& cmdLine)
{
    // We're only interested in one parameter, that being help.  Anything else
    // passed to this mode should be a cmdlineparser error.
    cmdLine.AddArg(cTWAdminCmdLine::MODE, TSTRING(_T("m")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY, true);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_HELP_ALL, TSTRING(_T("")), TSTRING(_T("all")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_CREATE_CONFIG,
                   TSTRING(_T("F")),
                   TSTRING(_T("create-cfgfile")),
                   cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(
        cTWAdminCmdLine::MODE_PRINT_CONFIG, TSTRING(_T("f")), TSTRING(_T("print-cfgfile")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_CREATE_POLICY,
                   TSTRING(_T("P")),
                   TSTRING(_T("create-polfile")),
                   cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(
        cTWAdminCmdLine::MODE_PRINT_POLICY, TSTRING(_T("p")), TSTRING(_T("print-polfile")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_REMOVE_ENCRYPTION,
                   TSTRING(_T("R")),
                   TSTRING(_T("remove-encryption")),
                   cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_ENCRYPT, TSTRING(_T("E")), TSTRING(_T("encrypt")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_EXAMINE, TSTRING(_T("e")), TSTRING(_T("examine")), cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_GENERATE_KEYS,
                   TSTRING(_T("G")),
                   TSTRING(_T("generate-keys")),
                   cCmdLineParser::PARAM_MANY);
    cmdLine.AddArg(cTWAdminCmdLine::MODE_CHANGE_PASSPHRASES,
                   TSTRING(_T("C")),
                   TSTRING(_T("change-passphrases")),
                   cCmdLineParser::PARAM_MANY);
}

///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////
bool cTWAModeHelp::Init(const cConfigFile* cf, const cCmdLineParser& cmdLine)
{
    cCmdLineIter iter(cmdLine);

    // Grab the arguments from the help parameter:
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case cTWAdminCmdLine::MODE_HELP:
        {
            int i;
            for (i = 0; i < iter.NumParams(); ++i)
            {
                mModes.insert(iter.ParamAt(i));
            }
        }
        break;
        case cTWAdminCmdLine::MODE:
        {
            int i;
            for (i = 0; i < iter.NumParams(); ++i)
            {
                mModes.insert(iter.ParamAt(i));
            }
        }
        break;
        case cTWAdminCmdLine::MODE_HELP_ALL:          //fall through
        case cTWAdminCmdLine::MODE_CREATE_CONFIG:     //fall through
        case cTWAdminCmdLine::MODE_PRINT_CONFIG:      //fall through
        case cTWAdminCmdLine::MODE_CREATE_POLICY:     //fall through
        case cTWAdminCmdLine::MODE_PRINT_POLICY:      //fall through
        case cTWAdminCmdLine::MODE_REMOVE_ENCRYPTION: //fall through
        case cTWAdminCmdLine::MODE_ENCRYPT:           //fall through
        case cTWAdminCmdLine::MODE_EXAMINE:           //fall through
        case cTWAdminCmdLine::MODE_GENERATE_KEYS:
        case cTWAdminCmdLine::MODE_CHANGE_PASSPHRASES:
        {
            int     i;
            TSTRING str = iter.ActualParam();
            // Kill off the initial "--" or "-"
            str.erase(0, 1);
            if (str.length() != 1)
                str.erase(0, 1);

            // push back the parameter that was actually passed.
            mModes.insert(str);
            // grab all the "parameters" following the mode/switch, since
            // they may be valid modes without the "--" prepended.
            for (i = 0; i < iter.NumParams(); ++i)
                mModes.insert(iter.ParamAt(i));
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
int cTWAModeHelp::Execute(cErrorQueue* pQueue)
{
    cDebug d("cTWAModeHelp::Execute");

    // The iterator we will use to traverse the list of arguments:
    std::set<TSTRING>::iterator it = mModes.begin();

    // We'll want to output the version information, regardless:
    TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_VERSION) << std::endl;
    TCOUT << TSS_GetString(cTW, tw::STR_VERSION) << std::endl;

    if (it == mModes.end()) // all that was passed was --help
    {
        // Output a shorter usage statement for each mode
        TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_USAGE_SUMMARY);
        //
        //That's it, return
        return 1;
    }

    for (; it != mModes.end(); ++it)
    {
        if (_tcscmp((*it).c_str(), _T("all")) == 0)
        {
            //Since --help all was passed, emit all help messages and return.
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CREATE_CFGFILE);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_PRINT_CFGFILE);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CREATE_POLFILE);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_PRINT_POLFILE);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_REMOVE_ENCRYPTION);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_ENCRYPT);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_EXAMINE);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_GENERATE_KEYS);
            TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CHANGE_PASSPHRASES);

            //We're done, return
            return 1;
        }
    }

    //We need some subset of the usage statements.  Figure out which modes have
    //been specified:
    it = mModes.begin();
    for (; it != mModes.end(); ++it)
    {
        if (_tcscmp((*it).c_str(), _T("create-cfgfile")) == 0 || _tcscmp((*it).c_str(), _T("F")) == 0)
        {
            //make sure we don't print the same help twice...
            if (mPrinted.find(_T("create-cfgfile")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CREATE_CFGFILE);
                mPrinted.insert(_T("create-cfgfile"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("print-cfgfile")) == 0 || _tcscmp((*it).c_str(), _T("f")) == 0)
        {
            if (mPrinted.find(_T("print-cfgfile")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_PRINT_CFGFILE);
                mPrinted.insert(_T("print-cfgfile"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("create-polfile")) == 0 || _tcscmp((*it).c_str(), _T("P")) == 0)
        {
            if (mPrinted.find(_T("create-polfile")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CREATE_POLFILE);
                mPrinted.insert(_T("create-polfile"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("print-polfile")) == 0 || _tcscmp((*it).c_str(), _T("p")) == 0)
        {
            if (mPrinted.find(_T("print-polfile")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_PRINT_POLFILE);
                mPrinted.insert(_T("print-polfile"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("remove-encryption")) == 0 || _tcscmp((*it).c_str(), _T("R")) == 0)
        {
            if (mPrinted.find(_T("remove-encryption")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_REMOVE_ENCRYPTION);
                mPrinted.insert(_T("remove-encryption"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("encrypt")) == 0 || _tcscmp((*it).c_str(), _T("E")) == 0)
        {
            if (mPrinted.find(_T("encrypt")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_ENCRYPT);
                mPrinted.insert(_T("encrypt"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("examine")) == 0 || _tcscmp((*it).c_str(), _T("e")) == 0)
        {
            if (mPrinted.find(_T("examine")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_EXAMINE);
                mPrinted.insert(_T("examine"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("generate-keys")) == 0 || _tcscmp((*it).c_str(), _T("G")) == 0)
        {
            if (mPrinted.find(_T("generate-keys")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_GENERATE_KEYS);
                mPrinted.insert(_T("generate-keys"));
            }
        }
        else if (_tcscmp((*it).c_str(), _T("change-passphrases")) == 0 || _tcscmp((*it).c_str(), _T("C")) == 0)
        {
            if (mPrinted.find(_T("change-passphrases")) == mPrinted.end())
            {
                TCOUT << TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_HELP_CHANGE_PASSPHRASES);
                mPrinted.insert(_T("change-passphrases"));
            }
        }
        else
        {
            cTWUtil::PrintErrorMsg(eTWAInvalidHelpMode((*it), eError::NON_FATAL));
            TCOUT << std::endl;
            // emit error string, this mode does not exist
        }
    }

    //Everything went okay
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Version Mode
///////////////////////////////////////////////////////////////////////////////
class cTWAModeVersion : public iTWAMode
{
public:
    cTWAModeVersion()
    {
    }
    virtual ~cTWAModeVersion()
    {
    }

    virtual void InitCmdLineParser(cCmdLineParser& cmdLine)
    {
        ASSERT(false);
    }
    virtual bool Init(const cConfigFile* cf, const cCmdLineParser& cmdLine)
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
    virtual cTWAdminCmdLine::CmdLineArgs GetModeID() const
    {
        return cTWAdminCmdLine::MODE_VERSION;
    }

private:
};


//#############################################################################

///////////////////////////////////////////////////////////////////////////////
// cTWAdminCmdLine::GetMode()
//

iTWAMode* cTWAdminCmdLine::GetMode(int argc, const TCHAR* const* argv)
{
    // note -- it is assumed the executable name is the first parameter
    if (argc < 2)
    {
        iUserNotify::GetInstance()->Notify(
            iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_VERSION).c_str());
        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
        iUserNotify::GetInstance()->Notify(
            iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTW, tw::STR_VERSION).c_str());
        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
        iUserNotify::GetInstance()->Notify(
            iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTW, tw::STR_GET_HELP).c_str());
        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
        return NULL;
    }

    int          mode = MODE;
    const TCHAR* pcMode;
    if (_tcscmp(argv[1], _T("-m")) == 0)
    {
        if (argc < 3)
        {
            iUserNotify::GetInstance()->Notify(
                iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTWAdmin, twadmin::STR_TWADMIN_VERSION).c_str());
            iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
            iUserNotify::GetInstance()->Notify(
                iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTW, tw::STR_VERSION).c_str());
            iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());

            cTWUtil::PrintErrorMsg(eBadCmdLine(TSS_GetString(cTW, tw::STR_ERR_NO_MODE), eError::SUPRESS_THIRD_MSG));

            iUserNotify::GetInstance()->Notify(
                iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTW, tw::STR_GET_HELP).c_str());
            iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());

            return NULL;
        }
        pcMode = argv[2];
        if (_tcscmp(argv[2], _T("F")) == 0)
            mode = MODE_CREATE_CONFIG;
        else if (_tcscmp(argv[2], _T("f")) == 0)
            mode = MODE_PRINT_CONFIG;
        else if (_tcscmp(argv[2], _T("P")) == 0)
            mode = MODE_CREATE_POLICY;
        else if (_tcscmp(argv[2], _T("p")) == 0)
            mode = MODE_PRINT_POLICY;
        else if (_tcscmp(argv[2], _T("R")) == 0)
            mode = MODE_REMOVE_ENCRYPTION;
        else if (_tcscmp(argv[2], _T("E")) == 0)
            mode = MODE_ENCRYPT;
        else if (_tcscmp(argv[2], _T("e")) == 0)
            mode = MODE_EXAMINE;
        else if (_tcscmp(argv[2], _T("G")) == 0)
            mode = MODE_GENERATE_KEYS;
        else if (_tcscmp(argv[2], _T("C")) == 0)
            mode = MODE_CHANGE_PASSPHRASES;
    }
    else
    {
        pcMode = argv[1];
        if (_tcscmp(argv[1], _T("--create-cfgfile")) == 0)
            mode = MODE_CREATE_CONFIG;
        else if (_tcscmp(argv[1], _T("--print-cfgfile")) == 0)
            mode = MODE_PRINT_CONFIG;
        else if (_tcscmp(argv[1], _T("--create-polfile")) == 0)
            mode = MODE_CREATE_POLICY;
        else if (_tcscmp(argv[1], _T("--print-polfile")) == 0)
            mode = MODE_PRINT_POLICY;
        else if (_tcscmp(argv[1], _T("--remove-encryption")) == 0)
            mode = MODE_REMOVE_ENCRYPTION;
        else if (_tcscmp(argv[1], _T("--encrypt")) == 0)
            mode = MODE_ENCRYPT;
        else if (_tcscmp(argv[1], _T("--examine")) == 0)
            mode = MODE_EXAMINE;
        else if (_tcscmp(argv[1], _T("--generate-keys")) == 0)
            mode = MODE_GENERATE_KEYS;
        else if (_tcscmp(argv[1], _T("--change-passphrases")) == 0)
            mode = MODE_CHANGE_PASSPHRASES;
        else if (_tcscmp(argv[1], _T("--version")) == 0)
            mode = MODE_VERSION;
    }

    // check for --help flag anywhere in the args
    int i;
    for (i = 1; i < argc; ++i)
        if (_tcscmp(argv[i], _T("--help")) == 0 || _tcscmp(argv[i], _T("-?")) == 0)
        {
            mode = MODE_HELP;
            break;
        }

    if (mode == MODE)
    {
        // unknown mode switch
        cDebug d("cTWAdminCmdLine::GetMode");
        d.TraceError("Error: Bad mode switch: %s\n", pcMode);

        TSTRING estr = TSS_GetString(cTW, tw::STR_ERR2_BAD_MODE_SWITCH);
        estr.append(pcMode);
        cTWUtil::PrintErrorMsg(eBadModeSwitch(estr, eError::SUPRESS_THIRD_MSG));

        iUserNotify::GetInstance()->Notify(
            iUserNotify::V_SILENT, _T("%s"), TSS_GetString(cTW, tw::STR_GET_HELP).c_str());
        iUserNotify::GetInstance()->Notify(iUserNotify::V_SILENT, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());
        return NULL;
    }


    iTWAMode* pRtn = 0;
    switch (mode)
    {
    case MODE_CREATE_CONFIG:
        pRtn = new cTWAModeCreateCfg;
        break;
    case MODE_CREATE_POLICY:
        pRtn = new cTWAModeCreatePol;
        break;
    case MODE_PRINT_CONFIG:
        pRtn = new cTWAModePrintCfg;
        break;
    case MODE_PRINT_POLICY:
        pRtn = new cTWAModePrintPol;
        break;
    case MODE_REMOVE_ENCRYPTION:
        pRtn = new cTWAModeRemoveEncryption;
        break;
    case MODE_ENCRYPT:
        pRtn = new cTWAModeEncrypt;
        break;
    case MODE_EXAMINE:
        pRtn = new cTWAModeExamine;
        break;
    case MODE_GENERATE_KEYS:
        pRtn = new cTWAModeGenerateKeys;
        break;
    case MODE_CHANGE_PASSPHRASES:
        pRtn = new cTWAModeChangePassphrases;
        break;
    case MODE_HELP:
        pRtn = new cTWAModeHelp;
        break;
    case MODE_VERSION:
        pRtn = new cTWAModeVersion;
        break;
    default:
        ASSERT(false);
    }

    return pRtn;
}

// Calls UserNotify(V_VERBOSE, ...) to print out type of file specified in cFileHeaderID.
// Returns false if cFileHeaderID not recognized.
// Used in changing and removing encryption algorithms
static bool NotifyFileType(const cFileHeaderID& id, uint32_t version, iUserNotify::VerboseLevel vl)
{
    if (id == cFCODatabaseFile::GetFileHeaderID())
    {
        iUserNotify::GetInstance()->Notify(vl,
                                           TSS_GetString(cTWAdmin, twadmin::STR_FILE_TYPE_DB).c_str(),
                                           (version & 0xff000000) >> 24,
                                           (version & 0x00ff0000) >> 16,
                                           (version & 0x0000ff00) >> 8,
                                           (version & 0x000000ff));
        return true;
    }
    else if (id == cFCOReport::GetFileHeaderID())
    {
        iUserNotify::GetInstance()->Notify(vl,
                                           TSS_GetString(cTWAdmin, twadmin::STR_FILE_TYPE_REP).c_str(),
                                           (version & 0xff000000) >> 24,
                                           (version & 0x00ff0000) >> 16,
                                           (version & 0x0000ff00) >> 8,
                                           (version & 0x000000ff));
        return true;
    }
    else if (id == cConfigFile::GetFileHeaderID())
    {
        iUserNotify::GetInstance()->Notify(vl,
                                           TSS_GetString(cTWAdmin, twadmin::STR_FILE_TYPE_CFG).c_str(),
                                           (version & 0xff000000) >> 24,
                                           (version & 0x00ff0000) >> 16,
                                           (version & 0x0000ff00) >> 8,
                                           (version & 0x000000ff));
        return true;
    }
    else if (id == cPolicyFile::GetFileHeaderID())
    {
        iUserNotify::GetInstance()->Notify(vl,
                                           TSS_GetString(cTWAdmin, twadmin::STR_FILE_TYPE_POL).c_str(),
                                           (version & 0xff000000) >> 24,
                                           (version & 0x00ff0000) >> 16,
                                           (version & 0x0000ff00) >> 8,
                                           (version & 0x000000ff));
        return true;
    }
    else if (id == cKeyFile::GetFileHeaderID())
    {
        iUserNotify::GetInstance()->Notify(vl,
                                           TSS_GetString(cTWAdmin, twadmin::STR_FILE_TYPE_KEY).c_str(),
                                           (version & 0xff000000) >> 24,
                                           (version & 0x00ff0000) >> 16,
                                           (version & 0x0000ff00) >> 8,
                                           (version & 0x000000ff));
        return true;
    }

    iUserNotify::GetInstance()->Notify(vl, TSS_GetString(cTWAdmin, twadmin::STR_FILE_TYPE_UNK).c_str());
    return false;
}

// Calls UserNotify(V_VERBOSE, ...) to print out type of file specified in cFileHeaderID.
// Returns false if encoding not recognized.
// Used in changing and removing encryption algorithms
static bool NotifyEncryptionType(cFileHeader::Encoding encoding, iUserNotify::VerboseLevel vl)
{
    switch (encoding)
    {
    case cFileHeader::NO_ENCODING:
        iUserNotify::GetInstance()->Notify(vl, TSS_GetString(cTWAdmin, twadmin::STR_ENCRYPT_TYPE_NONE).c_str());
        return true;
    case cFileHeader::COMPRESSED:
        iUserNotify::GetInstance()->Notify(vl, TSS_GetString(cTWAdmin, twadmin::STR_ENCRYPT_TYPE_COMP).c_str());
        return true;
    case cFileHeader::ASYM_ENCRYPTION:
        iUserNotify::GetInstance()->Notify(vl, TSS_GetString(cTWAdmin, twadmin::STR_ENCRYPT_TYPE_ASYM).c_str());
        return true;
    default:
        ASSERT("Unexpeced encryption type");
    }

    iUserNotify::GetInstance()->Notify(vl, TSS_GetString(cTWAdmin, twadmin::STR_ENCRYPT_TYPE_UNK).c_str());
    return false;
}
