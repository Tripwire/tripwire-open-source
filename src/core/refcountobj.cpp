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
// refcountobj.cpp
#include "stdcore.h"

#include "refcountobj.h"
#include "debug.h"

// TODO:dmb - not sure about commenting this out, do we want to include <typeinfo.h> ???
//#include "typeinfo.h"

#include "errorutil.h"

#ifdef DEBUG
int cRefCountObj::objectCounter    = 0;
int cRefCountObj::referenceCounter = 0;

#include <typeinfo>
// ok, this is getting serious!
#include <set>
typedef std::set<void*> RefSet; // collection of addresses of all reference counted objects
RefSet*                 gpRefCountObj_Objects = 0;

// a way to see what hasn't been accounted for....
struct cRefCountObj_Debug
{
    ~cRefCountObj_Debug()
    {
        RefSet::iterator i;
        cDebug           d("cRefCountObj_Debug");
        if (gpRefCountObj_Objects)
        {
            for (i = gpRefCountObj_Objects->begin(); i != gpRefCountObj_Objects->end(); i++)
            {
                d.TraceNever("Refrence Counted Object %p still exists\n", *i);
            }
        }
    }
} gRefCountObj_Debug;

#endif // DEBUG

cRefCountObj::cRefCountObj()
{
    mRefCount = 1;

    //std::cout << "Allocated RefObj(" << std::hex << (int)this << ")\n";

#ifdef DEBUG
    ++objectCounter;
    ++referenceCounter;

    cDebug d("cRefCountObj::cRefCountObj");
    d.TraceNever("Object Created[%p] %s\n", this, typeid(*this).name());

    if (!gpRefCountObj_Objects)
        gpRefCountObj_Objects = new RefSet;
    gpRefCountObj_Objects->insert(this);
#endif
}

cRefCountObj::~cRefCountObj()
{
    ASSERT(mRefCount == 0);

    //std::cout << "Deleted RefObj(" << std::hex << (int)this << ")\n";

#ifdef DEBUG
    --objectCounter;
    cDebug d("cRefCountObj::~cRefCountObj");
    d.TraceNever("Object Destroyed[%p] %s Objects Left = %d\n", this, typeid(*this).name(), objectCounter);
    if (objectCounter == 0)
    {
        d.TraceDebug("****** All Reference Counted Objects Destroyed! ******\n");
    }

    ASSERT(gpRefCountObj_Objects);
    RefSet::const_iterator i = gpRefCountObj_Objects->find(this);
    ASSERT(i != gpRefCountObj_Objects->end());
    gpRefCountObj_Objects->erase(this);
    if (gpRefCountObj_Objects->size() == 0)
    {
        delete gpRefCountObj_Objects;
        gpRefCountObj_Objects = 0;
    }

#endif
}

void cRefCountObj::AddRef() const
{
    ASSERT(this);

    if (mRefCount <= 0)
    {
        THROW_INTERNAL("refcountobj.cpp");
    }

    ++mRefCount;

#ifdef DEBUG
    ++referenceCounter;
#endif
}

void cRefCountObj::Release() const
{
    if (--mRefCount == 0)
    {
        Delete();
    }

#ifdef DEBUG
    --referenceCounter;
#endif
}

void cRefCountObj::Delete() const
{
    delete this;
}
