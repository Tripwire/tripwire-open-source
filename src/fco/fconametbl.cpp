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
// fconametbl.cpp
//

#include "stdfco.h"
#include "fconametbl.h"

//#############################################################################
// cFCONameTblNode
//#############################################################################

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//      grow heap for allocating strings
//      uncomment this stuff to use the grow heap...
//

#include "core/growheap.h"

// TODO:BAM -- maybe make a PAGE_SIZE in platform.h
enum
{
    FCONAME_HEAP_INITIAL_SIZE = 0x1000, // 4k
    FCONAME_HEAP_GROW_BY      = 0x1000
};

static cGrowHeap gFCONameHeap(FCONAME_HEAP_INITIAL_SIZE, FCONAME_HEAP_GROW_BY, _T("FCO Name"));

inline void* util_AllocMem(int size)
{
    return gFCONameHeap.Malloc(size);
}
inline void util_FreeMem(void* pData)
{
}

//
// ...and uncomment this to do allocing the normal way...
//
/*
inline void* util_AllocMem(int size)
{
    return malloc( size );
}
inline void util_FreeMem(void* pData)
{
    free(pData);
}
*/

//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


void cFCONameTblNode::SetLowercaseNode(cFCONameTblNode* pNewNode)
{
    ASSERT(pNewNode != 0); // doesn't make sense to set this to NULL

    if (mpLowerNode)
        mpLowerNode->Release();
    if (pNewNode == this)
    {
        // we can't store a pointer to ourselves, since reference counting doesn't go well
        // with circular pointers, so we will represent this with a NULL pointer
        mpLowerNode = NULL;
        return;
    }
    pNewNode->AddRef();
    mpLowerNode = pNewNode;
}

void cFCONameTblNode::SetString(const TSTRING& newStr)
{
    if (mpString != NULL)
    {
        delete [] mpString;
        mpString = NULL;
    }

    size_t alloc_size = sizeof(TCHAR) * (newStr.length() + 1);
    mpString          = (TCHAR*)util_AllocMem(alloc_size);
    strncpy(mpString, newStr.c_str(), alloc_size);

    // NOTE -- the lower case pointer is now invalid.
}

cFCONameTblNode::cFCONameTblNode(const TSTRING& name, const cFCONameTblNode* node) : mpString(NULL), mpLowerNode(node)
{
    SetString(name);
}

cFCONameTblNode::~cFCONameTblNode()
{
    if (mpLowerNode)
        mpLowerNode->Release();
    if (mpString != NULL)
    {
        util_FreeMem(mpString);
        mpString = NULL;
    }
}

void cFCONameTblNode::Delete() const
{
    ((cFCONameTblNode*)this)->~cFCONameTblNode();
    util_FreeMem((void*)this);
}


//#############################################################################
// cFCONameTbl
//#############################################################################

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cFCONameTbl::cFCONameTbl(int defSize) : mTable(defSize)
{
}

cFCONameTbl::~cFCONameTbl()
{
#ifdef DEBUG
    cDebug d("cFCONameTbl::~cFCONameTbl()");
    d.TraceDebug("Tracing cFCONameTblNode hash table statistics:\n");
    mTable.TraceDiagnostics();
#endif
    Clear();
}

///////////////////////////////////////////////////////////////////////////////
// CreateNode
///////////////////////////////////////////////////////////////////////////////

cFCONameTblNode* cFCONameTbl::CreateNode(const TSTRING& nodeName)
{
    cFCONameTblNode *pNode, *pLowerNode;
    if (mTable.Lookup(nodeName.c_str(), pNode))
    {
        // this already exists in the table; just return the existing node
        pNode->AddRef();
        return pNode;
    }
    pNode = new (util_AllocMem(sizeof(cFCONameTblNode))) cFCONameTblNode(nodeName, NULL);
    mTable.Insert(pNode->GetString(), pNode);

    // fill out the lower-case info
    TSTRING           lowStr = pNode->GetString();
    TSTRING::iterator i;

    // TODO:BAM -- does this have any meaning in mb?
    for (i = lowStr.begin(); i != lowStr.end(); ++i)
    {
        *i = tolower(*i);
    }
    // see if this exists in the table (it could potentially look up itself!)
    if (mTable.Lookup(lowStr.c_str(), pLowerNode))
    {
        pNode->SetLowercaseNode(pLowerNode);
    }
    else
    {
        // we know that the original string is not lower case if we got to this point
        pLowerNode = new (util_AllocMem(sizeof(cFCONameTblNode))) cFCONameTblNode(lowStr, NULL);
        pLowerNode->SetLowercaseNode(pLowerNode);
        pNode->SetLowercaseNode(pLowerNode);
        mTable.Insert(pLowerNode->GetString(), pLowerNode);
    }

    pNode->AddRef();
    return pNode;
}

///////////////////////////////////////////////////////////////////////////////
// Clear
///////////////////////////////////////////////////////////////////////////////
void cFCONameTbl::Clear()
{
    cHashTableIter<const TCHAR*, cFCONameTblNode*, cCharCmp> iter(mTable);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        cFCONameTblNode* p = iter.Val();
        if (p)
            p->Release();
    }

    mTable.Clear();
}
