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
// hierdatabase.h
//
#ifndef __HIERDATABASE_H
#define __HIERDATABASE_H

#ifndef __BLOCKRECORDFILE_H
#include "blockrecordfile.h"
#endif
#ifndef __HIERDBNODE_H
#include "hierdbnode.h"
#endif
#ifndef __ERROR_H
#include "core/error.h"
#endif

#ifndef __HIERDBPATH_H
#include "hierdbpath.h"
#endif

#if HAVE_STRINGS_H // RTEMS needs this for strcasecmp
#include <strings.h>
#endif


class cHierDatabaseIter;
class cErrorBucket;


TSS_EXCEPTION(eHierDatabase, eError);
TSS_EXCEPTION(eHierDbInternal, eHierDatabase); // not used
TSS_EXCEPTION(eHierDbDupeName, eHierDatabase);

//
// TODO -- store some header info with the root node -- at least whether the db is case-sensitive or not
//      and what the delimiting character is
//

//-----------------------------------------------------------------------------
// cHierDatabase -- a hierarchical database class
//-----------------------------------------------------------------------------
class cHierDatabase : public cBlockRecordFile
{
public:
    cHierDatabase(bool bCaseSensitive = true, TCHAR delChar = _T('/'));
    // NOTE -- these two parameters are only used if the database is being created; otherwise, they
    //      are overwritten when Open() is called.
    virtual ~cHierDatabase();

    //
    // overrides from base class
    //
    virtual void
                 Open(const TSTRING& fileName, int numPages = 5, bool bTruncate = false); //throw (eArchive, eHierDatabase)
    virtual void Open(cBidirArchive* pArch, int numPages = 5); //throw (eArchive, eHierDatabase)
        // for the second Open(), this class owns the destruction of the archive
        //TODO -- make numPages the last parameter

    bool IsCaseSensitive() const
    {
        return mbCaseSensitive;
    }
    TCHAR GetDelimitingChar() const
    {
        return mDelimitingChar;
    }

    enum
    {
        INVALID_INDEX = -1
    };

    typedef cHierDatabaseIter iterator;
    typedef cBlockRecordFile  inherited;

private:
    cHierAddr mRootArrayAddr; // this is the address of the top-level array
    bool      mbCaseSensitive;
    TCHAR     mDelimitingChar;

    cHierDatabase(const cHierDatabase& rhs);  //not impl
    void operator=(const cHierDatabase& rhs); //not impl

    void OpenImpl(bool bTruncate); //throw (eArchive, eHierDatabase)
        // both Open() methods call this to initialize the class after calling
        // inherited::Open()

    friend class cHierDatabaseIter;
};

//-----------------------------------------------------------------------------
// cHierDatabaseIter -- the iterator class
//-----------------------------------------------------------------------------
class cHierDatabaseIter
{
public:
    explicit cHierDatabaseIter(cHierDatabase* pDb); //throw (eArchive)
    ~cHierDatabaseIter();

    //
    // copying
    //
    cHierDatabaseIter(const cHierDatabaseIter& rhs);
    cHierDatabaseIter& operator=(const cHierDatabaseIter& rhs);

    //
    // name characteristics -- these are alternatives to specifying in the ctor
    //
    void  SetCaseSensitive(bool bCaseSensitive);
    bool  IsCaseSensitive(void) const;
    void  SetDelimitingChar(TCHAR c);
    TCHAR GetDelimitingChar(void) const;

    //
    // traverse up and down the hierarchy
    //
    bool AtRoot() const; //throw (eHierDatabase)
    bool CanDescend() const;
    void Descend();
    void Ascend();
    //
    // traverse my peers (objects in the same directory as me)
    //
    void SeekBegin();
    bool Done() const;
    void Next();
    bool SeekTo(const TCHAR* pName);
    // seeks to the named peer; returns false and is positioned in an undefined
    // place if it can't find the peer.
    // it is either case sensitive or insensitive, depending on what was passed to the
    // ctor.
    void SeekToRoot(); //throw (eArchive)
        // positions the iterator at the root node

    void Refresh(); //throw (eArchive, eHierDatabase)
        // refresh the iterator's contents in case the db has changed since we first entered
        // this directory.
        // note that this will move the iterator back to the beginning of the array

    //
    // getting at the entry's data...
    //
    const TCHAR*  GetName() const;
    const TSTRING GetCwd() const;
    //
    // creating arrays and entries (ie directories and files)
    //
    void CreateChildArray();               //throw (eArchive, eHierDatabase)
    void CreateEntry(const TSTRING& name); //throw (eArchive, eHierDatabase)
        // after the entry is added, this iterator points at it.
        // if the entry already exists in the database, an error is added to pError.

    //
    // getting and setting the data associated with a given entry
    //
    int8_t* GetData(int32_t& length) const; //throw (eArchive, eHierDatabase)
        // returns the data associated with the current entry; this asserts that the iterator is
        // not done and the current entry has data associated with it
    bool HasData() const;
    // returns true if the current entry has data
    void SetData(int8_t* pData, int32_t length); //throw (eArchive, eHierDatabase)
    void RemoveData();                       //throw (eArchive, eHierDatabase)
        // removes the data associated with the current entry; this asserts that the current
        // entry actually _has_ data
    //
    // deleting arrays and entries
    //
    void DeleteEntry(); //throw (eArchive, eHierDatabase)
        // deletes the currently pointed-at entry; this assumes that Done() is not true, and
        // advances to the next node.
    bool ChildArrayEmpty(); //throw (eArchive, eHierDatabase)
        // returns true if the node's child array is empty
    void DeleteChildArray(); //throw (eArchive, eHierDatabase)
        // this method deletes the child array of the current object. This method asserts that
        // Done() is not true, that the current node has a child, and that the child array is

    // empty (TODO -- perhaps in the future we will want to make the equivalant of "rm -rf"

    //
    // util function for sorting/searching
    //
    bool CompareForUpperBound(const cHierEntry& he, const TCHAR* pchName) const;
    // compares shortname of FCO to pchName.  returns < == or > 0 as strcmp does.

protected:
    cHierAddr GetCurrentAddr() const;
    // returns the address of the current cHierEntry by examining either the previous pNext pointer or the
    // array info pointer if the first entry is current. This asserts (! Done())

    //-------------------------------------------------------------------------
    // member data
    //-------------------------------------------------------------------------
    // TODO -- maybe this should be a fully insulated class?
    // TODO -- maybe this should be a set to reduce insert and lookup time?
    typedef std::vector<cHierEntry> EntryArray;


    cHierDatabase*       mpDb;     // the database
    EntryArray           mEntries; // vector of entries for the current level
    cHierArrayInfo       mInfo;
    cHierAddr            mInfoAddr;
    EntryArray::iterator mIter;    // my current position in the array
    cHierDbPath          mCurPath; // The current working directory.

    //-------------------------------------------------------------------------
    // helper methods
    //-------------------------------------------------------------------------
    void LoadArrayAt(const cHierAddr& addr); //throw (eArchive, eHierDatabase)
        // initializes the iterator so that it is pointing at the array named by addr.
    int Compare(const TCHAR* pc1, const TCHAR* pc2) const;
    // this acts just like strcmp, but takes into account whether the database is case-sensitive or not.

    EntryArray::iterator UpperBound(const TCHAR* pchName);
    // same as std::upper_bound except it calls CompareForUpperBound() to
    // do its comparison.
};

//#############################################################################
// inline implementation
//#############################################################################
inline void cHierDatabaseIter::Refresh()
{
    cHierAddr infoAddr = mInfoAddr;
    LoadArrayAt(infoAddr);
}

inline void cHierDatabaseIter::SetCaseSensitive(bool bCaseSensitive)
{
    mCurPath.SetCaseSensitive(bCaseSensitive);
}

inline bool cHierDatabaseIter::IsCaseSensitive(void) const
{
    return mCurPath.IsCaseSensitive();
}

inline void cHierDatabaseIter::SetDelimitingChar(TCHAR c)
{
    mCurPath.SetDelimiter(c);
}

inline TCHAR cHierDatabaseIter::GetDelimitingChar(void) const
{
    return mCurPath.GetDelimiter();
}

///////////////////////////////////////////////////////////////////////////////
// Compare
///////////////////////////////////////////////////////////////////////////////
inline int cHierDatabaseIter::Compare(const TCHAR* pc1, const TCHAR* pc2) const
{
    if (mCurPath.IsCaseSensitive())
    {
        return (_tcscmp(pc1, pc2));
    }
    else
    {
        return (_tcsicmp(pc1, pc2));
    }
}


#endif //__HIERDATABASE_H
