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
// keyfile.h

#ifndef __KEYFILE_H
#define __KEYFILE_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __ERROR_H
#include "core/error.h"
#endif

#ifndef __FILEERROR_H
#include "core/fileerror.h"
#endif

class cElGamalSigPrivateKey;
class cElGamalSigPublicKey;
class cFileHeaderID;

TSS_EXCEPTION(eKeyFile, eFileError);
TSS_EXCEPTION(eKeyFileInvalidFmt, eKeyFile);
TSS_EXCEPTION(eKeyFileBadPassphrase, eKeyFile);
TSS_EXCEPTION(eKeyFileArchive, eKeyFile);
TSS_EXCEPTION(eKeyFileUninitialized, eKeyFile);


class cKeyFile
{
public:
    cKeyFile();
    ~cKeyFile();

    void ReadFile(const TCHAR* filename);        // throw eKeyFile()
    void WriteFile(const TCHAR* filename) const; // throw eKeyFile()
        // Read and write keys to a keyfile

    void ReadMem(const int8_t* pMem);  // throw eKeyFile()
    void WriteMem(int8_t* pMem) const; // throw eKeyFile()
    int  GetWriteLen();              // throw eKeyFile()
        // Functions to read and write the key to memory.  GetWriteLen() will throw an
        // exception if keys are not currently loaded.

    bool KeysLoaded() const;

    void GenerateKeys(int keySize, int8_t* passphrase, int passphraseLen); // throw eKeyFile()
        // Generate new keys
        // Note: Bytes in passphrase will be cleared after keys are generated for safety

    void ChangePassphrase(int8_t* passphraseOld,
                          int     passphraseOldLen,
                          int8_t* passphrase,
                          int     passphraseLen); // throw eKeyFile()
    // Change passphrase
    // Note: Bytes in passphrases will be cleared after change for safety

    const cElGamalSigPrivateKey* GetPrivateKey(int8_t* passphrase, int passphraseLen);
    void                         ReleasePrivateKey();
    // Access to the private key.  Key is normally stored encrypted for safety.  Call
    // ReleasePrivateKey() to destory the plaintext version of the key as soon as you
    // are done using the key.
    // Note: Bytes in passphrase will be cleared after keys are generated for safety

    const cElGamalSigPublicKey* GetPublicKey() const;

    static const cFileHeaderID& GetFileHeaderID();

protected:
    int8_t* mpPrivateKeyMem; // encrypted version of private key
    int     mPrivateKeyMemLen;

    cElGamalSigPrivateKey* mpPrivateKey; // only valid between calls to GetPrivateKey() and ReleasePrivateKey()
    cElGamalSigPublicKey*  mpPublicKey;

    int mPrivateKeyUseCount;

private:
    void ProtectKeys(int8_t* passphrase, int passphraseLen); // throw eKeyFile()
    void ReleaseMem();
};


///////////////////////////////////////////////////////////////////////////////
// class cPrivateKeyProxy
//
// Useful as a local var that releases the private key on destruction, thus
// a thrown exception will cause the key to be released propperly

class cPrivateKeyProxy
{
public:
    cPrivateKeyProxy();
    ~cPrivateKeyProxy();

    bool AquireKey(cKeyFile& keyFile, int8_t* passphrase, int passphraseLen);
    // note: be sure to check return value for failure!!!

    bool Valid() const
    {
        return mpKey != 0;
    }

    const cElGamalSigPrivateKey* GetKey() const;
    const cElGamalSigPrivateKey& operator->() const
    {
        return *GetKey();
    }
    const cElGamalSigPrivateKey& operator*() const
    {
        return *GetKey();
    }
    // In all access to the key, an exception will be thrown
    // if key has not been successfully aquired.
private:
    cKeyFile*                    mpKeyFile;
    const cElGamalSigPrivateKey* mpKey;
};

inline cPrivateKeyProxy::cPrivateKeyProxy() : mpKeyFile(0), mpKey(0)
{
}

#endif
