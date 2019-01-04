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
// dbdatasource.h
//
#ifndef __DBDATASOURCE_H
#define __DBDATASOURCE_H

#ifndef __FCODATASOURCEITER_H
#include "fco/fcodatasourceiter.h"
#endif
#ifndef __HIERDATABASE_H
#include "db/hierdatabase.h"
#endif
#ifndef __SREFCOUNTOBJ_H
#include "core/srefcountobj.h"
#endif

class cDbDataSourceIter : public iFCODataSourceIter
{
public:
    cDbDataSourceIter(cHierDatabase* pDb, int genreNum = -1); //throw (eError)
        // this ctor asserts that the case-sensitiveness and delimiting char match that of the passed in genre...
        // if -1 is passed for the genre num, then the current genre is used.

    cDbDataSourceIter(const cDbDataSourceIter& rhs);
    cDbDataSourceIter& operator=(const cDbDataSourceIter& rhs);


    virtual ~cDbDataSourceIter();

    virtual iFCODataSourceIter* CreateCopy() const;

    virtual const TCHAR* GetShortName() const;
    virtual cFCOName     GetName() const;
    virtual cFCOName     GetParentName() const;

    virtual bool AtRoot() const;     //throw (eError)
    virtual bool CanDescend() const; //throw (eError)
    virtual void Descend();          //throw (eError)
    virtual void Ascend();           //throw (eError)

    virtual void SeekBegin();
    virtual bool Done() const;
    virtual void Next();
    virtual bool SeekTo(const TCHAR* shortName);
    virtual bool IsCaseSensitive() const;

    virtual iFCO* CreateFCO(); //throw (eError)
    virtual void  SetErrorBucket(cErrorBucket* pBucket);

    ///////////////////////////////////////////////
    // This stuff is in this derived class _only_
    void SeekToRoot(); //throw( eError )
        // seeks to the root of the database
    virtual void AddFCO(const TSTRING& shortName, const iFCO* pFCO); //throw (eFCODataSourceIter, eError)
        // adds the fco to the current directory; this throws an exception if
        // the insert fails or an fco already named the same exists in the directory. After the insertion,
        // the iterator points at the new node.
    virtual void RemoveFCO(); //throw (eError)
        // removes the current fco; iterator then points at the next object or done() is true if it is at the end.
    virtual void SetFCOData(const iFCO* pFCO); //throw (eError)
        // sets the data associated with the current FCO. This method pays no attention to the FCO's name. (maybe I
        // should assert?)
    virtual void RemoveFCOData(); //throw (eError)
        // removes the fco data associated with the current fco. This asserts the iterator is not done and contains
        // data.
    bool HasFCOData() const;
    // this returns true if the current node contains fco data. If this returns false, then it is not
    // legal to call CreateFCO()
    void CreatePath(const cFCOName& name); //throw (eError)
        // a convenience method that creates the named path (or any part that is currently not created)
        // the iterator ends up with the named path as the current item (ie -- it will be returned from
        // GetName())
    void AddChildArray(); //throw (eError)
        // this adds a child array to the current FCO. This asserts that one does not currently exist and
        // that Done() is not true
    void RemoveChildArray(); //throw (eError)
        //this asserts that Done() is not true, and the child array contains nothing.
    bool CanRemoveChildArray(); //throw (eError)
        // returns true if the child array is empty, and can thus be removed with RemoveChildArray()
    void Refresh(); // throw (eError)

    //
    ///////////////////////////////////////////////
    virtual void SeekToFCO(const cFCOName& name, bool bCreatePeers = true); //throw(eFCODataSourceIter)
        // NOTE -- this class ignores bCreatePeers

    virtual int  GetIterFlags() const;
    virtual void SetIterFlags(int i);

private:
    //
    // helper methods
    //
    bool SeekToDirectory(const cFCOName& parentName, bool bCreate = false);
    // seeks the iterator to the named directory. This method attempts to take the most direct route possible,
    // seeking from the current working directory instead of seeking to the root and then descending to the
    // named directory. When this operation returns, it is positioned at the first element in the named directory
    // unless false is returned, in which case the iterator is positioned at an undefined location. If bCreate
    // is true, the directory is created if it does not currently exist.


    cHierDatabase::iterator     mDbIter;
    iSerRefCountObj::CreateFunc mFCOCreateFunc; // points to the function that creates the fcos we return
    uint32_t                    mFlags;         // flags used for iteration
    cErrorBucket*               mpErrorBucket;
};

//#############################################################################
// inline implementation
//#############################################################################

inline void cDbDataSourceIter::Refresh()
{
    mDbIter.Refresh();
}

///////////////////////////////////////////////////////////////////////////////
// SeekTo
///////////////////////////////////////////////////////////////////////////////
inline bool cDbDataSourceIter::SeekTo(const TCHAR* shortName)
{
    return mDbIter.SeekTo(shortName);
}


///////////////////////////////////////////////////////////////////////////////
// SeekToRoot
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::SeekToRoot()
{
    mDbIter.SeekToRoot();
}

///////////////////////////////////////////////////////////////////////////////
// GetParentName
///////////////////////////////////////////////////////////////////////////////
inline cFCOName cDbDataSourceIter::GetParentName() const
{
    cFCOName rtn(mDbIter.GetCwd());
    return rtn;
}

///////////////////////////////////////////////////////////////////////////////
// AtRoot
///////////////////////////////////////////////////////////////////////////////
inline bool cDbDataSourceIter::AtRoot() const //throw (eError)
{
    return (mDbIter.AtRoot());
}

///////////////////////////////////////////////////////////////////////////////
// CanDescend
///////////////////////////////////////////////////////////////////////////////
inline bool cDbDataSourceIter::CanDescend() const //throw (eError)
{
    return (mDbIter.CanDescend());
}

///////////////////////////////////////////////////////////////////////////////
// Descend
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::Descend() //throw (eError)
{
    mDbIter.Descend();
}

///////////////////////////////////////////////////////////////////////////////
// Ascend
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::Ascend() //throw (eError)
{
    mDbIter.Ascend();
}

///////////////////////////////////////////////////////////////////////////////
// SeekBegin
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::SeekBegin()
{
    mDbIter.SeekBegin();
}

///////////////////////////////////////////////////////////////////////////////
// Done
///////////////////////////////////////////////////////////////////////////////
inline bool cDbDataSourceIter::Done() const
{
    return (mDbIter.Done());
}

///////////////////////////////////////////////////////////////////////////////
// Next
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::Next()
{
    mDbIter.Next();
}

///////////////////////////////////////////////////////////////////////////////
// IsCaseSensitive
///////////////////////////////////////////////////////////////////////////////
inline bool cDbDataSourceIter::IsCaseSensitive() const
{
    return mDbIter.IsCaseSensitive();
}

///////////////////////////////////////////////////////////////////////////////
// GetShortName
///////////////////////////////////////////////////////////////////////////////
inline const TCHAR* cDbDataSourceIter::GetShortName() const
{
    ASSERT(!Done());

    return mDbIter.GetName();
}

///////////////////////////////////////////////////////////////////////////////
// CanRemoveChildArray
///////////////////////////////////////////////////////////////////////////////
inline bool cDbDataSourceIter::CanRemoveChildArray()
{
    return (mDbIter.ChildArrayEmpty());
}

///////////////////////////////////////////////////////////////////////////////
// GetIterFlags
///////////////////////////////////////////////////////////////////////////////
inline int cDbDataSourceIter::GetIterFlags() const
{
    return mFlags;
}

///////////////////////////////////////////////////////////////////////////////
// SetIterFlags
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::SetIterFlags(int i)
{
    mFlags = i;
}

///////////////////////////////////////////////////////////////////////////////
// SetIterFlags
///////////////////////////////////////////////////////////////////////////////
inline void cDbDataSourceIter::SetErrorBucket(cErrorBucket* pBucket)
{
    mpErrorBucket = pBucket;
}

#endif
