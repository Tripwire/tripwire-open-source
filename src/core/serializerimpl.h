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
// serializerimpl.h
//
// cSerializerImpl -- an implementation of the iSerializer interface

#ifndef __SERIALIZERIMPL_H
#define __SERIALIZERIMPL_H


#ifndef __SERIALIZER_H
#include "serializer.h"
#endif

#ifndef __SERIALIZABLE_H
#include "serializable.h"
#endif

#ifndef __SREFCOUNTOBJ_H
#include "srefcountobj.h"
#endif

#ifndef __SREFCOUNTTBL_H
#include "srefcounttbl.h"
#endif

class cArchive;


class cSerializerImpl : public iSerializer
{
public:
    enum Mode
    {
        S_READ,
        S_WRITE
    };
    cSerializerImpl(cArchive& archive, Mode action, const TSTRING& fileName = _T(""));
    // fileName is only used for error reporting purposes
    virtual ~cSerializerImpl();

    bool IsWriting() const;

    // Initializing and closing the archive
    virtual void Init(); // throw eSerializer
        // initializes the serializer; must be called before any reading or writing is done
    virtual void Finit();
    // called after a session of serialization is done; called implicitely by the destructor
    // if not called explicitely before destruction

    //Reading and writing objects Init() should have already been called or all these will fail.
    virtual void WriteObjectDynCreate(const iTypedSerializable* pObj); // throw (eSerializer, eArchive)
        // writes an object such that it can be dynamically created when read back in with
        // ReadObject.
    virtual iTypedSerializable* ReadObjectDynCreate(); // throw (eSerializer, eArchive);
        // reads an object from the archive, returning a pointer to it. The caller is responsible for
        // deleteing or Release()ing it when done.
    virtual void WriteObject(const iTypedSerializable* pObj); // throw (eSerializer, eArchive)
        // writes an object to the archive that will not be dynamically created
    virtual void ReadObject(iTypedSerializable* pObj); // throw (eSerializer, eArchive)
        // reads an object that was written with WriteObject()

    // members for registering classes to be serialized. One of these must be called exactly once
    // for each class that is to be serialized.
    static void RegisterSerializable(const cType& type, iTypedSerializable::CreateFunc pFunc);
    static void RegisterSerializableRefCt(const cType& type, iSerRefCountObj::CreateFunc pFunc);

    // writing interface
    // TODO -- I am not sure if I want to keep these or just have the serializer expose the archive. Actually,
    // I think the best thing might be to have iSerializable only know about the archive
    // Standard data read/write
    // (All functions can throw eArchave exceptions).
    virtual void ReadInt16(int16_t& ret);
    virtual void ReadInt32(int32_t& ret);
    virtual void ReadInt64(int64_t& ret);
    virtual void ReadString(TSTRING& ret);
    virtual int  ReadBlob(void* pBlob, int count);
    virtual void WriteInt16(int16_t i);
    virtual void WriteInt32(int32_t i);
    virtual void WriteInt64(int64_t i);
    virtual void WriteString(const TSTRING& s);
    virtual void WriteBlob(const void* pBlob, int count);

    virtual TSTRING GetFileName() const;

private:
    cArchive*         mpArchive;    // the archive we are serializing to
    Mode              mMode;        // are we writing or reading?
    bool              mbInit;       // has init been called?
    cSerRefCountTable mRefCtObjTbl; // keeps track of all ref counted objects that
                                    // have been read or written during serialization
    TSTRING mFileName;

    // creation function maps
    typedef std::map<uint32_t, iTypedSerializable::CreateFunc> SerMap;
    typedef std::map<uint32_t, iSerRefCountObj::CreateFunc>    SerRefCountMap;
    static SerMap                                            mSerCreateMap;
    static SerRefCountMap                                    mSerRefCountCreateMap;

    static void InitSerializableMaps();
    static void FinitSerializableMaps();
};


#endif
