//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
// cryptoarchive_t.cpp -- test classes that abstract a raw byte archive

#include "twcrypto/stdtwcrypto.h"
#include "twcrypto/cryptoarchive.h"
#include "twcrypto/crypto.h"
#include "twtest/test.h"

void TestCryptoArchive()
{
    cDebug d("TestCryptoArchive()");

    cMemoryArchive memory(0x800000); // max size 8 meg

    const int SIZEOF_CHUNK   = 7;
    const int NUM_CHUNKS     = 1024 * 32; // 32k
    const int TEST_CHUNKSIZE = 1023 * 7;

    d.TraceDetail("Building test memory image of size %dK bytes\n", SIZEOF_CHUNK * NUM_CHUNKS / 1024);

    int8_t chunk[7];
    memcpy(chunk, "1234567", 7);

    int i;
    for (i = 0; i < NUM_CHUNKS; i++)
    {
        RandomizeBytes(chunk, 7);
        memory.WriteBlob(chunk, SIZEOF_CHUNK);
    }

    // Testing cCryptoArchive

#ifdef _IDEA_ENCRYPTION

    cIDEA         idea;
    cHashedKey128 ideaKey(_T("puddy, puddy, puddy, poo!"));

    {
        d.TraceDetail("Encrypting using symmetric key\n");

        cFileArchive outFile;
        outFile.OpenReadWrite(TwTestPath("crypted.bin"));

        idea.SetKey(iCipher::ENCRYPT, ideaKey);
        cCryptoArchive outCrypt;
        outCrypt.Start(&outFile, &idea);

        for (memory.Seek(0, cBidirArchive::BEGINNING); !memory.EndOfFile();)
        {
            int8_t buf[SIZEOF_CHUNK];
            memory.ReadBlob(buf, SIZEOF_CHUNK);
            outCrypt.WriteBlob(buf, SIZEOF_CHUNK);
        }

        outCrypt.Finish();
    }

    {
        d.TraceDetail("Decrypting using symmetric key\n");

        cFileArchive inFile;
        inFile.OpenRead(TwTestPath("crypted.bin"));

        idea.SetKey(iCipher::DECRYPT, ideaKey);
        cCryptoArchive inCrypt;
        inCrypt.Start(&inFile, &idea);

        int index;
        for (index = 0; index < NUM_CHUNKS * SIZEOF_CHUNK; index += TEST_CHUNKSIZE)
        {
            int8_t buf[TEST_CHUNKSIZE];

            int s = (index + TEST_CHUNKSIZE <= NUM_CHUNKS * SIZEOF_CHUNK) ? TEST_CHUNKSIZE :
                                                                            NUM_CHUNKS * SIZEOF_CHUNK - index;

            inCrypt.ReadBlob(buf, s);

            memory.MapArchive(index, s);
            TEST(memcmp(buf, memory.GetMap(), s) == 0);
        }

        inCrypt.Finish();
    }

#endif

    // Testing cElGamalSigArchive

    {
        cElGamalSig            cipher(cElGamalSig::KEY1024);
        cElGamalSigPrivateKey* privateKey;
        cElGamalSigPublicKey*  publicKey;
        cipher.GenerateKeys(privateKey, publicKey);

        {
            // encypt using private key
            d.TraceDetail("Signing using asymmetric key\n");

            cFileArchive outFile;
            outFile.OpenReadWrite(TwTestPath("rsacrypted.bin").c_str());

            cElGamalSigArchive outCrypt;
            outCrypt.SetWrite(&outFile, privateKey);

            for (memory.Seek(0, cBidirArchive::BEGINNING); !memory.EndOfFile();)
            {
                int8_t buf[SIZEOF_CHUNK];
                memory.ReadBlob(buf, SIZEOF_CHUNK);
                outCrypt.WriteBlob(buf, SIZEOF_CHUNK);
            }

            outCrypt.FlushWrite();
        }

        {
            // decrypt using public key
            d.TraceDetail("Verifying using asymmetric key\n");

            cFileArchive inFile;
            inFile.OpenRead(TwTestPath("rsacrypted.bin").c_str());

            cElGamalSigArchive inCrypt;
            inCrypt.SetRead(&inFile, publicKey);

            int index;
            for (index = 0; index < NUM_CHUNKS * SIZEOF_CHUNK; index += TEST_CHUNKSIZE)
            {
                int8_t buf[TEST_CHUNKSIZE];

                int s = (index + TEST_CHUNKSIZE <= NUM_CHUNKS * SIZEOF_CHUNK) ? TEST_CHUNKSIZE :
                                                                                NUM_CHUNKS * SIZEOF_CHUNK - index;

                inCrypt.ReadBlob(buf, s);

                memory.MapArchive(index, s);
                TEST(memcmp(buf, memory.GetMap(), s) == 0);
            }
        }

        delete privateKey;
        delete publicKey;
    }

#ifdef _RSA_ENCRYPTION
    cRSA            cipher(cRSA::KEY256);
    cRSAPrivateKey* privateKey;
    cRSAPublicKey*  publicKey;
    cipher.GenerateKeys(privateKey, publicKey);

    cIDEA         idea;
    cHashedKey128 ideaKey(_T("puddy, puddy, puddy, poo!"));

    const int SIZEOF_CHUNK   = 7;
    const int NUM_CHUNKS     = 1024 * 32; // 32k
    const int TEST_CHUNKSIZE = 1023 * 7;

    d.TraceDetail("Building test memory image of size %dK bytes\n", SIZEOF_CHUNK * NUM_CHUNKS / 1024);

    int8_t chunk[7];
    memcpy(chunk, "1234567", 7);

    int i;
    for (i = 0; i < NUM_CHUNKS; i++)
    {
        RandomizeBytes(chunk, 7);
        memory.WriteBlob(chunk, SIZEOF_CHUNK);
    }

    // Testing cRSAArchive

    {
        // encypt using public key
        d.TraceDetail("Encrypting using asymmetric key\n");

        cFileArchive outFile;
        outFile.OpenReadWrite(TwTestPath("rsacrypted.bin").c_str());

        cRSAArchive outCrypt;
        outCrypt.SetWrite(&outFile, publicKey);

        for (memory.Seek(0, cBidirArchive::BEGINNING); !memory.EndOfFile();)
        {
            int8_t buf[SIZEOF_CHUNK];
            memory.ReadBlob(buf, SIZEOF_CHUNK);
            outCrypt.WriteBlob(buf, SIZEOF_CHUNK);
        }

        outCrypt.FlushWrite();
    }

    {
        // decrypt using private key
        d.TraceDetail("Decrypting using asymmetric key\n");

        cFileArchive inFile;
        inFile.OpenRead(TwTestPath("rsacrypted.bin").c_str());

        cRSAArchive inCrypt;
        inCrypt.SetRead(&inFile, privateKey);

        int index;
        for (index = 0; index < NUM_CHUNKS * SIZEOF_CHUNK; index += TEST_CHUNKSIZE)
        {
            int8_t buf[TEST_CHUNKSIZE];

            int s = (index + TEST_CHUNKSIZE <= NUM_CHUNKS * SIZEOF_CHUNK) ? TEST_CHUNKSIZE :
                                                                            NUM_CHUNKS * SIZEOF_CHUNK - index;

            inCrypt.ReadBlob(buf, s);

            memory.MapArchive(index, s);
            TEST(memcmp(buf, memory.GetMap(), s) == 0);
        }
    }

    {
        // encypt using private key
        d.TraceDetail("Signing using asymmetric key\n");

        cFileArchive outFile;
        outFile.OpenReadWrite(TwTestPath("rsacrypted.bin").c_str());

        cRSAArchive outCrypt;
        outCrypt.SetWrite(&outFile, privateKey);

        for (memory.Seek(0, cBidirArchive::BEGINNING); !memory.EndOfFile();)
        {
            int8_t buf[SIZEOF_CHUNK];
            memory.ReadBlob(buf, SIZEOF_CHUNK);
            outCrypt.WriteBlob(buf, SIZEOF_CHUNK);
        }

        outCrypt.FlushWrite();
    }

    {
        // decrypt using public key
        d.TraceDetail("Verifying using asymmetric key\n");

        cFileArchive inFile;
        inFile.OpenRead(TwTestPath("rsacrypted.bin").c_str());

        cRSAArchive inCrypt;
        inCrypt.SetRead(&inFile, publicKey);

        int index;
        for (index = 0; index < NUM_CHUNKS * SIZEOF_CHUNK; index += TEST_CHUNKSIZE)
        {
            int8_t buf[TEST_CHUNKSIZE];

            int s = (index + TEST_CHUNKSIZE <= NUM_CHUNKS * SIZEOF_CHUNK) ? TEST_CHUNKSIZE :
                                                                            NUM_CHUNKS * SIZEOF_CHUNK - index;

            inCrypt.ReadBlob(buf, s);

            memory.MapArchive(index, s);
            TEST(memcmp(buf, memory.GetMap(), s) == 0);
        }
    }

    delete privateKey;
    delete publicKey;

#endif
}

void RegisterSuite_CryptoArchive()
{
    RegisterTest("CryptoArchive", "Basic", TestCryptoArchive);
}
