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
// hashtable.h  : a template class for mapping tuples using TCHAR*'s
//
// implements cHashTable, which maps a key of arbitrary type to a value
// of arbitrary type.  The key data type MUST have the const uint8_t*()
// operator overloaded in order for this to work.  TSTRINGS will always
// work as the key value because of the overloaded-template-function
//
//      Note: Any overloaded const uint8_t*() operator must return an
// length of key as well.  see cDefaultConvert
//
// IMPORTANT -- cDefaultConvert only works for pointers to objects
//           -- cDefaultCompare only (mostly) works for objects

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __DEBUG_H
#include "debug.h"
#endif

#ifndef __ERROR_H
#include "error.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// Comparison function objects ... these are used by the hash table to determine
//      equality. The one defined should work for objects that use op== to define
//      equality. There is also a specialization for TSTRINGS. If neither of these
//      fit your needs, you must pass the hash table your own fn pointer or class
///////////////////////////////////////////////////////////////////////////////
template<class T> class cDefaultCompare
{
public:
    bool operator()(const T& lhs, const T& rhs)
    {
        return lhs == rhs;
    }
};
/////////////////////////////////////////////////////////
// specialization for TSTRINGS
/////////////////////////////////////////////////////////
template<> inline bool cDefaultCompare<TSTRING>::operator()(const TSTRING& lhs, const TSTRING& rhs)
{
    return (lhs.compare(rhs) == 0);
}

///////////////////////////////////////////////////////////////////////////////
// Conversion function objects ... used by the hash table to locate the key in KEY_TYPE
//      into a uint8_t* and a key length (for hashing purposes). The default implementation
//      just does a cast. A specialization is also provided for TSTRINGs.
///////////////////////////////////////////////////////////////////////////////
template<class T> class cDefaultConvert
{
public:
    const uint8_t* operator()(const T& obj, int* const pcbKeyLen)
    {
        // HACK!  TODO: in the interest of time, I've left this as it is.....
        *pcbKeyLen = sizeof(TCHAR) * _tcslen(obj);
        return (uint8_t*)obj;
    }
};
/////////////////////////////////////////////////////////
// specialization for TSTRINGS
/////////////////////////////////////////////////////////
template<> inline const uint8_t* cDefaultConvert<TSTRING>::operator()(const TSTRING& obj, int* const pcbKeyLen)
{
    *pcbKeyLen = sizeof(TCHAR) * obj.length();
    return (uint8_t*)obj.c_str();
}

///////////////////////////////////////////////////////////////////////////////
// cHashTable<KEY, VAL, CMP, CONVERTER>
//      KEY -- the key you are hashing on
//      VAL -- the value you want associated with that key
//      CMP -- a function object that takes (KEY, KEY) and returns true if they
//              are equal.
//      CONVERTER -- function object that takes (KEY, int* pcbKeyLen) and returns a const uint8_t*
//              ( points to start of key ) and a uint8_t length (in pcbKeyLen) that tells the hashtable
//              how long the key is
///////////////////////////////////////////////////////////////////////////////
// these were moved outside of the class because it sucks to have to name the class with template parameters
// ie -- mTable(cHashTable<TSTRING, int>::MEDIUM
enum cHashTable_TableSize
{
    HASH_VERY_SMALL = 17,
    HASH_SMALL      = 2007,
    HASH_MEDIUM     = 6007,
    HASH_LARGE      = 13007,
    HASH_VERY_LARGE = 49999
};

// forward declaration
template<class KEY_TYPE,
         class VAL_TYPE,
         class COMPARE_OP = cDefaultCompare<KEY_TYPE>,
         class CONVERTER  = cDefaultConvert<KEY_TYPE> >
class cHashTableIter;

//General version of cHashTable template:
template<class KEY_TYPE,
         class VAL_TYPE,
         class COMPARE_OP = cDefaultCompare<KEY_TYPE>,
         class CONVERTER  = cDefaultConvert<KEY_TYPE> >
class cHashTable
{
    friend class cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>;

public:
    //structure for hash table nodes.
    struct node
    {
        KEY_TYPE nKey;
        VAL_TYPE nData;
        node*    next;
    };

    cHashTable(int tblSize = HASH_MEDIUM);
    ~cHashTable();

    bool Insert(KEY_TYPE key, VAL_TYPE data_in);
    // The pointer, data_in, is stored in a node based on string_in's hashing.
    //
    // if (key) already exists in the table, then it's value is replaced by (data_in)
    // returns true if (key) already existed in table.  otherwise, returns false

    bool Lookup(KEY_TYPE key, VAL_TYPE& data_out) const;
    //bool Lookup(TSTRING key, VAL_TYPE& data_out) const;
    //Lookup returns true if a match is found for string_check.  A reference
    //to the node in the table that matches string_check is passed back (by ref).
    bool Remove(KEY_TYPE key);
    //The node that matches string_out is de-allocated.

    bool Clear(void);
    //Clears the entire table and sets all node pointers to NULL
    bool   IsEmpty(void) const;
    uint32_t Hash(const KEY_TYPE& key) const;
    //The hashing function, taken from old Tripwire
    int32_t GetNumValues() const
    {
        return mValuesInTable;
    };
        // returns number of table entries filled

#ifdef DEBUG
    void TraceDiagnostics() const;
    // traces hash table statistics
#endif

private:
    cHashTable(const cHashTable& rhs);     // not impl
    void operator=(const cHashTable& rhs); // not impl

    node**   mTable;
    int      mTableSize;
    int32_t  mValuesInTable;
};

///////////////////////////////////////////////////////////////////////////////
// cHashTableIter
///////////////////////////////////////////////////////////////////////////////

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER> class cHashTableIter
{
public:
    cHashTableIter(const cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>& hashTbl);

    void SeekBegin() const;
    bool Done() const;
    void Next() const;

    const KEY_TYPE& Key() const;
    const VAL_TYPE& Val() const;
    VAL_TYPE&       Val();

private:
    mutable int                                                                   mCurIndex;
    mutable typename cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::node* mpCurNode;
    const cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>&                  mHashTable;

    // helper function
    void SeekNextValid() const;
};


//#############################################################################
// implementation

///////////////////////////////////////////////////////////////////////////////
// iterator
///////////////////////////////////////////////////////////////////////////////
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::cHashTableIter(
    const cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>& hashTbl)
    : mHashTable(hashTbl)
{
    SeekBegin();
}

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline void cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::SeekBegin() const
{
    mCurIndex = 0;
    mpCurNode = mHashTable.mTable[0];
    if (!mpCurNode)
        SeekNextValid();
}

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline bool cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Done() const
{
    return ((mCurIndex < 0) || (mCurIndex >= mHashTable.mTableSize));
}

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline void cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Next() const
{
    SeekNextValid();
}

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline void cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::SeekNextValid() const
{
    if (mpCurNode)
        mpCurNode = mpCurNode->next;

    // if we're out of range, bail out w/o incrementing index
    if (mCurIndex >= mHashTable.mTableSize)
        return;

    while ((!mpCurNode) && (++mCurIndex < mHashTable.mTableSize))
    {
        mpCurNode = mHashTable.mTable[mCurIndex];
    }
}

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline const KEY_TYPE& cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Key() const
{
    ASSERT(!Done());
    return mpCurNode->nKey;
}
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline const VAL_TYPE& cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Val() const
{
    ASSERT(!Done());
    return mpCurNode->nData;
}
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
inline VAL_TYPE& cHashTableIter<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Val()
{
    ASSERT(!Done());
    return mpCurNode->nData;
}

///////////////////////////////////////////////////////////////////////////////
// Constructors & Destructor
///////////////////////////////////////////////////////////////////////////////

//Default value for tblSize == 6007
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::cHashTable(int tblSize)
{
    mValuesInTable = 0;
    mTableSize     = tblSize;
    mTable         = new node*[mTableSize];

    for (int i = 0; i < mTableSize; ++i)
        mTable[i] = NULL;
}

//Destructor steps through table and deallocates all dynamic memory
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::~cHashTable()
{
    for (int i = 0; i < mTableSize; ++i)
    {
        if (mTable[i] != NULL)
        {
            //delete the entire chain:
            node* curr = mTable[i];
            node* del;
            while (curr != NULL)
            {
                del  = curr;
                curr = curr->next;

                delete del;
            }
        }
    }
    delete [] mTable;
}

////////////////////////////////////////////////////////////////////////////////
// Insert -- Hashes a const TCHAR* to a new index.  Collisions are resolved
//  using seperate chaining (link lists).
////////////////////////////////////////////////////////////////////////////////
// General Version:
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
bool cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Insert(KEY_TYPE key, VAL_TYPE d_in)
{
    COMPARE_OP compare;

    int hindex = Hash(key);
    if (mTable[hindex] == NULL)
    {
        //open index, perform insert
        mTable[hindex]          = new node;
        (mTable[hindex])->nKey  = key;
        (mTable[hindex])->next  = NULL;
        (mTable[hindex])->nData = d_in;
        mValuesInTable++;

        return false;
    }
    else //collision, do linked list insert
    {
        // case 1: key already exists in list -- replace existing one
        // case 2: key does not exist -- add to end of list

        node* nodeptr = mTable[hindex];

        bool found = false;
        while (true)
        {
            if (compare(nodeptr->nKey, key))
            {
                // we found a duplicate!
                found = true;
                break;
            }

            // break if this is the last node in the list
            if (!nodeptr->next)
                break;

            // otherwise, keep traversing
            nodeptr = nodeptr->next;
        }

        // add a node if the key was not found
        if (!found)
        {
            node* prev    = nodeptr;
            nodeptr       = new node;
            nodeptr->nKey = key;
            nodeptr->next = NULL;
            prev->next    = nodeptr;

            mValuesInTable++;
        }

        // whether it is a new node or not, set the data to this new value
        nodeptr->nData = d_in;

        return found;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Lookup -- Attempts to find 'string' in the hash table.
////////////////////////////////////////////////////////////////////////////////
// General Version:
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
bool cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Lookup(KEY_TYPE key, VAL_TYPE& d_out) const
{
    COMPARE_OP compare;

    int hindex = Hash(key);
    if (mTable[hindex] == NULL)
        return false;
    else
    {
        node* nodeptr = mTable[hindex];
        while (nodeptr != NULL)
        {
            if (compare(nodeptr->nKey, key))
            {
                d_out = nodeptr->nData;
                return true;
            }
            nodeptr = nodeptr->next;
        }
    }
    return false; //mTable entries exhausted without a match
}

////////////////////////////////////////////////////////////////////////////////
// Remove -- Removes a single entry from the hash table.  Returns false if
//  the nKey is not found in the table.
////////////////////////////////////////////////////////////////////////////////
// General Version -
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
bool cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Remove(KEY_TYPE key)
{
    COMPARE_OP compare;

    int hindex = Hash(key);
    if (mTable[hindex] == NULL)
    {
        delete (mTable[hindex]);
        mTable[hindex] = NULL;
        return true;
    }
    else
    {
        node* nodeptr = mTable[hindex];
        node* prev;
        while (nodeptr != NULL)
        {
            prev = nodeptr;
            if (compare(mTable[hindex]->nKey, key))
            {
                prev->next = nodeptr->next;
                delete nodeptr;
                if (nodeptr == mTable[hindex])
                    mTable[hindex] = NULL;
                nodeptr = NULL;
                return true;
            } //end if
            nodeptr = nodeptr->next;
        }         //end while
    }             //end else
    return false; //match was not found, no node deleted
}

////////////////////////////////////////////////////////////////////////////////
// Clear -- Clears entire hash table so that all indices are NULL
////////////////////////////////////////////////////////////////////////////////
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
bool cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Clear(void)
{
    for (int i = 0; i < mTableSize; ++i)
    {
        if (mTable[i] != NULL)
        {
            node* curr = mTable[i];
            node* del;
            while (curr != NULL)
            {
                del  = curr;
                curr = curr->next;
                delete del;
                if (del == mTable[i])
                    mTable[i] = NULL;
                del = NULL;

            } //end delete chain loop
        }     //end if mTable[i]!= NULL
    }         //end for
    return (IsEmpty());
}

////////////////////////////////////////////////////////////////////////////////
// IsEmpty --
////////////////////////////////////////////////////////////////////////////////
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
bool cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::IsEmpty(void) const
{
    bool ret = true;
    for (int i = 0; i < mTableSize; ++i)
        ret &= (mTable[i] == NULL);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Hash -- performs hashing on key, returns an integer index val.
////////////////////////////////////////////////////////////////////////////////
template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
uint32_t cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::Hash(const KEY_TYPE& key) const
{
    CONVERTER   converter;
    int         len;
    const uint8_t* pb = converter(key, &len); //locates key
    uint32_t    hindex;

    hindex = *pb;
    while (len-- > 0)
        hindex = ((hindex << 9) ^ *pb++) % mTableSize;
    return hindex;
}

#ifdef DEBUG

template<class KEY_TYPE, class VAL_TYPE, class COMPARE_OP, class CONVERTER>
void cHashTable<KEY_TYPE, VAL_TYPE, COMPARE_OP, CONVERTER>::TraceDiagnostics() const
{
    cDebug d("cHashTable::Diagnostics");

    int   slotsFilled = 0, numItems = 0, numMultiSlot = 0;
    node* pNode;

    for (int i = 0; i < mTableSize; i++)
    {
        if (mTable[i] != NULL)
        {
            slotsFilled++;
            numItems++;
            pNode = (mTable[i])->next;
            if (pNode != NULL)
                numMultiSlot++;
            while (pNode)
            {
                numItems++;
                pNode = pNode->next;
            }
        }
    }

    d.TraceDebug("---------------Hash Table Statisics---------------\n");
    d.TraceDebug("-- Number of slots:                        %d\n", mTableSize);
    d.TraceDebug("-- Number of items:                        %d\n", numItems);
    d.TraceDebug("-- Slots filled:                           %d (%lf %%)\n",
                 slotsFilled,
                 ((double)slotsFilled / (double)mTableSize) * 100.0);
    d.TraceDebug("-- Slots with >1 item:                     %d (%lf %%)\n",
                 numMultiSlot,
                 ((double)numMultiSlot / (double)slotsFilled) * 100.0);
    d.TraceDebug("--------------------------------------------------\n");
}
#    endif // DEBUG


#endif //__HASHTABLE_H
