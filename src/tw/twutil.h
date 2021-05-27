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
// twutil.h
//
// cTWUtil -- top-level utility functions that can be shared between multiple
//      executables

#ifndef __TWUTIL_H
#define __TWUTIL_H

#ifndef __TYPES_H
#include "core/types.h"
#endif

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __WCHAR16_H
#include "core/wchar16.h"
#endif

#ifndef __ERROR_H
#include "core/error.h"
#endif

#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif

class cFCOReport;
class cElGamalSigPrivateKey;
class cElGamalSigPublicKey;
class cKeyFile;
class cPrivateKeyProxy;
class cFCOSpecList;
class cConfigFile;
class cCmdLineParser;
class cErrorBucket;
class cFileHeaderID;
class eError;
class cFCODbHeader;
class cFCOReportHeader;
class cFCOName;
class cFCODatabaseFile;
class cArchive;
class cMemoryArchive;

TSS_EXCEPTION(eTWUtil, eError)
TSS_EXCEPTION(eTWUtilNotFullPath, eTWUtil)
TSS_EXCEPTION(eTWUtilCfgKeyMismatch, eTWUtil)
TSS_EXCEPTION(eTWUtilCorruptedFile, eTWUtil)
TSS_EXCEPTION(eTWUtilBadPassLocal, eTWUtil)
TSS_EXCEPTION(eTWUtilBadPassSite, eTWUtil)
TSS_EXCEPTION(eTWUtilEchoModeSet, eTWUtil)
TSS_EXCEPTION(eTWUtilDbDoesntHaveGenre, eTWUtil)
TSS_EXCEPTION(eTWUtilPolUnencrypted, eTWUtil)
TSS_EXCEPTION(eTWUtilObjNotInDb, eTWUtil)


class cTWUtil
{
public:
    //-------------------------------------------------------------------------
    // Reading and Writing tripwire file objects (database, report, config
    // and policy files).
    //-------------------------------------------------------------------------

    static void WriteDatabase(const TCHAR*                 filename,
                              cFCODatabaseFile&            db,
                              bool                         bEncrypt,
                              const cElGamalSigPrivateKey* pPrivateKey); // throw eError
    // writes a database to disk. If an error occurs, a message is spit to stderr and an eError is thrown.
    // if bEncrypt is false, then the key pointer may be false, otherwise, it must be non-null
    static void ReadDatabase(const TCHAR*                filename,
                             cFCODatabaseFile&           db,
                             const cElGamalSigPublicKey* pPublicKey,
                             bool&                       bEncrypted); // throw (eError);
    // reads a database from disk. If the db is encrypted, then it will load the named key file into keyFile and
    // set bEncrypted to true; otherwise keyFile is not modified and bEncrypted is set to false.
    // if keyFile is already open, then the currently loaded keys are used and keyFileName is ignored.
    // if an error occurs, this will print the error message to stderr and throw eError.

    static void WriteReport(const TCHAR*                 filename,
                            const cFCOReportHeader&      reportHeader,
                            const cFCOReport&            r,
                            bool                         bEncrypt,
                            const cElGamalSigPrivateKey* pPrivateKey); // throw eError

    static void ReadReport(const TCHAR*                reportFileName,
                           cFCOReportHeader&           reportHeader,
                           cFCOReport&                 r,
                           const cElGamalSigPublicKey* pPublicKey,
                           bool                        silent,
                           bool&                       bEncrypted); // throw eError

    // same as Read/WriteDatabase above, except it operates on reports
    // if an error occurs, this will print the error message to stderr and throw eError.

    static void WriteConfigText(const TCHAR*                 filename,
                                const TSTRING                configText,
                                bool                         bEncrypt,
                                const cElGamalSigPrivateKey* pPrivateKey); // throw (eArchive())

    static void ReadConfigText(const TCHAR* filename, TSTRING& configText, cArchive* pBaggage = 0);

    // read and write config file text to and from disk.
    // if pBaggage is non-NULL, the contents of the baggage( ie -- the public key ) is copied to the archive.
    // eArchive is thrown if filename can not be opened
    // eSerializer is thrown if reading or writing fails
    // eConfigFile is thrown if config file does not parse correctly during reading

    static void WritePolicyText(const TCHAR*                 filename,
                                const std::string&           policyText,
                                bool                         bEncrypt,
                                const cElGamalSigPrivateKey* pPrivateKey);

    static void ReadPolicyText(const TCHAR* filename, std::string& policyText, const cElGamalSigPublicKey* pPublicKey);

    // read and write policy file to and from disk
    // eError() will be thrown on error

    //-------------------------------------------------------------------------
    // Higher level manipulation of Tripwire file objects
    //-------------------------------------------------------------------------

    static TSTRING GetCfgFilePath(const cCmdLineParser& cmdLine, int cfgFileID);
    // Get's the path of the config file, either from the commandline, or from
    // the default location.

    static void OpenConfigFile(
        cConfigFile& cf, cCmdLineParser& cmdLine, int cfgFileID, cErrorBucket& errorBucket, TSTRING& configFileOut);
    // opens the config file, either from a known location or
    // from the location specified on the command line. Returns false if it fails
    // reads config info into cf
    // cfgFileID is the ID of the config file that cCmdLineParser::ArgId() will know about
    // if pBaggage is non-NULL, the contents of the baggage( ie -- the public key ) is copied to the archive.
    // errors will be reported to errorBucket
    // configFileOut returns actual config file used
    // returns false on failure

    static void UpdatePolicyFile(const TCHAR*                 destFileName,
                                 const TCHAR*                 srcFileName,
                                 bool                         bEncrypt,
                                 const cElGamalSigPrivateKey* pPrivateKey); // throw eError
    // saves the plain text policy file pointed at by srcFileName encrypted with pPrivateKey in destFileName
    // if an error occurs an exception suitable for printing with

    //-------------------------------------------------------------------------
    // key manipulation utilities
    //-------------------------------------------------------------------------

    static bool VerifyCfgSiteKey(const TSTRING& mstrConfigFile, const TSTRING& siteKeyPath); //throw (eTWUtil)
        // This method verifies that either the config file is unencrypted or the public key in the
        // config file mstrConfigFile matches the public key contained in the file specified by siteKeyPath.
        // Returns true if the keys match or false if the config file is unencrypted.
        // Throws eTWUtil if the config file is encrypted but does not match the site key.
        // Throws eError if the site keyfile can not be opened.

    static void GetString(wc16_string& ret);
    static void GetStringNoEcho(wc16_string& ret);
    // This takes an input string from the user with or without displaying it on the screen.
    // To use passphrazes compatible between Unicode and non-Unicode, we return the most portable
    // type of string, the wide string.
    // This simply calls GetString in the scope of a NoEcho object.

    class NoEcho
    {
    public:
        NoEcho();
        ~NoEcho();
    };
    // For the lifetime of a cTWUtil::NoEcho object,
    // tty echoing on standard input is disabled.
    // At construction time, pending input is discarded.
    // In order to be friendly to programs like expect,
    // whose scripts typically supply input immediately after
    // prompting, such prompting should happen after NoEcho
    // construction to ensure that such input is not discarded.

    static void OpenKeyFile(cKeyFile& keyFile, TSTRING fileName); // throw (eError)
        // this opens the passed in keyfile and associates it with keyFile.
        // If an error occurs an exception is thrown with appropriate information.

    enum KeyType
    {
        KEY_SITE,
        KEY_LOCAL,
        KEY_PROVIDED
    };
    enum SleepPeriodInSecs
    {
        SLEEP_DEFAULT = 2
    };
    static const cElGamalSigPrivateKey*
                CreatePrivateKey(cKeyFile& keyFile, const WCHAR16* usePassphrase, KeyType keyType, int nSecs = SLEEP_DEFAULT);
    static void CreatePrivateKey(cPrivateKeyProxy& proxy,
                                 cKeyFile&         keyFile,
                                 const WCHAR16*    usePassphrase,
                                 KeyType           keyType,
                                 int               nSecs = SLEEP_DEFAULT);
    // This will attempt to create the private key from the given key file using the given password.
    // If usePassphrase is NULL then the user will be promtped to enter the passphrase.
    // If usePassphrase is non-NULL, then it will be used to attempt to opent the private
    // key. If this is the wrong password, an error message will be printed and an exception thrown.
    // The keyfile must already be open (using OpenKeyFile above).
    // The key must be freed using cKeyFile::ReleasePrivateKey().
    // keyType is just used so that an appropriate string can be displayed to the user
    // nSecs is the number of to wait before showing user if password was correct.  hinders brute
    // force (dictionary, etc.) attacks. defaults to 2 seconds.
    // NOTE: An eError() exception with an appropriate error message will be thrown on failure.
    // TODO: Try to use the second version as we will want to upgrade to this some day - dmb

    //-------------------------------------------------------------------------
    // Command Line
    //-------------------------------------------------------------------------

    typedef std::vector<TSTRING>              ObjList;
    typedef std::pair<cGenre::Genre, ObjList> GenreObjs;
    typedef std::vector<GenreObjs>            GenreObjList;
    static void ParseObjectList(GenreObjList& listOut, const ObjList& listIn); // throw eError()
        // takes a list of strings (listIn) and turns them into a collection of <genre, object list> pairs.
        // This is used while integrity checking or printing a set of FCOs. This takes the form: [genre|objName]+
        // where each object is associated with the most recently mentioned genre. If an object appears
        // before any genre identifiers, it is assumed to be in FS on unix or NTFS on NT.
        // eError will be thrown if an unknown genre is encountered
    static cFCOName ParseObjectName(const TSTRING& fcoName); // throw (eError)
        // converts a string representation of an FCO name (that was passed in on the command line for integrity
        // check or database printing) into a cFCOName. It is assumed that the name is for an object in the current
        // genre. If the string cannot be converted (ie -- relative path or invalid formatting), eError is thrown.

    //-------------------------------------------------------------------------
    // Database and report header helpers
    //-------------------------------------------------------------------------

    // These are all used for filling out HeaderInfo structures
    static TSTRING GetSystemName();
    static TSTRING GetIPAddress();
    static TSTRING GetCurrentUser();
    static TSTRING GetHostID();

    //-------------------------------------------------------------------------
    // Micellaneous
    //-------------------------------------------------------------------------

    static bool IsObjectEncrypted(const TCHAR* objFileName, const cFileHeaderID& fhid, const TSTRING& errorMsg);
    static bool IsObjectEncrypted(cArchive& arch, const cFileHeaderID& fhid, const TSTRING& errorMsg);
    // Tests an object serialized to objFileName to see if it's encrypted.  fhid ensures that the object in the
    // file is of the correct type.
    // If an error occurs while reading the file, this WILL PRINT an error message errorMsg,
    // then it will throw eError(NULL).

    static void PrintErrorMsg(const eError& e, const TSTRING& strExtra = _T(""));
    // nicely displays the contents of the exception to stderr.
    // currently, it has the format:
    // ### Error e.GetErrorNum(): e.GetMsg() : cErrorTable::GetErrorString(e.GetErrorNum())

    static bool ConfirmYN(const TCHAR* prompt);
    // prompt the user with given string and wait for a 'Y' or 'N' response.
    // returns true if 'Y' entered.
};


#endif //__TWUTIL_H
