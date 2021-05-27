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
// serializable.h -- the serializable interface
///////////////////////////////////////////////////////////////////////////////

#ifndef __SERIALIZABLE_H
#define __SERIALIZABLE_H

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __TYPED_H
#include "typed.h"
#endif

class iSerializer;

// Some version documentation is in order here:
//
// Objects that derive from iSerializable do not need to worry about versions, they
// should just read and write themselves to and from the serializer.
//
// Objects that wish to implement versioning should derive themselves from iTypedSerializerable.
// They should use the IMPLEMENT_TYPEDSERIALIZABLE() macro to declare their major and minor
// version number.  Neither major or minor version number can be negative and at least one
// must be greater than zero.
//
// When an object is serialized using iSerializer::WriteObject() a header will be written that
// includes the current version number will be saved in the archive by.  Therefore
// iSerializable::Write() implementations do not need to worry about versioning, They need only
// write out the object in current format.
//
// When an object is read back from an archive using iSerializer::ReadObject(),
// the version of the serialized object will be read from the header and is passed to
// iSerializable::Read().  Each implementation of Read() should this passed in version.
// If the version is greater than the current version Read() should throw a eSerializer
// exception. If the version is older than the current implementation, Read() should either
// read the older format or throw an eSerializer exception.
//
// IMPORTANT: If the version is 0 (the default parameter) then Read() it should read
// in the current format of the object. This is important if Read() is called directly.
//
// In some cases it may be desirable to not incur the overhead of the serializer writing
// header information for each serialized object.  In this case call iSerializable::Read() and
// iSerializable::Write() directly rather than using ReadObject() and WriteObject().

class iSerializable
{
public:
    virtual void Read(iSerializer* pSerializer, int32_t version = 0) = 0; // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const             = 0; // throw (eSerializer, eArchive)
        // objects implement these methods to read and write themselves to a serializer.

    virtual ~iSerializable()
    {
    }
};

class iTypedSerializable : public iTyped, public iSerializable
{
public:
    typedef iTypedSerializable* (*CreateFunc)();
    // Pointer to a function that creates an empty version of each typed serializable object

    virtual int32_t Version() const = 0;
    // Return the current version of that this serializable object writes.
    // As a convention version number should be (major_version << 16) | minor_version.

    static int32_t MkVersion(int16_t major, int16_t minor)
    {
        return (int32_t)(((uint32_t)major << 16) | (uint32_t)minor);
    }
    static int16_t MajorVersion(int32_t version)
    {
        return (int16_t)((uint32_t)version >> 16);
    }
    static int16_t MinorVersion(int32_t version)
    {
        return (int16_t)version;
    }

    virtual ~iTypedSerializable()
    {
    }
};

//////////////////////////////
// convenience macros
#    define DECLARE_TYPEDSERIALIZABLE()      \
        DECLARE_TYPED()                      \
    public:                                  \
        static iTypedSerializable* Create(); \
        virtual int32_t            Version() const;

#    define IMPLEMENT_TYPEDSERIALIZABLE(CLASS, TYPEDSTRING, VERSION_MAJOR, VERSION_MINOR) \
        IMPLEMENT_TYPED(CLASS, TYPEDSTRING)                                               \
        iTypedSerializable* CLASS::Create()                                               \
        {                                                                                 \
            return new CLASS;                                                             \
        }                                                                                 \
        int32_t CLASS::Version() const                                                      \
        {                                                                                 \
            return iTypedSerializable::MkVersion(VERSION_MAJOR, VERSION_MINOR);           \
        }

#endif // __SERIALIZABLE_H
