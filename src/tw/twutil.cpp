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
// twutil.cpp
//

#include "stdtw.h"
#include "twutil.h"

#include "twcrypto/crypto.h"
#include "core/archive.h"
#include "core/serializerimpl.h"
#include "twcrypto/cryptoarchive.h"
#include "fcoreport.h"
#include "twcrypto/keyfile.h"
#include "fco/fcospeclist.h"
#include "fco/genreswitcher.h"
#include "core/errorbucketimpl.h"
#include "core/cmdlineparser.h"
#include "configfile.h"
#include "core/fileheader.h"
#include "core/usernotify.h"
#include "policyfile.h"
#include "core/serstring.h"
#include "headerinfo.h"
#include "systeminfo.h"
#include "core/errorgeneral.h"
#include "fco/parsergenreutil.h"
#include "fco/twfactory.h"
#include "fco/fconame.h"
#include "fcodatabasefile.h"
#include "util/fileutil.h"
#include "core/stringutil.h"
#include "tw/twstrings.h"
#include "tw/twerrors.h"
#include "core/ntmbs.h"
#include "core/displayencoder.h"
#include "core/tw_signal.h"

#ifdef TW_PROFILE
#include "core/tasktimer.h"
#endif

#include <unistd.h>
#include <fcntl.h>
#if SUPPORTS_TERMIOS
#include <termios.h>
#include <sys/ioctl.h>
int _getch(void);
#endif


// constants
static const char*    POLICY_FILE_MAGIC_8BYTE = "#POLTXT\n";
static const char*    CONFIG_FILE_MAGIC_8BYTE = "#CFGTXT\n";
static const uint32_t CURRENT_FIXED_VERSION   = 0x02020000;


///////////////////////////////////////////////////////////////////////////////
// WriteObjectToArchive -- called from WriteObject, does most of the work
///////////////////////////////////////////////////////////////////////////////
static void WriteObjectToArchive(cArchive&                 arch,
                                 const TCHAR*              filename, // filename is used only for issuing error messages
                                 const iTypedSerializable* pObjHeader,
                                 const iTypedSerializable& obj,
                                 cFileHeader&              fileHeader,
                                 bool                      bEncrypt,
                                 const cElGamalSigPrivateKey* pPrivateKey)
{
    try
    {
        // Set file version.
        // If we in the future we wish to support reading databases of different versions,
        // we will have to move this set to outside WriteObject().
        fileHeader.SetVersion(CURRENT_FIXED_VERSION);

        fileHeader.SetEncoding(bEncrypt ? cFileHeader::ASYM_ENCRYPTION : cFileHeader::COMPRESSED);

        {
            cSerializerImpl fhSer(arch, cSerializerImpl::S_WRITE, filename);
            fileHeader.Write(&fhSer);
        }

        if (bEncrypt)
        {
            cElGamalSigArchive cryptoArchive;
            cryptoArchive.SetWrite(&arch, pPrivateKey);
            cSerializerImpl ser(cryptoArchive, cSerializerImpl::S_WRITE, filename);
            ser.Init();
            if (pObjHeader)
                ser.WriteObject(pObjHeader);
            ser.WriteObject(&obj);
            ser.Finit();
            cryptoArchive.FlushWrite();
        }
        else
        {
            // not encrypted
            cNullCryptoArchive cryptoArchive;
            cryptoArchive.Start(&arch);
            cSerializerImpl ser(cryptoArchive, cSerializerImpl::S_WRITE, filename);
            ser.Init();
            if (pObjHeader)
                ser.WriteObject(pObjHeader);
            ser.WriteObject(&obj);
            ser.Finit();
            cryptoArchive.Finish();
        }
    }
    catch (eError& e)
    {
        throw ePoly(e.GetID(), cErrorUtil::MakeFileError(e.GetMsg(), filename), e.GetFlags());
    }
}

///////////////////////////////////////////////////////////////////////////////
// WriteObject -- template class that writes an object to disk (ie -- report,
//      db, etc) either encrypted or unencrypted.
//      The only requrement on the object is that it is typed serializable.
//      errorMsg is a number that can be passed to iUserString to indicate an
//      appropriate error message if the object fails to load
//
// 10/30 -- this function has been expanded to take two objects -- a header and
//      an object. Both are typed serializable, but the header can be NULL if
//      none is desired.
///////////////////////////////////////////////////////////////////////////////
static void WriteObject(const TCHAR*                 filename,
                        const iTypedSerializable*    pObjHeader,
                        const iTypedSerializable&    obj,
                        cFileHeader&                 fileHeader,
                        bool                         bEncrypt,
                        const cElGamalSigPrivateKey* pPrivateKey)
{
    cDebug d("WriteObject");
    d.TraceDebug(_T("Writing %s to file %s\n"), obj.GetType().AsString(), filename);

    ASSERT(pPrivateKey || (!bEncrypt));

    cFileArchive arch;

    if (!cFileUtil::IsRegularFile(filename) && cFileUtil::FileExists(filename))
        throw eArchiveNotRegularFile(filename);

    try
    {
        arch.OpenReadWrite(filename);
    }
    catch (eArchive&)
    {
        // probably better to rethrow this as a write failed exception
        throw eArchiveWrite(filename, iFSServices::GetInstance()->GetErrString());
    }

    WriteObjectToArchive(arch, filename, pObjHeader, obj, fileHeader, bEncrypt, pPrivateKey);

    arch.Close();
}


///////////////////////////////////////////////////////////////////////////////
// WriteObjectToArchive -- called from WriteObject, does most of the work
///////////////////////////////////////////////////////////////////////////////
static void ReadObjectFromArchive(cArchive&                   arch,
                                  const TCHAR*                objFileName,
                                  iTypedSerializable*         pObjHeader,
                                  iTypedSerializable&         obj,
                                  const cFileHeaderID&        fhid,
                                  const cElGamalSigPublicKey* pPublicKey,
                                  bool&                       bEncrypted)
{
    cFileHeader fileHeader;

    {
        cSerializerImpl fhSer(arch, cSerializerImpl::S_READ, objFileName);
        fileHeader.Read(&fhSer);
    }

    // check for a mismatched header
    if (fileHeader.GetID() != fhid)
        ThrowAndAssert(eSerializerInputStreamFmt(_T(""), objFileName, eSerializer::TY_FILE));

    // Check file version.
    // If we in the future we wish to support reading objects of different versions,
    // we will have to move this check to outside ReadObject().
    if (fileHeader.GetVersion() != CURRENT_FIXED_VERSION)
        ThrowAndAssert(eSerializerVersionMismatch(_T(""), objFileName, eSerializer::TY_FILE));

    try
    {
        // switch on the type of encoding...
        if (fileHeader.GetEncoding() == cFileHeader::ASYM_ENCRYPTION)
        {
            // tell the user the db is encrypted
            iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                               TSS_GetString(cTW, tw::STR_FILE_ENCRYPTED).c_str());
            bEncrypted = true;

            if (pPublicKey == 0)
                ThrowAndAssert(eSerializerEncryption(_T("")));

            cElGamalSigArchive cryptoArchive;
            cryptoArchive.SetRead(&arch, pPublicKey);

            cSerializerImpl ser(cryptoArchive, cSerializerImpl::S_READ, objFileName);
            ser.Init();
            if (pObjHeader)
                ser.ReadObject(pObjHeader);
            ser.ReadObject(&obj);
            ser.Finit();
        }
        else if (fileHeader.GetEncoding() == cFileHeader::COMPRESSED)
        {
            //not encrypted db...
            bEncrypted = false;

            cNullCryptoArchive cryptoArchive;
            cryptoArchive.Start(&arch);

            cSerializerImpl ser(cryptoArchive, cSerializerImpl::S_READ, objFileName);
            ser.Init();
            if (pObjHeader)
                ser.ReadObject(pObjHeader);
            ser.ReadObject(&obj);
            ser.Finit();
        }
        else
            // unknown encoding...
            ThrowAndAssert(eSerializerInputStreamFmt(_T("")));
    }
    catch (eError& e)
    {
        // include filename in error msg
        throw ePoly(e.GetID(), cErrorUtil::MakeFileError(e.GetMsg(), objFileName), e.GetFlags());
    }
}

///////////////////////////////////////////////////////////////////////////////
// ReadObject -- writes an object from disk, either encrypted or not, that was
//      written using WriteObject above.
///////////////////////////////////////////////////////////////////////////////
static void ReadObject(const TCHAR*                objFileName,
                       iTypedSerializable*         pObjHeader,
                       iTypedSerializable&         obj,
                       const cFileHeaderID&        fhid,
                       const cElGamalSigPublicKey* pPublicKey,
                       bool&                       bEncrypted)
{
    cDebug d("ReadObject");
    d.TraceDebug(_T("Reading %s from file %s\n"), obj.GetType().AsString(), objFileName);

    cFileArchive arch;
    arch.OpenRead(objFileName);

    ReadObjectFromArchive(arch, objFileName, pObjHeader, obj, fhid, pPublicKey, bEncrypted);
}

///////////////////////////////////////////////////////////////////////////////
// IsObjectEncrypted
///////////////////////////////////////////////////////////////////////////////
bool cTWUtil::IsObjectEncrypted(const TCHAR* objFileName, const cFileHeaderID& fhid, const TSTRING& errorMsg)
{
    bool   fEncrypted = false;
    cDebug d("IsObjectEncrypted");
    d.TraceDebug(_T("Reading from file %s\n"), objFileName);

    try
    {
        cFileArchive arch;
        arch.OpenRead(objFileName);

        cFileHeader     fileHeader;
        cSerializerImpl fhSer(arch, cSerializerImpl::S_READ, objFileName);
        fileHeader.Read(&fhSer);

        // check for a mismatched header
        if (fileHeader.GetID() != fhid)
            ThrowAndAssert(eSerializerInputStreamFmt(_T(""), objFileName, eSerializer::TY_FILE));

        // switch on the type of encoding...
        if (fileHeader.GetEncoding() == cFileHeader::ASYM_ENCRYPTION)
        {
            fEncrypted = true;
        }
        else if (fileHeader.GetEncoding() == cFileHeader::COMPRESSED)
        {
            fEncrypted = false;
        }
        else
            // unknown encoding...
            ThrowAndAssert(eSerializerInputStreamFmt(_T(""), objFileName, eSerializer::TY_FILE));
    }
    catch (eArchive& e)
    {
        // Note: Output to TCERR is O.K. here, it is documented that this is what this function does
        TSTRING msg = e.GetMsg();
        if (!msg.empty())
            msg += _T("\n");
        msg += errorMsg;

        cTWUtil::PrintErrorMsg(ePoly(e.GetID(), msg, e.GetFlags()));
        ThrowAndAssert(ePoly());
    }
    catch (eSerializer& e)
    {
        // Note: Output to TCERR is O.K. here, it is documented that this is what this function does
        TSTRING msg = e.GetMsg();
        if (!msg.empty())
            msg += _T("\n");
        msg += errorMsg;

        cTWUtil::PrintErrorMsg(ePoly(e.GetID(), msg, e.GetFlags()));
        ThrowAndAssert(ePoly());
    }

    return (fEncrypted);
}


///////////////////////////////////////////////////////////////////////////////
// WriteDatabase
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::WriteDatabase(const TCHAR*                 filename,
                            cFCODatabaseFile&            db,
                            bool                         bEncrypt,
                            const cElGamalSigPrivateKey* pPrivateKey)
{
    cFileHeader fileHeader;
    fileHeader.SetID(db.GetFileHeaderID());

    // I am almost positive that this does nothing, WriteObject() sets the version in the cFileHeader - Jun 8, 1999 - dmb
    //fileHeader.SetVersion(1);

#ifdef TW_PROFILE
    cTaskTimer timer(_T("Write Database"));
    timer.Start();
#endif

    WriteObject(filename, 0, db, fileHeader, bEncrypt, pPrivateKey);

#ifdef TW_PROFILE
    timer.Stop();
#endif

    iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_WRITE_DB_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(filename).c_str());
}

///////////////////////////////////////////////////////////////////////////////
// ReadDatabase
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::ReadDatabase(const TCHAR*                filename,
                           cFCODatabaseFile&           db,
                           const cElGamalSigPublicKey* pPublicKey,
                           bool&                       bEncrypted)
{
    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_OPEN_DB_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(filename).c_str());

#ifdef TW_PROFILE
    cTaskTimer timer("cTWUtil::ReadDatabase");
    timer.Start();
#endif

    ReadObject(filename, 0, db, cFCODatabaseFile::GetFileHeaderID(), pPublicKey, bEncrypted);

#ifdef TW_PROFILE
    timer.Stop();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// WriteReport
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::WriteReport(const TCHAR*                 filename,
                          const cFCOReportHeader&      reportHeader,
                          const cFCOReport&            r,
                          bool                         bEncrypt,
                          const cElGamalSigPrivateKey* pPrivateKey)
{
    cFileHeader fileHeader;
    fileHeader.SetID(cFCOReport::GetFileHeaderID());

    WriteObject(filename, &reportHeader, r, fileHeader, bEncrypt, pPrivateKey);

    iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_WRITE_REPORT_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(filename).c_str());
}


///////////////////////////////////////////////////////////////////////////////
// ReadReport
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::ReadReport(const TCHAR*                reportFileName,
                         cFCOReportHeader&           reportHeader,
                         cFCOReport&                 r,
                         const cElGamalSigPublicKey* pPublicKey,
                         bool                        silent,
                         bool&                       bEncrypted)
{
    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_OPEN_REPORT_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(reportFileName).c_str());

    ReadObject(reportFileName, &reportHeader, r, cFCOReport::GetFileHeaderID(), pPublicKey, bEncrypted);
}


///////////////////////////////////////////////////////////////////////////////
// UpdatePolicyFile
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::UpdatePolicyFile(const TCHAR*                 destFileName,
                               const TCHAR*                 srcFileName,
                               bool                         bEncrypt,
                               const cElGamalSigPrivateKey* pPrivateKey)
{
    cFileArchive src;
    src.OpenRead(srcFileName); // note: eArchive may float up

    std::string policyText;
    policyText.resize(src.Length());

    src.ReadBlob((void*)policyText.data(), src.Length());

    // NOTE -- this method will emit a message to stdout when the file has been written
    cTWUtil::WritePolicyText(destFileName, policyText, bEncrypt, pPrivateKey);
}

TSTRING cTWUtil::GetCfgFilePath(const cCmdLineParser& cmdLine, int cfgFileID)
{

    TSTRING cfgFilePath;
    bool    fConfigOnCmdLine = false;

    // let's see if this was specified on the command line...
    cCmdLineIter iter(cmdLine);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        if (iter.ArgId() == cfgFileID)
        {
            ASSERT(iter.NumParams() > 0);
            cfgFilePath      = iter.ParamAt(0);
            fConfigOnCmdLine = true;
        }
    }

    // OK, now we need to figure out where to find the config file.
    // If the location was specified on the command line, get the full path to it.
    // otherwise, the location is the dir that this exe is in.
    if (fConfigOnCmdLine)
    {
        TSTRING pathOut;
        iFSServices::GetInstance()->FullPath(pathOut, cfgFilePath);
        cfgFilePath = pathOut;
    }
    else
    {
        iFSServices::GetInstance()->FullPath(
            cfgFilePath, TSS_GetString(cTW, tw::STR_DEF_CFG_FILENAME), cSystemInfo::GetExeDir());
    }


    cFileUtil::TestFileExists(cfgFilePath);


    return (cfgFilePath);
}

///////////////////////////////////////////////////////////////////////////////
// OpenConfigFile -- opens the config file, either from a known location or
//      from the location specified on the command line. Returns false if it fails.
//      cfgFileID is the ID of the config file switch
//      TODO -- should this guy spit the error to cerr or fill up an error bucket?
/////////////////////////////////////////////////////////////////////////////////
void cTWUtil::OpenConfigFile(
    cConfigFile& cf, cCmdLineParser& cmdLine, int cfgFileID, cErrorBucket& errorBucket, TSTRING& configFileOut)
{

    configFileOut = GetCfgFilePath(cmdLine, cfgFileID);

    // ok, now let's load it!
    TSTRING configText;
    cTWUtil::ReadConfigText(configFileOut.c_str(), configText);
    cf.ReadString(configText);
}

///////////////////////////////////////////////////////////////////////////////
// WriteConfigText
//      eArchive is thrown if filename can not be opened
//      eSerializer is thrown if reading or writing fails

void cTWUtil::WriteConfigText(const TCHAR*                 filename,
                              const TSTRING                configText,
                              bool                         bEncrypt,
                              const cElGamalSigPrivateKey* pPrivateKey)
{
    cSerializableNString nstring;

    nstring.mString = CONFIG_FILE_MAGIC_8BYTE;

    std::string ns;
    cStringUtil::Convert(ns, configText);
    nstring.mString += ns;

    cFileHeader fileHeader;
    fileHeader.SetID(cConfigFile::GetFileHeaderID());

    fileHeader.SetVersion(CURRENT_FIXED_VERSION);

    if (bEncrypt)
    {
        ASSERT(pPrivateKey != 0);
        cElGamalSigPublicKey publicKey(*pPrivateKey);
        fileHeader.GetBaggage().MapArchive(0, publicKey.GetWriteLen());
        publicKey.Write(fileHeader.GetBaggage().GetMap());
    }

    WriteObject(filename, NULL, nstring, fileHeader, bEncrypt, pPrivateKey);

    iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_WRITE_CONFIG_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(filename).c_str());
}


///////////////////////////////////////////////////////////////////////////////
// ReadConfigText
//      eArchive is thrown if filename can not be opened
//      eSerializer is thrown if reading or writing fails
//      eConfigFile is thrown if config file does not parse correctly during reading

void cTWUtil::ReadConfigText(const TCHAR* filename, TSTRING& configText, cArchive* pBaggage)
{
    // TODO -- neat up this function; try to use LoadObject() above...

    cSerializableNString nstring;

    // This was coppied from ReadObject().  We need to use the baggage of the
    // file header to obtain the public key, thus the special casing.
    cDebug d("ReadConfigText");
    d.TraceDebug(_T("Reading %s from file %s\n"), nstring.GetType().AsString(), filename);

    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_OPEN_CONFIG_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(filename).c_str());

    cFileArchive arch;
    arch.OpenRead(filename);

    cFileHeader fileHeader;

    try
    {
        cSerializerImpl fhSer(arch, cSerializerImpl::S_READ);
        fileHeader.Read(&fhSer);
    }
    catch (eError&)
    {
        throw eSerializerInputStreamFmt(_T(""), filename, eSerializer::TY_FILE);
    }

#if 0 // XXX: This is broken, what the h*ll are they trying to write here? -PH
    d.TraceDebug("Found a file header of type %d.\n", fileHeader.GetEncoding());
#endif

    // check for a mismatched header
    if (fileHeader.GetID() != cConfigFile::GetFileHeaderID())
        throw eSerializerInputStreamFmt(_T(""), filename, eSerializer::TY_FILE);

    // check the version
    if (fileHeader.GetVersion() != CURRENT_FIXED_VERSION)
        throw eSerializerVersionMismatch(_T(""), filename, eSerializer::TY_FILE);

    // switch on the type of encoding...
    if (fileHeader.GetEncoding() == cFileHeader::ASYM_ENCRYPTION)
    {
        d.TraceDebug("Config file is compressed, public key len %d.\n", fileHeader.GetBaggage().Length());

        // tell the user the db is encrypted
        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE, TSS_GetString(cTW, tw::STR_FILE_ENCRYPTED).c_str());
        iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE, TSS_GetString(cTW, tw::STR_NEWLINE).c_str());

        ASSERT(fileHeader.GetBaggage().Length() > 0);
        if (fileHeader.GetBaggage().Length() <= 0)
            ThrowAndAssert(eSerializerInputStreamFmt(_T(""), filename, eSerializer::TY_FILE));

        fileHeader.GetBaggage().MapArchive(0, fileHeader.GetBaggage().Length());

        cElGamalSigPublicKey publicKey(fileHeader.GetBaggage().GetMap());

        cElGamalSigArchive cryptoArchive;
        cryptoArchive.SetRead(&arch, &publicKey);

        cSerializerImpl ser(cryptoArchive, cSerializerImpl::S_READ);
        ser.Init();
        ser.ReadObject(&nstring);
        ser.Finit();

        // copy the baggage into the archive, if it was passed in
        // Note: We rely in VerifySiteKey that we only fill out pBaggage if
        // the config file is encrypted.
        //
        if (pBaggage)
        {
            fileHeader.GetBaggage().Seek(0, cBidirArchive::BEGINNING);
            pBaggage->Copy(&fileHeader.GetBaggage(), fileHeader.GetBaggage().Length());
        }
    }
    else if (fileHeader.GetEncoding() == cFileHeader::COMPRESSED)
    {
        d.TraceDebug("Config file is not compressed.\n");

        //not encrypted db...
        cNullCryptoArchive cryptoArchive;
        cryptoArchive.Start(&arch);

        cSerializerImpl ser(cryptoArchive, cSerializerImpl::S_READ);
        ser.Init();
        ser.ReadObject(&nstring);
        ser.Finit();
    }
    else
        // unknown encoding...
        throw eSerializerInputStreamFmt(_T(""), filename, eSerializer::TY_FILE);

    // check 8 byte header
#if !ARCHAIC_STL    
    if (nstring.mString.compare(0, 8 * sizeof(uint8_t), CONFIG_FILE_MAGIC_8BYTE) != 0)
#else
    if (_tcsncmp(nstring.mString.c_str(), CONFIG_FILE_MAGIC_8BYTE,  8 * sizeof(uint8_t)) != 0) 
#endif      
        ThrowAndAssert(eSerializerInputStreamFmt(_T(""), filename, eSerializer::TY_FILE));

    // remove 8 byte header
    nstring.mString.assign(nstring.mString.substr(8));

    cStringUtil::Convert(configText, nstring.mString);
}


///////////////////////////////////////////////////////////////////////////////
// Given a filename and the text of a policy file, write an encrypted version
// of the policy file text to disk.
// Will throw eError on failure.

void cTWUtil::WritePolicyText(const TCHAR*                 filename,
                              const std::string&           polText,
                              bool                         bEncrypt,
                              const cElGamalSigPrivateKey* pPrivateKey)
{
    cSerializableNString nstring;

    // add a 8 byte header we can use to verify decryption
    nstring.mString = POLICY_FILE_MAGIC_8BYTE;

    nstring.mString += polText;

    cFileHeader fileHeader;
    fileHeader.SetID(cPolicyFile::GetFileHeaderID());

    WriteObject(filename, NULL, nstring, fileHeader, bEncrypt, pPrivateKey);

    iUserNotify::GetInstance()->Notify(iUserNotify::V_NORMAL,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_WRITE_POLICY_FILE).c_str(),
                                       cDisplayEncoder::EncodeInline(filename).c_str());
}


///////////////////////////////////////////////////////////////////////////////
// ReadPolicyText
//
// Read the policy file.  Read the text of a policy language into configText
// Will throw eError on failure.
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::ReadPolicyText(const TCHAR* filename, std::string& polText, const cElGamalSigPublicKey* pPublicKey)
{
    cSerializableNString nstring;

    cFileUtil::TestFileExists(filename);

    bool bEncrypted;
    ReadObject(filename, NULL, nstring, cPolicyFile::GetFileHeaderID(), pPublicKey, bEncrypted);

    // check 8 byte header
#if !ARCHAIC_STL    
    if (nstring.mString.compare(0, 8 * sizeof(uint8_t), POLICY_FILE_MAGIC_8BYTE) != 0)
#else
    if (_tcsncmp(nstring.mString.c_str(), POLICY_FILE_MAGIC_8BYTE, 8 * sizeof(uint8_t)) != 0)
#endif      
        ThrowAndAssert(eSerializerInputStreamFmt(_T(""), filename, eSerializer::TY_FILE));

    // remove 8 byte header
    nstring.mString.assign(nstring.mString.substr(8));

    polText = nstring.mString;
}


///////////////////////////////////////////////////////////////////////////////
// OpenKeyFile
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::OpenKeyFile(cKeyFile& keyFile, TSTRING fileName)
{
    // first, make sure the file exists...
    cFileUtil::TestFileExists(fileName);

    iUserNotify::GetInstance()->Notify(iUserNotify::V_VERBOSE,
                                       _T("%s%s\n"),
                                       TSS_GetString(cTW, tw::STR_OPEN_KEYFILE).c_str(),
                                       cDisplayEncoder::EncodeInline(fileName).c_str());

    keyFile.ReadFile(fileName.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// CreatePrivateKey
//      we will attempt to get the correct passphrase three times before we
//      give up.
///////////////////////////////////////////////////////////////////////////////
const cElGamalSigPrivateKey*
cTWUtil::CreatePrivateKey(cKeyFile& keyFile, const WCHAR16* usePassphrase, KeyType keyType, int nSecs)
{
    ASSERT(keyType == KEY_SITE || keyType == KEY_LOCAL || keyType == KEY_PROVIDED);

    const cElGamalSigPrivateKey* pPrivateKey = NULL;
    wc16_string                  passphrase;

    if (usePassphrase)
    {
        // sleep to hinder brute force (dictionary, etc.) attacks
        iFSServices::GetInstance()->Sleep(nSecs);

        passphrase = usePassphrase;

        TSS_SwapBytes(passphrase);

        pPrivateKey = keyFile.GetPrivateKey((int8_t*)passphrase.data(), passphrase.length() * sizeof(WCHAR16));

        if (pPrivateKey)
            return pPrivateKey;

        // if we got here, then a passphrase was provided on the command line that
        // was not correct; this is an error condition.
        //
        if (keyType == KEY_LOCAL)
            throw eTWUtilBadPassLocal();
        else
            throw eTWUtilBadPassSite();
    }

    int count = 0;
    while (count < 3)
    {
        cTWUtil::NoEcho noEcho;
        switch (keyType)
        {
        case KEY_LOCAL:
            TCOUT << TSS_GetString(cTW, tw::STR_ENTER_LOCAL_PASSPHRASE);
            break;
        case KEY_SITE:
            TCOUT << TSS_GetString(cTW, tw::STR_ENTER_SITE_PASSPHRASE);
            break;
        case KEY_PROVIDED:
        default:
            TCOUT << TSS_GetString(cTW, tw::STR_ENTER_PROVIDED_PASSPHRASE);
            break;
        }

        cTWUtil::GetString(passphrase);
        TCOUT << std::endl;

        // sleep to hinder brute force (dictionary, etc.) attacks
        iFSServices::GetInstance()->Sleep(nSecs);

        TSS_SwapBytes(passphrase);

        pPrivateKey = keyFile.GetPrivateKey((int8_t*)passphrase.data(), passphrase.length() * sizeof(WCHAR16));

        if (pPrivateKey)
            break;

        // tell the user that they entered the wrong passphrase
        int strId = (keyType == KEY_LOCAL) ? tw::STR_ERR_WRONG_PASSPHRASE_LOCAL : tw::STR_ERR_WRONG_PASSPHRASE_SITE;
        TCOUT << TSS_GetString(cTW, strId) << std::endl;
        passphrase.resize(0);
        count++;
    }

    if (!pPrivateKey)
        throw ePoly();

    return pPrivateKey;
}

void cTWUtil::CreatePrivateKey(
    cPrivateKeyProxy& proxy, cKeyFile& keyFile, const WCHAR16* usePassphrase, KeyType keyType, int nSecs)
{
    ASSERT(keyType == KEY_SITE || keyType == KEY_LOCAL || keyType == KEY_PROVIDED);

    wc16_string passphrase;

    if (usePassphrase != 0)
    {
        // sleep to hinder brute force (dictionary, etc.) attacks
        iFSServices::GetInstance()->Sleep(nSecs);

        passphrase = usePassphrase;

        TSS_SwapBytes(passphrase);

        if (proxy.AquireKey(keyFile, (int8_t*)passphrase.data(), passphrase.length() * sizeof(WCHAR16)))
            return;

        // if we got here, then a passphrase was provided on the command line that
        // was not correct; this is an error condition.
        //
        if (keyType == KEY_LOCAL)
            throw eTWUtilBadPassLocal();
        else
            throw eTWUtilBadPassSite();
    }

    int count = 0;
    while (count < 3)
    {
        cTWUtil::NoEcho noEcho;
        switch (keyType)
        {
        case KEY_LOCAL:
            TCOUT << TSS_GetString(cTW, tw::STR_ENTER_LOCAL_PASSPHRASE);
            break;
        case KEY_SITE:
            TCOUT << TSS_GetString(cTW, tw::STR_ENTER_SITE_PASSPHRASE);
            break;
        case KEY_PROVIDED:
        default:
            TCOUT << TSS_GetString(cTW, tw::STR_ENTER_PROVIDED_PASSPHRASE);
            break;
        }

        cTWUtil::GetString(passphrase);
        TCOUT << std::endl;

        // sleep to hinder brute force (dictionary, etc.) attacks
        iFSServices::GetInstance()->Sleep(nSecs);

        TSS_SwapBytes(passphrase);

        if (proxy.AquireKey(keyFile, (int8_t*)passphrase.data(), passphrase.length() * sizeof(WCHAR16)))
            return;

        // tell the user that they entered the wrong passphrase
        int strId = (keyType == KEY_LOCAL) ?
                        tw::STR_ERR_WRONG_PASSPHRASE_LOCAL :
                        (keyType == KEY_SITE) ? tw::STR_ERR_WRONG_PASSPHRASE_SITE :
                                                tw::STR_ERR_WRONG_PASSPHRASE_LOCAL; // TODO: make this provided

        TCOUT << TSS_GetString(cTW, strId) << std::endl;
        passphrase.resize(0);
        count++;
    }

    throw ePoly();
}

///////////////////////////////////////////////////////////////////////////////
// GetStringNoEcho -- Get a string from the user without echoing it
///////////////////////////////////////////////////////////////////////////////

static void (*old_SIGINT)(int);
static void (*old_SIGQUIT)(int);

#if SUPPORTS_TERMIOS
static struct termios Otty;
#endif


static void RestoreEcho(int sig)
{
#if SUPPORTS_TERMIOS
#    ifdef DEBUG
    std::cout << "Caught signal, resetting echo." << std::endl;
    sleep(2);
#    endif

    tcsetattr(0, TCSAFLUSH, &Otty);
    tw_signal(SIGINT, old_SIGINT);
    tw_signal(SIGQUIT, old_SIGQUIT);
#endif
    tw_raise(sig);
}

void cTWUtil::GetString(wc16_string& ret)
{
#ifdef _UNICODE
#    error We depend on Unix not being Unicode
#endif

// tcsetattr doesn't seem to work on AROS, so tweak ANSI terminal settings to hide passphrases.
#if IS_AROS
    printf("\e[8m"); // set the 'concealed' flag
    fflush(stdout);
#endif

    // get the string
    const int MAX_STRING = 1024;
    char      buf[MAX_STRING];
    int       len;

    TCOUT.flush();
    len = read(0, buf, MAX_STRING);

#if IS_AROS
    printf("\e[0m"); // reset back to normal text
    fflush(stdout);
#endif

    // TODO:BAM -- mb-enable this!
    if (len < MAX_STRING - 1)
        buf[len] = 0;

    char* psz = tss::strdec(buf, buf + len);
    if (*psz == '\n')
        *psz = 0;

    ret = cStringUtil::TstrToWstr(buf);
}


cTWUtil::NoEcho::NoEcho()
{
#if SUPPORTS_TERMIOS
    // set the terminal to no echo mode
    static struct termios Ntty;

    fflush(stdout);
    tcgetattr(0, &Otty);
    Ntty = Otty;

    Ntty.c_lflag &= ~ECHO;

    // catch SIGINT and SIGQUIT
    old_SIGINT  = tw_signal(SIGINT, RestoreEcho);
    old_SIGQUIT = tw_signal(SIGQUIT, RestoreEcho);

    if (tcsetattr(0, TCSAFLUSH, &Ntty) != 0 && isatty(0))
    {
        ThrowAndAssert(eTWUtilEchoModeSet());
    }
#endif
}

cTWUtil::NoEcho::~NoEcho()
{
#if SUPPORTS_TERMIOS
    tcsetattr(0, TCSAFLUSH, &Otty);
    tw_signal(SIGINT, old_SIGINT);
    tw_signal(SIGQUIT, old_SIGQUIT);
#endif
}

void cTWUtil::GetStringNoEcho(wc16_string& ret)
{
    NoEcho noEcho;
    GetString(ret);
}

///////////////////////////////////////////////////////////////////////////////
// ParseObjectList
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::ParseObjectList(cTWUtil::GenreObjList& listOut, const cTWUtil::ObjList& listIn)
{

    cGenre::Genre curGenre = cGenreSwitcher::GetInstance()->GetDefaultGenre();

    ASSERT(listIn.size() > 0);
    listOut.clear();
    GenreObjList::iterator curIter   = listOut.end();
    cGenre::Genre          iterGenre = cGenre::GENRE_INVALID;

    // iterate over all of the input...
    //
    for (ObjList::const_iterator i = listIn.begin(); i != listIn.end(); ++i)
    {
        // first, try to interperate the current string as a genre name...
        // 17 Mar 99 mdb -- we now only do this if the string ends in a ':'
        //
        cGenre::Genre g = cGenre::GENRE_INVALID;

        if (i->at(i->length() - 1) == _T(':'))
        {
            TSTRING genreStr;
            genreStr.assign(i->begin(), i->end() - 1);
            g = cGenreSwitcher::GetInstance()->StringToGenre(genreStr.c_str());

            //
            // if it is not a valid genre name, then test to see if it could
            // be an fconame.  If it is not, then it is a badly formed genre.
            //
            if (g == cGenre::GENRE_INVALID)
            {
                TW_UNIQUE_PTR<iParserGenreUtil> pParseUtil(iTWFactory::GetInstance()->CreateParserGenreUtil());
                if (!pParseUtil->IsAbsolutePath(*i))
                    throw eTWUnknownSectionName(*i);
            }
        }
        if (g == cGenre::GENRE_INVALID)
        {
            // assume that we are in the correct genre; this is a fully qualified object name
            // TODO -- throw here if cGenreParserHelper says it isn't a fully qualified name.
            //
            if (iterGenre != curGenre)
            {
                // seek to right list; create it if it is not there...
                //
                for (curIter = listOut.begin(); curIter != listOut.end(); ++curIter)
                {
                    if (curIter->first == curGenre)
                        break;
                }
                if (curIter == listOut.end())
                {
                    // it doesn't exist; we will have to create a new one.
                    //
                    listOut.push_back(GenreObjs());
                    listOut.back().first = curGenre;
                    curIter              = listOut.end() - 1;
                }

                iterGenre = curGenre;
            }
            ASSERT(curIter != listOut.end());
            //
            // add this to the list; assert that it has not been added yet.
            //
            ObjList::iterator oi;
            for (oi = curIter->second.begin(); oi != curIter->second.end(); ++oi)
            {
                if (*oi == *i)
                {
                    //TODO -- what should I do here? probably warn and continue...
                    ASSERT(false);
                    break;
                }
            }
            if (oi == curIter->second.end())
                curIter->second.push_back(*i);
        }
        else
        {
            // set the current genre to this and continue...
            //
            curGenre = g;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// ParseObjectName
///////////////////////////////////////////////////////////////////////////////
cFCOName cTWUtil::ParseObjectName(const TSTRING& fcoName)
{
    TW_UNIQUE_PTR<iParserGenreUtil> pParseUtil(iTWFactory::GetInstance()->CreateParserGenreUtil());
    cFCOName                        name(iTWFactory::GetInstance()->GetNameInfo());
    //
    // make sure the fco name is a full path...
    //
    if (!pParseUtil->IsAbsolutePath(fcoName))
        throw eTWUtilNotFullPath(fcoName);
    //
    // construct the list that InterpretFCOName needs....
    std::list<TSTRING> inputNameList;
    //
    // dice up the string into a list
    // currently, we only slice it up based on "|" (for nt registry entries)
    //
    TSTRING::size_type pos = fcoName.find_first_of(_T('|'));
    if (pos != TSTRING::npos)
    {
        // if the input string is "foo|bar" then we want the list to
        // look like this: "foo", "|", "bar" (three entries)
        //
        TSTRING str;
        str.assign(fcoName, 0, pos);
        inputNameList.push_back(str);
        inputNameList.push_back(_T("|"));
        str.assign(fcoName, pos + 1, fcoName.length() - pos);
        inputNameList.push_back(str);
    }
    else
    {
        inputNameList.push_back(fcoName);
    }

    pParseUtil->InterpretFCOName(inputNameList, name);

    return name;
}


///////////////////////////////////////////////////////////////////////////////
// Database and report header helpers
///////////////////////////////////////////////////////////////////////////////

TSTRING cTWUtil::GetSystemName()
{
    TSTRING ret;
    iFSServices::GetInstance()->GetMachineName(ret);
    return ret;
}

TSTRING cTWUtil::GetIPAddress()
{
    uint32_t ipaddress;
    if (iFSServices::GetInstance()->GetIPAddress(ipaddress) == false)
        return TSS_GetString(cTW, tw::STR_IP_UNKNOWN);

    // convert to host byte-order
    ipaddress = tw_ntohl(ipaddress);

    TOSTRINGSTREAM ret;
    ret << (int)(ipaddress >> 24) << _T(".") << (int)((ipaddress >> 16) & 0xff) << _T(".")
        << (int)((ipaddress >> 8) & 0xff) << _T(".") << (int)(ipaddress & 0xff);

    return ret.str();
}

TSTRING cTWUtil::GetCurrentUser()
{
    TSTRING ret;
    iFSServices::GetInstance()->GetCurrentUserName(ret);
    return ret;
}

TSTRING cTWUtil::GetHostID()
{
    TSTRING ret;
    iFSServices::GetInstance()->GetHostID(ret);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// PrintErrorMsg
///////////////////////////////////////////////////////////////////////////////
void cTWUtil::PrintErrorMsg(const eError& e, const TSTRING& strExtra)
{
    ASSERT(e.GetID() != 0); // NOTE: BAM  5/9/99 -- there should no longer be an ID of ZERO

    /* BAM 5/9/99 -- this is old stuff
    if((e.GetID() == 0) && (e.GetMsg().empty()))
    {
        // this should only occur at the top level of a program (ie -- twcmdline.cpp) and
        // indicates that an error occurred and an error message has already been printed out.
        // Therefore, we will do nothing here but return.
        return;
    }
    */

    cErrorReporter::PrintErrorMsg(e, strExtra);
}

///////////////////////////////////////////////////////////////////////////////
// ConfirmYN(const TCHAR* prompt)
//
// prompt the user with given string and wait for a 'Y' or 'N' response.
// returns true if 'Y' entered.
///////////////////////////////////////////////////////////////////////////////
bool cTWUtil::ConfirmYN(const TCHAR* prompt)
{
    wc16_string reply;
    TSTRING     s;
    int         x;

    while (1)
    {
        TCOUT << prompt;
        GetString(reply);

        cStringUtil::Convert(s, reply);

#ifdef UNICODE
        for (x = 0; s[x] && iswctype(s[x], wctype("space")); x++)
            ;
#else
#if !ARCHAIC_STL
        for (x = 0; s[x] && std::isspace<TCHAR>(s[x], std::locale()); x++)
#else
        for (x = 0; s[x] && isspace(s[x]); x++)
#endif	  
            ;
#endif

        if (_totupper(s[x]) == _T('Y'))
            return true;
        else if (_totupper(s[x]) == _T('N'))
            return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// VerifySiteKey
///////////////////////////////////////////////////////////////////////////////
bool cTWUtil::VerifyCfgSiteKey(const TSTRING& strConfigFile, const TSTRING& siteKeyPath)
{
    // open the config file
    //
    cMemoryArchive memArch;
    try
    {
        TSTRING dummyString;
        cTWUtil::ReadConfigText(strConfigFile.c_str(), dummyString, &memArch);
    }
    catch (eArchive& e)
    {
        // if the file is corrupted, eArchive may be thrown.
        // For sanity sake we need to re-throw this as eTWUtil
        TSTRING estr;
        estr.assign(TSS_GetString(cCore, core::STR_ERROR_FILENAME));
        estr.append(strConfigFile);
        estr.append(TSS_GetString(cCore, core::STR_NEWLINE));
        estr.append(e.GetMsg());

        throw eTWUtilCorruptedFile(estr);
    }

    // only do the test if there is baggage (indicating the cfg file is encrypted)
    //
    ASSERT(memArch.Length() >= 0);
    if (memArch.Length() <= 0)
        return false;

    // get site public key
    //
    cKeyFile siteKeyfile;
    cTWUtil::OpenKeyFile(siteKeyfile, siteKeyPath);

    // create the two public keys...
    //
    memArch.Seek(0, cBidirArchive::BEGINNING);
    cElGamalSigPublicKey pubKey(memArch.GetMemory());

    // compare the two ....
    //
    if (!pubKey.IsEqual(*siteKeyfile.GetPublicKey()))
    {
        TSTRING estr;
        estr.assign(TSS_GetString(cTW, tw::STR_ERR2_CFG_KEY_MISMATCH1));
        estr.append(strConfigFile);
        estr.append(TSS_GetString(cTW, tw::STR_ERR2_CFG_KEY_MISMATCH2));
        estr.append(siteKeyPath);
        estr.append(TSS_GetString(cTW, tw::STR_ERR2_CFG_KEY_MISMATCH3));

        throw eTWUtilCfgKeyMismatch(estr);
    }

    return true;
}
