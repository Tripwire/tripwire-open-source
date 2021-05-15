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

#include "core/stdcore.h"
#include "core/refcountobj.h"
#include "core/debug.h"
#include "test.h"

class cRefCountTestObj : public cRefCountObj
{
public:
    cRefCountTestObj();

    virtual void Release() const;

    void AddChild(cRefCountTestObj* pChild);

private:
    virtual ~cRefCountTestObj();

    std::list<cRefCountTestObj*> mChildren;
    int8_t*                      mpSomeMem;
};

cRefCountTestObj::cRefCountTestObj()
{
    // allocate some mem that should be caught
    // by the memory manager if this object does not
    // get destructed.
    mpSomeMem = new int8_t[10];
}

cRefCountTestObj::~cRefCountTestObj()
{
    while (!mChildren.empty())
    {
        mChildren.front()->Release();
        mChildren.pop_front();
    }

    delete [] mpSomeMem;
}

void cRefCountTestObj::AddChild(cRefCountTestObj* pChild)
{
    mChildren.push_back(pChild);
}

void cRefCountTestObj::Release() const
{
    cRefCountObj::Release();
}

// --

class cRefCountTestObjList
{
public:
    cRefCountTestObjList();
    ~cRefCountTestObjList();

    void Add(cRefCountTestObj* pNewObj);

    std::list<cRefCountTestObj*> mList;
};

cRefCountTestObjList::cRefCountTestObjList()
{
}

cRefCountTestObjList::~cRefCountTestObjList()
{
    while (!mList.empty())
    {
        mList.front()->Release();
        mList.pop_front();
    }
}

void cRefCountTestObjList::Add(cRefCountTestObj* pNewObj)
{
    mList.push_back(pNewObj);
}


///////////////////////////////////////////////////////////////////////////////

void TestRefCountObj()
{
    cDebug db("TestRefCountObj()");

    db.TraceAlways("Entering...\n");

    // Note the following test of Reference counted objects containing
    // copies of themselves failed.  Perhaps someday we will upgrade the
    // reference counting interface to allow this type of structure.
    /*
    cRefCountTestObj* pObj1 = new cRefCountTestObj;
    cRefCountTestObj* pObj2 = new cRefCountTestObj;
    pObj1->AddChild(pObj2);
    pObj1->AddRef();
    pObj2->AddChild(pObj1);
    pObj1->Release();
    
    ASSERT(cRefCountObj::objectCounter == 0);
    ASSERT(cRefCountObj::referenceCounter == 0);
    */

    int i;
    {
        cRefCountTestObjList list1, list2, list3;

        for (i = 0; i < 3; ++i)
            list1.Add(new cRefCountTestObj);

        std::list<cRefCountTestObj*>::iterator itr;
        for (itr = list1.mList.begin(); itr != list1.mList.end(); ++itr)
        {
            (*itr)->AddRef();
            list2.Add(*itr);
        }

        for (itr = list2.mList.begin(); itr != list2.mList.end(); ++itr)
        {
            (*itr)->AddRef();
            list3.Add(*itr);
            if (++itr == list2.mList.end())
                break;
        }
    }

#ifdef DEBUG
    //These fields only exist in debug builds
    TEST(cRefCountObj::objectCounter == 0);
    TEST(cRefCountObj::referenceCounter == 0);
#else
    TEST("This test can only make useful assertions in debug builds");
#endif

    db.TraceAlways("Done...\n");

    return;
}

void RegisterSuite_RefCountObj()
{
    RegisterTest("RefCountObj", "Basic", TestRefCountObj);
}
