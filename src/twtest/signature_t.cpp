//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2017 Tripwire,
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

void TestSignature()
{
    // Signature usage example (?)
    cCRC32Signature     crcSig;
    cDebug              d("TestSignature");

    byte abData[ 64 ];
    int i;
    for( i = 0; i < 64; i++ )
        abData[i] = static_cast< byte >( rand() );

    crcSig.Init();
    crcSig.Update( &abData[0], 32 );
    crcSig.Update( &abData[32], 32 );
    crcSig.Finit();
    TCOUT << _T("new way: ") << crcSig.AsString() << endl;

    cMemoryArchive arch;
    arch.WriteBlob( &abData[0], 32 );
    arch.WriteBlob( &abData[32], 32 );
    arch.Seek( 0, cBidirArchive::BEGINNING );
    cCRC32Signature crc;
    cArchiveSigGen asg;
    asg.AddSig( &crc );
    asg.CalculateSignatures( arch );

    TCOUT << _T("old way: ") << crc.AsString() << endl;


    // Note: The following causes an ASSERT() in iSignature::Compare(), as it should, but
    // we don't want asserts to occur in a working test suite!
//    TEST(nullSig.Compare(&checksumSig, iFCOProp::OP_EQ) == iFCOProp::CMP_WRONG_PROP_TYPE);


    
    // Create a file for which we know the signatures
    //
    //% siggen ~/signature_test.bin 
    //crc      : AAAAAAAAAAy
    //md5      : B/Y8ttBnlyw/NPCUu353ao
    //crc32    : B1kP9v
    //sha      : Oia1aljHD793tfj7M55tND+3OG/
    //haval    : BL6bFSo0EP5zf8lGSueeed

    TSTRING sigFileName = TwTestPath("signature_test.bin");

    cFileArchive fileArc;
    fileArc.OpenReadWrite(sigFileName.c_str());
    fileArc.WriteBlob("\x1\x2\x3\x4\x5\x6\x7\x8\x9\x0", 10);
    fileArc.Close();


    // test begins here

    // general signature & archive variables
    byte abBuf[iSignature::SUGGESTED_BLOCK_SIZE];
    const int cbToRead = iSignature::SUGGESTED_BLOCK_SIZE;
    int cbRead;

    
    // test checksum
    cChecksumSignature check1, check2;
    d.TraceDetail("Testing checksum.\n");

    // check1
    fileArc.OpenRead(sigFileName.c_str());
    check1.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        check1.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    check1.Finit();
    fileArc.Close();

    // check2
    fileArc.OpenRead(sigFileName.c_str());
    check2.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        check2.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    check2.Finit();
    fileArc.Close();

    // compare to each other and the known values
    TEST(check1.Compare(&check2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    TEST(check1.AsString().compare(_T("AAAAAAAAAAt")) == 0);

    // test write capabilities
    {
        cMemoryArchive sigArchive;
        cSerializerImpl writeSer(sigArchive, cSerializerImpl::S_WRITE);
        check1.Write(&writeSer);
        sigArchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl readSer(sigArchive, cSerializerImpl::S_READ);
        check2.Read(&readSer);
        TEST(check1.Compare(&check2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    }

    
    // test CRC32
    cCRC32Signature crc1, crc2;
    d.TraceDetail("Testing CRC32.\n");

    // crc1
    fileArc.OpenRead(sigFileName.c_str());
    crc1.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        crc1.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    crc1.Finit();
    fileArc.Close();

    // crc2
    fileArc.OpenRead(sigFileName.c_str());
    crc2.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        crc2.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    crc2.Finit();
    fileArc.Close();

    // compare to each other and the known values
    TEST(crc1.Compare(&crc2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    TEST(crc1.AsString().compare(_T("B1kP9v")) == 0);
    TEST(crc1.AsStringHex().compare(_T("7590ff6f")) == 0);

    // test write capabilities
    {
        cMemoryArchive sigArchive;
        cSerializerImpl writeSer(sigArchive, cSerializerImpl::S_WRITE);
        crc1.Write(&writeSer);
        sigArchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl readSer(sigArchive, cSerializerImpl::S_READ);
        crc2.Read(&readSer);
        TEST(crc1.Compare(&crc2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    }


    // test MD5
    cMD5Signature md51, md52;
    d.TraceDetail("Testing MD5.\n");

    // md51
    fileArc.OpenRead(sigFileName.c_str());
    md51.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        md51.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    md51.Finit();
    fileArc.Close();

    // md52
    fileArc.OpenRead(sigFileName.c_str());
    md52.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        md52.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    md52.Finit();
    fileArc.Close();

    // compare to each other and the known values
    TEST(md51.Compare(&md52, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    TEST(md51.AsString().compare(_T("B/Y8ttBnlyw/NPCUu353ao")) == 0);
    TEST(md51.AsStringHex().compare(_T("7f63cb6d067972c3f34f094bb7e776a8")) == 0);

    // test write capabilities
    {
        cMemoryArchive sigArchive;
        cSerializerImpl writeSer(sigArchive, cSerializerImpl::S_WRITE);
        md51.Write(&writeSer);
        sigArchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl readSer(sigArchive, cSerializerImpl::S_READ);
        md52.Read(&readSer);
        TEST(md51.Compare(&md52, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    }


    // test SHA
    cSHASignature sha1, sha2;
    d.TraceDetail("Testing SHA.\n");

    // sha1
    fileArc.OpenRead(sigFileName.c_str());
    sha1.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        sha1.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    sha1.Finit();
    fileArc.Close();

    // sha2
    fileArc.OpenRead(sigFileName.c_str());
    sha2.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        sha2.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    sha2.Finit();
    fileArc.Close();

    // compare to each other and the known values
    TEST(sha1.Compare(&sha2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    TEST(sha1.AsString().compare(_T("Oia1aljHD793tfj7M55tND+3OG/")) == 0);
    TEST(sha1.AsStringHex().compare(_T("e89ad5a9631c3efdded7e3ecce79b4d0fedce1bf")) == 0);

    // test write capabilities
    {
        cMemoryArchive sigArchive;
        cSerializerImpl writeSer(sigArchive, cSerializerImpl::S_WRITE);
        sha1.Write(&writeSer);
        sigArchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl readSer(sigArchive, cSerializerImpl::S_READ);
        sha2.Read(&readSer);
        TEST(sha1.Compare(&sha2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    }

    
    // test HAVAL
    cHAVALSignature haval1, haval2;
    d.TraceDetail("Testing HAVAL.\n");

    // haval1
    fileArc.OpenRead(sigFileName.c_str());
    haval1.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        haval1.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    haval1.Finit();
    fileArc.Close();

    // haval2
    fileArc.OpenRead(sigFileName.c_str());
    haval2.Init();
    do
    {
        cbRead = fileArc.ReadBlob( abBuf, cbToRead );
        haval2.Update( abBuf, cbRead );
    }
    while ( cbRead == cbToRead );
    haval2.Finit();
    fileArc.Close();

    // compare to each other and the known values
    TEST(haval1.Compare(&haval2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    TEST(haval1.AsString().compare(_T("BL6bFSo0EP5zf8lGSueeed")) == 0);
    TEST(haval1.AsStringHex().compare(_T("4be9b152a3410fe737fc9464ae79e79d")) == 0);

    // test write capabilities
    {
        cMemoryArchive sigArchive;
        cSerializerImpl writeSer(sigArchive, cSerializerImpl::S_WRITE);
        haval1.Write(&writeSer);
        sigArchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl readSer(sigArchive, cSerializerImpl::S_READ);
        md52.Read(&readSer);
        TEST(haval1.Compare(&haval2, iFCOProp::OP_EQ) == iFCOProp::CMP_TRUE);
    }
    

    // test cArchiveSigGen
    cArchiveSigGen asgtest;
    cCRC32Signature crc3;
    cMD5Signature md53;
    cSHASignature sha3;
    cHAVALSignature haval3;
    d.TraceDetail("Testing cArchiveSigGen\n");
    
    asgtest.AddSig( &crc3 );
    asgtest.AddSig( &md53 );
    asgtest.AddSig( &sha3 );
    asgtest.AddSig( &haval3 );
    
    // calculate the signatures
    fileArc.OpenRead(sigFileName.c_str());
    fileArc.Seek( 0, cBidirArchive::BEGINNING );
    
    asgtest.CalculateSignatures( fileArc );

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

    return;
}

void RegisterSuite_Signature()
{
    RegisterTest("Signature", "Basic", TestSignature);
}
