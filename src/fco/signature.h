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
// signature.h -- interface for calculating file signatures
///////////////////////////////////////////////////////////////////////////////

#ifndef __SIGNATURE_H
#define __SIGNATURE_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __FCOPROP_H
#include "fcoprop.h"
#endif

#include <vector>
#include "core/crc32.h"

#ifdef HAVE_OPENSSL_MD5_H
#include <openssl/md5.h>
#    define digest data
#else
#include "core/md5.h"
#    ifndef MD5_DIGEST_LENGTH
#        define MD5_DIGEST_LENGTH 16
#    endif
#endif

#ifdef HAVE_OPENSSL_SHA_H
#include <openssl/sha.h>
#else
#include "core/sha.h"
#    define SHA_CTX SHS_INFO
#    ifndef SHA_DIGEST_LENGTH
#        define SHA_DIGEST_LENGTH 20
#    endif
#endif

/*Use OSX CommonCrypto lib if available*/
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
#include <CommonCrypto/CommonDigest.h>
#endif


#include "core/haval.h"
// TODO: figure out a way to do this without including these headers.
// pool of objects?


///////////////////////////////////////////////////////////////////////////////
// class iSignatrue -- Interface all signatures will implement.
///////////////////////////////////////////////////////////////////////////////

class iSignature : public iFCOProp
{
public:
    //
    // ctors and dtors
    //
    virtual ~iSignature(){};
    iSignature(){};

    //
    // enums
    //
    enum
    {
        SUGGESTED_BLOCK_SIZE = 0x1000
    }; // best size of block to hash at a time

    //
    // basic functionality
    //
    virtual void Init() = 0;
    // call before beginning hashing
    virtual void Update(const uint8_t* const pbData, int cbDataLen) = 0;
    // may be called multiple times -- best to call with blocks of size SUGGESTED_BLOCK_SIZE,
    // but can handle any size data.
    virtual void Finit() = 0;
    // call to finish hashing

    virtual TSTRING AsStringHex() const = 0;

    //
    // from iFCOProp
    //
    virtual CmpResult Compare(const iFCOProp* rhs, Op op) const;

protected:
    //
    // don't let C++ create these functions
    //
    iSignature(const iSignature&);
    iSignature& operator=(const iSignature&);

    //
    // private util functions
    //
    virtual bool IsEqual(const iSignature& rhs) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
// class cArchiveSigGen -- utility class to facilitate archive hashing
//
//      Stores a list of signatures (added by AddSig()), and when
//      CalculateSignatures is called, makes ONE sweep through the archive,
//      calculating hashes for all signatures in the list.
///////////////////////////////////////////////////////////////////////////////
class cArchiveSigGen
{
public:
    cArchiveSigGen(){};

    void AddSig(iSignature* pSig);
    // adds a signature to the list

    void CalculateSignatures(cArchive& a);
    // produces signature of archive for all signatures in the list
    // remember to rewind archive!

    static bool Hex();
    static void SetHex(bool);

    static bool UseDirectIO()
    {
        return s_direct;
    }
    static void SetUseDirectIO(bool b)
    {
        s_direct = b;
    }

private:
    // don't let C++ create these functions
    cArchiveSigGen(const cArchiveSigGen&);
    cArchiveSigGen& operator=(const cArchiveSigGen&);

    typedef std::vector<iSignature*> container_type;
    container_type                   mSigList;

    static bool s_direct;
    static bool s_hex;
};


///////////////////////////////////////////////////////////////////////////////
// class cNullSignature -- The signature that is always 0
///////////////////////////////////////////////////////////////////////////////

class cNullSignature : public iSignature
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cNullSignature();
    virtual ~cNullSignature();

    virtual void    Init();
    virtual void    Update(const uint8_t* const pbData, int cbDataLen);
    virtual void    Finit();
    virtual TSTRING AsString() const;
    virtual TSTRING AsStringHex() const;
    virtual void    Copy(const iFCOProp* rhs);

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

protected:
    virtual bool IsEqual(const iSignature& rhs) const;
};

///////////////////////////////////////////////////////////////////////////////
// class cChecksumSignature -- A simple signature consisting of all the bytes
//      in an archive.  Useful as an example if nothing else.
///////////////////////////////////////////////////////////////////////////////

class cChecksumSignature : public iSignature
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cChecksumSignature();
    virtual ~cChecksumSignature();

    virtual void    Init();
    virtual void    Update(const uint8_t* const pbData, int cbDataLen);
    virtual void    Finit();
    virtual TSTRING AsString() const;
    virtual TSTRING AsStringHex() const;
    virtual void    Copy(const iFCOProp* rhs);

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

protected:
    virtual bool IsEqual(const iSignature& rhs) const;

    uint64_t mChecksum;
};

///////////////////////////////////////////////////////////////////////////////
// class cCRC32Signature -- A CRC32 signature
///////////////////////////////////////////////////////////////////////////////

class cCRC32Signature : public iSignature
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cCRC32Signature();
    virtual ~cCRC32Signature();

    virtual void Init();
    virtual void Update(const uint8_t* const pbData, int cbDataLen);
    virtual void Finit();

    virtual TSTRING AsString() const;
    virtual TSTRING AsStringHex() const;
    virtual void    Copy(const iFCOProp* rhs);

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

protected:
    virtual bool IsEqual(const iSignature& rhs) const;

    CRC_INFO mCRCInfo;
};

///////////////////////////////////////////////////////////////////////////////
// class cMD5Signature -- A MD5 signature
///////////////////////////////////////////////////////////////////////////////

class cMD5Signature : public iSignature
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cMD5Signature();
    virtual ~cMD5Signature();

    virtual void    Init();
    virtual void    Update(const uint8_t* const pbData, int cbDataLen);
    virtual void    Finit();
    virtual TSTRING AsString() const;
    virtual TSTRING AsStringHex() const;
    virtual void    Copy(const iFCOProp* rhs);

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

protected:
    enum
    {
        SIG_BYTE_SIZE = MD5_DIGEST_LENGTH
    };

    virtual bool IsEqual(const iSignature& rhs) const;
#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    CC_MD5_CTX mMD5Info;
    uint8_t    md5_digest[CC_MD5_DIGEST_LENGTH];
#else
    MD5_CTX mMD5Info;
    uint8_t md5_digest[MD5_DIGEST_LENGTH];
#endif
};

///////////////////////////////////////////////////////////////////////////////
// class cSHASignature -- A "Secure Hash Algorithm" signature
///////////////////////////////////////////////////////////////////////////////
class cSHASignature : public iSignature
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cSHASignature();
    virtual ~cSHASignature();

    virtual void    Init();
    virtual void    Update(const uint8_t* const pbData, int cbDataLen);
    virtual void    Finit();
    virtual TSTRING AsString() const;
    virtual TSTRING AsStringHex() const;
    virtual void    Copy(const iFCOProp* rhs);

    virtual void Read(iSerializer* pSerializer, int32_t version = 0);
    virtual void Write(iSerializer* pSerializer) const;

protected:
    virtual bool IsEqual(const iSignature& rhs) const;

#ifdef HAVE_COMMONCRYPTO_COMMONDIGEST_H
    enum {SIG_UINT32_SIZE = CC_SHA1_DIGEST_LENGTH / 4};
    CC_SHA1_CTX mSHAInfo;
    uint32_t    sha_digest[SIG_UINT32_SIZE];
#elif HAVE_OPENSSL_SHA_H
    enum {SIG_UINT32_SIZE = SHA_DIGEST_LENGTH / 4};
    SHA_CTX     mSHAInfo;
    uint32_t    sha_digest[SIG_UINT32_SIZE];
#else
    enum
    {
        SIG_UINT32_SIZE = 5
    };
    SHS_INFO mSHAInfo;
#endif
};

///////////////////////////////////////////////////////////////////////////////
// class cHAVALSignature --
///////////////////////////////////////////////////////////////////////////////
class cHAVALSignature : public iSignature
{

    DECLARE_TYPEDSERIALIZABLE()

public:
    cHAVALSignature();
    virtual ~cHAVALSignature();

    virtual void    Init();
    virtual void    Update(const uint8_t* const pbData, int cbDataLen);
    virtual void    Finit();
    virtual TSTRING AsString() const;
    virtual TSTRING AsStringHex() const;
    virtual void    Copy(const iFCOProp* rhs);

    virtual void Read(iSerializer* pSerializer, int32_t version = 0);
    virtual void Write(iSerializer* pSerializer) const;

protected:
    enum
    {
        SIG_BYTE_SIZE = 16
    };

    virtual bool IsEqual(const iSignature& rhs) const;

    haval_state mHavalState;
    uint8_t     mSignature[SIG_BYTE_SIZE];
};

#endif // __SIGNATURE_H
