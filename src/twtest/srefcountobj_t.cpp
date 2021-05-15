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
// srefcountobj_t.cpp
///////////////////////////////////////////////////////////////////////////////

#include "core/stdcore.h"
#include "core/serializerimpl.h"

#include "core/errorgeneral.h"
#include "core/debug.h"
#include "core/archive.h"
#include "core/srefcountobj.h"
#include "twtest/test.h"

class cSerRefCountObjTest : public iSerRefCountObj
{
    DECLARE_SERREFCOUNT()

public:
    cSerRefCountObjTest();

protected:
    ~cSerRefCountObjTest();

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

private:
    int mDummyData;
};

IMPLEMENT_SERREFCOUNT(cSerRefCountObjTest, _T("cSerRefCountObjTest"), 0, 1);

cSerRefCountObjTest::cSerRefCountObjTest()
{
    static int counter;
    mDummyData = ++counter;
}

cSerRefCountObjTest::~cSerRefCountObjTest()
{
}


void cSerRefCountObjTest::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("cSerRefCountObjTest::Read()")));

    int32_t dummy;
    pSerializer->ReadInt32(dummy);
    mDummyData = dummy;
}

void cSerRefCountObjTest::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(mDummyData);
}


////////////////////////////////////////////////////////////////////////////////

void TestSerRefCountObj()
{
    // first, we need to register the object with the serializer class...
    cSerializerImpl::RegisterSerializableRefCt(CLASS_TYPE(cSerRefCountObjTest), cSerRefCountObjTest::Create);

    cSerRefCountObjTest* pObj1 = new cSerRefCountObjTest;
    cSerRefCountObjTest* pObj2 = new cSerRefCountObjTest;
    pObj1->AddRef();
    cSerRefCountObjTest* pObj3 = pObj1;
    pObj1->AddRef();
    cSerRefCountObjTest* pObj4 = pObj1;

    cMemoryArchive memarchive;
    {
        cSerializerImpl serializer(memarchive, cSerializerImpl::S_WRITE);
        serializer.Init();
        serializer.WriteObjectDynCreate(pObj1);
        serializer.WriteObjectDynCreate(pObj2);
        serializer.WriteObjectDynCreate(pObj3);
        serializer.WriteObjectDynCreate(pObj4);
        serializer.Finit();
    }

    pObj1->Release();
    pObj2->Release();
    pObj3->Release();
    pObj4->Release();

    {
        memarchive.Seek(0, cBidirArchive::BEGINNING);
        cSerializerImpl serializer(memarchive, cSerializerImpl::S_READ);
        serializer.Init();
        pObj1 = static_cast<cSerRefCountObjTest*>(serializer.ReadObjectDynCreate());
        pObj2 = static_cast<cSerRefCountObjTest*>(serializer.ReadObjectDynCreate());
        pObj3 = static_cast<cSerRefCountObjTest*>(serializer.ReadObjectDynCreate());
        pObj4 = static_cast<cSerRefCountObjTest*>(serializer.ReadObjectDynCreate());
        serializer.Finit();
    }

    TEST(pObj1 == pObj3);
    TEST(pObj1 == pObj4);
    TEST(pObj1 != pObj2);

    pObj1->Release();
    pObj2->Release();
    pObj3->Release();
    pObj4->Release();

    return;
}

void RegisterSuite_SerRefCountObj()
{
    RegisterTest("SerRefCountObj", "Basic", TestSerRefCountObj);
}
