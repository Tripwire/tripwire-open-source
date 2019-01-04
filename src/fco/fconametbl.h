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
// fconametbl.h
//
// cFCONameTbl -- a hash table used in the implementation of cFCOName
#ifndef __FCONAMETBL_H
#define __FCONAMETBL_H

#ifndef __DEBUG_H
#include "core/debug.h"
#endif
#ifndef __REFCOUNTOBJ_H
#include "core/refcountobj.h"
#endif
#ifndef __HASHTABLE_H
#include "core/hashtable.h"
#endif

class cFCONameTbl_i;

///////////////////////////////////////////////////////////////////////////////
// cFCONameTblNode -- a node in the cFCONameTbl, this class has the following
//      properties:
//      (1) it is reference counted, so ambiguity over who owns it doesn't exist
//      (2) it keeps track of a case-sensitive character string
//      (3) it keeps a pointer to another cFCONameTblNode that is the lower-case
//          version of the node (if the node is totally lower case, then mpLowerNode
//          is NULL, but GetLowercaseNode() still returns a valid pointer)
//      TODO -- make this serializable
///////////////////////////////////////////////////////////////////////////////
class cFCONameTblNode : public cRefCountObj
{
    friend class cFCONameTbl;

public:
    bool operator==(const cFCONameTblNode& rhs) const;
    bool operator!=(const cFCONameTblNode& rhs) const;
    // for efficency, op== is defined as (this == &rhs)

    const cFCONameTblNode* GetLowercaseNode() const;
    void                   SetLowercaseNode(cFCONameTblNode* pNewNode);
    // gets and sets the lowercase node pointer. Might point to itself if the string
    // this node contains is totally lower case.

    TCHAR* GetString() const;
    // gets the string associated with this node
    void SetString(const TSTRING& newStr);
    // sets the string associated with this node
    // TODO -- this is a very dangerous operation to do; do we ever want the client
    //      to be able to do this (that is, should this method exist)?

    // uncomment to allocate these from a pool
    //  static cObjectPoolBase msAllocator;
    //  void* operator new(size_t)                  { return msAllocator.Alloc(); }
    //  void  operator delete(void* addr)           { msAllocator.Free(addr); }

protected:
    // these are protected to keep the client from creating nodes on the stack
    // or delete()ing them.
    cFCONameTblNode(const TSTRING& name, const cFCONameTblNode* node);
    ~cFCONameTblNode();
    virtual void Delete() const;
    // overridden from cRefCountObj; this is done so that if we are alloced
    // from a memory pool, we can be deleted appropriately.
private:
    cFCONameTblNode(const cFCONameTblNode& rhs);            // not impl
    cFCONameTblNode& operator=(const cFCONameTblNode& rhs); // not impl

    TCHAR*                 mpString;
    const cFCONameTblNode* mpLowerNode;
};

///////////////////////////////////////////////////////////////////////////////
// cFCONameTbl -- keeps a hash table of cFCONameTblNodes. This class does the following:
//      (1) serves as a factory for cFCONameTableNodes, returning objects that have
//          properly filled out mpLowerNode fields
//      (2) Keeps track of all nodes created; two successive calls to create "aBc" will
//          return the same pointer; a call to create "abc" will return a pointer to the
//          Node("aBc")->GetLowercaseNode()
///////////////////////////////////////////////////////////////////////////////
class cFCONameTbl
{
public:
    explicit cFCONameTbl(int defSize = HASH_VERY_LARGE);
    // defSize is the initial hash table size
    ~cFCONameTbl();

    cFCONameTblNode* CreateNode(const TSTRING& nodeName);
    // if the named node already exists in the table, it returns a pointer to the
    // existing node; otherwise, it creates the node and its lower case representation,
    // (if the lowercase node doesn't already exist),adds them to the table
    // and returns a pointer to them. The returned object has been AddRef()ed

    void Clear();
    // clears out all the entries in the table

    ///////////////////////////////////////////////////////////////////////////////
    // cCharCmp -- class used with the hash table that does a string compare
    //      of two TCHAR*s to determine equality
    class cCharCmp
    {
    public:
        bool operator()(const TCHAR* lhs, const TCHAR* rhs)
        {
            return (_tcscmp(lhs, rhs) == 0);
        }
    };

private:
    // TODO -- it is not evident what type the KEY_TYPE should be for the hash tbl; if it is
    // a string, we will be storing two copies of all the strings! If it is a const TCHAR*, then
    // the equality operator won't work (maybe we should template the hash table with the operators
    // needed for the hash as well)
    cHashTable<const TCHAR*, cFCONameTblNode*, cCharCmp> mTable;
};

//#############################################################################
// inline implementation
//#############################################################################

////////////////////////
// cFCONameTblNode
////////////////////////
inline bool cFCONameTblNode::operator==(const cFCONameTblNode& rhs) const
{
    return (this == &rhs);
}
inline bool cFCONameTblNode::operator!=(const cFCONameTblNode& rhs) const
{
    return (this != &rhs);
}
inline const cFCONameTblNode* cFCONameTblNode::GetLowercaseNode() const
{
    // if mpLowerNode == NULL, then we assume that we are the lowercase node.
    if (!mpLowerNode)
        return this;
    return mpLowerNode;
}

inline TCHAR* cFCONameTblNode::GetString() const
{
    return mpString;
}


#endif
