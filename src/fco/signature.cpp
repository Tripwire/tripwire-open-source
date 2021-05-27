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
// signature.cpp -- interface for calculating file signatures
///////////////////////////////////////////////////////////////////////////////

#include "stdfco.h"
#include "signature.h"
#include "core/errorgeneral.h"
#include "core/serializer.h"
#include <fstream>
#include <iomanip>
#include "fcoundefprop.h"
#include "core/archive.h"
#include "core/debug.h"
#ifndef HAVE_OPENSSL_MD5_H
#    ifdef HAVE_STRINGS_H
#        include <strings.h> /* for bcopy(), this is only needed for Solaris */
#    endif
#endif

using namespace std;

// Vector for base64 conversion
//
// The standard "digits" for base64 are "[A-Z][a-z][0-9]+/"
// (see RFC 2045 at http://sunsite.doc.ic.ac.uk/rfc/rfc2045.txt).
// However, Tripwire 1.3 used a different set of characters.
//
// In order to more closely follow the standard I changed our
// encoding to the standard.  dmb 09-30-1998
//
static char base64vec[] =
    //"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:.";
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

///////////////////////////////////////////////////////////////////////////////
// class iSignature -- Interface all signatures will implement.
///////////////////////////////////////////////////////////////////////////////

iFCOProp::CmpResult iSignature::Compare(const iFCOProp* rhs, Op op) const
{
    // make sure we support this operation
    if (op != iFCOProp::OP_EQ && op != iFCOProp::OP_NE)
    {
        ASSERT(false);
        return iFCOProp::CMP_UNSUPPORTED;
    }

    // compares with undefined props are not equal
    if (rhs->GetType() == cFCOUndefinedProp::GetInstance()->GetType())
    {
        return (op == iFCOProp::OP_EQ) ? iFCOProp::CMP_FALSE : iFCOProp::CMP_TRUE;
    }

    // make sure we are the right type...
    if (this->GetType() != rhs->GetType())
    {
        ASSERT(false);
        return iFCOProp::CMP_WRONG_PROP_TYPE;
    }

    ASSERT((op == iFCOProp::OP_EQ) || (op == iFCOProp::OP_NE));

    if (IsEqual(*static_cast<const iSignature*>(rhs)))
        return (op == iFCOProp::OP_EQ) ? iFCOProp::CMP_TRUE : iFCOProp::CMP_FALSE;
    else
        return (op == iFCOProp::OP_NE) ? iFCOProp::CMP_TRUE : iFCOProp::CMP_FALSE;
}

bool cArchiveSigGen::s_hex    = false;
bool cArchiveSigGen::s_direct = false;

void cArchiveSigGen::AddSig(iSignature* pSig)
{
    mSigList.push_back(pSig);
}

void cArchiveSigGen::CalculateSignatures(cArchive& a)
{
    uint8_t                   abBuf[iSignature::SUGGESTED_BLOCK_SIZE * 2];
    int                       cbRead;
    container_type::size_type i;
    uint8_t*                  pBuf = abBuf;

    if (s_direct)
    {
        uintptr_t mod    = (uintptr_t)abBuf % iSignature::SUGGESTED_BLOCK_SIZE;
        uintptr_t offset = (iSignature::SUGGESTED_BLOCK_SIZE - mod);
        pBuf                 = abBuf + offset;
    }

    // init hash
    for (i = 0; i < mSigList.size(); i++)
        mSigList[i]->Init();

    // hash data
    do
    {
        cbRead = a.ReadBlob(pBuf, iSignature::SUGGESTED_BLOCK_SIZE);

        for (i = 0; i < mSigList.size(); i++)
            mSigList[i]->Update(pBuf, cbRead);
    } while (cbRead == iSignature::SUGGESTED_BLOCK_SIZE);

    // finalize hash
    for (i = 0; i < mSigList.size(); i++)
        mSigList[i]->Finit();
}

bool cArchiveSigGen::Hex()
{
    return s_hex;
}

void cArchiveSigGen::SetHex(bool hex)
{
    s_hex = hex;
}

///////////////////////////////////////////////////////////////////////////////
// btob64  -- convert arbitrary bits to base 64 string
//
// Input: bit array (represented as u_char array)
//        number of bits in the array
//        ptr-to-str for return string val
///////////////////////////////////////////////////////////////////////////////
char* btob64(const uint8_t* pcbitvec, char* pcout, int numbits)
{
    unsigned int val;
    int          offset;
    uint8_t*     pcorig = (uint8_t*)pcout;

    ASSERT(sizeof(uint8_t) == sizeof(uint8_t)); /* everything breaks otherwise */
    assert(numbits > 0);

    val = *pcbitvec;

    offset = numbits % 6; /* how many bits initially? */
    if (offset)
    {
        val >>= (8 - offset);
        *pcout++ = base64vec[val & 0x1f];
    }

    for (numbits -= offset; numbits > 0; offset += 6, numbits -= 6)
    {
        val = *pcbitvec;
        if (offset > 2)
        {
            offset -= 8;
            val <<= 8;
            val |= *++pcbitvec;
        }
        val >>= (2 - offset);

        *pcout++ = base64vec[val & 0x3f];
    }

    *pcout = '\0';

    return (char*)pcorig;
}

///////////////////////////////////////////////////////////////////////////////
// pltob64 -- walk through a vector of int32s, convert them to
// network byte ordering, and then convert to base 64
// this is the preferred interface to btob64.
///////////////////////////////////////////////////////////////////////////////

#define NUMTMPLONGS 1000
char* pltob64(uint32_t* pl, char* pcout, int numlongs)
{
    int     i;
    uint32_t* plto;
    uint32_t  larray[NUMTMPLONGS];

    assert(numlongs < NUMTMPLONGS);
    /* we use our own ntohl() routines, but we have to do it in-place */
    memcpy((char*)larray, (char*)pl, numlongs * sizeof(uint32_t));

    for (i = 0, plto = larray; i < numlongs; i++)
    {
        *plto = tw_htonl(*plto);
        ++plto;
    }

    return btob64((uint8_t*)larray, (char*)pcout, numlongs * sizeof(uint32_t) * 8);
}

///////////////////////////////////////////////////////////////////////////////
// class cNullSignature -- The signature that is always 0
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cNullSignature, _T("cNullSignature"), 0, 1)

cNullSignature::cNullSignature()
{
}

cNullSignature::~cNullSignature()
{
}

void cNullSignature::Init()
{
}

void cNullSignature::Update(const uint8_t* const pbData, int cbDataLen)
{
}

void cNullSignature::Finit()
{
}

TSTRING cNullSignature::AsString() const
{
    TSTRING s = _T("0");
    return s;
}

TSTRING cNullSignature::AsStringHex() const
{
    TSTRING s = _T("0");
    return s;
}


bool cNullSignature::IsEqual(const iSignature& rhs) const
{
    return true;
}

void cNullSignature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("Null Signature Read")));

#ifdef DEBUG
    int16_t ret;
    pSerializer->ReadInt16(ret);
    ASSERT(ret == 123);
#endif
}

void cNullSignature::Write(iSerializer* pSerializer) const
{
#ifdef DEBUG
    pSerializer->WriteInt16(123);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// Copy
///////////////////////////////////////////////////////////////////////////////
void cNullSignature::Copy(const iFCOProp* rhs)
{
    // do nothing!
}


///////////////////////////////////////////////////////////////////////////////
// class cChecksumSignature -- A simple signature consisting of all the bytes
//      in an archive.  Useful as an example if nothing else.
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cChecksumSignature, _T("cChecksumSignature"), 0, 1)

cChecksumSignature::cChecksumSignature()
{
    mChecksum = 0;
}

cChecksumSignature::~cChecksumSignature()
{
}

void cChecksumSignature::Init()
{
}

void cChecksumSignature::Update(const uint8_t* const pbDataC, int cbDataLen)
{
    uint8_t* pbData = (uint8_t*)pbDataC;
    for (int i = 0; i < cbDataLen; i++, pbData++)
        mChecksum += *pbData;
}

void cChecksumSignature::Finit()
{
}


TSTRING cChecksumSignature::AsString() const
{
    TSTRING ret;
    char*     ps_signature;
    char      buf[100];
    uint32_t  local[2];
    local[0] = (uint32_t)(mChecksum >> 32); // note we put the MSB first
    local[1] = (uint32_t)(mChecksum);

    ps_signature = pltob64(local, buf, 2);
    //ps_signature holds base64 representation of mCRC

    ret.append(ps_signature);

    return ret;
}

TSTRING cChecksumSignature::AsStringHex() const
{
    TOSTRINGSTREAM ss;
    tss_classic_locale(ss);
    ss.setf(std::ios::hex, std::ios::basefield);
    
    ASSERT(false);
    ss << (size_t)(uint32_t)mChecksum; // TODO:BAM -- this is truncating a 64-bit value to 32 bits!

    tss_mkstr(out, ss);
    return out;
}

bool cChecksumSignature::IsEqual(const iSignature& rhs) const
{
    return mChecksum == ((cChecksumSignature&)rhs).mChecksum;
}

void cChecksumSignature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("Checksum Signature Read")));

    pSerializer->ReadInt64((int64_t&)mChecksum);
}

void cChecksumSignature::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt64((int64_t)mChecksum);
}

///////////////////////////////////////////////////////////////////////////////
// Copy
///////////////////////////////////////////////////////////////////////////////
void cChecksumSignature::Copy(const iFCOProp* rhs)
{
    ASSERT(GetType() == rhs->GetType());
    mChecksum = static_cast<const cChecksumSignature*>(rhs)->mChecksum;
}

///////////////////////////////////////////////////////////////////////////////
// class cCRC32Signature -- A CRC32 signature
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cCRC32Signature, _T("cCRC32Signature"), 0, 1);

cCRC32Signature::cCRC32Signature()
{
}

cCRC32Signature::~cCRC32Signature()
{
}

void cCRC32Signature::Init()
{
    crcInit(mCRCInfo);
}

void cCRC32Signature::Update(const uint8_t* const pbData, int cbDataLen)
{
    ASSERT(sizeof(uint8_t) == sizeof(uint8_t));
    crcUpdate(mCRCInfo, (uint8_t*)pbData, cbDataLen);
}

void cCRC32Signature::Finit()
{
    crcFinit(mCRCInfo);
}

///////////////////////////////////////////////////////////////////////////////
// AsString -- Returns a TSTRING that holds the base64 representation of
//  mCRC
TSTRING cCRC32Signature::AsString() const
{
    if (cArchiveSigGen::Hex())
        return AsStringHex();

    TSTRING ret;
    char*     ps_signature;
    char      buf[100];
    uint32_t  local = mCRCInfo.crc;

    ps_signature = pltob64(&local, buf, 1);
    //ps_signature holds base64 representation of mCRCInfo.crc
    ret.append(ps_signature);
    return ret;
}

TSTRING cCRC32Signature::AsStringHex() const
{
    TOSTRINGSTREAM ss;
    tss_classic_locale(ss);
    ss.setf(std::ios::hex, std::ios::basefield);
    
    ss << (size_t)mCRCInfo.crc;
    tss_mkstr(out, ss);
    
    return out;
}

bool cCRC32Signature::IsEqual(const iSignature& rhs) const
{
    if (this == &rhs)
        return true;
    else
        return (mCRCInfo.crc == ((cCRC32Signature&)rhs).mCRCInfo.crc);
}

void cCRC32Signature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("CRC32 Read")));

    pSerializer->ReadInt32((int32_t&)mCRCInfo.crc);
}

void cCRC32Signature::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32((int32_t)mCRCInfo.crc);
}

///////////////////////////////////////////////////////////////////////////////
// Copy
///////////////////////////////////////////////////////////////////////////////
void cCRC32Signature::Copy(const iFCOProp* rhs)
{
    ASSERT(GetType() == rhs->GetType());
    mCRCInfo.crc = static_cast<const cCRC32Signature*>(rhs)->mCRCInfo.crc;
}

///////////////////////////////////////////////////////////////////////////////
// class cMD5Signature -- A MD5 signature
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cMD5Signature, _T("cMD5Signature"), 0, 1)

cMD5Signature::cMD5Signature()
{
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    memset(mMD5Info.data, 0, sizeof(mMD5Info.data));
#else
    memset(mMD5Info.digest, 0, sizeof(mMD5Info.digest));
#endif

    memset(md5_digest, 0, MD5_DIGEST_LENGTH);
}

cMD5Signature::~cMD5Signature()
{
}

void cMD5Signature::Init()
{
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_MD5_Init(&mMD5Info);
#elif HAVE_OPENSSL_MD5_H
    MD5_Init(&mMD5Info);
#else
    MD5Init(&mMD5Info);
#endif
}

void cMD5Signature::Update(const uint8_t* const pbData, int cbDataLen)
{
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_MD5_Update(&mMD5Info, (uint8_t*)pbData, cbDataLen);
#elif HAVE_OPENSSL_MD5_H
    MD5_Update(&mMD5Info, (uint8_t*)pbData, cbDataLen);
#else
    MD5Update(&mMD5Info, (uint8_t*)pbData, cbDataLen);
#endif
}

void cMD5Signature::Finit()
{
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_MD5_Final(md5_digest, &mMD5Info);
#elif HAVE_OPENSSL_MD5_H
    MD5_Final(md5_digest, &mMD5Info);
#else
    MD5Final(&mMD5Info);

#if HAVE_MEMCPY_S
    memcpy_s(md5_digest, MD5_DIGEST_LENGTH, mMD5Info.digest, sizeof(mMD5Info.digest));
#else
    memcpy(md5_digest, mMD5Info.digest, MD5_DIGEST_LENGTH);
#endif
    
#endif
}

////////////////////////////////////////////////////////////////////////////////
// AsString -- Converts to Base64 representation and returns a TSTRING
TSTRING cMD5Signature::AsString() const
{
    if (cArchiveSigGen::Hex())
        return AsStringHex();

    TSTRING ret;
    char    buf[24];

    ASSERT(sizeof(uint8_t) == sizeof(uint8_t)); /* everything breaks otherwise */
    btob64((uint8_t*)md5_digest, buf, SIG_BYTE_SIZE * 8);
    //converting to base64 representation.

    ret.append(buf);

    return ret;
}

TSTRING cMD5Signature::AsStringHex() const
{
    TSTRING ret;

    TCHAR stringBuffer[128];
    TCHAR sigStringOut[129];
    sigStringOut[0] = '\0';
    uint8_t* dbuf   = (uint8_t*)md5_digest;

    for (int i = 0; i < SIG_BYTE_SIZE; ++i)
    {
        snprintf(stringBuffer, 128, _T("%02lx"), (unsigned long)dbuf[i]);
        strncat(sigStringOut, stringBuffer, 128);
    }
    ret.append(sigStringOut);

    return ret;
}

bool cMD5Signature::IsEqual(const iSignature& rhs) const
{
    if (this == &rhs)
        return true;
    else
    {
        return (memcmp(md5_digest, ((cMD5Signature&)rhs).md5_digest, SIG_BYTE_SIZE) == 0);
    }
}

void cMD5Signature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("MD5 Read")));

    pSerializer->ReadBlob(md5_digest, SIG_BYTE_SIZE);
}

void cMD5Signature::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteBlob(md5_digest, SIG_BYTE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////
// Copy
///////////////////////////////////////////////////////////////////////////////
void cMD5Signature::Copy(const iFCOProp* rhs)
{
    ASSERT(GetType() == rhs->GetType());
    memcpy(md5_digest, &(static_cast<const cMD5Signature*>(rhs)->md5_digest), SIG_BYTE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////
// class cSHASignature --   Implementation for cSHASignature:
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cSHASignature, _T("cSHASignature"), 0, 1)

cSHASignature::cSHASignature()
{
    memset(&mSHAInfo, 0, sizeof(mSHAInfo));

#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    memset(sha_digest, 0, CC_SHA1_DIGEST_LENGTH);
#elif HAVE_OPENSSL_SHA_H
    memset(sha_digest, 0, SHA_DIGEST_LENGTH);
#endif
}

cSHASignature::~cSHASignature()
{
}

void cSHASignature::Init()
{
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_SHA1_Init(&mSHAInfo);
#elif HAVE_OPENSSL_SHA_H
    SHA1_Init(&mSHAInfo);
#else
    shsInit(&mSHAInfo);
#endif
}

void cSHASignature::Update(const uint8_t* const pbData, int cbDataLen)
{
    ASSERT(sizeof(uint8_t) == sizeof(uint8_t));
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_SHA1_Update(&mSHAInfo, (uint8_t*)pbData, cbDataLen);
#elif HAVE_OPENSSL_SHA_H
    SHA1_Update(&mSHAInfo, (uint8_t*)pbData, cbDataLen);
#else
    shsUpdate(&mSHAInfo, (uint8_t*)pbData, cbDataLen);
#endif
}

void cSHASignature::Finit()
{
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_SHA1_Final((unsigned char*)sha_digest, &mSHAInfo);
#elif HAVE_OPENSSL_SHA_H
    SHA1_Final((unsigned char*)sha_digest, &mSHAInfo);
#else
    shsFinal(&mSHAInfo);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// AsString -- Converts to Base64 representation and returns a TSTRING
#if defined(HAVE_OPENSSL_SHA_H) || defined(HAVE_COMMONCRYPTO_COMMONDIGEST_H)
TSTRING cSHASignature::AsString(void) const
{
    if (cArchiveSigGen::Hex())
        return AsStringHex();

    TSTRING ret;
    char*   ps_signature;
    char    buf[100];

    ps_signature = btob64((uint8_t*)sha_digest, buf, SIG_UINT32_SIZE * sizeof(uint32_t) * 8);
    //converting to base64 representation.

    ret.append(ps_signature);
    return ret;
}

TSTRING cSHASignature::AsStringHex() const
{
    TSTRING ret;

    TCHAR stringBuffer[128];
    TCHAR sigStringOut[129];
    sigStringOut[0] = '\0';
    uint8_t* dbuf   = (uint8_t*)sha_digest;

    for (int i = 0; i < SIG_UINT32_SIZE * (int)sizeof(uint32_t); ++i)
    {
        snprintf(stringBuffer, 128, _T("%02x"), dbuf[i]);
        strncat(sigStringOut, stringBuffer, 128);
    }
    ret.append(sigStringOut);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Copy -- Copies a new sig value from a base pointer
void cSHASignature::Copy(const iFCOProp* rhs)
{
    ASSERT(GetType() == rhs->GetType());
    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
        sha_digest[i] = ((static_cast<const cSHASignature*>(rhs))->sha_digest)[i];
}

///////////////////////////////////////////////////////////////////////////////
// Serializer Implementation: Read and Write
void cSHASignature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("SHA Read")));

    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
        pSerializer->ReadInt32((int32_t&)sha_digest[i]);
}

void cSHASignature::Write(iSerializer* pSerializer) const
{
    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
        pSerializer->WriteInt32(sha_digest[i]);
}

///////////////////////////////////////////////////////////////////////////////
// IsEqual -- Tests for equality, given a base pointer (iSignature)
bool cSHASignature::IsEqual(const iSignature& rhs) const
{
    if (this == &rhs)
        return true;
    else
    {
        return (memcmp(sha_digest, ((cSHASignature&)rhs).sha_digest, SIG_UINT32_SIZE * sizeof(uint32_t)) == 0);
    }
}

#else // HAVE_OPENSSL_SHA_H

TSTRING cSHASignature::AsString(void) const
{
    if (cArchiveSigGen::Hex())
        return AsStringHex();

    TSTRING ret;
    char* ps_signature;
    char buf[100];
    buf[99] = 0;

    ps_signature = pltob64((uint32_t*)mSHAInfo.digest, buf, SIG_UINT32_SIZE);
    //converting to base64 representation.

    ret.append(ps_signature);
    return ret;
    //return ret;
}

TSTRING cSHASignature::AsStringHex() const
{
    TSTRING ret;

    TCHAR stringBuffer[128];
    TCHAR sigStringOut[129];
    sigStringOut[0] = '\0';

    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
    {
        snprintf(stringBuffer, 128, _T("%08x"), mSHAInfo.digest[i]);
        strncat(sigStringOut, stringBuffer, 128);
    }
    ret.append(sigStringOut);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Copy -- Copies a new sig value from a base pointer
void cSHASignature::Copy(const iFCOProp* rhs)
{
    ASSERT(GetType() == rhs->GetType());
    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
        mSHAInfo.digest[i] = ((static_cast<const cSHASignature*>(rhs))->mSHAInfo.digest)[i];
}

///////////////////////////////////////////////////////////////////////////////
// Serializer Implementation: Read and Write
void cSHASignature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("SHA Read")));

    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
        pSerializer->ReadInt32((int32_t&)mSHAInfo.digest[i]);
}

void cSHASignature::Write(iSerializer* pSerializer) const
{
    for (int i = 0; i < SIG_UINT32_SIZE; ++i)
        pSerializer->WriteInt32(mSHAInfo.digest[i]);
}

///////////////////////////////////////////////////////////////////////////////
// IsEqual -- Tests for equality, given a base pointer (iSignature)
bool cSHASignature::IsEqual(const iSignature& rhs) const
{
    if (this == &rhs)
        return true;
    else
    {
        return (memcmp(mSHAInfo.digest, ((cSHASignature&)rhs).mSHAInfo.digest, SIG_UINT32_SIZE * sizeof(uint32_t)) == 0);
    }
}
#endif
///////////////////////////////////////////////////////////////////////////////
// class cHAVALSignature --
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPEDSERIALIZABLE(cHAVALSignature, _T("cHAVALSignature"), 0, 1)

cHAVALSignature::cHAVALSignature()
{
    memset(mSignature, 0, sizeof(mSignature));
}

cHAVALSignature::~cHAVALSignature()
{
}


void cHAVALSignature::Init()
{
    haval_start(&mHavalState);
}

void cHAVALSignature::Update(const uint8_t* const pbData, int cbDataLen)
{
    haval_hash(&mHavalState, (uint8_t*)pbData, cbDataLen);
}

void cHAVALSignature::Finit()
{
    haval_end(&mHavalState, mSignature);
}

///////////////////////////////////////////////////////////////////////////////
// AsString -- Returns Base64 representation of mSignature in a TSTRING
TSTRING cHAVALSignature::AsString() const
{
    if (cArchiveSigGen::Hex())
        return AsStringHex();

    TSTRING ret;
    char    buf[24];

    btob64((uint8_t*)mSignature, buf, 128);
    //converting to base64 representation.

    ret.append(buf);
    return ret;
}

TSTRING cHAVALSignature::AsStringHex() const
{
    TSTRING ret;

    TCHAR stringBuffer[128];
    TCHAR sigStringOut[129];
    sigStringOut[0] = _T('\0');

    for (int i = 0; i < SIG_BYTE_SIZE; ++i)
    {
        snprintf(stringBuffer, 128, _T("%02x"), mSignature[i]);
        strncat(sigStringOut, stringBuffer, 128);
    }
    ret.append(sigStringOut);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Copy -- Copies the sig value using a base class pointer.
void cHAVALSignature::Copy(const iFCOProp* rhs)
{
    ASSERT(GetType() == rhs->GetType());
    memcpy(&mSignature, &(static_cast<const cHAVALSignature*>(rhs)->mSignature), SIG_BYTE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////
// Serializer Implementation: Read and Write
void cHAVALSignature::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("HAVAL Read")));

    pSerializer->ReadBlob(mSignature, SIG_BYTE_SIZE);
}

void cHAVALSignature::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteBlob(mSignature, SIG_BYTE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////
// Equal -- Tests for equality given a base pointer.
bool cHAVALSignature::IsEqual(const iSignature& rhs) const
{
    if (this == &rhs)
        return true;
    else
    {
        return (memcmp(mSignature, ((cHAVALSignature&)rhs).mSignature, SIG_BYTE_SIZE) == 0);
    }
}
