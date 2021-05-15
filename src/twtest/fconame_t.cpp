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
// fconame_t.cpp
///////////////////////////////////////////////////////////////////////////////

#include "fco/stdfco.h"

#include "fco/fconame.h"

#include "twtest/test.h"
#include "core/serializer.h"
#include "core/serializerimpl.h"
#include "core/archive.h"
#include "fco/genreswitcher.h"

void TestFCOName1()
{
    // test the relationship operator...
    cFCOName above(_T("/etc"));
    cFCOName extraDel(_T("/etc/"));
    cFCOName below(_T("/etc/passwd"));
    cFCOName same(_T("/etc/passwd"));
    cFCOName unrel(_T("/var/spool/mail"));
    cFCOName caseTest(_T("/etc/PASsWD"));
    cFCOName caseTest2(_T("/etc/passwd/dog"));
    cFCOName test(_T("/etcc"));

    TEST(above.GetRelationship(below) == cFCOName::REL_ABOVE);
    TEST(below.GetRelationship(above) == cFCOName::REL_BELOW);
    TEST(below.GetRelationship(same) == cFCOName::REL_EQUAL);
    TEST(unrel.GetRelationship(above) == cFCOName::REL_UNRELATED);
    TEST(test.GetRelationship(above) == cFCOName::REL_UNRELATED);
    TEST(caseTest.GetRelationship(same) == cFCOName::REL_UNRELATED);
    // test the extra delimiter removal...
    caseTest.SetCaseSensitive(false);
    caseTest2.SetCaseSensitive(false);
    same.SetCaseSensitive(false);
    above.SetCaseSensitive(false);
    TEST(caseTest.GetRelationship(caseTest2) == cFCOName::REL_ABOVE);
    TEST(caseTest.GetRelationship(same) == cFCOName::REL_EQUAL);

    // test push() and pop()
    cFCOName dog(_T("/a/brown/dog"));
    cFCOName cat(_T("/a/brown/cat"));
    TEST(dog.GetSize() == 4);
    dog.Push(_T("woof"));
    TEST(dog.AsString().compare(_T("/a/brown/dog/woof")) == 0);
    TEST(dog.GetSize() == 5);
    TEST(_tcscmp(dog.Pop(), _T("woof")) == 0);
    TEST(_tcscmp(dog.Pop(), _T("dog")) == 0);
    TEST(cat.GetRelationship(dog) == cFCOName::REL_BELOW);
    TEST(_tcscmp(cat.Pop(), _T("cat")) == 0);
    TEST(dog.AsString().compare(cat.AsString()) == 0);
    TEST(dog.GetRelationship(cat) == cFCOName::REL_EQUAL);
}

void TestFCOName2()
{
    cFCOName nullName;
    TEST(*nullName.AsString().c_str() == 0);

    cFCOName charName(_T("/a/character/name"));
    TEST(_tcscmp(charName.AsString().c_str(), _T("/a/character/name")) == 0);

    cFCOName stringName(_T("/a/string/name"));
    TEST(_tcscmp(stringName.AsString().c_str(), _T("/a/string/name")) == 0);

    cFCOName dosName(_T("c:/a/dos/name"));
    TEST(_tcscmp(dosName.AsString().c_str(), _T("c:/a/dos/name")) == 0);

    cFCOName copyName(stringName);
    TEST(_tcscmp(copyName.AsString().c_str(), _T("/a/string/name")) == 0);

    cFCOName name(_T("/new name"));
    nullName = name;
    TEST(_tcscmp(nullName.AsString().c_str(), _T("/new name")) == 0);

    nullName = _T("/newer name");
    TEST(_tcscmp(nullName.AsString().c_str(), _T("/newer name")) == 0);

    cMemoryArchive memArc;

    {
        cSerializerImpl ser(memArc, cSerializerImpl::S_WRITE);
        ser.Init();
        ser.WriteObject(&charName);

        //Note: backslash delimiters aren't supported (& don't work) in OST
        //stringName.SetDelimiter(_T('\\'));

        ser.WriteObject(&stringName);
        ser.Finit();
    }

    memArc.Seek(0, cBidirArchive::BEGINNING);

    {
        cSerializerImpl ser(memArc, cSerializerImpl::S_READ);
        cFCOName        name1, name2;

        ser.Init();
        ser.ReadObject(&name1);
        ser.ReadObject(&name2);
        ser.Finit();

        TEST(name1.IsEqual(charName));
        TEST(name2.IsEqual(stringName));
        TEST(name1.IsCaseSensitive());
        TEST(name2.IsCaseSensitive());
        TEST(name2.GetDelimiter() == _T('/'));
        TEST(name1.GetDelimiter() == _T('/'));
    }
}

void RegisterSuite_FCOName()
{
    RegisterTest("FCOName", "Basic1", TestFCOName1);
    RegisterTest("FCOName", "Basic2", TestFCOName2);
}
