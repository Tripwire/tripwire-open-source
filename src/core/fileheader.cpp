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
// fileheader.cpp

#include "stdcore.h"
#include "fileheader.h"
#include "serializer.h"
#include "ntmbs.h"

// A magic number to put at top of file headers
// I changed this magic number in 2.1 since we now support versioning in the file header.
// (the old magic number was 0x00202039)
// I generated the random number using the random.org web site.
const uint32_t FILE_HEADER_MAGIC_NUMBER = 0x78f9beb3;

///////////////////////////////////////////////////////////////////////////////
// class cFileHeaderID

cFileHeaderID::~cFileHeaderID()
{
}

void cFileHeaderID::operator=(const TCHAR* pszId)
{
    // RAD:10/1/99 -- Not Needed
    // TODO:BAM -- I'm pretty sure that there's a bug hiding here...
    //size_t len = _tcslen(id);
    //if (len >= 256)
    //{
    //    ASSERT(false);
    //    mIDLen = 0;
    //    return;
    //}

    size_t N = ::strlen(pszId);
    if (!(N < cFileHeaderID::MAXBYTES))
        throw eCharacter(TSS_GetString(cCore, core::STR_ERR_OVERFLOW));

    mIDLen = static_cast<int16_t>(N); // know len is less than MAXBYTES
    ::memcpy(mID, pszId, N * sizeof(char));
}

void cFileHeaderID::operator=(const cFileHeaderID& rhs)
{
    ASSERT(rhs.mIDLen < cFileHeaderID::MAXBYTES);
    mIDLen = rhs.mIDLen;
    memcpy(mID, rhs.mID, mIDLen * sizeof(char));
}

int cFileHeaderID::operator==(const cFileHeaderID& rhs) const
{
    return (mIDLen == rhs.mIDLen) && (::memcmp(mID, rhs.mID, mIDLen * sizeof(char)) == 0);
}

void cFileHeaderID::Read(iSerializer* pSerializer, int32_t /*version*/) // throw (eSerializer, eArchive)
{
    int16_t len;
    pSerializer->ReadInt16(len);
    if ((len < 0) || (len >= cFileHeaderID::MAXBYTES))
    {
        // this is invalid!
        throw eSerializerInputStreamFmt(_T("File Header ID invalid"));
    }
    pSerializer->ReadBlob(mID, len * sizeof(char));
    mIDLen = len;
}

void cFileHeaderID::Write(iSerializer* pSerializer) const // throw (eSerializer, eArchive)
{
    ASSERT(mIDLen >= 0 && mIDLen < cFileHeaderID::MAXBYTES);

    pSerializer->WriteInt16(mIDLen);
    pSerializer->WriteBlob(mID, mIDLen * sizeof(char));
}

///////////////////////////////////////////////////////////////////////////////
// class cFileHeader

cFileHeader::cFileHeader() : mVersion(0)
{
#ifdef DEBUG
    mEncoding = LAST_ENCODING; // set to invalid value so we can assert on write
#else
    mEncoding = NO_ENCODING;
#endif
}

cFileHeader::cFileHeader(const cFileHeader& rhs)
    : iSerializable(), mID(rhs.mID), mVersion(rhs.mVersion), mEncoding(rhs.mEncoding)
{
    if (rhs.mBaggage.Length() > 0)
    {
        mBaggage.MapArchive(0, rhs.mBaggage.Length());
        rhs.mBaggage.MapArchive(0, rhs.mBaggage.Length());

        ::memcpy(mBaggage.GetMap(), rhs.mBaggage.GetMap(), static_cast<size_t>(rhs.mBaggage.Length()));

        mBaggage.MapArchive(0, 0);
        rhs.mBaggage.MapArchive(0, 0);
    }
}

cFileHeader::~cFileHeader()
{
}

void cFileHeader::SetID(const cFileHeaderID& id)
{
    mID = id;
}

void cFileHeader::SetVersion(uint32_t v)
{
    mVersion = v;
}

void cFileHeader::SetEncoding(Encoding e)
{
    mEncoding = e;
}

void cFileHeader::Read(iSerializer* pSerializer, int32_t /*version*/) // throw (eSerializer, eArchive)
{
    int16_t e;
    int32_t len;
    int32_t magicNumber;
    int32_t version;

    cDebug d("cFileHeader::Read");

    pSerializer->ReadInt32(magicNumber);
    if ((unsigned int)magicNumber != FILE_HEADER_MAGIC_NUMBER)
    {
        d.TraceDebug("Bad magic number");
        throw eSerializerInputStreamFmt(pSerializer->GetFileName());
    }

    // Note this version refers to the format of this data structure in the
    // file image.  mVersion refers to the version of data that accompanies
    // this cFileHeader, for example the database or report format.
    pSerializer->ReadInt32(version);
    if (version > 1)
    {
        // don't understand the version
        d.TraceDebug("Bad version");
        throw eSerializerInputStreamFmt(pSerializer->GetFileName());
    }

    mID.Read(pSerializer);

    pSerializer->ReadInt32(version);
    mVersion = version;

    pSerializer->ReadInt16(e);
    mEncoding = (Encoding)e;

    pSerializer->ReadInt32(len);

    if (len < 0 || len > 0xffff)
        throw eSerializerInputStreamFmt(pSerializer->GetFileName());

    mBaggage.MapArchive(0, len);

    if (len > 0)
    {
        mBaggage.MapArchive(0, len);
        pSerializer->ReadBlob(mBaggage.GetMap(), len);
    }

    mBaggage.Seek(len, cBidirArchive::BEGINNING);
    mBaggage.Truncate();
    mBaggage.Seek(0, cBidirArchive::BEGINNING);
}

void cFileHeader::Write(iSerializer* pSerializer) const // throw (eSerializer, eArchive)
{
#ifdef DEBUG
    // check that we set some values
    cFileHeaderID id;
    ASSERT(mID != id);
    ASSERT(mEncoding != LAST_ENCODING);
#endif

    // we need to have our own versioning since we are not
    // a iTypedSerializable object.
    pSerializer->WriteInt32(FILE_HEADER_MAGIC_NUMBER);

    // Note this version refers to the format of this data structure in the
    // file image.  mVersion refers to the version of data that accompanies
    // this cFileHeader, for example the database or report format.
    pSerializer->WriteInt32(1); // version

    mID.Write(pSerializer);

    pSerializer->WriteInt32(mVersion);

    pSerializer->WriteInt16(static_cast<int16_t>(mEncoding));

    int32_t len = static_cast<int32_t>(mBaggage.Length());

    ASSERT(len >= 0);
    ASSERT(len <= 0xFFFF);

    if (len < 0 || len > 0xFFFF)
        throw eSerializerOutputStreamFmt();

    pSerializer->WriteInt32(len);
    if (len > 0)
    {
        mBaggage.MapArchive(0, len);
        pSerializer->WriteBlob(mBaggage.GetMap(), len);
    }
}
