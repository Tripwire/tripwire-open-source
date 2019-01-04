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
// serializer.h -- The object that does the serialization work.
///////////////////////////////////////////////////////////////////////////////

#ifndef __SERIALIZER_H
#define __SERIALIZER_H

#ifndef __ERROR_H
#include "error.h"
#endif

class iTypedSerializable;
class iSerializable;

////////////////////////////////////////////////////////////
// Serializer Base Exception
////////////////////////////////////////////////////////////
TSS_BEGIN_EXCEPTION_NO_CTOR(eSerializer, eError)
// TODO: What else to add to this enumeration? Locked File? Temp file?
enum DataSourceType
{
    TY_UNDEFINED = 0,
    TY_FILE,
    TY_TEMPFILE,
    TY_MEMORY,
    TY_PIPE,
    TY_SOCKET
};

eSerializer(const TSTRING& msg, const TSTRING& dataSource = _T(""), DataSourceType paramType = TY_UNDEFINED)
    : eError(msg), mDataSource(dataSource), mSourceType(paramType)
{
}

virtual TSTRING GetMsg() const;

private:
TSTRING mDataSource;
// TSTRING indentifier of the datasource associated with a particular error
// (if one exists) EX: a filename.
DataSourceType mSourceType;
TSS_END_EXCEPTION();

////////////////////////////////////////////////////////////
// Helper Macro For Serializer Exceptions
////////////////////////////////////////////////////////////
#define TSS_SERIALIZER_EXCEPTION(except)                                                                    \
    TSS_BEGIN_EXCEPTION_NO_CTOR(except, eSerializer)                                                        \
    except(const TSTRING& msg, const TSTRING& dataSource = _T(""), DataSourceType paramType = TY_UNDEFINED) \
        : eSerializer(msg, dataSource, paramType){};                                                        \
    TSS_END_EXCEPTION()

////////////////////////////////////////////////////////////
// Specific Exceptions
////////////////////////////////////////////////////////////
TSS_SERIALIZER_EXCEPTION(eSerializerUnknownType);
TSS_SERIALIZER_EXCEPTION(eSerializerInputStreamFmt);
TSS_SERIALIZER_EXCEPTION(eSerializerOutputStreamFmt);
TSS_SERIALIZER_EXCEPTION(eSerializerInputStremTypeArray);
TSS_SERIALIZER_EXCEPTION(eSerializerArchive); // not used
TSS_SERIALIZER_EXCEPTION(eSerializerVersionMismatch);
TSS_SERIALIZER_EXCEPTION(eSerializerEncryption);

/*
        E_UNKNOWN_TYPE          = 700,
        E_INPUT_STREAM_FORMAT   = 701,
        E_OUTPUT_STREAM_FORMAT  = 706,
        E_INPUT_STR_TYPEARRAY   = 702,  // bad index in to type array
        E_ARCHIVE               = 703,
        E_VERSION_MISMATCH      = 704,
        E_ENCRYPTION_ERROR      = 705,
*/

class iSerializer
{
public:
    // Initializing and closing the archive
    virtual void Init() = 0; // throw eSerializer
        // initializes the serializer; must be called before any reading or writing is done
    virtual void Finit() = 0;
    // called after a session of serialization is done; called implicitely by the destructor
    // if not called explicitely before destruction

    //Reading and writing objects Init() should have already been called or all these will fail.
    virtual void WriteObjectDynCreate(const iTypedSerializable* pObj) = 0; // throw (eSerializer, eArchive)
        // writes an object such that it can be dynamically created when read back in with
        // ReadObject.
    virtual iTypedSerializable* ReadObjectDynCreate() = 0; // throw (eSerializer, eArchive);
        // reads an object from the archive, returning a pointer to it. The caller is responsible for
        // deleteing or Release()ing it when done.
    virtual void WriteObject(const iTypedSerializable* pObj) = 0; // throw (eSerializer, eArchive)
        // writes an object to the archive that will not be dynamically created
    virtual void ReadObject(iTypedSerializable* pObj) = 0; // throw (eSerializer, eArchive)
        // reads an object that was written with WriteObject()


    // writing interface
    // all of these can throw eArchive
    virtual void ReadInt16(int16_t& ret)                   = 0;
    virtual void ReadInt32(int32_t& ret)                   = 0;
    virtual void ReadInt64(int64_t& ret)                   = 0;
    virtual void ReadString(TSTRING& ret)                = 0;
    virtual int  ReadBlob(void* pBlob, int count)        = 0;
    virtual void WriteInt16(int16_t i)                     = 0;
    virtual void WriteInt32(int32_t i)                     = 0;
    virtual void WriteInt64(int64_t i)                     = 0;
    virtual void WriteString(const TSTRING& s)           = 0;
    virtual void WriteBlob(const void* pBlob, int count) = 0;

    virtual TSTRING GetFileName() const
    {
        return _T("");
    }
    // derived classes can implement this to return the file name associated with the serializer.
    // it is only used in error reporting.

    // the error enumeration: 700-799
    enum ErrorNum
    {
        E_UNKNOWN_TYPE         = 700,
        E_INPUT_STREAM_FORMAT  = 701,
        E_INPUT_STR_TYPEARRAY  = 702, // bad index in to type array
        E_ARCHIVE              = 703,
        E_VERSION_MISMATCH     = 704,
        E_ENCRYPTION_ERROR     = 705,
        E_OUTPUT_STREAM_FORMAT = 706,
        E_NUMITEMS
    };

    virtual ~iSerializer()
    {
    }
};


#endif
