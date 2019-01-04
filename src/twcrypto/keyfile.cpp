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
// keyfile.cpp

#include "stdtwcrypto.h"

#include "keyfile.h"
#include "crypto.h"
#include "core/archive.h"
#include "core/debug.h"
#include "core/fileheader.h"
#include "core/serializerimpl.h"

// A magic number to identify key files
// I just picked this at random (I took the current time (14:34) squared it,
// converted it to hex and xor'ed with 0xffffffff, just in case you are wondering) - dmb
const uint32_t        KEY_MAGIC_NUMBER      = 0xffe09f5b;
static const uint32_t CURRENT_FIXED_VERSION = 0x02020000;
static const uint32_t TW_21_VERSION         = 0x02010000;

///////////////////////////////////////////////////////////////////////////////
// class cKeyFile

cKeyFile::cKeyFile()
{
    mpPrivateKeyMem     = 0;
    mPrivateKeyMemLen   = 0;
    mpPrivateKey        = 0;
    mpPublicKey         = 0;
    mPrivateKeyUseCount = 0;
}

cKeyFile::~cKeyFile()
{
    ReleaseMem();
}

void cKeyFile::ReleaseMem()
{
    //ASSERT(mPrivateKeyUseCount == 0); // though not an error, you should have called RealeasePrivateKey

    delete [] mpPrivateKeyMem;
    delete mpPrivateKey;
    delete mpPublicKey;
    mpPrivateKeyMem     = 0;
    mPrivateKeyMemLen   = 0;
    mpPrivateKey        = 0;
    mpPublicKey         = 0;
    mPrivateKeyUseCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
// GetFileHeaderID()
///////////////////////////////////////////////////////////////////////////////

struct cKeyFileFHID
{
    cFileHeaderID* KeyFileID;

    cKeyFileFHID()
    {
        KeyFileID = 0;
    }
    ~cKeyFileFHID()
    {
        delete KeyFileID;
    }
} gKeyFileFHID;

const cFileHeaderID& cKeyFile::GetFileHeaderID()
{
    if (gKeyFileFHID.KeyFileID == 0)
        gKeyFileFHID.KeyFileID = new cFileHeaderID(_T("cKeyFile"));

    // sanity check
    ASSERT(*gKeyFileFHID.KeyFileID == cFileHeaderID(_T("cKeyFile")));

    return *gKeyFileFHID.KeyFileID;
}

bool cKeyFile::KeysLoaded() const
{
    ASSERT(mpPrivateKeyMem == 0 || mpPublicKey != 0);

    return (mpPrivateKeyMem != 0);
}

// Read and write keys to a keyfile

void cKeyFile::ReadFile(const TCHAR* filename) // throw eKeyFile()
{
    ReleaseMem();

    int16_t len;

    try
    {
        cFileArchive inFile;
        cFileHeader  fileHeader;

        inFile.OpenRead(filename);

        try
        {
            cSerializerImpl fhSer(inFile, cSerializerImpl::S_READ, filename);
            fileHeader.Read(&fhSer);
        }
        catch (const eError&)
        {
            throw eKeyFileInvalidFmt();
        }

        // check id and version
        if (fileHeader.GetID() != cKeyFile::GetFileHeaderID())
        {
            ASSERT(false);
            throw eKeyFileInvalidFmt();
        }

        // NOTE:mdb 20 July 99 -- I changed this so that we can load both tw22 and
        //      tw21 key files, since their formatting is exactly the same besides
        //      the version number.
        //
        if ((fileHeader.GetVersion() != CURRENT_FIXED_VERSION) && (fileHeader.GetVersion() != TW_21_VERSION))
        {
            ASSERT(false);
            throw eKeyFileInvalidFmt();
        }

        // read public key
        inFile.ReadInt16(len);
        if (len <= 0 || len >= 9000)
        {
            ASSERT(false);
            throw eKeyFileInvalidFmt();
        }

        int8_t* publicMem = new int8_t[len];
        if (inFile.ReadBlob(publicMem, len) != len)
        {
            ASSERT(false);
            delete [] publicMem;
            throw eKeyFileInvalidFmt();
        }

        mpPublicKey = new cElGamalSigPublicKey(publicMem);
        delete [] publicMem;

        // read private key;
        inFile.ReadInt16((int16_t&)len);
        if (len <= 0 || len > 9000)
        {
            ASSERT(false);
            delete mpPublicKey;
            mpPublicKey = 0;
            throw eKeyFileInvalidFmt();
        }

        mPrivateKeyMemLen = len;
        mpPrivateKeyMem   = new int8_t[len];
        if (inFile.ReadBlob(mpPrivateKeyMem, mPrivateKeyMemLen) < mPrivateKeyMemLen)
        {
            ASSERT(false);
            delete mpPublicKey;
            delete [] mpPrivateKeyMem;
            mpPublicKey       = 0;
            mpPrivateKeyMem   = 0;
            mPrivateKeyMemLen = 0;
            throw eKeyFileInvalidFmt();
        }
    }
    catch (const eArchive&)
    {
        delete mpPublicKey;
        delete [] mpPrivateKeyMem;
        mpPublicKey       = 0;
        mpPrivateKeyMem   = 0;
        mPrivateKeyMemLen = 0;
        throw eKeyFileArchive(filename);
    }
    catch (const eKeyFile&)
    {
        throw;
    }
}

void cKeyFile::WriteFile(const TCHAR* filename) const // throw eKeyFile()
{
    ASSERT(KeysLoaded());

    try
    {
        cFileArchive outFile;
        cFileHeader  fileHeader;

        outFile.OpenReadWrite(filename);

        fileHeader.SetID(cKeyFile::GetFileHeaderID());

        // Set file version.
        // If we in the future we wish to support reading keys different versions,
        // we will have to move this
        fileHeader.SetVersion(CURRENT_FIXED_VERSION);

        fileHeader.SetEncoding(cFileHeader::NO_ENCODING);

        {
            cSerializerImpl fhSer(outFile, cSerializerImpl::S_WRITE, filename);
            fileHeader.Write(&fhSer);
        }

        // save public key
        int16_t len       = mpPublicKey->GetWriteLen();
        int8_t* publicMem = new int8_t[len];
        mpPublicKey->Write(publicMem);

        outFile.WriteInt16(len);
        outFile.WriteBlob(publicMem, len);

        delete [] publicMem;

        // save private key
        len = mPrivateKeyMemLen;
        outFile.WriteInt16(len);
        outFile.WriteBlob(mpPrivateKeyMem, mPrivateKeyMemLen);
    }
    catch (const eArchive&)
    {
        throw eKeyFileArchive(filename);
    }
}

// Functions to read and write the key to memory.  GetWriteLen() will throw an
// exception if keys are not currently loaded.

void cKeyFile::ReadMem(const int8_t* pMem) // throw eKeyFile()
{
    int16_t i16;
    int32_t i32;

    ReleaseMem();

    memcpy(&i32, pMem, sizeof(i32));
    if ((unsigned int)tw_ntohl(i32) != KEY_MAGIC_NUMBER)
        throw eKeyFileInvalidFmt();
    pMem += sizeof(int32_t);

    memcpy(&i32, pMem, sizeof(i32));
    if (tw_ntohl(i32) != 1) // version check
        throw eKeyFileInvalidFmt();
    pMem += sizeof(int32_t);

    memcpy(&i16, pMem, sizeof(i16));
    int16_t len = tw_ntohs(i16);
    if (len <= 0 || len > 9000)
    {
        ASSERT(false);
        throw eKeyFileInvalidFmt();
    }

    mpPublicKey = new cElGamalSigPublicKey((void*)(pMem + sizeof(int16_t)));
    pMem += sizeof(int16_t) + len;

    memcpy(&i16, pMem, sizeof(i16));
    mPrivateKeyMemLen = tw_ntohs(i16);
    if (mPrivateKeyMemLen <= 0 || mPrivateKeyMemLen > 9000)
    {
        ASSERT(false);
        delete mpPublicKey;
        mpPublicKey = 0;
        throw eKeyFileInvalidFmt();
    }

    mpPrivateKeyMem = new int8_t[mPrivateKeyMemLen];
    memcpy(mpPrivateKeyMem, pMem + sizeof(int16_t), mPrivateKeyMemLen);
}

void cKeyFile::WriteMem(int8_t* pMem) const // throw eKeyFile()
{
    int16_t i16;
    int32_t i32;

    if (!KeysLoaded())
    {
        ASSERT(false);
        throw eKeyFileUninitialized(_T("cKeyFile not initialized"));
    }

    // magic number and version
    i32 = tw_htonl(KEY_MAGIC_NUMBER);
    memcpy(pMem, &i32, sizeof(i32));
    pMem += sizeof(int32_t);

    i32 = tw_htonl(1);
    memcpy(pMem, &i32, sizeof(i32));
    pMem += sizeof(int32_t);

    // save public key
    int16_t len = mpPublicKey->GetWriteLen();
    i16       = tw_htons(len);
    memcpy(pMem, &i16, sizeof(i16));
    pMem += sizeof(int16_t);

    mpPublicKey->Write(pMem);
    pMem += len;

    // save private key
    len = mPrivateKeyMemLen;
    i16 = tw_htons(len);
    memcpy(pMem, &i16, sizeof(i16));
    pMem += sizeof(int16_t);

    memcpy(pMem, mpPrivateKeyMem, mPrivateKeyMemLen);
}

int cKeyFile::GetWriteLen() // throw eKeyFile()
{
    if (!KeysLoaded())
    {
        ASSERT(false);
        throw eKeyFileUninitialized(_T("cKeyFile not initialized"));
    }

    return sizeof(int32_t) +              // the magic number
           sizeof(int32_t) +              // version
           sizeof(int16_t) +              // sizeof public key
           mpPublicKey->GetWriteLen() + // the public key
           sizeof(int16_t) +              // sizeof private key
           mPrivateKeyMemLen;           // the private key
}

void cKeyFile::ProtectKeys(int8_t* passphrase, int passphraseLen) // throw eKeyFile()
{
    int     i;
    int16_t i16;

    // generate the hash value of the private key
    int len                = mpPrivateKey->GetWriteLen();
    int8_t* privateKeyBits = new int8_t[len];
    mpPrivateKey->Write(privateKeyBits);
    cHashedKey128 privateHash(privateKeyBits, len);
    RandomizeBytes(privateKeyBits, len);
    delete [] privateKeyBits;

    // generate cipher for encrypting private key
    cHashedKey192 desKey(passphrase, passphraseLen);
    cTripleDES    des;
    des.SetKey(iCipher::ENCRYPT, desKey);

    ASSERT(des.GetBlockSizeCipher() == des.GetBlockSizePlain());

    // calculate size of the memory image of the private key
    mPrivateKeyMemLen = cHashedKey128::GetWriteLen() + sizeof(int16_t) + mpPrivateKey->GetWriteLen();
    mPrivateKeyMemLen = (mPrivateKeyMemLen / des.GetBlockSizePlain() + 1) * des.GetBlockSizePlain();
    int sluff         = mPrivateKeyMemLen - cHashedKey128::GetWriteLen() - sizeof(int16_t) - mpPrivateKey->GetWriteLen();

    // write the hash of the private key, the size of the private key, and the private key,
    // all as plaintext.
    mpPrivateKeyMem = new int8_t[mPrivateKeyMemLen];
    privateHash.Write(mpPrivateKeyMem);
    i16 = tw_htons(mpPrivateKey->GetWriteLen());
    memcpy(mpPrivateKeyMem + cHashedKey128::GetWriteLen(), &i16, sizeof(i16));
    mpPrivateKey->Write(mpPrivateKeyMem + cHashedKey128::GetWriteLen() + sizeof(int16_t));

    RandomizeBytes(mpPrivateKeyMem + mPrivateKeyMemLen - sluff, sluff);

    // delete the private key
    delete mpPrivateKey;
    mpPrivateKey = 0;

    // encrypt the plaintext
    for (i = 0; i < mPrivateKeyMemLen; i += des.GetBlockSizePlain())
    {
        des.ProcessBlock(mpPrivateKeyMem + i, mpPrivateKeyMem + i);
    }

#ifdef DEBUG
    // try decoding it just to see if it worked
    if (GetPrivateKey(passphrase, passphraseLen) == 0)
    {
        throw eKeyFileInvalidFmt();
    }

    ReleasePrivateKey();
#endif

    RandomizeBytes(passphrase, passphraseLen);
}

// Generate new keys

void cKeyFile::GenerateKeys(int keySize, int8_t* passphrase, int passphraseLen) // throw eKeyFile()
{
    ReleaseMem();

    cElGamalSig rsa((cElGamalSig::KeySize)keySize);

    // generate both keys
    rsa.GenerateKeys(mpPrivateKey, mpPublicKey);

    ProtectKeys(passphrase, passphraseLen);
}

void cKeyFile::ChangePassphrase(int8_t* passphraseOld,
                                int     passphraseOldLen,
                                int8_t* passphrase,
                                int     passphraseLen) // throw eKeyFile()
{
    if (GetPrivateKey(passphraseOld, passphraseOldLen) == 0)
    {
        throw eKeyFileInvalidFmt();
    }

    ProtectKeys(passphrase, passphraseLen);
}

// Access to the private key.  Key is normally stored encrypted for safety.  Call
// ReleasePrivateKey() to destory the plaintext version of the key as soon as you
// are done using the key.

const cElGamalSigPrivateKey* cKeyFile::GetPrivateKey(int8_t* passphrase, int passphraseLen)
{
    int16_t i16;

    if (!KeysLoaded())
    {
        ASSERT(false);
        throw eKeyFileUninitialized(_T("cKeyFile not initialized"));
    }

    ASSERT(mPrivateKeyUseCount >= 0);

    if (mPrivateKeyUseCount == 0)
    {
        // generate cipher for decrypting private key
        cHashedKey192 desKey(passphrase, passphraseLen);
        cTripleDES    des;
        des.SetKey(iCipher::DECRYPT, desKey);

        ASSERT(des.GetBlockSizeCipher() == des.GetBlockSizePlain());

        // get a copy of the ciphertext and decrypt it
        int8_t* plainPrivateKeyMem = new int8_t[mPrivateKeyMemLen];
        memcpy(plainPrivateKeyMem, mpPrivateKeyMem, mPrivateKeyMemLen);

        ASSERT(mPrivateKeyMemLen % des.GetBlockSizePlain() == 0);

        for (int i = 0; i < mPrivateKeyMemLen; i += des.GetBlockSizePlain())
        {
            des.ProcessBlock(plainPrivateKeyMem + i, plainPrivateKeyMem + i);
        }

        // calculate the hash on the private key
        memcpy(&i16, plainPrivateKeyMem + cHashedKey128::GetWriteLen(), sizeof(i16));
        int len = tw_ntohs(i16);

        if (len + cHashedKey128::GetWriteLen() + sizeof(int16_t) > (unsigned int)mPrivateKeyMemLen)
        {
            RandomizeBytes(plainPrivateKeyMem, mPrivateKeyMemLen);
            delete [] plainPrivateKeyMem;
            RandomizeBytes(passphrase, passphraseLen);
            return 0;
        }

        cHashedKey128 privateHash(plainPrivateKeyMem + cHashedKey128::GetWriteLen() + sizeof(int16_t), len);

        // compare to the stored value
        if (memcmp(plainPrivateKeyMem, privateHash.GetKey(), cHashedKey128::GetWriteLen()) != 0)
        {
            // passphrase didn't do the job
            RandomizeBytes(plainPrivateKeyMem, mPrivateKeyMemLen);
            delete [] plainPrivateKeyMem;
            RandomizeBytes(passphrase, passphraseLen);
            return 0;
        }

        mpPrivateKey = new cElGamalSigPrivateKey(plainPrivateKeyMem + cHashedKey128::GetWriteLen() + sizeof(int16_t));

        RandomizeBytes(plainPrivateKeyMem, mPrivateKeyMemLen);
        delete [] plainPrivateKeyMem;

        RandomizeBytes(passphrase, passphraseLen);
    }
    else
    {
        // TODO: make sure the passphrase is correct
    }

    ++mPrivateKeyUseCount;
    return mpPrivateKey;
}

void cKeyFile::ReleasePrivateKey() // throw eKeyFile()
{
    if (!KeysLoaded())
    {
        ASSERT(false);
        throw eKeyFileUninitialized(_T("cKeyFile not initialized"));
    }

    ASSERT(mPrivateKeyUseCount > 0);

    if (--mPrivateKeyUseCount == 0)
    {
        delete mpPrivateKey;
        mpPrivateKey = 0;
    }
}

const cElGamalSigPublicKey* cKeyFile::GetPublicKey() const // throw eKeyFile()
{
    if (!KeysLoaded())
    {
        ASSERT(false);
        throw eKeyFileUninitialized(_T("cKeyFile not initialized"));
    }

    return mpPublicKey;
}

///////////////////////////////////////////////////////////////////////////////
// class cPrivateKeyProxy

bool cPrivateKeyProxy::AquireKey(cKeyFile& keyFile, int8_t* passphrase, int passphraseLen)
{
    if (mpKey != 0)
    {
        ASSERT(mpKeyFile != 0);
        mpKeyFile->ReleasePrivateKey();
    }

    mpKeyFile = &keyFile;
    mpKey     = keyFile.GetPrivateKey(passphrase, passphraseLen);

    return mpKey != 0;
}

cPrivateKeyProxy::~cPrivateKeyProxy()
{
    if (mpKey != 0)
    {
        ASSERT(mpKeyFile != 0);
        mpKeyFile->ReleasePrivateKey();
    }
}

const cElGamalSigPrivateKey* cPrivateKeyProxy::GetKey() const // throw eKeyFile()
{
    if (mpKey == 0)
    {
        // This is a programming error
        ASSERT(false);
        throw eKeyFileUninitialized(_T("Private Key not initialized"));
    }

    return mpKey;
}
