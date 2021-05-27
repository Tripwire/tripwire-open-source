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
// fcospeclist.h
//
// cFCOSpecList -- a list of iFCOSpec's with in a well defined order.

#ifndef __FCOSPECLIST_H
#define __FCOSPECLIST_H

#ifndef __FCOSPEC_H
#include "fcospec.h"
#endif
#ifndef __SERIALIZABLE_H
#include "serializable.h"
#endif
#ifndef __FCOSPECATTR_H
#include "fcospecattr.h"
#endif


///////////////////////////////////////////////////////////////////////////////
// class cFCOSpecList -- A list of iFCOSpec's with an optimized lookup method
//      and preservation of order of addition of items.  The optimized lookup
//      is useful because we will want to look up specs quickly.  Preserving
//      the order in which items were added is useful because we will use
//      cFCOSpecList's in the report and database were we wish to use the
//      added order of the list to match to other lists.
///////////////////////////////////////////////////////////////////////////////

class cFCOSpecList : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()

    friend class cFCOSpecListCanonicalIter;
    friend class cFCOSpecListAddedIter;

public:
    cFCOSpecList();
    cFCOSpecList(const cFCOSpecList& rhs);
    virtual ~cFCOSpecList();

    const cFCOSpecList& operator=(const cFCOSpecList& rhs);

    void Clear();
    // Clear the list of specs, deleting them all

    bool IsEmpty() const;
    int  Size() const;

    void Add(iFCOSpec* pSpec, cFCOSpecAttr* pAttr = 0);
    // Add iFCOSpec to list.  The objects will be AddRef()ed by the list
    // and Release()d on destruction. If pAttr == NULL, then an empty
    // attribute list will be created and associated with the spec.

    iFCOSpec* Lookup(iFCOSpec* pSpec) const;
    // Search the list a spec that matches pSpec.  This search is fairly
    // low cost if pSpec points to an actual spec in this list.  Otherwise
    // string compares must be done to determine the result.  Returns
    // NULL if it is NOT in the list, otherwise a pointer to the object in
    // this list that matches pSpec. Either way, if non-null is returned, the
    // object was AddRef()ed

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

protected:
    typedef std::pair<iFCOSpec*, cFCOSpecAttr*> PairType;

    mutable std::list<PairType> mAddedList;     // list in added order
    mutable std::list<PairType> mCanonicalList; // list in well defined order
};

class cFCOSpecListAddedIter
{
public:
    explicit cFCOSpecListAddedIter(const cFCOSpecList& list);
    ~cFCOSpecListAddedIter();

    void SeekBegin() const;
    bool Done() const;
    bool IsEmpty() const;
    void Next() const;
    void Remove();
    // removes the node the iterator is currently pointing at.
    // asserts !Done() and moves the iterator to the next node,
    // or to the end if it is the last node.

    const iFCOSpec*     Spec() const;
    iFCOSpec*           Spec();
    const cFCOSpecAttr* Attr() const;
    cFCOSpecAttr*       Attr();

protected:
    cFCOSpecList* mpSpecList;

    mutable std::list<cFCOSpecList::PairType>::iterator mIter;
};

class cFCOSpecListCanonicalIter
{
public:
    explicit cFCOSpecListCanonicalIter(const cFCOSpecList& list);
    ~cFCOSpecListCanonicalIter();

    void SeekBegin() const;
    bool Done() const;
    bool IsEmpty() const;
    void Next() const;
    //void              Remove();
    // TODO --implement this the same as Remove() in the
    // added iter.
    const iFCOSpec*     Spec() const;
    iFCOSpec*           Spec();
    const cFCOSpecAttr* Attr() const;
    cFCOSpecAttr*       Attr();

protected:
    cFCOSpecList* mpSpecList;

    mutable std::list<cFCOSpecList::PairType>::iterator mIter;
};

inline cFCOSpecList::cFCOSpecList(const cFCOSpecList& rhs) : iTypedSerializable()
{
    *this = rhs;
}

#endif //__FCOSPEC_H
