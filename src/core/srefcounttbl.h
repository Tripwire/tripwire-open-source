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
// srefcounttbl.h
///////////////////////////////////////////////////////////////////////////////

#ifndef __SREFCOUNTTBL_H
#define __SREFCOUNTTBL_H

class iSerRefCountObj;
class iSerializer;

class cSerRefCountTable
{
public:
    cSerRefCountTable();
    virtual ~cSerRefCountTable();

    // find the table id for object.  returns 0 if not in table.
    int Lookup(const iSerRefCountObj* pObj);

    // find object for specified id.  returns NULL if not in table
    iSerRefCountObj* Lookup(int id);

    // Add an object to the table, optionally specifying an ID. Returns a
    // unique ID for the object.  ASSERTs and throws exception if object is
    // already in table or the ID is already taken.
    int Add(iSerRefCountObj* pObj, int id = 0);
    int Add(const iSerRefCountObj* pObj, int id = 0);
    // TODO -- Note that this class is not really const-correct in that the const version of
    // Add() just casts away the constness. The right thing to do is to make the serializer
    // use different versions of this class (const and non-const) depending on whether it is
    // reading or writing and (maybe?) make this class a template class so that it can map
    // to either const or non-const objects.


    // clears out the table
    void Clear();

protected:
    typedef std::map<int, iSerRefCountObj*> MapIDTObj;
    typedef std::map<iSerRefCountObj*, int> MapObjIDT;

    MapIDTObj mIDToObjTbl;
    MapObjIDT mObjToIdTbl;
};

// not used any more, so commented out -- mdb
/*
class cSerRefCountTableMap
{
public:
    std::map<const iSerializer*, cSerRefCountTable*> mMap;

    void AddSerializer(const iSerializer* pSerializer);
                                                // creates a new cSerRefCountTable
    void RemoveSerializer(const iSerializer* pSerializer); // removes table for specified serializer

    ~cSerRefCountTableMap();  // removes all contained cSerRefCountTable's
};
*/
#endif
