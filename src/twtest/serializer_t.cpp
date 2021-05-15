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
// serializer_t.cpp -- Test the object that does the serialization work.
///////////////////////////////////////////////////////////////////////////////

//changed 7/6/99 -dra

#include "core/stdcore.h"
#include "core/serializer.h"
#include "core/serializable.h"
#include "test.h"

// The reading and writing functionality of the serializer is tested in
// serializerimpl_t.cpp, so there's very little to be done here.

// Let's make a minimal test object just to make sure the interface is
// working properly...  and then instantiate it.  The actual implementation
// will be tested in serializerimpl_t.cpp
class cSerTestObject : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cSerTestObject();

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0)
    {
    }
    virtual void Write(iSerializer* pSerializer) const
    {
    }

private:
    int mDummy;
};

IMPLEMENT_TYPEDSERIALIZABLE(cSerTestObject, _T("cSerTestObject"), 0, 1)

cSerTestObject::cSerTestObject()
{
    mDummy = 0;
}

void TestSerializer()
{
    cSerTestObject test_obj;

    TEST(std::string(test_obj.GetType().AsString()) == std::string("cSerTestObject"));
    TEST(test_obj.Version() == 1);
}

void RegisterSuite_Serializer()
{
    RegisterTest("Serializer", "Basic", TestSerializer);
}
