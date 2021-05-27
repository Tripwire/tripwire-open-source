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
// crypto.cpp -- Tripwire crypto implementation
//

#include "stdtwcrypto.h"

#include "crypto.h"
#include "core/errorgeneral.h"
#include "time.h"
#include "core/archive.h"
#include "core/usernotify.h"

#include "cryptlib/sha.h"
#include "cryptlib/rng.h"
#include "cryptlib/des.h"
#include "cryptlib/integer.h"
#include "cryptlib/elgamal.h"
#include "cryptlib/des.h"
#ifdef _IDEA_ENCRYPTION
#include "cryptlib/idea.h"
#endif
#ifdef _RSA_ENCRYPTION
#include "cryptlib/rsa.h"
#endif

#include <unistd.h>
#include <fcntl.h>

const uint32_t EL_GAMAL_SIG_PUBLIC_MAGIC_NUM  = 0x7ae2c945;
const uint32_t EL_GAMAL_SIG_PRIVATE_MAGIC_NUM = 0x0d0ffa12;

///////////////////////////////////////////////////////////////////////////////
// macros for reading and writing integers

#define WRITE_INTEGER(I)                       \
    len = I.MinEncodedSize(Integer::UNSIGNED); \
    ASSERT(len >= 0 && len < 9000);            \
    i32 = tw_htonl(len);                       \
    memcpy(pOut, &i32, sizeof(i32));           \
    pOut += sizeof(int32_t);                     \
    I.Encode(pOut, len, Integer::UNSIGNED);    \
    pOut += len;

#define READ_INTEGER(I)                    \
    memcpy(&i32, pIn, sizeof(i32));        \
    len = tw_ntohl(i32);                   \
    ASSERT(len >= 0 && len < 9000);        \
    pIn += sizeof(int32_t);                  \
    I.Decode(pIn, len, Integer::UNSIGNED); \
    pIn += len;

///////////////////////////////////////////////////////////////////////////////
// class cNullCipher -- no encryption cipher

int cNullCipher::GetBlockSizePlain()
{
    // lets use the blocksize of TripleDES_Encryption since that may be
    // better than picking a blocksize at random
    return TripleDES_Encryption::BLOCKSIZE;
}

// return the size of data blocks for plaintext and cipertext
int cNullCipher::GetBlockSizeCipher()
{
    return TripleDES_Encryption::BLOCKSIZE;
}

// process a block of data.  indata and outdata may be the same memory
void cNullCipher::ProcessBlock(const void* indata, void* outdata)
{
    memmove(outdata, indata, TripleDES_Encryption::BLOCKSIZE);
}

//////////////////////////////////////////////////////////////////////////////
// class cIDEA
//////////////////////////////////////////////////////////////////////////////

#ifdef _IDEA_ENCRYPTION

class cIDEA_i
{
public:
    IDEA* mpIDEA;
};

cIDEA::cIDEA()
{
    mpData = new cIDEA_i;

    mpData->mpIDEA = 0;
}

cIDEA::~cIDEA()
{
    delete mpData->mpIDEA;
    delete mpData;
    mpData = 0;
}

void cIDEA::SetKey(iCipher::EncryptionDir dir, const cHashedKey128& key)
{
    // some things we are assuming
    ASSERT(IDEA::KEYLENGTH == 16);
    ASSERT(mpData);

    delete mpData->mpIDEA;
    mpData->mpIDEA = new IDEA((uint8_t*)key.GetKey(), dir == iCipher::ENCRYPT ? ENCRYPTION : DECRYPTION);
}

// return the size of data block this crypter works on
int cIDEA::GetBlockSizePlain()
{
    return IDEA::BLOCKSIZE;
}

int cIDEA::GetBlockSizeCipher()
{
    return IDEA::BLOCKSIZE;
}

// process a block of data.  indata and outdata may be the same memory
void cIDEA::ProcessBlock(const void* indata, void* outdata)
{
    ASSERT(mpData->mpIDEA);

    if (!mpData->mpIDEA)
    {
        ThrowAndAssert(eInternal(_T("Key not set in symmetric encryption.")));
    }

    mpData->mpIDEA->ProcessBlock((uint8_t*)indata, (uint8_t*)outdata);
}

#endif // _IDEA_ENCRYPTION

//////////////////////////////////////////////////////////////////////////////
// class cTripleDES
//////////////////////////////////////////////////////////////////////////////

class cTripleDES_i
{
public:
    TripleDES_Encryption* mpEncryptor;
    TripleDES_Decryption* mpDecryptor;
};

cTripleDES::cTripleDES()
{
    mpData = new cTripleDES_i;

    mpData->mpEncryptor = 0;
    mpData->mpDecryptor = 0;
}

cTripleDES::~cTripleDES()
{
    delete mpData->mpEncryptor;
    delete mpData->mpDecryptor;
    delete mpData;
    mpData = 0;
}

void cTripleDES::SetKey(iCipher::EncryptionDir dir, const cHashedKey192& key)
{
    // some things we are assuming
    ASSERT(TripleDES_Encryption::KEYLENGTH == 24);
    ASSERT(cHashedKey192::GetWriteLen() == 24);
    ASSERT(mpData != 0);

    if (dir == iCipher::ENCRYPT)
    {
        delete mpData->mpEncryptor;
        delete mpData->mpDecryptor;
        mpData->mpDecryptor = 0;
        mpData->mpEncryptor = new TripleDES_Encryption((uint8_t*)key.GetKey());
    }
    else
    {
        delete mpData->mpEncryptor;
        delete mpData->mpDecryptor;
        mpData->mpEncryptor = 0;
        mpData->mpDecryptor = new TripleDES_Decryption((uint8_t*)key.GetKey());
    }
}

// return the size of data block this crypter works on
int cTripleDES::GetBlockSizePlain()
{
    ASSERT(TripleDES_Encryption::BLOCKSIZE == TripleDES_Decryption::BLOCKSIZE);
    return TripleDES_Encryption::BLOCKSIZE;
}

int cTripleDES::GetBlockSizeCipher()
{
    ASSERT(TripleDES_Encryption::BLOCKSIZE == TripleDES_Decryption::BLOCKSIZE);
    return TripleDES_Encryption::BLOCKSIZE;
}

// process a block of data.  indata and outdata may be the same memory
void cTripleDES::ProcessBlock(const void* indata, void* outdata)
{
    ASSERT(mpData != 0);
    ASSERT(mpData->mpEncryptor || mpData->mpDecryptor);

    if (!mpData->mpEncryptor && !mpData->mpDecryptor)
    {
        ThrowAndAssert(INTERNAL_ERROR(
            "crypto.cpp")); //cTWError::E_INTERNAL, TSTRING(_T("Key not set in symmetric encryption."))));
    }

    if (mpData->mpEncryptor)
        mpData->mpEncryptor->ProcessBlock((uint8_t*)indata, (uint8_t*)outdata);
    else
        mpData->mpDecryptor->ProcessBlock((uint8_t*)indata, (uint8_t*)outdata);
}

///////////////////////////////////////////////////////////////////////////////
// class cRSA

#ifdef _RSA_ENCRYPTION

// class cRSAPrivateKey

class cRSAPrivateKey_i
{
public:
    int16_t        mKeyLength;
    RSAPrivateKey* mpKey;
};

cRSAPrivateKey::cRSAPrivateKey()
{
    mpData             = new cRSAPrivateKey_i;
    mpData->mpKey      = 0;
    mpData->mKeyLength = 0;
}

cRSAPrivateKey::cRSAPrivateKey(void* pDataStream)
{
    mpData = new cRSAPrivateKey_i;

    int32_t len;
    int32_t i32;
    int16_t i16;

    uint8_t* pIn = (uint8_t*)pDataStream;

    memcpy(&i16, pIn, sizeof(i16));
    mpData->mKeyLength = tw_ntohs(i16);
    pIn += sizeof(int16_t);

    Integer n, e, d, p, q, dp, dq, u;

    READ_INTEGER(n);
    READ_INTEGER(e);
    READ_INTEGER(d);
    READ_INTEGER(p);
    READ_INTEGER(q);
    READ_INTEGER(dp);
    READ_INTEGER(dq);
    READ_INTEGER(u);

    mpData->mpKey = new RSAPrivateKey(n, e, d, p, q, dp, dq, u);
}

cRSAPrivateKey::~cRSAPrivateKey()
{
    if (mpData)
    {
        delete mpData->mpKey;
        delete mpData;
    }
}

int cRSAPrivateKey::GetWriteLen() const
{
    ASSERT(mpData->mpKey);

    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetModulus().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetExponent().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetPrime1().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetPrime2().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetDecryptionExponent().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetParameterDP().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetParameterDQ().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetParameterU().IsPositive());

    int len = sizeof(int16_t) + mpData->mpKey->GetTrapdoorFunction().GetModulus().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetExponent().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetPrime1().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetPrime2().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) +
              mpData->mpKey->GetTrapdoorFunction().GetDecryptionExponent().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetParameterDP().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetParameterDQ().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetParameterU().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t);

    return len;
}

void cRSAPrivateKey::Write(void* pDataStream) const
{
    ASSERT(mpData->mpKey);

    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetModulus().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetExponent().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetPrime1().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetPrime2().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetDecryptionExponent().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetParameterDP().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetParameterDQ().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetParameterU().IsPositive());

    uint8_t* pOut = (uint8_t*)pDataStream;
    int16_t i16;
    int32_t i32;

    i16 = tw_htons(mpData->mKeyLength);
    memcpy(pOut, &i16, sizeof(i16));
    pOut += sizeof(int16_t);

    Integer n, e, d, p, q, dp, dq, u;

    n  = mpData->mpKey->GetTrapdoorFunction().GetModulus();
    e  = mpData->mpKey->GetTrapdoorFunction().GetExponent();
    p  = mpData->mpKey->GetTrapdoorFunction().GetPrime1();
    q  = mpData->mpKey->GetTrapdoorFunction().GetPrime2();
    d  = mpData->mpKey->GetTrapdoorFunction().GetDecryptionExponent();
    dp = mpData->mpKey->GetTrapdoorFunction().GetParameterDP();
    dq = mpData->mpKey->GetTrapdoorFunction().GetParameterDQ();
    u  = mpData->mpKey->GetTrapdoorFunction().GetParameterU();

    int32_t len;

#    define WRITE_INTEGER(I)                       \
        len = I.MinEncodedSize(Integer::UNSIGNED); \
        i32 = tw_htonl(len);                       \
        memcpy(pOut, &i32, sizeof(i32));           \
        pOut += sizeof(int32_t);                     \
        I.Encode(pOut, len, Integer::UNSIGNED);    \
        pOut += len;

    WRITE_INTEGER(n);
    WRITE_INTEGER(e);
    WRITE_INTEGER(d);
    WRITE_INTEGER(p);
    WRITE_INTEGER(q);
    WRITE_INTEGER(dp);
    WRITE_INTEGER(dq);
    WRITE_INTEGER(u);
}

// class cRSAPublicKey

class cRSAPublicKey_i
{
public:
    int16_t       mKeyLength;
    RSAPublicKey* mpKey;
};

cRSAPublicKey::cRSAPublicKey()
{
    mpData             = new cRSAPublicKey_i;
    mpData->mpKey      = 0;
    mpData->mKeyLength = 0;
}

cRSAPublicKey::cRSAPublicKey(void* pDataStream)
{
    mpData = new cRSAPublicKey_i;

    Integer n, e;
    int32_t   len;
    int16_t   i16;
    int32_t   i32;

    uint8_t* pIn = (uint8_t*)pDataStream;

    memcpy(&i16, pIn, sizeof(i16));
    mpData->mKeyLength = tw_ntohs(i16);
    pIn += sizeof(int16_t);

    READ_INTEGER(n);
    READ_INTEGER(e);

    //std::cout << "cRSAPublicKey n = " << n << std::endl;
    //std::cout << "cRSAPublicKey e = " << e << std::endl;

    mpData->mpKey = new RSAPublicKey(n, e);
}

cRSAPublicKey::cRSAPublicKey(const cRSAPrivateKey& privateKey)
{
    mpData = new cRSAPublicKey_i;

    ASSERT(privateKey.mpData);
    ASSERT(privateKey.mpData->mpKey);

    mpData->mKeyLength = privateKey.mpData->mKeyLength;
    mpData->mpKey      = new RSAPublicKey(*privateKey.mpData->mpKey);
}

cRSAPublicKey::~cRSAPublicKey()
{
    if (mpData)
    {
        delete mpData->mpKey;
        delete mpData;
    }
}

int cRSAPublicKey::GetWriteLen() const
{
    ASSERT(mpData->mpKey);

    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetModulus().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetExponent().IsPositive());

    int len = sizeof(int16_t) + mpData->mpKey->GetTrapdoorFunction().GetModulus().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetTrapdoorFunction().GetExponent().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t);

    return len;
}

void cRSAPublicKey::Write(void* pDataStream) const
{
    ASSERT(mpData->mpKey);

    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetModulus().IsPositive());
    ASSERT(mpData->mpKey->GetTrapdoorFunction().GetExponent().IsPositive());

    int16_t i16;
    int32_t i32;
    uint8_t* pOut = (uint8_t*)pDataStream;

    i16 = tw_htons(mpData->mKeyLength);
    memcpy(pOut, &i16, sizeof(i16));
    pOut += sizeof(int16_t);

    Integer n, e;

    n = mpData->mpKey->GetTrapdoorFunction().GetModulus();
    e = mpData->mpKey->GetTrapdoorFunction().GetExponent();

    int32_t len;

    WRITE_INTEGER(n);
    WRITE_INTEGER(e);
}

#    ifdef DEBUG
void cRSAPublicKey::TraceContents()
{
    cDebug d("cRSAPublicKey::TraceContents");

    {
        TOSTRINGSTREAM os;
        os << mpData->mpKey->GetTrapdoorFunction().GetModulus();
	tss_mkstr(msg, os);
        d.TraceDebug("Modulus = %s\n", msg.c_str());
    }
    {
        TOSTRINGSTREAM os;
        os << mpData->mpKey->GetTrapdoorFunction().GetExponent();
	tss_mkstr(msg, os);
        d.TraceDebug("Exponent = %s\n", msg.c_str());
    }
}
#    endif

// class cRSA

class cRSA_i
{
public:
    int mKeyBits;

    enum Action
    {
        ENCRYPT,
        DECRYPT,
        SIGN,
        VERIFY
    };

    Action mAction;

    const cRSAPrivateKey* mpPrivateKey;
    const cRSAPublicKey*  mpPublicKey;

    X917RNG* mpRNG;
};

cRSA::cRSA(KeySize keysize)
{
    Init(keysize);
}

cRSA::cRSA(const cRSAPublicKey& publicKey)
{
    ASSERT(publicKey.mpData);
    Init((KeySize)publicKey.mpData->mKeyLength);
}

cRSA::cRSA(const cRSAPrivateKey& privateKey)
{
    ASSERT(privateKey.mpData);
    Init((KeySize)privateKey.mpData->mKeyLength);
}

void cRSA::Init(KeySize keysize)
{
    mpData               = new cRSA_i;
    mpData->mpPrivateKey = 0;
    mpData->mpPublicKey  = 0;

    mpData->mKeyBits = (keysize == KEY256) ?
                           256 :
                           (keysize == KEY512) ? 512 : (keysize == KEY1024) ? 1024 : (keysize == KEY2048) ? 2048 : 256;

    // Create a random seed and a key
    uint8_t seed[MD5::DATASIZE];
    uint8_t deskey[TripleDES_Encryption::KEYLENGTH];
    RandomizeBytes((int8_t*)seed, MD5::DATASIZE);
    RandomizeBytes((int8_t*)deskey, TripleDES_Encryption::KEYLENGTH);

    mpData->mpRNG = new X917RNG(new TripleDES_Encryption(deskey), seed);
}

cRSA::~cRSA()
{
    if (mpData)
    {
        delete mpData->mpRNG;
        delete mpData;
        mpData = 0;
    }
}

void cRSA::SetEncrypting(const cRSAPublicKey* pKey)
{
    ASSERT(pKey->mpData->mKeyLength == mpData->mKeyBits);
    if (pKey->mpData->mKeyLength != mpData->mKeyBits)
        ThrowAndAssert(eInternal(_T("RSA Key length mismatch.")));


    mpData->mAction      = cRSA_i::ENCRYPT;
    mpData->mpPublicKey  = pKey;
    mpData->mpPrivateKey = 0;
}

void cRSA::SetDecrypting(const cRSAPrivateKey* pKey)
{
    ASSERT(pKey->mpData->mKeyLength == mpData->mKeyBits);
    if (pKey->mpData->mKeyLength != mpData->mKeyBits)
        ThrowAndAssert(eInternal(_T("RSA Key length mismatch.")));

    mpData->mAction      = cRSA_i::DECRYPT;
    mpData->mpPrivateKey = pKey;
    mpData->mpPublicKey  = 0;
}

void cRSA::SetSigning(const cRSAPrivateKey* pKey)
{
    ASSERT(pKey->mpData->mKeyLength == mpData->mKeyBits);
    if (pKey->mpData->mKeyLength != mpData->mKeyBits)
        ThrowAndAssert(eInternal(_T("RSA Key length mismatch.")));

    mpData->mAction      = cRSA_i::SIGN;
    mpData->mpPrivateKey = pKey;
    mpData->mpPublicKey  = 0;
}

void cRSA::SetVerifying(const cRSAPublicKey* pKey)
{
    ASSERT(pKey->mpData->mKeyLength == mpData->mKeyBits);
    if (pKey->mpData->mKeyLength != mpData->mKeyBits)
        ThrowAndAssert(eInternal(_T("RSA Key length mismatch.")));

    mpData->mAction      = cRSA_i::VERIFY;
    mpData->mpPublicKey  = pKey;
    mpData->mpPrivateKey = 0;
}

// Lets encrypt in 128 bit chunks, even though the crypto
// lib implementation can do arbitrary length plaintext
// encryptions.  We will most likely only use this to
// encrypt a 128 bit random number anyway.
int cRSA::GetBlockSizePlain()
{
    //Integer i = mpData->mKeyBits  - 1;

    return (mpData->mKeyBits >> 3) - 11;
}

int cRSA::GetBlockSizeCipher()
{
    return mpData->mKeyBits >> 3;
}

void cRSA::ProcessBlock(const void* indata, void* outdata)
{
    ASSERT(mpData);
    ASSERT(mpData->mpPrivateKey != 0 || mpData->mpPublicKey != 0);

    if (mpData->mpPrivateKey == 0 && mpData->mpPublicKey == 0)
        ThrowAndAssert(eInternal(_T("RSA Key length mismatch.")));

    unsigned int l;

    switch (mpData->mAction)
    {
    case cRSA_i::ENCRYPT:
    {
        ASSERT(mpData->mpPublicKey);
        ASSERT(mpData->mpPublicKey->mpData->mpKey->MaxPlainTextLength() == GetBlockSizePlain());
        ASSERT(mpData->mpPublicKey->mpData->mpKey->CipherTextLength() == GetBlockSizeCipher());
        mpData->mpPublicKey->mpData->mpKey->Encrypt(
            *mpData->mpRNG, (const uint8_t*)indata, GetBlockSizePlain(), (uint8_t*)outdata);
        break;
    }
    case cRSA_i::DECRYPT:
    {
        ASSERT(mpData->mpPrivateKey);
        ASSERT(mpData->mpPrivateKey->mpData->mpKey->CipherTextLength() == GetBlockSizeCipher());
        l = mpData->mpPrivateKey->mpData->mpKey->Decrypt((const uint8_t*)indata, (uint8_t*)outdata);
        if (l != GetBlockSizePlain())
            throw eArchiveCrypto();
        break;
    }
    case cRSA_i::SIGN:
    {
        ASSERT(mpData->mpPrivateKey);
        ASSERT(mpData->mpPrivateKey->mpData->mpKey->MaxMessageLength() == GetBlockSizePlain());
        ASSERT(mpData->mpPrivateKey->mpData->mpKey->SignatureLength() == GetBlockSizeCipher());
        mpData->mpPrivateKey->mpData->mpKey->Sign(
            *mpData->mpRNG, (const uint8_t*)indata, GetBlockSizePlain(), (uint8_t*)outdata);
        break;
    }
    case cRSA_i::VERIFY:
    {
        ASSERT(mpData->mpPublicKey);
        ASSERT(mpData->mpPublicKey->mpData->mpKey->SignatureLength() == GetBlockSizeCipher());
        l = mpData->mpPublicKey->mpData->mpKey->Recover((const uint8_t*)indata, (uint8_t*)outdata);
        if (l != GetBlockSizePlain())
            throw eArchiveCrypto();
        break;
    }
    default:
    {
        ASSERT(false);
        break;
    }
    }
}

void cRSA::GenerateKeys(cRSAPrivateKey*& retPrivate, cRSAPublicKey*& retPublic)
{
    RSAPrivateKey* pNewPrivateKey = new RSAPrivateKey(*mpData->mpRNG, mpData->mKeyBits);
    RSAPublicKey*  pNewPublicKey  = new RSAPublicKey(*pNewPrivateKey);

    retPrivate                     = new cRSAPrivateKey();
    retPrivate->mpData->mpKey      = pNewPrivateKey;
    retPrivate->mpData->mKeyLength = mpData->mKeyBits;

    retPublic                     = new cRSAPublicKey();
    retPublic->mpData->mpKey      = pNewPublicKey;
    retPublic->mpData->mKeyLength = mpData->mKeyBits;

#    ifdef DEBUG
    int l;
    l = retPublic->mpData->mpKey->MaxPlainTextLength();
    ASSERT(l == GetBlockSizePlain());
    l = retPublic->mpData->mpKey->CipherTextLength();
    ASSERT(l == GetBlockSizeCipher());
    l = retPrivate->mpData->mpKey->CipherTextLength();
    ASSERT(l == GetBlockSizeCipher());
#    endif
}

#endif // _RSA_ENCRYPTION

///////////////////////////////////////////////////////////////////////////////
// class cElGamalSig

// class cElGamalSigPrivateKey

class cElGamalSigPrivateKey_i
{
public:
    int16_t               mKeyLength;
    ElGamalSigPrivateKey* mpKey;
};

cElGamalSigPrivateKey::cElGamalSigPrivateKey()
{
    mpData             = new cElGamalSigPrivateKey_i;
    mpData->mpKey      = 0;
    mpData->mKeyLength = 0;
}

cElGamalSigPrivateKey::cElGamalSigPrivateKey(void* pDataStream)
{
    mpData = new cElGamalSigPrivateKey_i;

    int32_t  len;
    int32_t  i32;
    int16_t  i16;
    uint32_t magicNum;

    uint8_t* pIn = (uint8_t*)pDataStream;

    memcpy(&i16, pIn, sizeof(i16));
    mpData->mKeyLength = tw_ntohs(i16);
    pIn += sizeof(int16_t);

    memcpy(&i32, pIn, sizeof(i32));
    magicNum = tw_ntohl(i32);
    pIn += sizeof(int32_t);

    if (magicNum != EL_GAMAL_SIG_PRIVATE_MAGIC_NUM)
        ThrowAndAssert(eArchiveOpen());

    Integer p, q, g, y, x;

    READ_INTEGER(p);
    READ_INTEGER(q);
    READ_INTEGER(g);
    READ_INTEGER(y);
    READ_INTEGER(x);

    mpData->mpKey = new ElGamalSigPrivateKey(p, q, g, y, x);
}

cElGamalSigPrivateKey::~cElGamalSigPrivateKey()
{
    if (mpData)
    {
        delete mpData->mpKey;
        delete mpData;
    }
}

int cElGamalSigPrivateKey::GetWriteLen() const
{
    ASSERT(mpData->mpKey != 0);

    ASSERT(mpData->mpKey->GetPrime().IsPositive());
    ASSERT(mpData->mpKey->GetParameterQ().IsPositive());
    ASSERT(mpData->mpKey->GetParameterG().IsPositive());
    ASSERT(mpData->mpKey->GetParameterY().IsPositive());
    ASSERT(mpData->mpKey->GetParameterX().IsPositive());

    int len = sizeof(int16_t) + sizeof(int32_t) + mpData->mpKey->GetPrime().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetParameterQ().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t) +
              mpData->mpKey->GetParameterG().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t) +
              mpData->mpKey->GetParameterY().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t) +
              mpData->mpKey->GetParameterX().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t);

    return len;
}

void cElGamalSigPrivateKey::Write(void* pDataStream) const
{
    ASSERT(mpData->mpKey != 0);

    ASSERT(mpData->mpKey->GetPrime().IsPositive());
    ASSERT(mpData->mpKey->GetParameterQ().IsPositive());
    ASSERT(mpData->mpKey->GetParameterG().IsPositive());
    ASSERT(mpData->mpKey->GetParameterY().IsPositive());
    ASSERT(mpData->mpKey->GetParameterX().IsPositive());

    uint8_t* pOut = (uint8_t*)pDataStream;
    int16_t i16;
    int32_t i32;

    i16 = tw_htons(mpData->mKeyLength);
    memcpy(pOut, &i16, sizeof(i16));
    pOut += sizeof(int16_t);

    i32 = tw_htonl(EL_GAMAL_SIG_PRIVATE_MAGIC_NUM);
    memcpy(pOut, &i32, sizeof(i32));
    pOut += sizeof(int32_t);

    Integer p, q, g, y, x;

    p = mpData->mpKey->GetPrime();
    q = mpData->mpKey->GetParameterQ();
    g = mpData->mpKey->GetParameterG();
    y = mpData->mpKey->GetParameterY();
    x = mpData->mpKey->GetParameterX();

    int32_t len;

    WRITE_INTEGER(p);
    WRITE_INTEGER(q);
    WRITE_INTEGER(g);
    WRITE_INTEGER(y);
    WRITE_INTEGER(x);
}

// class cElGamalSigPublicKey

class cElGamalSigPublicKey_i
{
public:
    int16_t              mKeyLength;
    ElGamalSigPublicKey* mpKey;
};

cElGamalSigPublicKey::cElGamalSigPublicKey()
{
    mpData             = new cElGamalSigPublicKey_i;
    mpData->mpKey      = 0;
    mpData->mKeyLength = 0;
}

cElGamalSigPublicKey::cElGamalSigPublicKey(void* pDataStream)
{
    mpData = new cElGamalSigPublicKey_i;

    Integer   p, q, g, y;
    int32_t   len;
    int16_t   i16;
    int32_t   i32;
    uint32_t  magicNum;

    uint8_t* pIn = (uint8_t*)pDataStream;

    memcpy(&i16, pIn, sizeof(i16));
    mpData->mKeyLength = tw_ntohs(i16);
    pIn += sizeof(int16_t);

    memcpy(&i32, pIn, sizeof(i32));
    magicNum = tw_ntohl(i32);
    pIn += sizeof(int32_t);

    if (magicNum != EL_GAMAL_SIG_PUBLIC_MAGIC_NUM)
        ThrowAndAssert(eArchiveOpen());

    READ_INTEGER(p);
    READ_INTEGER(q);
    READ_INTEGER(g);
    READ_INTEGER(y);

    //std::cout << "cElGamalSigPublicKey n = " << n << std::endl;
    //std::cout << "cElGamalSigPublicKey e = " << e << std::endl;

    mpData->mpKey = new ElGamalSigPublicKey(p, q, g, y);
}

cElGamalSigPublicKey::cElGamalSigPublicKey(const cElGamalSigPrivateKey& privateKey)
{
    mpData = new cElGamalSigPublicKey_i;

    ASSERT(privateKey.mpData != 0);
    ASSERT(privateKey.mpData->mpKey != 0);

    mpData->mKeyLength = privateKey.mpData->mKeyLength;
    mpData->mpKey      = new ElGamalSigPublicKey(*privateKey.mpData->mpKey);
}

cElGamalSigPublicKey::~cElGamalSigPublicKey()
{
    if (mpData)
    {
        delete mpData->mpKey;
        delete mpData;
    }
}

int cElGamalSigPublicKey::GetWriteLen() const
{
    ASSERT(mpData->mpKey != 0);

    ASSERT(mpData->mpKey->GetPrime().IsPositive());
    ASSERT(mpData->mpKey->GetParameterQ().IsPositive());
    ASSERT(mpData->mpKey->GetParameterG().IsPositive());
    ASSERT(mpData->mpKey->GetParameterY().IsPositive());

    int len = sizeof(int16_t) + sizeof(int32_t) + mpData->mpKey->GetPrime().MinEncodedSize(Integer::UNSIGNED) +
              sizeof(int32_t) + mpData->mpKey->GetParameterQ().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t) +
              mpData->mpKey->GetParameterG().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t) +
              mpData->mpKey->GetParameterY().MinEncodedSize(Integer::UNSIGNED) + sizeof(int32_t);

    return len;
}

void cElGamalSigPublicKey::Write(void* pDataStream) const
{
    ASSERT(mpData->mpKey != 0);

    ASSERT(mpData->mpKey->GetPrime().IsPositive());
    ASSERT(mpData->mpKey->GetParameterQ().IsPositive());
    ASSERT(mpData->mpKey->GetParameterG().IsPositive());
    ASSERT(mpData->mpKey->GetParameterY().IsPositive());

    uint8_t* pOut = (uint8_t*)pDataStream;
    int16_t i16;
    int32_t i32;

    i16 = tw_htons(mpData->mKeyLength);
    memcpy(pOut, &i16, sizeof(i16));
    pOut += sizeof(int16_t);

    i32 = tw_htonl(EL_GAMAL_SIG_PUBLIC_MAGIC_NUM);
    memcpy(pOut, &i32, sizeof(i32));
    pOut += sizeof(int32_t);

    Integer p, q, g, y;

    p = mpData->mpKey->GetPrime();
    q = mpData->mpKey->GetParameterQ();
    g = mpData->mpKey->GetParameterG();
    y = mpData->mpKey->GetParameterY();

    int32_t len;

    WRITE_INTEGER(p);
    WRITE_INTEGER(q);
    WRITE_INTEGER(g);
    WRITE_INTEGER(y);
}

bool cElGamalSigPublicKey::IsEqual(const cElGamalSigPublicKey& rhs) const
{
    return this->mpData->mpKey->GetPrime() == rhs.mpData->mpKey->GetPrime() &&
           this->mpData->mpKey->GetParameterQ() == rhs.mpData->mpKey->GetParameterQ() &&
           this->mpData->mpKey->GetParameterG() == rhs.mpData->mpKey->GetParameterG() &&
           this->mpData->mpKey->GetParameterY() == rhs.mpData->mpKey->GetParameterY();
}

#ifdef DEBUG
void cElGamalSigPublicKey::TraceContents()
{
    cDebug d("cElGamalSigPublicKey::TraceContents");

    {
        TOSTRINGSTREAM os;
        os << mpData->mpKey->GetPrime();
	tss_mkstr(msg, os);
        d.TraceDebug("Prime = %s\n", msg.c_str());
    }
    {
        TOSTRINGSTREAM os;
        os << mpData->mpKey->GetParameterQ();
	tss_mkstr(msg, os);
        d.TraceDebug("Q = %s\n", msg.c_str());
    }
    {
        TOSTRINGSTREAM os;
        os << mpData->mpKey->GetParameterG();
	tss_mkstr(msg, os);
        d.TraceDebug("G = %s\n", msg.c_str());
    }
    {
        TOSTRINGSTREAM os;
        os << mpData->mpKey->GetParameterY();
	tss_mkstr(msg, os);
        d.TraceDebug("Y = %s\n", msg.c_str());
    }
}
#endif

// class cElGamalSig

class cElGamalSig_i
{
public:
    int mKeyBits;

    enum Action
    { /*ENCRYPT, DECRYPT,*/ SIGN,
      VERIFY };

    Action mAction;

    const cElGamalSigPrivateKey* mpPrivateKey;
    const cElGamalSigPublicKey*  mpPublicKey;

    SHA      mSHA;
    X917RNG* mpRNG;
};

cElGamalSig::cElGamalSig(KeySize keysize)
{
    Init(keysize);
}

cElGamalSig::cElGamalSig(const cElGamalSigPublicKey& publicKey)
{
    ASSERT(publicKey.mpData != 0);
    Init((KeySize)publicKey.mpData->mKeyLength);
}

cElGamalSig::cElGamalSig(const cElGamalSigPrivateKey& privateKey)
{
    ASSERT(privateKey.mpData != 0);
    Init((KeySize)privateKey.mpData->mKeyLength);
}

void cElGamalSig::Init(KeySize keysize)
{
    mpData               = new cElGamalSig_i;
    mpData->mpPrivateKey = 0;
    mpData->mpPublicKey  = 0;

    mpData->mKeyBits = (keysize == KEY256) ?
                           256 :
                           (keysize == KEY512) ? 512 : (keysize == KEY1024) ? 1024 : (keysize == KEY2048) ? 2048 : 256;

    // Create a random seed and a key
    uint8_t seed[SHA::DATASIZE];
    uint8_t deskey[TripleDES_Encryption::KEYLENGTH];
    RandomizeBytes((int8_t*)seed,   SHA::DATASIZE);
    RandomizeBytes((int8_t*)deskey, TripleDES_Encryption::KEYLENGTH);

    mpData->mpRNG = new X917RNG(new TripleDES_Encryption(deskey), seed);
}

cElGamalSig::~cElGamalSig()
{
    if (mpData)
    {
        delete mpData->mpRNG;
        delete mpData;
        mpData = 0;
    }
}

void cElGamalSig::SetSigning(const cElGamalSigPrivateKey* pKey)
{
    ASSERT(pKey->mpData->mKeyLength == mpData->mKeyBits);
    if (pKey->mpData->mKeyLength != mpData->mKeyBits)
        ThrowAndAssert(eInternal(_T("Signature Key length mismatch.")));

    mpData->mAction      = cElGamalSig_i::SIGN;
    mpData->mpPrivateKey = pKey;
    mpData->mpPublicKey  = 0;
}

void cElGamalSig::SetVerifying(const cElGamalSigPublicKey* pKey)
{
    ASSERT(pKey->mpData->mKeyLength == mpData->mKeyBits);
    if (pKey->mpData->mKeyLength != mpData->mKeyBits)
        ThrowAndAssert(eInternal(_T("Signature Key length mismatch.")));

    mpData->mAction      = cElGamalSig_i::VERIFY;
    mpData->mpPublicKey  = pKey;
    mpData->mpPrivateKey = 0;
}

int cElGamalSig::GetBlockSizePlain()
{
    return PLAIN_BLOCK_SIZE;
}

int cElGamalSig::GetBlockSizeCipher()
{
    return PLAIN_BLOCK_SIZE + (NumberTheory::DiscreteLogWorkFactor(mpData->mKeyBits) >> 1) + 4;
    // got this from nbtheory.cpp in crypto++ lib
    // El Gamal's sig size = 2 * 2 * (DiscreteLogWorkFactor() >> 3)
}

void cElGamalSig::ProcessBlock(const void* indata, void* outdata)
{
    ASSERT(mpData != 0);
    ASSERT(mpData->mpPrivateKey != 0 || mpData->mpPublicKey != 0);

    int8_t shaSig[SHA::DIGESTSIZE];

    if (mpData->mpPrivateKey == 0 && mpData->mpPublicKey == 0)
        ThrowAndAssert(eInternal(_T("Signature Key length mismatch.")));

    switch (mpData->mAction)
    {
    case cElGamalSig_i::SIGN:
    {
        ASSERT(mpData->mpPrivateKey != 0);
        ASSERT((int)mpData->mpPrivateKey->mpData->mpKey->SignatureLength() + PLAIN_BLOCK_SIZE <= GetBlockSizeCipher());

        memmove(outdata, indata, PLAIN_BLOCK_SIZE);

        mpData->mSHA.CalculateDigest((uint8_t*)shaSig, (uint8_t*)outdata, PLAIN_BLOCK_SIZE);

        RandomizeBytes((int8_t*)outdata + PLAIN_BLOCK_SIZE, GetBlockSizeCipher() - PLAIN_BLOCK_SIZE);
        mpData->mpPrivateKey->mpData->mpKey->Sign(
            *mpData->mpRNG, (const uint8_t*)shaSig, SHA::DIGESTSIZE, (uint8_t*)outdata + PLAIN_BLOCK_SIZE);

        /*
                Integer m((const uint8_t*)indata, PLAIN_BLOCK_SIZE);

                std::cout << "Signing:\n";
                std::cout << "M = " << m << std::endl;

                const uint8_t* signature = (const uint8_t *)outdata + PLAIN_BLOCK_SIZE;
                int qLen = mpData->mpPrivateKey->mpData->mpKey->q.ByteCount();
                Integer rs(signature, qLen);
                Integer ss(signature+qLen, qLen);
                std::cout << "Stored R = " << rs << std::endl;
                std::cout << "Stored S = " << ss << std::endl;
            */

        break;
    }
    case cElGamalSig_i::VERIFY:
    {
        ASSERT(mpData->mpPublicKey != 0);
        ASSERT((int)mpData->mpPublicKey->mpData->mpKey->SignatureLength() + PLAIN_BLOCK_SIZE <= GetBlockSizeCipher());

        mpData->mSHA.CalculateDigest((uint8_t*)shaSig, (uint8_t*)indata, PLAIN_BLOCK_SIZE);

        /*
                const uint8_t* signature = (const uint8_t *)indata + PLAIN_BLOCK_SIZE;
                int qLen = mpData->mpPublicKey->mpData->mpKey->q.ByteCount();
                Integer m((const uint8_t*)indata, PLAIN_BLOCK_SIZE);
                Integer r(signature, qLen);
                Integer s(signature+qLen, qLen);
                std::cout << "Verifying:\n";
                std::cout << "M = " << m << std::endl;
                std::cout << "R = " << r << std::endl;
                std::cout << "S = " << s << std::endl;
            */

        if (mpData->mpPublicKey->mpData->mpKey->Verify(
                (const uint8_t*)shaSig, SHA::DIGESTSIZE, (const uint8_t*)indata + PLAIN_BLOCK_SIZE) == false)
            throw eArchiveCrypto();
        memmove(outdata, indata, PLAIN_BLOCK_SIZE);
        break;
    }
    default:
    {
        ASSERT(false);
        break;
    }
    }
}

void cElGamalSig::GenerateKeys(cElGamalSigPrivateKey*& retPrivate, cElGamalSigPublicKey*& retPublic)
{
    ElGamalSigPrivateKey* pNewPrivateKey = new ElGamalSigPrivateKey(*mpData->mpRNG, mpData->mKeyBits);
    ElGamalSigPublicKey*  pNewPublicKey  = new ElGamalSigPublicKey(*pNewPrivateKey);

    retPrivate                     = new cElGamalSigPrivateKey();
    retPrivate->mpData->mpKey      = pNewPrivateKey;
    retPrivate->mpData->mKeyLength = (int16_t)mpData->mKeyBits;

    retPublic                     = new cElGamalSigPublicKey();
    retPublic->mpData->mpKey      = pNewPublicKey;
    retPublic->mpData->mKeyLength = (int16_t)mpData->mKeyBits;

#ifdef DEBUG
    int l;
    l = retPublic->mpData->mpKey->SignatureLength();
    ASSERT(l + PLAIN_BLOCK_SIZE <= GetBlockSizeCipher());
    l = retPrivate->mpData->mpKey->SignatureLength();
    ASSERT(l + PLAIN_BLOCK_SIZE <= GetBlockSizeCipher());
#endif
}

///////////////////////////////////////////////////////////////////////////////
// class cHashedKey128 -- A 128 bit key based on the hash value of some data

cHashedKey128::cHashedKey128(const TSTRING& data)
{
    SHA sha;

    ASSERT(SHA::DIGESTSIZE >= KEYLEN);
    sha.CalculateDigest((uint8_t*)mKey, (uint8_t*)data.data(), data.length() * sizeof(TCHAR));
}

cHashedKey128::cHashedKey128(void* pData, int32_t dataLen)
{
    SHA sha;

    ASSERT(SHA::DIGESTSIZE >= KEYLEN);
    ASSERT(SHA::DIGESTSIZE <= BUFFER_SIZE);
    sha.CalculateDigest((uint8_t*)mKey, (uint8_t*)pData, dataLen);
}

cHashedKey128::~cHashedKey128()
{
    RandomizeBytes(mKey, KEYLEN);
}

///////////////////////////////////////////////////////////////////////////////
// class cHashedKey192 -- A 192 bit key based on the hash value of some data

cHashedKey192::cHashedKey192(const TSTRING& data)
{
    SHA sha;

    ASSERT(SHA::DIGESTSIZE == 20);
    ASSERT(40 >= KEYLEN);

    uint8_t localKey[40];
    sha.CalculateDigest(localKey, (uint8_t*)data.data(), data.length() * sizeof(TCHAR));
    sha.CalculateDigest(localKey + 20, localKey, 20);
    memcpy(mKey, localKey, KEYLEN);
}

cHashedKey192::cHashedKey192(void* pData, int32_t dataLen)
{
    SHA sha;

    ASSERT(SHA::DIGESTSIZE == 20);
    ASSERT(40 >= KEYLEN);

    uint8_t localKey[40];
    sha.CalculateDigest(localKey, (uint8_t*)pData, dataLen);
    sha.CalculateDigest(localKey + 20, localKey, 20);
    memcpy(mKey, localKey, KEYLEN);
}

cHashedKey192::~cHashedKey192()
{
    RandomizeBytes(mKey, KEYLEN);
}

/////////////////////////////////////////////////////////

#if USE_DEV_URANDOM
static bool randomize_by_device(const char* device_name, int8_t* destbuf, int len)
{
    static int rng_device = -1;

    if (-1 == rng_device)
        rng_device = open(device_name, O_RDONLY | O_NONBLOCK);

    if (rng_device >= 0)
    {
        int bytes_read = read(rng_device, destbuf, len);
        if (bytes_read == len)
            return true;
    }

    return false;
}
#else
static bool gRandomizeBytesSeeded = false;
#endif

///////////////////////////////////////////////////////////////////////////////
// void RandomizeBytes(byte* destbuf, int len) -- Fill a buffer with random bytes

void RandomizeBytes(int8_t* destbuf, int len)
{
#if USE_DEV_URANDOM
    if (randomize_by_device("/dev/urandom", destbuf, len))
        return;

    ThrowAndAssert(eInternal(_T("Failed to read from RNG device")));
#else
    if (!gRandomizeBytesSeeded)
    {
        // generate a rancom number from processor timing.
        // this should be fairly unpredictable.
        time_t mask, start = time(NULL);
        for (mask = 0xb147688c; time(NULL) - start < 1; mask += 0x8984cc88)
            ;

#    ifdef DEBUG
        time_t t = time(NULL);
        t ^= mask;

        //time_t t = 920492046 - 3600; // try to the same random values each time
        //std::cerr << (*(time_t*)mask()) << ":" << t << std::endl;

        srand(t);
#    else
        srand(time(NULL) ^ mask);
#    endif
        gRandomizeBytesSeeded = true;
    }

    for (int i = 0; i < len; ++i) {
        destbuf[i] = (int8_t)((rand() * 256 / RAND_MAX) ^ 0xdc); // 0xdc came from random.org
    }
#endif
}
