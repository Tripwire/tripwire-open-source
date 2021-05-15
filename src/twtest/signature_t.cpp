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

#include "fco/stdfco.h"
#include <stdio.h>
#include <iostream>
#include "fco/signature.h"
#include "core/tchar.h"
#include "twtest/test.h"
#include "core/errorgeneral.h"
#include "core/serializerimpl.h"
#include "core/crc32.h"
#include "core/archive.h"

using namespace std;

std::string getTestFile()
{
    // Create a file for which we know the signatures
    //
    //% siggen ~/signature_test.bin
    //crc      : AAAAAAAAAAy
    //md5      : B/Y8ttBnlyw/NPCUu353ao
    //crc32    : B1kP9v
    //sha      : Oia1aljHD793tfj7M55tND+3OG/
    //haval    : BL6bFSo0EP5zf8lGSueeed

    static TSTRING sigFileName;

    if (sigFileName.empty())
    {
        sigFileName = TwTestPath("signature_test.bin");

        cFileArchive fileArc;
        fileArc.OpenReadWrite(sigFileName.c_str());
        fileArc.WriteBlob("\x1\x2\x3\x4\x5\x6\x7\x8\x9\x0", 10);
        fileArc.Close();
    }

    return sigFileName;
}


void TestSignatureBasic()
{
    // Signature usage example (?)
    cCRC32Signature crcSig;
    cDebug          d("TestSignature1");

    uint8_t abData[64];
    int  i;
    for (i = 0; i < 64; i++)
        abData[i] = static_cast<uint8_t>(rand());

    crcSig.Init();
    crcSig.Update(&abData[0], 32);
    crcSig.Update(&abData[32], 32);
    crcSig.Finit();

    cMemoryArchive arch;
    arch.WriteBlob(&abData[0], 32);
    arch.WriteBlob(&abData[32], 32);
    arch.Seek(0, cBidirArchive::BEGINNING);
    cCRC32Signature crc;
    cArchiveSigGen  asg;
    asg.AddSig(&crc);
    asg.CalculateSignatures(arch);

    TEST(crc.AsStringHex() == crcSig.AsStringHex());
}

// Note: The following causes an ASSERT() in iSignature::Compare(), as it should, but
// we don't want asserts to occur in a working test suite!
//    TEST(nullSig.Compare(&checksumSig, iFCOProp::OP_EQ) == iFCOProp::CMP_WRONG_PROP_TYPE);


template<class T>
void testSignature(const TSTRING& sigFileName, const TSTRING& expectedBase64, const TSTRING& expectedHex)
{
    cFileArchive fileArc;
    cDebug       d("testSignature");

    // general signature & archive variables
    uint8_t      abBuf[iSignature::SUGGESTED_BLOCK_SIZE] = {0};
    const int cbToRead = iSignature::SUGGESTED_BLOCK_SIZE;
    int       cbRead   =0;

    // test checksum
    T sig1, sig2;
    d.TraceDetail("Testing signature...\n");

    // sig1
    fileArc.OpenRead(sigFileName.c_str());
    sig1.Init();
    do
    {
        cbRead = fileArc.ReadBlob(abBuf, cbToRead);
        sig1.Update(abBuf, cbRead);
    } while (cbRead == cbToRead);
    sig1.Finit();
    fileArc.Close();

    // sig2
    fileArc.OpenRead(sigFileName.c_str());
    sig2.Init();
    do
    {
        cbRead = fileArc.ReadBlob(abBuf, cbToRead);
        sig2.Update(abBuf, cbRead);
    } while (cbRead == cbToRead);
    sig2.Finit();
    fileArc.Close();

    // compare to each other and the known values
    TEST(sig1.Compare(&sig2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    if (!expectedBase64.empty())
    {
        TEST(sig1.AsString().compare(expectedBase64) == 0);
    }
    if (!expectedHex.empty())
    {
        TEST(sig1.AsStringHex().compare(expectedHex) == 0);
    }

    // test write capabilities
    {
        cMemoryArchive  sigArchive;
        cSerializerImpl writeSer(sigArchive, cSerializerImpl::S_WRITE);
        sig1.Write(&writeSer);
        sigArchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl readSer(sigArchive, cSerializerImpl::S_READ);
        sig2.Read(&readSer);
        TEST(sig1.Compare(&sig2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    }
}

void TestChecksum()
{
    testSignature<cChecksumSignature>(getTestFile(), _T("AAAAAAAAAAt"), _T(""));
}
		       
void TestCRC32()
{
    testSignature<cCRC32Signature>(getTestFile(), _T("B1kP9v"), _T("7590ff6f"));
}

void TestMD5()
{
    testSignature<cMD5Signature>(getTestFile(),
				 _T("B/Y8ttBnlyw/NPCUu353ao"),
				 _T("7f63cb6d067972c3f34f094bb7e776a8"));
}

void TestSHA1()
{
    testSignature<cSHASignature>(getTestFile(),
				 _T("Oia1aljHD793tfj7M55tND+3OG/"),
				 _T("e89ad5a9631c3efdded7e3ecce79b4d0fedce1bf"));
}

void TestHAVAL()
{
    testSignature<cHAVALSignature>(getTestFile(),
				  _T("BL6bFSo0EP5zf8lGSueeed"),
				  _T("4be9b152a3410fe737fc9464ae79e79d"));
}

void TestArchiveSigGen()
{
    TSTRING      sigFileName = getTestFile();
    cFileArchive fileArc;
    cDebug       d("TestArchiveSigGen");

    // test cArchiveSigGen
    cArchiveSigGen  asgtest;
    cCRC32Signature crc3;
    cMD5Signature   md53;
    cSHASignature   sha3;
    cHAVALSignature haval3;
    d.TraceDetail("Testing cArchiveSigGen\n");

    asgtest.AddSig(&crc3);
    asgtest.AddSig(&md53);
    asgtest.AddSig(&sha3);
    asgtest.AddSig(&haval3);

    // calculate the signatures
    fileArc.OpenRead(sigFileName.c_str());
    fileArc.Seek(0, cBidirArchive::BEGINNING);

    asgtest.CalculateSignatures(fileArc);

    // compare to known values
    TEST(crc3.AsString().compare(_T("B1kP9v")) == 0);
    TEST(crc3.AsStringHex().compare(_T("7590ff6f")) == 0);
    TEST(md53.AsString().compare(_T("B/Y8ttBnlyw/NPCUu353ao")) == 0);
    TEST(md53.AsStringHex().compare(_T("7f63cb6d067972c3f34f094bb7e776a8")) == 0);
    TEST(sha3.AsString().compare(_T("Oia1aljHD793tfj7M55tND+3OG/")) == 0);
    TEST(sha3.AsStringHex().compare(_T("e89ad5a9631c3efdded7e3ecce79b4d0fedce1bf")) == 0);
    TEST(haval3.AsString().compare(_T("BL6bFSo0EP5zf8lGSueeed")) == 0);
    TEST(haval3.AsStringHex().compare(_T("4be9b152a3410fe737fc9464ae79e79d")) == 0);

    fileArc.Close();
}

template<class T>
void assertSignature(const std::string& source, const std::string& expectedHex)
{
    // Signature usage example (?)
    T signature;

    signature.Init();
    signature.Update((const uint8_t*)source.c_str(), source.length());
    signature.Finit();

    TEST(signature.AsStringHex() == expectedHex);
}

void assertMD5(const std::string& source, const std::string& expectedHex)
{
    assertSignature<cMD5Signature>(source, expectedHex);
}

void assertSHA1(const std::string& source, const std::string& expectedHex)
{
    assertSignature<cSHASignature>(source, expectedHex);  
}


void TestRFC1321()
{
    // All MD5 test cases from RFC 1321, appendix A.5
    // https://www.ietf.org/rfc/rfc1321.txt

    assertMD5("", "d41d8cd98f00b204e9800998ecf8427e");
    assertMD5("a", "0cc175b9c0f1b6a831c399e269772661");
    assertMD5("abc", "900150983cd24fb0d6963f7d28e17f72");
    assertMD5("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
    assertMD5("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
    assertMD5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f");
    assertMD5("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
              "57edf4a22be3c955ac49da2e2107b67a");
}

void TestRFC3174()
{
    // SHA1 test cases from RFC 3174, section 7.3
    // https://www.ietf.org/rfc/rfc3174.txt
    // plus BSD libmd test cases
    // https://opensource.apple.com/source/libmd/libmd-3/Makefile
    //
    // TODO: Compare against NIST test vectors for extra pedanticity
    // http://csrc.nist.gov/groups/STM/cavp/secure-hashing.html

    assertSHA1("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
    assertSHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "84983e441c3bd26ebaae4aa1f95129e5e54670f1");

    assertSHA1("a", "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8");
    assertSHA1("0123456701234567012345670123456701234567012345670123456701234567",
               "e0c094e867ef46c350ef54a7f59dd60bed92ae83");

    assertSHA1("", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    assertSHA1("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
    assertSHA1("message digest", "c12252ceda8be8994d5fa0290a47231c1d16aae3");
    assertSHA1("abcdefghijklmnopqrstuvwxyz", "32d10c7b8cf96570ca04ce37f2a19d84240d3a89");
    assertSHA1("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
               "761c457bf73b14d27e9e9265c46f4b4dda11f940");
    assertSHA1("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
               "50abf5706a150990a08b2c5ea40fa0e585554732");
}


void RegisterSuite_Signature()
{
    RegisterTest("Signature", "Basic", TestSignatureBasic);
    RegisterTest("Signature", "Checksum", TestChecksum);
    RegisterTest("Signature", "CRC32", TestCRC32);
    RegisterTest("Signature", "MD5", TestMD5);
    RegisterTest("Signature", "SHA1", TestSHA1);
    RegisterTest("Signature", "HAVAL", TestHAVAL);
    RegisterTest("Signature", "ArchiveSigGen", TestArchiveSigGen);
    RegisterTest("Signature", "RFC1321", TestRFC1321);
    RegisterTest("Signature", "RFC3174", TestRFC3174);
}
