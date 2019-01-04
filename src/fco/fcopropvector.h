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
// fcopropvector.h
//
// cFCOPropVector -- a vector of integers that, whan associated with a property
//      set, represent a set of properties.
///////////////////////////////////////////////////////////////////////////////
#ifndef __FCOPROPVECTOR_H
#define __FCOPROPVECTOR_H

#ifndef __DEBUG_H
#include "core/debug.h"
#endif

#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif

class cFCOPropVector : public iSerializable // note: this is not iTypedSerializable
{
public:
    explicit cFCOPropVector(int size = 32);
    cFCOPropVector(const cFCOPropVector& rhs);
    virtual ~cFCOPropVector(void);
    bool            operator==(const cFCOPropVector& rhs) const;
    bool            operator!=(const cFCOPropVector& rhs) const;
    cFCOPropVector& operator=(const cFCOPropVector& rhs);
    cFCOPropVector  operator&(const cFCOPropVector& rhs) const;
    cFCOPropVector  operator^(const cFCOPropVector& rhs) const;
    cFCOPropVector  operator|(const cFCOPropVector& rhs) const;
    cFCOPropVector& operator&=(const cFCOPropVector& rhs);
    cFCOPropVector& operator|=(const cFCOPropVector& rhs);
    cFCOPropVector& operator^=(const cFCOPropVector& rhs);

    int GetSize(void) const;
    // returns the number of items that can be stored in the vector.
    // This will be some multiple of 32. (item 0 exists). The higest number this
    // vector can store is GetSize()-1
    int SetSize(int max);
    // sets the maximum item that can be stored in the vector. This function
    // is only useful if one wants to extend the maximum size of the bitset.
    // It will NOT truncate a bitset, i.e., once the vector mpBuf is allocated
    // it will not be deallocated.  The return value is one more than the highest
    // value that can be stored in the vector (0 can be stored)
    bool AddItem(int i);
    // Adds the item i to the vector. Behavior is undefined if i > GetSize()
    bool AddItemAndGrow(int i);
    // Adds the item i to the vector. if (i > GetSize()), grows the vector
    bool RemoveItem(int i);
    // Removes an item from the set if it is in the set.  Returns true if the
    // item is not in the set prior to call.
    bool ContainsItem(int i) const;
    // returns true if the item is in the set. Returns false if i>GetSize.
    void Clear(void);
    // removes all items from the vector...
    void check(cDebug& d) const;
    // Temp function for testing purposes.  Outputs vector info. TO DO:
    // Get rid of this when it's no longer useful! DA
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
        // iSerializable interface
    void TraceContents(int dl = -1) const;
    // debugging utility; traces out the contents of the vector

private:
    bool isExtended(void) const;
    // Helper function that discerns if an object is using mpBuf beyond [0]

    int                    mSize;
    static int             msBitlength;
    uint32_t               mMask;
    std::vector<uint32_t>* mpBuf;
};

#endif //__FCOPROPVECTOR_H
