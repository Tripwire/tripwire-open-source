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
// srefcounttbl.cpp
///////////////////////////////////////////////////////////////////////////////
#include "stdcore.h"
#include "srefcounttbl.h"
#include "debug.h"
#include "errorgeneral.h"
#include "error.h"
#include "errorutil.h"


///////////////////////////////////////////////////////////////////////////////
// class cSerRefCountTable
///////////////////////////////////////////////////////////////////////////////

cSerRefCountTable::cSerRefCountTable()
{
}

cSerRefCountTable::~cSerRefCountTable()
{
}

void cSerRefCountTable::Clear()
{
    mIDToObjTbl.clear();
    mObjToIdTbl.clear();
}

// find the table id for object.  returns 0 if not in table.
int cSerRefCountTable::Lookup(const iSerRefCountObj* pObj)
{
    std::map<iSerRefCountObj*, int>::iterator itr;

    // pay no attention to this cast :-)
    itr = mObjToIdTbl.find(const_cast<iSerRefCountObj*>(pObj));

    return (itr == mObjToIdTbl.end()) ? 0 : itr->second;
}

// find object for specified id.  returns NULL if not in table
iSerRefCountObj* cSerRefCountTable::Lookup(int id)
{
    std::map<int, iSerRefCountObj*>::iterator itr;

    itr = mIDToObjTbl.find(id);

    return itr == mIDToObjTbl.end() ? NULL : itr->second;
}

// Add an object to the table, optionally specifying an ID. Returns a
// unique ID for the object.  ASSERTs and throws exception if object is
// already in table or the ID is already taken.
int cSerRefCountTable::Add(iSerRefCountObj* pObj, int id)
{
    if (Lookup(pObj) != 0)
    {
        // this should be a programming error, but we will throw just to be safe...
        ThrowAndAssert(eInternal(_T("cSerRefCountTable::Add() passed object already in table.")));
    }
    if (id == 0)
    {
        id = mIDToObjTbl.empty() ? 1 : mIDToObjTbl.rbegin()->first + 1;
        ASSERT(Lookup(id) == NULL);
    }
    else if (Lookup(id) != NULL)
    {
        // this should be a programming error, but we will throw just to be safe...
        ThrowAndAssert(eInternal(_T("cSerRefCountTable::Add() passed ID already in table.")));
    }

    mIDToObjTbl.insert(MapIDTObj::value_type(id, pObj));
    mObjToIdTbl.insert(MapObjIDT::value_type(pObj, id));

    return id;
}

int cSerRefCountTable::Add(const iSerRefCountObj* pObj, int id)
{
    iSerRefCountObj* pNonConst = const_cast<iSerRefCountObj*>(pObj);
    return Add(pNonConst, id);
}

///////////////////////////////////////////////////////////////////////////////
// class cSerRefCountTableMap
///////////////////////////////////////////////////////////////////////////////
/*
// creates a new cSerRefCountTable
void cSerRefCountTableMap::AddSerializer(const cSerializer* pSerializer)
{
    #ifdef DEBUG
    // make sure we don't have this serialzier in here yet
    {
    std::map<const cSerializer*, cSerRefCountTable*>::iterator itr;
    itr = mMap.find(pSerializer);
    ASSERT(itr == mMap.end());
    }
    #endif

    std::pair<const cSerializer* const, cSerRefCountTable*> new_value(pSerializer, new cSerRefCountTable);

    mMap.insert(mMap.end(), new_value);
}

// removes table for specified serializer
void cSerRefCountTableMap::RemoveSerializer(const cSerializer* pSerializer)
{
    std::map<const cSerializer*, cSerRefCountTable*>::iterator itr;
    itr = mMap.find(pSerializer);
    ASSERT(itr != mMap.end());

    delete itr->second;
    mMap.erase(itr);
}

// removes all contained cSerRefCountTable's
cSerRefCountTableMap::~cSerRefCountTableMap()
{
    while (!mMap.empty())
    {
        delete mMap.begin()->second;
        mMap.erase(mMap.begin());
    }
}
*/
