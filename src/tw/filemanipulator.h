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
// filemanipulator.h -- Provides functionality to examine tripwire file headers
//      for file type and encryption type.  Also provides a mechanism for
//      changing encryption keys or removing encryption.
//

#ifndef __FILEMANIPULATOR_H
#define __FILEMANIPULATOR_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __FILEHEADER_H
#include "core/fileheader.h"
#endif

class cElGamalSigPublicKey;
class cElGamalSigPrivateKey;

/*
class eFileManipulator : public eError
{
public:
    eFileManipulator(int errorNum, TSTRING filename);
    ~eFileManipulator();

    virtual TSTRING GetMsg() const;
};
    enum ErrorNum
    {
        ERR_FILE_MANIPULATOR_INVALID_FILE = 2300,
        ERR_LAST
    };

    TSTRING eFileManipulator::GetMsg() const
    {
        TSTRING ret = iUserString::GetInstance()->GetString(STR_ERROR_FILENAME);
        ret.append(mMsg);
        return ret;
    }
*/

//=============================================================================
// eFileManip
//=============================================================================
TSS_FILE_EXCEPTION(eFileManip, eFileError)
TSS_FILE_EXCEPTION(eFileManipInvalidFile, eFileManip)
TSS_FILE_EXCEPTION(eFileManipFileRead, eFileManip)
TSS_FILE_EXCEPTION(eFileManipHeaderNotFound, eFileManip)
TSS_FILE_EXCEPTION(eFileManipFileNotFound, eFileManip)
TSS_FILE_EXCEPTION(eFileManipNotEncrypted, eFileManip)
TSS_FILE_EXCEPTION(eFileManipMissingKey, eFileManip)
TSS_FILE_EXCEPTION(eFileManipNotWritable, eFileManip)
TSS_FILE_EXCEPTION(eFileManipUnrecognizedFileType, eFileManip)

//=============================================================================
// cFileManipulator
//=============================================================================
class cFileManipulator
{
public:
    explicit cFileManipulator(const TCHAR* filename);
    // throws eArchive if file does not exist
    cFileManipulator(const cFileManipulator& rhs);
    ~cFileManipulator();
    void Init();

    // information for this file
    TSTRING               GetFileName() const;
    const cFileHeaderID*  GetHeaderID();    // returns NULL if error code is non-zero
    uint32_t              GetFileVersion(); // throws eFileManipulator if error code is non-zero
    cFileHeader::Encoding GetEncoding();    // throws eFileManipulator if error code is non-zero

    // things you can do to this file
    bool TestDecryption(const cElGamalSigPublicKey& key, bool thorough);
    // Try to decrypt the file using the given key. If thorough is true then
    // the entire file is read into memory.  Returns true if decryption was
    // successful.
    // throws eFileManipulator if error code is non-zero
    void ChangeEncryption(const cElGamalSigPublicKey* pOldKey, const cElGamalSigPrivateKey* pNewKey, bool backup);
    // Change the encryption on a file.
    // If pNewKey is NULL, then encryption on the file will be removed.
    // If pOldKey is NULL and the file is currently encrypted, then the
    // function will throw an exception;
    // If pOldKey is not NULL and the file is encrypted, then the fuction
    // will throw an exception;
    // If pOldKey is NULL and pNewKey is NULL and the file is not
    // currently encrypted, the function will not do anything and will just
    // return.

    // utility functions
    static int LoadFiles(std::list<cFileManipulator>& ret, TSTRING mask);
    // method for getting a list of manipulators
    // returns number of files matched, or zero if no files matched mask passed
    static bool UseSiteKey(const cFileHeaderID& headerID);
    // convenience function that returns true if header is a config file or policy file
    // and returns false if it is a database or report.

private:
    bool        mbInit;
    TSTRING     mFileName;
    cFileHeader mFileHeader;
};

inline TSTRING cFileManipulator::GetFileName() const
{
    return mFileName;
}
#endif
