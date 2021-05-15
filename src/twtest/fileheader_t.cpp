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
// fileheader_t.cpp

#include "core/stdcore.h"
#include "core/fileheader.h"
#include "twtest/test.h"
#include "core/serializerimpl.h"
#include "core/archive.h"

void TestFileHeader()
{
    cMemoryArchive memarch;

    // cFileHeaderID testing
    cFileHeaderID id(_T("testheader1"));
    cFileHeaderID id_dup(id);
    cFileHeaderID id_null;

    TEST(id == id);
    TEST(id == id_dup);
    TEST(id != id_null);

    {
        cSerializerImpl ser(memarch, cSerializerImpl::S_WRITE);

        ser.Init();
        id.Write(&ser);
        id_dup.Write(&ser);
        id_null.Write(&ser);
        ser.Finit();
    }

    memarch.Seek(0, cBidirArchive::BEGINNING);

    {
        cSerializerImpl ser(memarch, cSerializerImpl::S_READ);
        cFileHeaderID   id_read;

        ser.Init();
        id_read.Read(&ser);
        TEST(id_read == id);
        id_read.Read(&ser);
        TEST(id_read == id_dup);
        id_read.Read(&ser);
        TEST(id_read == id_null);
        ser.Finit();
    }

    memarch.Seek(0, cBidirArchive::BEGINNING);
    memarch.Truncate();


    // cFileHeader testing
    cFileHeader header1;
    cFileHeader header2;

    header1.SetID(cFileHeaderID(_T("myheader")));
    TEST(header1.GetID() == cFileHeaderID(_T("myheader")));
    header1.SetEncoding(cFileHeader::SYM_ENCRYPTION);
    TEST(header1.GetEncoding() == cFileHeader::SYM_ENCRYPTION);

    header2.SetID(id);
    TEST(header2.GetID() == id);
    header2.SetEncoding(cFileHeader::COMPRESSED);
    header2.GetBaggage().WriteBlob("abc123", 6);

    {
        cSerializerImpl ser(memarch, cSerializerImpl::S_WRITE);

        ser.Init();
        header1.Write(&ser);
        header2.Write(&ser);
        ser.Finit();
    }

    memarch.Seek(0, cBidirArchive::BEGINNING);

    {
        cSerializerImpl ser(memarch, cSerializerImpl::S_READ);
        cFileHeader     header_read;

        ser.Init();
        header_read.Read(&ser);
        TEST(header_read.GetID() == header1.GetID());
        TEST(header_read.GetEncoding() == header1.GetEncoding());
        TEST(header_read.GetBaggage().Length() == 0);
        header_read.Read(&ser);
        TEST(header_read.GetID() == header2.GetID());
        TEST(header_read.GetEncoding() == header2.GetEncoding());
        TEST(header_read.GetBaggage().Length() != 0);
        ser.Finit();

        char buf[6];
        header_read.GetBaggage().ReadBlob(buf, 6);
        TEST(memcmp(buf, "abc123", 6) == 0);
    }
}

void RegisterSuite_FileHeader()
{
    RegisterTest("FileHeader", "Basic", TestFileHeader);
}
