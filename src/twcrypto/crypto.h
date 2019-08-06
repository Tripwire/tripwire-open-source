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
// crypto.h -- Tripwire crypto header
//

#ifndef __CRYPTO_H
#define __CRYPTO_H

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// class iCipher

class iCipher
{
public:
    virtual ~iCipher()
    {
    }

    enum EncryptionDir
    {
        ENCRYPT,
        DECRYPT
    };

    virtual int GetBlockSizePlain()  = 0;
    virtual int GetBlockSizeCipher() = 0;
    // return the size of data blocks this crypter works on.
    virtual void ProcessBlock(const void* indata, void* outdata) = 0; // throw eArchive
        // process a block of data.  indata and outdata may be the same memory
};

///////////////////////////////////////////////////////////////////////////////
// class cNullCipher -- no encryption cipher

class cNullCipher : public iCipher
{
public:
    cNullCipher()
    {
    }
    virtual ~cNullCipher()
    {
    }

    virtual int GetBlockSizePlain();
    virtual int GetBlockSizeCipher();
    // return the size of data blocks for plaintext and cipertext
    virtual void ProcessBlock(const void* indata, void* outdata);
    // process a block of data.  indata and outdata may be the same memory
};

///////////////////////////////////////////////////////////////////////////////
// class cIDEACipher

#ifdef _IDEA_ENCRYPTION
class cIDEA_i;
class cHashedKey128;

class cIDEA : public iCipher
{
public:
    cIDEA();
    virtual ~cIDEA();

    void SetKey(iCipher::EncryptionDir dir, const cHashedKey128& key);

    virtual int GetBlockSizePlain();
    virtual int GetBlockSizeCipher();
    // return the size of data blocks for plaintext and cipertext
    virtual void ProcessBlock(const void* indata, void* outdata);
    // process a block of data.  indata and outdata may be the same memory

protected:
    cIDEA_i* mpData;
};
#endif // _IDEA_ENCRYPTION

///////////////////////////////////////////////////////////////////////////////
// class cTripleDES

class cTripleDES_i;
class cHashedKey192;

class cTripleDES : public iCipher
{
public:
    cTripleDES();
    virtual ~cTripleDES();

    void SetKey(iCipher::EncryptionDir dir, const cHashedKey192& key);

    virtual int GetBlockSizePlain();
    virtual int GetBlockSizeCipher();
    // return the size of data blocks for plaintext and cipertext
    virtual void ProcessBlock(const void* indata, void* outdata);
    // process a block of data.  indata and outdata may be the same memory

protected:
    cTripleDES_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// class cRSA

#ifdef _RSA_ENCRYPTION

class cRSA_i;
class cRSAPublicKey;
class cRSAPrivateKey;

class cRSA : public iCipher
{
public:
    enum KeySize
    {
        KEY256  = 256,
        KEY512  = 512,
        KEY1024 = 1024,
        KEY2048 = 2048
    };

    explicit cRSA(KeySize keysize);
    explicit cRSA(const cRSAPublicKey& publicKey);   // read keysize from key
    explicit cRSA(const cRSAPrivateKey& privateKey); // ditto
    virtual ~cRSA();

    void SetEncrypting(const cRSAPublicKey* pKey);
    void SetDecrypting(const cRSAPrivateKey* pKey);
    void SetSigning(const cRSAPrivateKey* pKey);
    void SetVerifying(const cRSAPublicKey* pKey);

    virtual int GetBlockSizePlain();
    virtual int GetBlockSizeCipher();
    // return the size of data blocks for plaintext and cipertext
    virtual void ProcessBlock(const void* indata, void* outdata);
    // process a block of data.  indata and outdata may be the same memory

    void GenerateKeys(cRSAPrivateKey*& retPrivate, cRSAPublicKey*& retPublic);
    // generate public and private keys.  Caller is responsible for deleting these keys when done

protected:
    cRSA_i* mpData;

private:
    void Init(KeySize keysize);
};

class cRSAPrivateKey_i;
class cRSAPublicKey_i;

// cRSAPrivateKey

class cRSAPrivateKey
{
    friend class cRSA;
    friend class cRSAPublicKey;

public:
    explicit cRSAPrivateKey(void* pDataStream);
    ~cRSAPrivateKey();

    int  GetWriteLen() const;
    void Write(void* pDataStream) const;

protected:
    cRSAPrivateKey_i* mpData;

private:
    cRSAPrivateKey(); // cRSA should be used to generate keys
};

// cRSAPublicKey

class cRSAPublicKey
{
    friend class cRSA;

public:
    explicit cRSAPublicKey(void* pDataStream);
    explicit cRSAPublicKey(const cRSAPrivateKey& privateKey);
    ~cRSAPublicKey();

    int  GetWriteLen() const;
    void Write(void* pDataStream) const;

#    ifdef DEBUG
    void TraceContents();
#    endif

protected:
    cRSAPublicKey_i* mpData;

private:
    cRSAPublicKey(); // cRSA should be used to generate keys
};

#endif // _RSA_ENCRYPTION


///////////////////////////////////////////////////////////////////////////////
// class cElGamalSig
//
// This class does not encrypt the cipher stream.  For each block written
// it signs a hash of the block and appends the signature to the end of the
// block.  When reading an exception will be thrown if the signature does
// not match the data.
//
// We use El Gamal because it is patent free..
//

class cElGamalSig_i;
class cElGamalSigPublicKey;
class cElGamalSigPrivateKey;

class cElGamalSig : public iCipher
{
public:
    enum KeySize
    {
        KEY256  = 256,
        KEY512  = 512,
        KEY1024 = 1024,
        KEY2048 = 2048
    };

    explicit cElGamalSig(KeySize keysize);
    explicit cElGamalSig(const cElGamalSigPublicKey& publicKey);   // read keysize from key
    explicit cElGamalSig(const cElGamalSigPrivateKey& privateKey); // ditto
    virtual ~cElGamalSig();

    void SetSigning(const cElGamalSigPrivateKey* pKey);
    void SetVerifying(const cElGamalSigPublicKey* pKey);

    virtual int GetBlockSizePlain();
    virtual int GetBlockSizeCipher();
    // return the size of data blocks for plaintext and cipertext
    virtual void ProcessBlock(const void* indata, void* outdata);
    // process a block of data.  indata and outdata may be the same memory

    void GenerateKeys(cElGamalSigPrivateKey*& retPrivate, cElGamalSigPublicKey*& retPublic);
    // generate public and private keys.  Caller is responsible for deleting these keys when done

protected:
    cElGamalSig_i* mpData;

private:
    void Init(KeySize keysize);

    enum
    {
        PLAIN_BLOCK_SIZE = 4083
    };
};

class cElGamalSigPrivateKey_i;
class cElGamalSigPublicKey_i;

// cElGamalSigPrivateKey

class cElGamalSigPrivateKey
{
    friend class cElGamalSig;
    friend class cElGamalSigPublicKey;

public:
    explicit cElGamalSigPrivateKey(void* pDataStream);
    ~cElGamalSigPrivateKey();

    int  GetWriteLen() const;
    void Write(void* pDataStream) const;

protected:
    cElGamalSigPrivateKey_i* mpData;

private:
    cElGamalSigPrivateKey(); // cElGamal should be used to generate keys
};

// cElGamalSigPublicKey

class cElGamalSigPublicKey
{
    friend class cElGamalSig;

public:
    explicit cElGamalSigPublicKey(void* pDataStream);
    explicit cElGamalSigPublicKey(const cElGamalSigPrivateKey& privateKey);
    ~cElGamalSigPublicKey();

    int  GetWriteLen() const;
    void Write(void* pDataStream) const;

    bool IsEqual(const cElGamalSigPublicKey& rhs) const;
    // This is used to make sure the key used to sign the config
    // file is the same as the key we are currently using.

#ifdef DEBUG
    void TraceContents();
#endif

protected:
    cElGamalSigPublicKey_i* mpData;

private:
    cElGamalSigPublicKey(); // cElGamal should be used to generate keys
};

///////////////////////////////////////////////////////////////////////////////
// class cHashedKey128 -- A 128 bit key based on the hash value of some data

class cHashedKey128
{
public:
    explicit cHashedKey128(const TSTRING& data);
    cHashedKey128(void* pData, int32_t dataLen);
    ~cHashedKey128();

    const int8_t* GetKey() const;

    static int GetWriteLen();
    void Write(void* pDataStream);

protected:
    enum
    {
        KEYLEN  = 16,
        BUFFER_SIZE = 20
    };

    int8_t mKey[BUFFER_SIZE];
};

inline const int8_t* cHashedKey128::GetKey() const
{
    return mKey;
}

inline int cHashedKey128::GetWriteLen()
{
    return KEYLEN;
}

inline void cHashedKey128::Write(void* pDataStream)
{
    memcpy(pDataStream, mKey, KEYLEN);
}

///////////////////////////////////////////////////////////////////////////////
// class cHashedKey192 -- A 192 bit key based on the hash value of some data

class cHashedKey192
{
public:
    explicit cHashedKey192(const TSTRING& data);
    cHashedKey192(void* pData, int32_t dataLen);
    ~cHashedKey192();

    const int8_t* GetKey() const;

    static int GetWriteLen();
    void Write(void* pDataStream);

protected:
    enum
    {
        KEYLEN = 24
    };

    int8_t mKey[KEYLEN];
};

inline const int8_t* cHashedKey192::GetKey() const
{
    return mKey;
}

inline int cHashedKey192::GetWriteLen()
{
    return KEYLEN;
}

inline void cHashedKey192::Write(void* pDataStream)
{
    memcpy(pDataStream, mKey, KEYLEN);
}

///////////////////////////////////////////////////////////////////////////////
// void RandomizeBytes(byte* destbuf, int len) -- Fill a buffer with random bytes

void RandomizeBytes(int8_t* destbuf, int len);

#endif // __CRYPTO_H
