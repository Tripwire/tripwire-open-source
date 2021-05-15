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
// keyfile_t.cpp
//

#include "twcrypto/stdtwcrypto.h"
#include "twcrypto/keyfile.h"
#include "twcrypto/crypto.h"
#include "core/archive.h"
#include "core/debug.h"
#include "twtest/test.h"
#include "tw/twutil.h"

////////////////////////////////////////////////////////////////////////////////

void TestKeyFile()
{
    cDebug d("TestKeyFile");

    // test keyfile exceptions all work correctly
    d.TraceDebug("Testing expception strings, IGNORE THESE ERRORS...\n");
    cTWUtil::PrintErrorMsg(eKeyFile(_T("Testing only, ignore this.")));
    cTWUtil::PrintErrorMsg(eKeyFileInvalidFmt(_T("Testing only, ignore this.")));
    cTWUtil::PrintErrorMsg(eKeyFileBadPassphrase(_T("Testing only, ignore this.")));
    cTWUtil::PrintErrorMsg(eKeyFileArchive(_T("Testing only, ignore this.")));
    cTWUtil::PrintErrorMsg(eKeyFileUninitialized(_T("Testing only, ignore this.")));

    cKeyFile keyfile;

    d.TraceDebug("Generating keys...\n");
    std::string s = "haybaby";
    keyfile.GenerateKeys(cElGamalSig::KEY1024, (int8_t*)s.data(), 7);

    char plaintext[9000];
    char ciphertext[9000];
    memset(plaintext, 42, 9000);

    // encrypt
    d.TraceDebug("Encrypting...\n");
    {
        cElGamalSig elGamal(*keyfile.GetPublicKey());

        TEST(elGamal.GetBlockSizePlain() < 9000);
        TEST(elGamal.GetBlockSizeCipher() < 9000);
        std::string      s = "haybaby";
        cPrivateKeyProxy key;
        TEST(key.AquireKey(keyfile, (int8_t*)s.data(), 7));
        elGamal.SetSigning(key.GetKey());
        elGamal.ProcessBlock(plaintext, ciphertext);
    }

    // decrypt
    d.TraceDebug("Decrypting...\n");
    {
        cElGamalSig elGamal(*keyfile.GetPublicKey());
        char        recovered_text[9000];

        elGamal.SetVerifying(keyfile.GetPublicKey());
        elGamal.ProcessBlock(ciphertext, recovered_text);

        TEST(memcmp(recovered_text, plaintext, elGamal.GetBlockSizePlain()) == 0);
    }

    // save to and read from memory
    d.TraceDebug("Read/Write to memory...\n");
    {
        int8_t* pMem = new int8_t[keyfile.GetWriteLen()];
        keyfile.WriteMem(pMem);

        cKeyFile keyfile2;
        TEST(!keyfile2.KeysLoaded());
        keyfile2.ReadMem(pMem);
        TEST(keyfile2.KeysLoaded());

        cElGamalSig elGamal(*keyfile2.GetPublicKey());
        char        recovered_text[9000];

        elGamal.SetVerifying(keyfile2.GetPublicKey());
        elGamal.ProcessBlock(ciphertext, recovered_text);

        TEST(memcmp(recovered_text, plaintext, elGamal.GetBlockSizePlain()) == 0);
        delete [] pMem;
    }

    // save to and read from disk
    d.TraceDebug("Read/Write to file...\n");
    {
        keyfile.WriteFile(TwTestPath("keyfile.key").c_str());

        cKeyFile keyfile2;
        TEST(!keyfile2.KeysLoaded());
        keyfile2.ReadFile(TwTestPath("keyfile.key").c_str());
        TEST(keyfile2.KeysLoaded());

        cElGamalSig elGamal(*keyfile2.GetPublicKey());
        char        recovered_text[9000];

        elGamal.SetVerifying(keyfile2.GetPublicKey());
        elGamal.ProcessBlock(ciphertext, recovered_text);

        TEST(memcmp(recovered_text, plaintext, elGamal.GetBlockSizePlain()) == 0);
    }

    return;
}

void RegisterSuite_KeyFile()
{
    RegisterTest("KeyFile", "Basic", TestKeyFile);
}
