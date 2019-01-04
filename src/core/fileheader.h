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
// fileheader.h

#ifndef __FILEHEADER_H
#define __FILEHEADER_H

#ifndef __ARCHIVE_H
#include "archive.h"
#endif

#ifndef __SERIALIZABLE_H
#include "serializable.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// class cFileHeaderID

class cFileHeaderID : public iSerializable
{
public:
    cFileHeaderID();
    cFileHeaderID(const TCHAR* id);
    cFileHeaderID(const cFileHeaderID& rhs);
    virtual ~cFileHeaderID();

    void operator=(const TCHAR* id);
    void operator=(const cFileHeaderID& rhs);
    int  operator==(const cFileHeaderID& rhs) const;
    int  operator!=(const cFileHeaderID& rhs) const;

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

private:
    // For now we will store the id as a string.
    // At some point we may want to make this more efficient like
    // the way we implemented cFCONames.
    // Note: We store the string as narrow chars, since
    // the program is the only person who will see them.
    int16_t mIDLen;

    enum
    {
        MAXBYTES = 256
    };
    char mID[MAXBYTES];
};

inline cFileHeaderID::cFileHeaderID()
{
    mIDLen = 0;
}

inline cFileHeaderID::cFileHeaderID(const TCHAR* id)
{
    *this = id;
}

inline cFileHeaderID::cFileHeaderID(const cFileHeaderID& rhs) : iSerializable(), mIDLen(rhs.mIDLen)
{
    memcpy(mID, rhs.mID, MAXBYTES);
}

inline int cFileHeaderID::operator!=(const cFileHeaderID& rhs) const
{
    return !(*this == rhs);
}

///////////////////////////////////////////////////////////////////////////////
// class cFileHeader

class cFileHeader : public iSerializable
{
public:
    cFileHeader();
    cFileHeader(const cFileHeader& rhs);
    virtual ~cFileHeader();

    enum Encoding
    {
        NO_ENCODING = 0,
        COMPRESSED,
        ASYM_ENCRYPTION,
        SYM_ENCRYPTION,
        LAST_ENCODING
    };

    void                 SetID(const cFileHeaderID& id);
    const cFileHeaderID& GetID() const;

    void     SetVersion(uint32_t v);
    uint32_t GetVersion() const;

    void     SetEncoding(Encoding e);
    Encoding GetEncoding() const;

    cMemoryArchive&       GetBaggage();
    const cMemoryArchive& GetBaggage() const;

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;               // throw (eSerializer, eArchive)

protected:
    cFileHeaderID  mID;
    uint32_t       mVersion;
    Encoding       mEncoding;
    cMemoryArchive mBaggage; // items that have been serialized to this object
};

inline const cFileHeaderID& cFileHeader::GetID() const
{
    return mID;
}

inline uint32_t cFileHeader::GetVersion() const
{
    return mVersion;
}

inline cFileHeader::Encoding cFileHeader::GetEncoding() const
{
    return mEncoding;
}

inline cMemoryArchive& cFileHeader::GetBaggage()
{
    return mBaggage;
}

inline const cMemoryArchive& cFileHeader::GetBaggage() const
{
    return mBaggage;
}

#endif
