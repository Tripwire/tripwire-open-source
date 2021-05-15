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
// crypto-t.cpp -- generic crypto implementations
//

#include "twcrypto/stdtwcrypto.h"
#include "twcrypto/crypto.h"
#include "core/archive.h"
#include "twtest/test.h"

void TestCrypto()
{
    const int COUNT = 4000;

    const int BUFFER_SIZE = 9000;

    std::vector<char> source_buf(BUFFER_SIZE);
    std::vector<char> crypt_buf(COUNT + BUFFER_SIZE); // needs to be able to hold even number of blocks
    std::vector<char> dest_buf(COUNT);

    char* source = &source_buf[0];
    char* crypt  = &crypt_buf[0];
    char* dest   = &dest_buf[0];

    memcpy(source, "I love the smell of the sheep.", 31);


#ifdef _IDEA_ENCRYPTION
    ///////////////////////////////////////////////////////////////////////////
    // IDEA test
    {
        memset(crypt, 0, COUNT + 1024);
        memset(dest, 0, COUNT);

        cIDEA crypter;

        // we use buf for when the end of the source doesn't fall on a
        // blocksize boundry.
        TEST(crypter.GetBlockSizePlain() < 32);
        TEST(crypter.GetBlockSizeCipher() < 32);
        char buf[32];

        // encrypt the phrase
        crypter.SetKey(iCipher::ENCRYPT, cHashedKey128(_T("big cow")));

        int i, j;
        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                memcpy(buf, source + i, COUNT - i);
                memset(buf + COUNT - i, 0, crypter.GetBlockSizePlain() - COUNT + i);
                crypter.ProcessBlock(buf, crypt + j);
                break;
            }

            crypter.ProcessBlock(source + i, crypt + j);
        }

        // dycrypt the phrase
        crypter.SetKey(iCipher::DECRYPT, cHashedKey128(_T("big cow")));

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt + j, buf);
                memcpy(dest + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt + j, dest + i);
        }

        TEST(memcmp(source, dest, COUNT) == 0);
    }
#endif

#ifdef _RSA_ENCRYPTION
    ///////////////////////////////////////////////////////////////////////////
    // RSA test
    {
        int i, j;

        memset(crypt, 0, COUNT + 1024);
        memset(dest, 0, COUNT);

        cRSA crypter(cRSA::KEY2048);

        // we use buf for when the end of the source doesn't fall on a
        // blocksize boundry.
        TEST(crypter.GetBlockSizePlain() < BUFFER_SIZE);
        TEST(crypter.GetBlockSizeCipher() < BUFFER_SIZE);
        char buf[BUFFER_SIZE];

        cRSAPublicKey*  pPublic;
        cRSAPrivateKey* pPrivate;

        crypter.GenerateKeys(pPrivate, pPublic);

        // save the keys to a data stream and read back in
        cRSAPublicKey*  pPublic2;
        cRSAPrivateKey* pPrivate2;

        TEST(pPublic->GetWriteLen() < BUFFER_SIZE);
        pPublic->Write(buf);
        pPublic2 = new cRSAPublicKey(buf);

        TEST(pPrivate->GetWriteLen() < BUFFER_SIZE);
        pPrivate->Write(buf);
        pPrivate2 = new cRSAPrivateKey(buf);

        // we will try encrypting to a second pair of buffers and see if all goes well
        char crypt2[COUNT + BUFFER_SIZE];
        char dest2[COUNT];

        // encrypt the phrase
        crypter.SetEncrypting(pPublic);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                memcpy(buf, source + i, COUNT - i);
                memset(buf + COUNT - i, 0, crypter.GetBlockSizePlain() - COUNT + i);
                crypter.ProcessBlock(buf, crypt + j);
                break;
            }

            crypter.ProcessBlock(source + i, crypt + j);
        }

        crypter.SetEncrypting(pPublic2);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                memcpy(buf, source + i, COUNT - i);
                memset(buf + COUNT - i, 0, crypter.GetBlockSizePlain() - COUNT + i);
                crypter.ProcessBlock(buf, crypt2 + j);
                break;
            }

            crypter.ProcessBlock(source + i, crypt2 + j);
        }

        // You might think this test would be valid (I did), but it is not.
        // The Crypto++ lib pads input with random bytes so encrypting the
        // same plaintext twice will not create the same ciphertext.
        //TEST(memcmp(crypt, crypt2, crypter.GetBlockSizeCipher()) == 0);

        // dycrypt the phrase
        crypter.SetDecrypting(pPrivate);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt + j, buf);
                memcpy(dest + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt + j, dest + i);
        }

        TEST(memcmp(source, dest, COUNT) == 0);

        crypter.SetDecrypting(pPrivate2);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt2 + j, buf);
                memcpy(dest2 + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt2 + j, dest2 + i);
        }

        TEST(memcmp(source, dest2, COUNT) == 0);
        TEST(memcmp(dest, dest2, COUNT) == 0);

        // zero out things and try signing and verifying
        memset(crypt, 0, COUNT + 1024);
        memset(dest, 0, COUNT);

        // sign the phrase
        crypter.SetSigning(pPrivate);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                memcpy(buf, source + i, COUNT - i);
                memset(buf + COUNT - i, 0, crypter.GetBlockSizePlain() - COUNT + i);
                crypter.ProcessBlock(buf, crypt + j);
                break;
            }

            crypter.ProcessBlock(source + i, crypt + j);
        }

        // verify the phrase
        crypter.SetVerifying(pPublic);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt + j, buf);
                memcpy(dest + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt + j, dest + i);
        }

        TEST(memcmp(source, dest, COUNT) == 0);

        // verify the phrase again using a local public key
        cRSAPublicKey publicKey(*pPrivate);
        crypter.SetVerifying(&publicKey);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt + j, buf);
                memcpy(dest + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt + j, dest + i);
        }

        TEST(memcmp(source, dest, COUNT) == 0);

        delete pPublic;
        delete pPrivate;
    }
#endif


    ///////////////////////////////////////////////////////////////////////////
    // El Gamal test
    {
        int i, j;

        TCOUT << _T("El Gamal test\n");

        memset(crypt, 0, COUNT + 1024);
        memset(dest, 0, COUNT);

        cElGamalSig crypter(cElGamalSig::KEY512);

        // we use buf for when the end of the source doesn't fall on a
        // blocksize boundry.
        TEST(crypter.GetBlockSizePlain() < BUFFER_SIZE);
        TEST(crypter.GetBlockSizeCipher() < BUFFER_SIZE);
        char buf[BUFFER_SIZE];

        cElGamalSigPublicKey*  pPublic;
        cElGamalSigPrivateKey* pPrivate;

        crypter.GenerateKeys(pPrivate, pPublic);

        // save the keys to a data stream and read back in
        cElGamalSigPublicKey*  pPublic2;
        cElGamalSigPrivateKey* pPrivate2;

        TEST(pPublic->GetWriteLen() < BUFFER_SIZE);
        pPublic->Write(buf);
        pPublic2 = new cElGamalSigPublicKey(buf);

        TEST(pPrivate->GetWriteLen() < BUFFER_SIZE);
        pPrivate->Write(buf);
        pPrivate2 = new cElGamalSigPrivateKey(buf);

        // we will try encrypting to a second pair of buffers and see if all goes well
        //char crypt2[COUNT + BUFFER_SIZE];
        //char dest2[COUNT];

        // zero out things and try signing and verifying
        memset(crypt, 0, COUNT + 1024);
        memset(dest, 0, COUNT);

        TCOUT << _T("El Gamal Signing...\n");

        // sign the phrase
        crypter.SetSigning(pPrivate);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                memcpy(buf, source + i, COUNT - i);
                memset(buf + COUNT - i, 0, crypter.GetBlockSizePlain() - COUNT + i);
                crypter.ProcessBlock(buf, crypt + j);
                break;
            }

            crypter.ProcessBlock(source + i, crypt + j);
        }

        TCOUT << _T("El Gamal Verifying...\n");

        // verify the phrase
        crypter.SetVerifying(pPublic);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt + j, buf);
                memcpy(dest + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt + j, dest + i);
        }

        TEST(memcmp(source, dest, COUNT) == 0);

        TCOUT << _T("El Gamal Verifying with serialized key...\n");

        // verify the phrase again using a local public key
        cElGamalSigPublicKey publicKey(*pPrivate);
        crypter.SetVerifying(&publicKey);

        for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
        {
            if (i + crypter.GetBlockSizePlain() > COUNT)
            {
                crypter.ProcessBlock(crypt + j, buf);
                memcpy(dest + i, buf, COUNT - i);
                break;
            }

            crypter.ProcessBlock(crypt + j, dest + i);
        }

        TEST(memcmp(source, dest, COUNT) == 0);

        TCOUT << _T("El Gamal Testing altered buffer...\n");

        try
        {
            crypt[4] = '\x42';

            // verify the phrase again using a local public key
            cElGamalSigPublicKey publicKey(*pPrivate);
            crypter.SetVerifying(&publicKey);

            for (i = 0, j = 0;; i += crypter.GetBlockSizePlain(), j += crypter.GetBlockSizeCipher())
            {
                if (i + crypter.GetBlockSizePlain() > COUNT)
                {
                    crypter.ProcessBlock(crypt + j, buf);
                    memcpy(dest + i, buf, COUNT - i);
                    break;
                }

                crypter.ProcessBlock(crypt + j, dest + i);
            }

            TEST(memcmp(source, dest, COUNT) == 0);
        }
        catch (eArchiveCrypto&)
        {
            // accept eArchiveCrypto
        }
        catch (eArchive&)
        {
            TCOUT << _T("Modified Buffer Test failed!\n");
            TEST(false);
        }

        delete pPublic;
        delete pPrivate;
        delete pPublic2;
        delete pPrivate2;
    }
}

void RegisterSuite_Crypto()
{
    RegisterTest("Crypto", "Basic", TestCrypto);
}
