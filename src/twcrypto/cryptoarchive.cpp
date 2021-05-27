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
// cryptoarchive.cpp -- classes that abstract a raw byte archive

#include "stdtwcrypto.h"

#include "cryptoarchive.h"
#include "crypto.h"

#include "cryptlib/zinflate.h"
#include "cryptlib/zdeflate.h"
// old queue
//#include "cryptlib/queue.h"
#include "bytequeue.h"

enum mAction
{
    MA_UNSTARTED,
    MA_UNKNOWN,
    MA_READING,
    MA_WRITING,
    MA_FINISHED
};

const int CRYPTO_COMPRESSION_LEVEL = 6;


// folowing interfaces were added to use Crypto++'s implementation
// of gzip

// class that takes a plaintext input via Put() and
// outputs encrypted data to the passed in cArchive
class cCryptoSink : public Sink
{
public:
    cCryptoSink(cArchive* pDestArchive, iCipher* pCipher);
    ~cCryptoSink();

    virtual void Put(const uint8_t* inString, unsigned int length);
    virtual void InputFinished();

    virtual void Put(uint8_t inByte)
    {
        Put(&inByte, 1);
    }

private:
    cArchive* mpDestArchive;
    iCipher*  mpCipher;

    // buffer to cache bytes in
    int8_t* mpBuffer;
    int   mBufferLen;
    int   mBufferUsed;
};

class cCryptoSource : public Source
{
public:
    cCryptoSource(cArchive* pSrcArchive, iCipher* pCipher, BufferedTransformation* outQueue);
    ~cCryptoSource();

    unsigned int  Pump(unsigned int size);
    unsigned long PumpAll();

private:
    cArchive* mpSrcArchive;
    iCipher*  mpCipher;

    // buffer to cache bytes in
    int8_t* mpBuffer;
    int   mBufferLen;
    int   mBufferUsed;
};

cCryptoArchive::cCryptoArchive()
{
    mpArchive       = 0;
    mpCipher        = 0;
    mpDeflator      = 0;
    mpInflator      = 0;
    mpCryptoSink    = 0;
    mpCryptoSource  = 0;
    mpInflatedBytes = 0;
    mAction         = MA_UNSTARTED;
}

cCryptoArchive::~cCryptoArchive()
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_UNKNOWN || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    Finish();

    // Finish() normally zeroes these out, but hey.
    delete mpDeflator;
    delete mpInflator;
}

void cCryptoArchive::Start(cArchive* pArchive, iCipher* pCipher)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_UNKNOWN || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mpArchive = pArchive;
    mpCipher  = pCipher;

    delete mpDeflator;
    mpDeflator = 0;
    delete mpCryptoSink;
    mpCryptoSink = 0;
    delete mpInflator;
    mpInflator = 0;
    delete mpCryptoSource;
    mpCryptoSource = 0;
    delete mpInflatedBytes;
    mpInflatedBytes = 0;

    mAction = MA_UNKNOWN;
}

int cCryptoArchive::Write(const void* pSrc, int count)
{
    if (mAction == MA_UNKNOWN)
    {
        // this is the first write
        mAction = MA_WRITING;

        ASSERT(mpDeflator == 0);
        ASSERT(mpCryptoSink == 0);
        ASSERT(mpInflator == 0);
        ASSERT(mpCryptoSource == 0);
        ASSERT(mpInflatedBytes == 0);

        mpCryptoSink = new cCryptoSink(mpArchive, mpCipher);
        mpDeflator   = new Deflator(CRYPTO_COMPRESSION_LEVEL, mpCryptoSink);
    }
    else if (mAction != MA_WRITING)
    {
        ASSERT(false);
        throw eArchiveInvalidOp();
    }

    mpDeflator->Put((uint8_t*)pSrc, count);

    return count;
}

int cCryptoArchive::Read(void* pDest, int count)
{
    int len;

    if (mAction == MA_UNKNOWN)
    {
        // this is the first read
        mAction = MA_READING;

        ASSERT(mpDeflator == 0);
        ASSERT(mpCryptoSink == 0);
        ASSERT(mpInflator == 0);
        ASSERT(mpCryptoSource == 0);
        ASSERT(mpInflatedBytes == 0);

        mpInflatedBytes = new cByteQueue;
        mpInflator      = new Inflator(mpInflatedBytes);
        mpCryptoSource  = new cCryptoSource(mpArchive, mpCipher, mpInflator);
    }
    else if (mAction != MA_READING)
    {
        ASSERT(false);
        throw eArchiveInvalidOp();
    }

    while ((int)mpInflatedBytes->CurrentSize() < count)
    {
        len = mpCipher->GetBlockSizePlain();
        if ((int)mpCryptoSource->Pump(len) < len) // RAD: Cast to int
        {
            mpInflator->InputFinished();
            if ((int)mpInflatedBytes->CurrentSize() < count) // RAD: Cast to int
            {
                len = mpInflatedBytes->CurrentSize();
                mpInflatedBytes->Get((uint8_t*)pDest, len);
                return len;
            }
        }
    }

    len = mpInflatedBytes->Get((uint8_t*)pDest, count);
    ASSERT(len == count);

    return len;
}

bool cCryptoArchive::EndOfFile()
{
    ASSERT(mAction == MA_READING); // why would you call this if not reading?
    if (mAction != MA_READING)
        return true;

    // TODO: is this right?
    ASSERT(mpInflatedBytes != 0);
    return mpInflatedBytes->MaxRetrieveable() == 0;
}

// Call when done writing to the archive to flush any buffered bytes.
// An ASSERT() will occur if further reads or write are called.
// You may call when done reading to cause an ASSERT() if further
// reads are attempted.
void cCryptoArchive::Finish()
{
    if (mAction == MA_WRITING)
    {
        mpDeflator->InputFinished();
        mpCryptoSink->InputFinished();
        delete mpDeflator;
        mpDeflator   = 0;
        mpCryptoSink = 0; // mpCryptoSink is deleted by ~Deflator()

        mAction = MA_FINISHED;
    }
    else if (mAction == MA_READING)
    {
        delete mpCryptoSource;
        mpCryptoSource = 0;
        mpInflator     = 0; // deleting mpCryptoSource is deleted by ~Inflator()

        mAction = MA_FINISHED;
    }
    else
    {
        // what is up?
        ASSERT(false);
    }
}

cCryptoSink::cCryptoSink(cArchive* pDestArchive, iCipher* pCipher)
{
    mpDestArchive = pDestArchive;
    mpCipher      = pCipher;
    mpBuffer      = 0;
    mBufferLen    = 0;
    mBufferUsed   = 0;
}

cCryptoSink::~cCryptoSink()
{
    delete [] mpBuffer;
}

void cCryptoSink::Put(const uint8_t* inString, unsigned int length)
{
    if (mpBuffer == 0)
    {
        // this is the first write
        mBufferLen  = mpCipher->GetBlockSizePlain();
        mpBuffer    = new int8_t[mBufferLen];
        mBufferUsed = 0;
    }

    // RAD: Cast to int
    ASSERT(length <= std::numeric_limits<unsigned int>::max());
    int nLength = static_cast<int>(length);

    int i = 0;
    while (i < nLength)
    {
        int bytesToCopy = mBufferLen - mBufferUsed;
        if (bytesToCopy > nLength - i)
            bytesToCopy = nLength - i;

        memcpy(mpBuffer + mBufferUsed, (int8_t*)inString + i, bytesToCopy);
        mBufferUsed += bytesToCopy;

        if (mBufferUsed >= mBufferLen)
        {
            ASSERT(mBufferUsed == mBufferLen); // should be if our math is right

            int8_t* pTmp = new int8_t[mpCipher->GetBlockSizeCipher()];
            mpCipher->ProcessBlock(mpBuffer, pTmp);

            mpDestArchive->WriteBlob(pTmp, mpCipher->GetBlockSizeCipher());

            delete [] pTmp;
            mBufferUsed = 0;
        }

        i += bytesToCopy;
    }

    ASSERT(i == nLength); // should be if our math is right
}

void cCryptoSink::InputFinished()
{
    if (mBufferUsed > 0)
    {
        ASSERT(mBufferLen - mBufferUsed > 0); // should be, or the buffer should have already been written
        RandomizeBytes(mpBuffer + mBufferUsed, mBufferLen - mBufferUsed);

        int8_t* pTmp = new int8_t[mpCipher->GetBlockSizeCipher()];
        mpCipher->ProcessBlock(mpBuffer, pTmp);

        mpDestArchive->WriteBlob(pTmp, mpCipher->GetBlockSizeCipher());

        delete [] pTmp;
        mBufferUsed = 0;
    }

    delete [] mpBuffer;
    mpBuffer    = 0;
    mBufferLen  = 0;
    mBufferUsed = 0;
}


cCryptoSource::cCryptoSource(cArchive* pSrcArchive, iCipher* pCipher, BufferedTransformation* outQueue)
    : Source(outQueue)
{
    mpSrcArchive = pSrcArchive;
    mpCipher     = pCipher;

    mpBuffer    = 0;
    mBufferLen  = 0;
    mBufferUsed = 0;
}

cCryptoSource::~cCryptoSource()
{
    delete [] mpBuffer;
}

unsigned int cCryptoSource::Pump(unsigned int size)
{
    if (mpBuffer == 0)
    {
        // first time this has been called
        mBufferLen  = mpCipher->GetBlockSizePlain();
        mpBuffer    = new int8_t[mBufferLen];
        mBufferUsed = mBufferLen;
    }

    // RAD: Cast to int (Why are these locals signed if the interface is unsigned?)
    ASSERT(size <= std::numeric_limits<unsigned int>::max());
    int nSize = static_cast<int>(size);

    int i = 0;
    while (i < nSize)
    {
        if (mBufferUsed >= mBufferLen)
        {
            ASSERT(mBufferUsed == mBufferLen); // should be if our math is right

            int8_t* pTmp = new int8_t[mpCipher->GetBlockSizeCipher()];

            int l = mpSrcArchive->ReadBlob(pTmp, mpCipher->GetBlockSizeCipher());
            if (l != mpCipher->GetBlockSizeCipher())
            {
                delete [] pTmp;
                return 0;
            }

            mpCipher->ProcessBlock(pTmp, mpBuffer);

            delete [] pTmp;
            mBufferUsed = 0;
        }

        int bytesToCopy = mBufferLen - mBufferUsed;
        if (bytesToCopy > nSize - i)
            bytesToCopy = nSize - i;

        outQueue->Put((uint8_t*)(mpBuffer + mBufferUsed), bytesToCopy);

        mBufferUsed += bytesToCopy;
        i += bytesToCopy;
    }

    ASSERT(i == nSize); // should be if our math is right

    return i;
}

unsigned long cCryptoSource::PumpAll()
{
    unsigned long total = 0;
    unsigned int  l;

    if (mBufferLen == 0)
        mBufferLen = mpCipher->GetBlockSizePlain();

    while ((l = Pump(mBufferLen)) != 0)
        total += l;

    return total;
}

///////////////////////////////////////////////////////////////////////////////
// class cNullCryptoArchive
//
// Works like crypto archive but uses no encryption. However, the compression
// functionality of cCryptoArchive is still utilized.


cNullCryptoArchive::cNullCryptoArchive()
{
    mpNullCipher = new cNullCipher;
}

cNullCryptoArchive::~cNullCryptoArchive()
{
    delete mpNullCipher;
}

void cNullCryptoArchive::Start(cArchive* pArchive)
{
    mCryptoArchive.Start(pArchive, mpNullCipher);
}

void cNullCryptoArchive::Finish()
{
    mCryptoArchive.Finish();
}

bool cNullCryptoArchive::EndOfFile()
{
    return mCryptoArchive.EndOfFile();
}

int cNullCryptoArchive::Read(void* pDest, int count)
{
    return mCryptoArchive.ReadBlob(pDest, count);
}

int cNullCryptoArchive::Write(const void* pSrc, int count)
{
    mCryptoArchive.WriteBlob(pSrc, count);
    return count;
}

#ifdef _RSA_ENCRYPTION
///////////////////////////////////////////////////////////////////////////////
// class cRSAArchive

cCryptoArchive mCryptoArchive;

int mAction;

cArchive*       mpArchive;
cRSAPublicKey*  mpPublicKey;
cRSAPrivateKey* mpPrivateKey;

cRSAArchive::cRSAArchive() : mAction(MA_UNSTARTED), mpArchive(0), mpPublicKey(0), mpPrivaeKey(0), mpIDEA(new cIDEA)
{
}

cRSAArchive::~cRSAArchive()
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten
    delete mpIDEA;
}

void cRSAArchive::SetWrite(cArchive* pDestArchive, const cRSAPublicKey* pPublicKey)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mAction      = MA_WRITING;
    mpArchive    = pDestArchive;
    mpPublicKey  = pPublicKey;
    mpPrivateKey = 0;

    // Create a random number and encode using public key
    cRSA  rsa(*mpPublicKey);
    int8_t* key          = new int8_t[rsa.GetBlockSizePlain()];
    int8_t* encryptedKey = new int8_t[rsa.GetBlockSizeCipher()];

    RandomizeBytes(key, rsa.GetBlockSizePlain());
    rsa.SetEncrypting(mpPublicKey);
    rsa.ProcessBlock(key, encryptedKey);

    mpArchive->WriteBlob(encryptedKey, rsa.GetBlockSizeCipher());

    mpIDEA->SetKey(iCipher::ENCRYPT, cHashedKey128(key, rsa.GetBlockSizePlain()));

    // clear out key from memory
    RandomizeBytes(key, rsa.GetBlockSizePlain());

    mCryptoArchive.Start(mpArchive, mpIDEA);

    delete [] key;
    delete [] encryptedKey;
}

void cRSAArchive::SetWrite(cArchive* pDestArchive, const cRSAPrivateKey* pPrivateKey)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mAction      = MA_WRITING;
    mpArchive    = pDestArchive;
    mpPrivateKey = pPrivateKey;
    mpPublicKey  = 0;

    // Create a random number and encode using public key
    cRSA  rsa(*mpPrivateKey);
    int8_t* key          = new int8_t[rsa.GetBlockSizePlain()];
    int8_t* encryptedKey = new int8_t[rsa.GetBlockSizeCipher()];

    RandomizeBytes(key, rsa.GetBlockSizePlain());
    rsa.SetSigning(mpPrivateKey);
    rsa.ProcessBlock(key, encryptedKey);

    mpArchive->WriteBlob(encryptedKey, rsa.GetBlockSizeCipher());

    mpIDEA->SetKey(iCipher::ENCRYPT, cHashedKey128(key, rsa.GetBlockSizePlain()));
    mCryptoArchive.Start(mpArchive, mpIDEA);

    // clear out key from memory
    RandomizeBytes(key, rsa.GetBlockSizePlain());

    mCryptoArchive.Start(mpArchive, mpIDEA);

    delete [] key;
    delete [] encryptedKey;
}

void cRSAArchive::FlushWrite()
{
    ASSERT(mAction == MA_WRITING);
    if (mAction != MA_WRITING)
        throw eArchiveInvalidOp();

    mCryptoArchive.Finish();

    mAction = MA_FINISHED;
}

void cRSAArchive::SetRead(cArchive* pSrcArchive, const cRSAPublicKey* pPublicKey)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mAction      = MA_READING;
    mpArchive    = pSrcArchive;
    mpPublicKey  = pPublicKey;
    mpPrivateKey = 0;

    cRSA  rsa(*mpPublicKey);
    int8_t* key          = new int8_t[rsa.GetBlockSizePlain()];
    int8_t* encryptedKey = new int8_t[rsa.GetBlockSizeCipher()];

    mpArchive->ReadBlob(encryptedKey, rsa.GetBlockSizeCipher());
    rsa.SetVerifying(mpPublicKey);
    rsa.ProcessBlock(encryptedKey, key);

    mpIDEA->SetKey(iCipher::DECRYPT, cHashedKey128(key, rsa.GetBlockSizePlain()));
    mCryptoArchive.Start(mpArchive, mpIDEA);

    // clear out key from memory
    RandomizeBytes(key, rsa.GetBlockSizePlain());

    mCryptoArchive.Start(mpArchive, mpIDEA);

    delete [] key;
    delete [] encryptedKey;
}

void cRSAArchive::SetRead(cArchive* pSrcArchive, const cRSAPrivateKey* pPrivateKey)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mAction      = MA_READING;
    mpArchive    = pSrcArchive;
    mpPrivateKey = pPrivateKey;
    mpPublicKey  = 0;

    cRSA  rsa(*mpPrivateKey);
    int8_t* key          = new int8_t[rsa.GetBlockSizePlain()];
    int8_t* encryptedKey = new int8_t[rsa.GetBlockSizeCipher()];

    mpArchive->ReadBlob(encryptedKey, rsa.GetBlockSizeCipher());
    rsa.SetDecrypting(mpPrivateKey);
    rsa.ProcessBlock(encryptedKey, key);

    mpIDEA->SetKey(iCipher::DECRYPT, cHashedKey128(key, rsa.GetBlockSizePlain()));
    mCryptoArchive.Start(mpArchive, mpIDEA);

    // clear out key from memory
    RandomizeBytes(key, rsa.GetBlockSizePlain());

    mCryptoArchive.Start(mpArchive, mpIDEA);

    delete [] key;
    delete [] encryptedKey;
}

int cRSAArchive::Read(void* pDest, int count)
{
    ASSERT(mAction == MA_READING);
    if (mAction != MA_READING)
        throw eArchiveInvalidOp();

    return mCryptoArchive.ReadBlob(pDest, count);
}

int cRSAArchive::Write(const void* pSrc, int count)
{
    ASSERT(mAction == MA_WRITING);
    if (mAction != MA_WRITING)
        throw eArchiveInvalidOp();

    mCryptoArchive.WriteBlob(pSrc, count);
    return count;
}

bool cRSAArchive::EndOfFile()
{
    ASSERT(mAction == MA_READING); // why would you call this if not reading?
    if (mAction != MA_READING)
        return true;

    return mCryptoArchive.EndOfFile();
}

#endif // _RSA_ENCRYPTION


///////////////////////////////////////////////////////////////////////////////
// class cElGamalSigArchive

cElGamalSigArchive::cElGamalSigArchive()
    : mAction(MA_UNSTARTED), mpArchive(0), mpElGamal(0), mpPublicKey(0), mpPrivateKey(0)
{
}

cElGamalSigArchive::~cElGamalSigArchive()
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten
    delete mpElGamal;
}

void cElGamalSigArchive::SetWrite(cArchive* pDestArchive, const cElGamalSigPrivateKey* pPrivateKey)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mAction      = MA_WRITING;
    mpArchive    = pDestArchive;
    mpPrivateKey = pPrivateKey;
    mpPublicKey  = 0;

    mpElGamal = new cElGamalSig(*mpPrivateKey);
    mpElGamal->SetSigning(mpPrivateKey);

    mCryptoArchive.Start(mpArchive, mpElGamal);
}

void cElGamalSigArchive::FlushWrite()
{
    ASSERT(mAction == MA_WRITING);
    if (mAction != MA_WRITING)
        throw eArchiveInvalidOp();

    mCryptoArchive.Finish();

    mAction = MA_FINISHED;
}

void cElGamalSigArchive::SetRead(cArchive* pSrcArchive, const cElGamalSigPublicKey* pPublicKey)
{
    ASSERT(mAction == MA_UNSTARTED || mAction == MA_FINISHED || mAction == MA_READING);
    // check we did not leave a buffer unwritten

    mAction      = MA_READING;
    mpArchive    = pSrcArchive;
    mpPublicKey  = pPublicKey;
    mpPrivateKey = 0;

    mpElGamal = new cElGamalSig(*mpPublicKey);

    mpElGamal->SetVerifying(mpPublicKey);

    mCryptoArchive.Start(mpArchive, mpElGamal);
}

int cElGamalSigArchive::Read(void* pDest, int count)
{
    ASSERT(mAction == MA_READING);
    if (mAction != MA_READING)
        throw eArchiveInvalidOp();

    return mCryptoArchive.ReadBlob(pDest, count);
}

int cElGamalSigArchive::Write(const void* pSrc, int count)
{
    ASSERT(mAction == MA_WRITING);
    if (mAction != MA_WRITING)
        throw eArchiveInvalidOp();

    mCryptoArchive.WriteBlob(pSrc, count);
    return count;
}

bool cElGamalSigArchive::EndOfFile()
{
    ASSERT(mAction == MA_READING); // why would you call this if not reading?
    if (mAction != MA_READING)
        return true;

    return mCryptoArchive.EndOfFile();
}
