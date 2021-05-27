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
// filemanipulator.cpp -- Provides functionality to examine tripwire file headers
//      for file type and encryption type.  Also provides a mechanism for
//      changing encryption keys or removing encryption.
//

#include "stdtw.h"
#include "filemanipulator.h"
#include "core/serializerimpl.h"
#include "twutil.h"
#include "fcodatabasefile.h"
#include "fcoreport.h"
#include "configfile.h"
#include "policyfile.h"
#include "headerinfo.h"
#include "core/usernotify.h"
#include "twcrypto/crypto.h"
#include "util/fileutil.h"
#include "tw/twstrings.h"
#include "core/displayencoder.h"

//=============================================================================
// class cFileManipulator
//=============================================================================

cFileManipulator::cFileManipulator(const TCHAR* filename)
{
    mbInit    = false;
    mFileName = filename;
}

cFileManipulator::cFileManipulator(const cFileManipulator& rhs)
    : mbInit(false), mFileName(rhs.mFileName), mFileHeader(rhs.mFileHeader)
{
}

cFileManipulator::~cFileManipulator()
{
}

void cFileManipulator::Init()
{
    if (cFileUtil::FileExists(mFileName))
    {
        cFileArchive arch;
        try
        {
            try
            {
                arch.OpenRead(mFileName.c_str());
            }
            catch (eArchive&)
            {
                throw eFileManipFileRead(mFileName);
            }

            cSerializerImpl s(arch, cSerializerImpl::S_READ);
            mFileHeader.Read(&s);
        }
        catch (eArchive&)
        {
            throw eFileManipHeaderNotFound(mFileName);
        }
        catch (eSerializer&)
        {
            throw eFileManipHeaderNotFound(mFileName);
        }
    }
    else
    {
        throw eFileManipFileNotFound(mFileName);
    }
    mbInit = true;
}

const cFileHeaderID* cFileManipulator::GetHeaderID()
{
    ASSERT(mbInit);
    if (!mbInit)
    {
        //      return NULL;
        Init();
    }
    return &mFileHeader.GetID();
}

uint32_t cFileManipulator::GetFileVersion()
{
    ASSERT(mbInit);
    if (!mbInit)
    {
        //      return 0;
        Init();
    }
    return mFileHeader.GetVersion();
}

cFileHeader::Encoding cFileManipulator::GetEncoding()
{
    ASSERT(mbInit);
    if (!mbInit)
    {
        //      return (cFileHeader::Encoding)0;
        Init();
    }
    return mFileHeader.GetEncoding();
}

// Try to decrypt the file using the given key. If thorough is true then
// the entire file is read into memory.  Returns true if decryption was
// successful.
bool cFileManipulator::TestDecryption(const cElGamalSigPublicKey& key, bool thorough)
{
    // TODO: pay attention to thorough flag.  For now we will just always act thoroughly.

    ASSERT(mbInit);
    if (!mbInit)
    {
        return false;
    }

    bool fError = false;

    // "turn down" the verbosity of iUserNotify to V_NORMAL it it is V_VERBOSE to make output a little cleaner
    bool fChangedVerbosity = false;
    int  savedVerbosity    = iUserNotify::GetInstance()->GetVerboseLevel();
    if (savedVerbosity == iUserNotify::V_VERBOSE)
    {
        iUserNotify::GetInstance()->SetVerboseLevel(iUserNotify::V_NORMAL);
        fChangedVerbosity = true;
    }

    try
    {
        if (mFileHeader.GetID() == cFCODatabaseFile::GetFileHeaderID())
        {
            cFCODatabaseFile db;
            bool             encrypted;
            cTWUtil::ReadDatabase(mFileName.c_str(), db, &key, encrypted);
        }
        else if (mFileHeader.GetID() == cFCOReport::GetFileHeaderID())
        {
            cFCOReport       rep;
            cFCOReportHeader reph;
            bool             encrypted;
            cTWUtil::ReadReport(mFileName.c_str(), reph, rep, &key, true, encrypted);
        }
        else if (mFileHeader.GetID() == cConfigFile::GetFileHeaderID())
        {
            // read the embedded key from config file and see if it is the same
            // as the public key passed in.
            cMemoryArchive memArch;
            TSTRING        configText; //not used
            cTWUtil::ReadConfigText(mFileName.c_str(), configText, &memArch);
            memArch.Seek(0, cBidirArchive::BEGINNING);

            // only do the test if there is baggage (indicating the cfg file is encrypted)
            if (memArch.Length() > 0)
            {
                // create the two public keys...
                cElGamalSigPublicKey pubKey(memArch.GetMemory());

                // compare the two ....
                if (!pubKey.IsEqual(key))
                    throw ePoly();
            }
        }
        else if (mFileHeader.GetID() == cPolicyFile::GetFileHeaderID())
        {
            std::string policyText;
            cTWUtil::ReadPolicyText(mFileName.c_str(), policyText, &key);
        }
        else
            throw ePoly();
    }
    catch (eError&)
    {
        fError = true;
    }

    // reset verbosity
    if (fChangedVerbosity)
        iUserNotify::GetInstance()->SetVerboseLevel(savedVerbosity);

    return (fError == false);
}

// Change the encryption on a file.
// If pNewKey is NULL, then encryption on the file will be removed.
// If pOldKey is NULL and the file is currently encrypted, then the
// function will throw eFileManip;
// If pOldKey is not NULL and the file is encrypted, then the fuction
// will throw eFileManip;
// If pOldKey is NULL and pNewKey is NULL and the file is not
// currently encrypted, the function will not do anything and will just
// return.
// Returns on successful encryption change.
void cFileManipulator::ChangeEncryption(const cElGamalSigPublicKey*  pOldKey,
                                        const cElGamalSigPrivateKey* pNewKey,
                                        bool                         backup)
{
    ASSERT(mbInit);
    if (!mbInit)
    {
        Init();
    }

    // check the pOldKey matches the current encryption state
    if (mFileHeader.GetEncoding() != cFileHeader::ASYM_ENCRYPTION)
    {
        if (pOldKey != NULL)
            throw eFileManipNotEncrypted(mFileName);
        if (pNewKey == NULL)
            return; // NOOP
    }
    else if (pOldKey == NULL)
        throw eFileManipMissingKey();

    if (!cFileUtil::FileWritable(mFileName))
        throw eFileManipNotWritable(mFileName);

    if (mFileHeader.GetID() == cFCODatabaseFile::GetFileHeaderID())
    {
        cFCODatabaseFile db;
        bool             encrypted;

        cTWUtil::ReadDatabase(mFileName.c_str(), db, pOldKey, encrypted);
        if (backup)
        {
            TSTRING backname = mFileName + iFSServices::GetInstance()->GetStandardBackupExtension();
            iFSServices::GetInstance()->Rename(mFileName.c_str(), backname.c_str());
        }

        cTWUtil::WriteDatabase(mFileName.c_str(), db, pNewKey != NULL, pNewKey);
        return;
    }
    else if (mFileHeader.GetID() == cFCOReport::GetFileHeaderID())
    {
        cFCOReport       rep;
        cFCOReportHeader reph;
        bool             encrypted;
        cTWUtil::ReadReport(mFileName.c_str(), reph, rep, pOldKey, false, encrypted);
        if (backup)
        {
            TSTRING backname = mFileName + iFSServices::GetInstance()->GetStandardBackupExtension();
            iFSServices::GetInstance()->Rename(mFileName.c_str(), backname.c_str());
        }
        cTWUtil::WriteReport(mFileName.c_str(), reph, rep, pNewKey != NULL, pNewKey);
        return;
    }
    else if (mFileHeader.GetID() == cConfigFile::GetFileHeaderID())
    {
        TSTRING configText;

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           _T("%s%s\n"),
                                           TSS_GetString(cTW, tw::STR_OPEN_CONFIG_FILE).c_str(),
                                           cDisplayEncoder::EncodeInline(mFileName).c_str());

        cTWUtil::ReadConfigText(mFileName.c_str(), configText);
        if (backup)
        {
            TSTRING backname = mFileName + iFSServices::GetInstance()->GetStandardBackupExtension();
            iFSServices::GetInstance()->Rename(mFileName.c_str(), backname.c_str());
        }

        cTWUtil::WriteConfigText(mFileName.c_str(), configText, pNewKey != NULL, pNewKey);

        return;
    }
    else if (mFileHeader.GetID() == cPolicyFile::GetFileHeaderID())
    {
        std::string policyText;

        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                           _T("%s%s\n"),
                                           TSS_GetString(cTW, tw::STR_OPEN_POLICY_FILE).c_str(),
                                           cDisplayEncoder::EncodeInline(mFileName).c_str());

        cTWUtil::ReadPolicyText(mFileName.c_str(), policyText, pOldKey);
        if (backup)
        {
            TSTRING backname = mFileName + iFSServices::GetInstance()->GetStandardBackupExtension();
            iFSServices::GetInstance()->Rename(mFileName.c_str(), backname.c_str());
        }

        cTWUtil::WritePolicyText(mFileName.c_str(), policyText, pNewKey != NULL, pNewKey);

        return;
    }
}

// method for getting a list of manipulators
// returns number of files matched
int cFileManipulator::LoadFiles(std::list<cFileManipulator>& ret, TSTRING mask)
{
    ret.clear();

    // TODO: we need to be able to deal with masks for the NT version.
    //  Right now *.* will not work in NT.  We should not get a '*' for unix.

    try
    {
        ret.push_back(cFileManipulator(mask.c_str()));
    }
    catch (eError&)
    {
        // file does not exist
        return 0;
    }

    return 1;
}

bool cFileManipulator::UseSiteKey(const cFileHeaderID& headerID)
{
    if (headerID == cFCODatabaseFile::GetFileHeaderID() || headerID == cFCOReport::GetFileHeaderID())
        return false;
    else if (headerID == cConfigFile::GetFileHeaderID() || headerID == cPolicyFile::GetFileHeaderID())
        return true;

    ASSERT(false); // what could it be?  be careful not to en/decrypt keyfiles.
    return false;
}
