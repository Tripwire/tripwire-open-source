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
// serializerimpl_t.cpp

#include "core/stdcore.h"
#include "core/serializerimpl.h"
#include "core/serializable.h"
#include "core/types.h"
#include "core/archive.h"
#include "twtest/test.h"

class cSerializerTestObject : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cSerializerTestObject();

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    bool CheckValid();

private:
    int     mNumber;
    uint8_t mData[20];
    TSTRING mString;
};

IMPLEMENT_TYPEDSERIALIZABLE(cSerializerTestObject, _T("cSerializerTestObject"), 0, 1)

cSerializerTestObject::cSerializerTestObject()
{
    mNumber = 42;
    memset(mData, 69, 20);
    mString = _T("When the cows come home.");
}

bool cSerializerTestObject::CheckValid()
{
    if (mNumber != 42)
        return false;

    int i;
    for (i = 0; i < 20; i++)
        if (mData[i] != 69)
            return false;

    if (mString.compare(_T("When the cows come home.")) != 0)
        return false;

    return true;
}

void cSerializerTestObject::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("cSerializerTestObject::Read()")));

    // clear the object to test reading propperly
    mNumber = 0;
    memset(mData, 0, 20);
    mString.erase();
    int32_t number;
    pSerializer->ReadInt32(number);
    pSerializer->ReadBlob(mData, 20);
    pSerializer->ReadString(mString);
    mNumber = number;
}

void cSerializerTestObject::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(mNumber);
    pSerializer->WriteBlob(mData, 20);
    pSerializer->WriteString(mString);
}

void TestSerializerImpl()
{
    cDebug db("TestSerializerImpl()");

    db.TraceAlways("Entering...\n");

    // Got to the regisiter the test class
    cSerializerImpl::RegisterSerializable(CLASS_TYPE(cSerializerTestObject), cSerializerTestObject::Create);

    // writing
    {
        cFileArchive file;
        file.OpenReadWrite(TwTestPath("tmp.bin").c_str());
        cSerializerImpl serializer(file, cSerializerImpl::S_WRITE);

        serializer.Init();

        cSerializerTestObject testobj;
        testobj.Write(&serializer);

        db.TraceAlways("    Writing object 1...\n");
        serializer.WriteObject(&testobj);

        db.TraceAlways("    Writing object 2...\n");
        serializer.WriteObject(&testobj);

        db.TraceAlways("    Writing object 3...\n");
        serializer.WriteObject(&testobj);

        db.TraceAlways("    Writing object 4...\n");
        serializer.WriteObject(&testobj);

        serializer.Finit();
    }

    // reading
    {
        cFileArchive file;
        file.OpenRead(TwTestPath("tmp.bin").c_str());
        cSerializerImpl serializer(file, cSerializerImpl::S_READ);

        serializer.Init();

        cSerializerTestObject testobj;
        testobj.Read(&serializer);
        TEST(testobj.CheckValid());

        db.TraceAlways("    Reading and verifying object 1...\n");
        serializer.ReadObject(&testobj);
        TEST(testobj.CheckValid());

        db.TraceAlways("    Reading and verifying object 2...\n");
        serializer.ReadObject(&testobj);
        TEST(testobj.CheckValid());

        db.TraceAlways("    Reading and verifying object 3...\n");
        serializer.ReadObject(&testobj);
        TEST(testobj.CheckValid());

        db.TraceAlways("    Reading and verifying object 4...\n");
        serializer.ReadObject(&testobj);
        TEST(testobj.CheckValid());

        serializer.Finit();
    }

    db.TraceAlways("Done...\n");

    return;
}

void RegisterSuite_SerializerImpl()
{
    RegisterTest("SerializerImpl", "Basic", TestSerializerImpl);
}
