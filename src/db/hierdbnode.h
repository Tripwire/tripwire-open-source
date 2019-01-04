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
// hierdbnode.h
//
// these classes are used by cHierDatabase as the nodes in the hierarchy
//
#ifndef __HIERDBNODE_H
#define __HIERDBNODE_H

#ifndef __TYPES_H
#include "types.h"
#endif
#ifndef __ARCHIVE_H
#include "archive.h"
#endif

///////////////////////////
// BIG TODO -- I need to change all of these strings to be wchar_t instead of TSTRINGS
//              Also, they should serialize themselves in a platform-neutral byte order
///////////////////////////

//-----------------------------------------------------------------------------
// cHierNode -- the base class for all other nodes; this class contains type
//      information
//-----------------------------------------------------------------------------
class cHierNode
{
public:
    enum Type
    {
        TYPE_INVALID = -1,
        TYPE_MIN     = 0,
        TYPE_ROOT    = 1,
        TYPE_ENTRY,
        TYPE_ARRAY,
        TYPE_ARRAY_INFO,
        TYPE_MAX
    };


    explicit cHierNode(Type type = TYPE_INVALID) : mType(type)
    {
    }
    virtual ~cHierNode()
    {
    }

    int32_t mType;

    /////////////////////////////////////////
    // methods to override
    /////////////////////////////////////////
    virtual int32_t CalcArchiveSize() const
    {
        return (sizeof(mType));
    }
    // this method should return the amount of space that this class will take up in an archive
    // derived classes should override this if they have any data members that will be stored
    // with Read() or Write()
    virtual void Write(cArchive& arch) const //throw(eArchive)
    {
        arch.WriteInt32(mType);
    }
    virtual void Read(cArchive& arch) //throw(eArchive)
    {
        arch.ReadInt32(mType);
        //
        // do a rudimentary integrity check...
        //
        if ((mType <= TYPE_MIN) || (mType >= TYPE_MAX))
        {
            ASSERT(false);
            throw eArchiveFormat(_T("Invalid type encountered in cHierNode::Read"));
        }
    }
    // these methods read and write the class to an archive. We had better be sure that
    // we don't write more than we said we would in CalcArchiveSize() !
};

//-----------------------------------------------------------------------------
// cHierAddr -- the "address" of different items in the hierarchy
//-----------------------------------------------------------------------------
class cHierAddr
{
public:
    int32_t mBlockNum;
    int32_t mIndex;

    cHierAddr(int32_t block = -1, int32_t index = -1) : mBlockNum(block), mIndex(index)
    {
    }

    bool IsNull() const
    {
        return (mBlockNum == -1);
    }
    // returns true if the address points to "null" (ie -- no valid address)
    bool operator==(const cHierAddr& rhs) const
    {
        return ((mBlockNum == rhs.mBlockNum) && (mIndex == rhs.mIndex));
    }


    /////////////////////////////////////////////////
    // serialization methods
    /////////////////////////////////////////////////
    int32_t CalcArchiveSize() const
    {
        return (sizeof(mBlockNum) + sizeof(mIndex));
    }
    void Write(cArchive& arch) const //throw(eArchive)
    {
        arch.WriteInt32(mBlockNum);
        arch.WriteInt32(mIndex);
    }
    void Read(cArchive& arch) //throw(eArchive)
    {
        arch.ReadInt32(mBlockNum);
        arch.ReadInt32(mIndex);
    }
};


//-----------------------------------------------------------------------------
// cHierRoot -- the root of the hierarchy; you can always count on this being
//      at (0, 0)
//-----------------------------------------------------------------------------
class cHierRoot : public cHierNode
{
public:
    cHierRoot() : cHierNode(TYPE_ROOT), mbCaseSensitive(true), mDelimitingChar('/')
    {
    }

    cHierAddr mChild;          // points to a cHierArray or an invalid address
    bool      mbCaseSensitive; // determines the case-sensitiveness of lookups, ordering, etc.
    TCHAR     mDelimitingChar; // the delimiting character; this is used when displaying a path to the user

    /////////////////////////////////////////////////
    // serialization methods
    /////////////////////////////////////////////////
    virtual int32_t CalcArchiveSize() const
    {
        return (cHierNode::CalcArchiveSize() + mChild.CalcArchiveSize());
    }
    virtual void Write(cArchive& arch) const //throw(eArchive)
    {
        cHierNode::Write(arch);
        mChild.Write(arch);
        arch.WriteInt32(mbCaseSensitive ? 1 : 0);
        TSTRING dc(&mDelimitingChar, 1);
        arch.WriteString(dc);
    }
    virtual void Read(cArchive& arch) //throw(eArchive)
    {
        cHierNode::Read(arch);
        //
        // make sure the type is correct
        //
        if (mType != TYPE_ROOT)
        {
            ASSERT(false);
            throw eArchiveFormat(_T("Invalid type encountered; expected ROOT node"));
        }
        mChild.Read(arch);
        int32_t cs;
        arch.ReadInt32(cs);
        mbCaseSensitive = cs ? true : false;
        TSTRING dc;
        arch.ReadString(dc);

        if (dc.length() != 1)
        {
            ASSERT(false);
            throw eArchiveFormat(_T("Read of the root node failed; invalid delimiting character."));
        }
        mDelimitingChar = dc[0];
    }
};

//-----------------------------------------------------------------------------
// cHierEntry -- an entry that contains a name and points to some data
//-----------------------------------------------------------------------------
class cHierEntry : public cHierNode
{
public:
    cHierEntry() : cHierNode(TYPE_ENTRY)
    {
    }

    TSTRING   mName; // TODO -- change this to a wchar_t string
    cHierAddr mData;
    cHierAddr mChild; // points to a cHierArray or an invalid address
    cHierAddr mNext;  // the next peer entry in the linked list, or Null() if done

    /////////////////////////////////////////////////
    // serialization methods
    /////////////////////////////////////////////////
    virtual int32_t CalcArchiveSize() const
    {
        return (cHierNode::CalcArchiveSize() + mChild.CalcArchiveSize() + mData.CalcArchiveSize() +
                cArchive::GetStorageSize(mName) + mNext.CalcArchiveSize());
    }
    virtual void Write(cArchive& arch) const //throw(eArchive)
    {
        cHierNode::Write(arch);
        arch.WriteString(mName);
        mChild.Write(arch);
        mData.Write(arch);
        mNext.Write(arch);
    }
    virtual void Read(cArchive& arch) //throw(eArchive)
    {
        cHierNode::Read(arch);
        //
        // make sure the type is correct
        //
        if (mType != TYPE_ENTRY)
        {
            ASSERT(false);
            throw eArchiveFormat(_T("Invalid type encountered; expected ENTRY node"));
        }

        arch.ReadString(mName);
        mChild.Read(arch);
        mData.Read(arch);
        mNext.Read(arch);
    }
};

//-----------------------------------------------------------------------------
// cHierArrayInfo -- constant-size struct that contains information about an array
//-----------------------------------------------------------------------------
class cHierArrayInfo : public cHierNode
{
public:
    cHierArrayInfo() : cHierNode(TYPE_ARRAY_INFO)
    {
    }

    cHierAddr mParent; // points to a cHierArrayInfo or cHierRoot
    cHierAddr mArray;  // points to cHierEntryArray

    /////////////////////////////////////////////////
    // serialization methods
    /////////////////////////////////////////////////
    virtual int32_t CalcArchiveSize() const
    {
        return (cHierNode::CalcArchiveSize() + mParent.CalcArchiveSize() + mArray.CalcArchiveSize());
    }
    virtual void Write(cArchive& arch) const //throw(eArchive)
    {
        cHierNode::Write(arch);
        mParent.Write(arch);
        mArray.Write(arch);
    }
    virtual void Read(cArchive& arch) //throw(eArchive)
    {
        cHierNode::Read(arch);
        //
        // make sure the type is correct
        //
        if (mType != TYPE_ARRAY_INFO)
        {
            ASSERT(false);
            throw eArchiveFormat(_T("Invalid type encountered; expected TYPE_ARRAY_INFO node"));
        }

        mParent.Read(arch);
        mArray.Read(arch);
    }
};


#endif //__HIERDBNODE_H
