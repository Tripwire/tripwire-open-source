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
// cryptoarchive.h -- Write/Read encrypted bytes to an archive

#ifndef __CRYPTOARCHIVE_H
#define __CRYPTOARCHIVE_H

#ifndef __ARCHIVE_H
#include "core/archive.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// class cCryptoArchive
//
// Takes an archive and a cipher.  Bytes written to this archive are compressed,
// passed through the cipher, and then written to the archive provided.  When
// reading, bytes are read from the provided archive, run through the cipher,
// and then decompressed.
//
// Start() must be called before the first read or write to set the archive and
// cipher.  The cCryptoArchive can be reused for reading and writing, but
// Start() must be called each time.  Finish() should be called when done
// writing or not all bytes will be written.
//
// An eArchive exception will be thrown if the archvie is read from after being
// written to without calling Start();
//
// Will throw an EOF() exception if the data source runs out
// of data while reading.

class cArchive;
class iCipher;

// following classes are used internally, ignore them
class cCryptoSink;
class cCryptoSource;
class Deflator;
class Inflator;
class cByteQueue;

class cCryptoArchive : public cArchive
{
public:
    cCryptoArchive();
    virtual ~cCryptoArchive();

    // TODO: Start and Finish should be virtual so derived classes can override them
    // (and in the case of RSA and ElGamal archives, throw an exception or something).

    void Start(cArchive* pArchive, iCipher* pCipher);
    // Initialize this cCryptoArchive.  This function may be called
    // multiple times to set new dest archives and ciphers.  These
    // pointers will not be deleted by this class, it is the responsibility
    // of the caller to do so.
    void Finish();
    // Call when done writing to the archive to flush any buffered bytes.
    // An ASSERT() will occur if further reads or write are called
    // You may call when done reading to cause an ASSERT() if further
    // reads are attempted.

    virtual bool EndOfFile();

protected:
    cArchive* mpArchive;
    iCipher*  mpCipher;

    // set on first read/write or when Finish() is called
    int mAction;

    // members needed for writing
    Deflator*    mpDeflator;
    cCryptoSink* mpCryptoSink;

    // members needed for reading
    Inflator*      mpInflator;
    cCryptoSource* mpCryptoSource;
    cByteQueue*    mpInflatedBytes;

    virtual int Read(void* pDest, int count);
    virtual int Write(const void* pSrc, int count); // throw(eArchive);
};

///////////////////////////////////////////////////////////////////////////////
// class cNullCryptoArchive
//
// Works like crypto archive but uses no encryption. However, the compression
// functionality of cCryptoArchive is still utilized.

class cNullCipher;

class cNullCryptoArchive : public cArchive
{
public:
    cNullCryptoArchive();
    virtual ~cNullCryptoArchive();

    void Start(cArchive* pArchive);
    void Finish();

    virtual bool EndOfFile();

protected:
    cCryptoArchive mCryptoArchive;
    cNullCipher*   mpNullCipher;

    virtual int Read(void* pDest, int count);
    virtual int Write(const void* pSrc, int count); // throw(eArchive);
};

#ifdef _RSA_ENCRYPTION
///////////////////////////////////////////////////////////////////////////////
// class cRSAArchive

class cRSAPublicKey;
class cRSAPrivateKey;
class cIDEA;

class cRSAArchive : public cArchive
{
public:
    cRSAArchive();
    ~cRSAArchive();

    void SetWrite(cArchive* pDestArchive, const cRSAPublicKey* pPublicKey);
    void SetWrite(cArchive* pDestArchive, const cRSAPrivateKey* pPrivateKey);

    void FlushWrite();

    void SetRead(cArchive* pSrcArchive, const cRSAPublicKey* pPublicKey);
    void SetRead(cArchive* pSrcArchive, const cRSAPrivateKey* pPrivateKey);

    virtual bool EndOfFile();

protected:
    cCryptoArchive mCryptoArchive;
    cIDEA*         mpIDEA;

    int mAction;

    cArchive*             mpArchive;
    const cRSAPublicKey*  mpPublicKey;
    const cRSAPrivateKey* mpPrivateKey;

    virtual int Read(void* pDest, int count);
    virtual int Write(const void* pSrc, int count); // throw(eArchive);
};
#endif // _RSA_ENCRYPTION

///////////////////////////////////////////////////////////////////////////////
// class cElGamalSigArchive

class cElGamalSigPublicKey;
class cElGamalSigPrivateKey;
class cElGamalSig;

class cElGamalSigArchive : public cArchive
{
public:
    cElGamalSigArchive();
    virtual ~cElGamalSigArchive();

    void SetWrite(cArchive* pDestArchive, const cElGamalSigPrivateKey* pPrivateKey);

    void FlushWrite();

    void SetRead(cArchive* pSrcArchive, const cElGamalSigPublicKey* pPublicKey);

    virtual bool EndOfFile();

protected:
    cCryptoArchive mCryptoArchive;

    int mAction;

    cArchive*                    mpArchive;
    cElGamalSig*                 mpElGamal;
    const cElGamalSigPublicKey*  mpPublicKey;
    const cElGamalSigPrivateKey* mpPrivateKey;

    virtual int Read(void* pDest, int count);
    virtual int Write(const void* pSrc, int count); // throw(eArchive);
};

#endif // __CRYPTOARCHIVE_H
