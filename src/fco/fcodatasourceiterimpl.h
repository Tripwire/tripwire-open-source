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
// fsdatasourceiter.h
//
#ifndef __FCODATASOURCEITERIMPL_H
#define __FCODATASOURCEITERIMPL_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __FCODATASOURCEITER_H
#include "fcodatasourceiter.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================


class cFCODataSourceIterImpl : public iFCODataSourceIter
{
public:
    cFCODataSourceIterImpl();
    virtual ~cFCODataSourceIterImpl();
    cFCODataSourceIterImpl& operator=(const cFCODataSourceIterImpl& rhs);


    virtual const TCHAR* GetShortName() const;
    virtual cFCOName     GetName() const;
    virtual cFCOName     GetParentName() const;

    virtual bool AtRoot() const;     //throw (eError);
    virtual bool CanDescend() const; //throw (eError);
    virtual void Descend();          //throw (eError);
    virtual void Ascend();           //throw (eError);

    virtual void SeekBegin();
    virtual bool Done() const;
    virtual void Next();

    virtual iFCO* CreateFCO();                                               //throw (eError);
    virtual void  SeekToFCO(const cFCOName& name, bool bCreatePeers = true); //throw (eFCODataSourceIter);

    virtual void SetErrorBucket(cErrorBucket* pBucket)
    {
        mpErrorBucket = pBucket;
    }
    virtual bool SeekTo(const TCHAR* shortName);
    virtual bool IsCaseSensitive() const;

    virtual int  GetIterFlags() const;
    virtual void SetIterFlags(int i);


    //
    // must override this functions
    //
    virtual iFCODataSourceIter* CreateCopy() const = 0;

    // debug functions
    virtual void TraceContents(int dl = -1) const;


    //
    // util function for sorting/searching
    //
    bool CompareForUpperBound(const iFCO* pFCO, const TCHAR* pchName) const;
    // compares shortname of FCO to pchName.  returns < == or > 0 as strcmp does.

protected:
    cFCODataSourceIterImpl(const cFCODataSourceIterImpl&);

    //TODO -- turn this into a set?
    //
    typedef std::vector<iFCO*> FCOList; // sorted in increasing order by name of the FCO

    cErrorBucket*           mpErrorBucket;
    cFCOName                mParentName;
    FCOList                 mPeers;
    FCOList::const_iterator mCurPos;
    uint32_t                mFlags;

    //-------------------------------------------------------------------------
    // helper methods
    //-------------------------------------------------------------------------
    void GeneratePeers();
    // used to implement Descend() -- also used by SeekToFCO() ... it takes the value of
    // mParentName and fills out mPeers and sets mCurPos to mPeers.begin()
    // TODO -- this fills out the error queue if the readdir() or any stats fail.
    void ClearList();
    // releases everything in the list, empties the list, and sets mCurPos to end()

    bool InsertIntoPeers(iFCO* pFCO);
    // inserts the fco into peers.  Peers sorted in increasing order by name of the FCO.
    // Uses binary search to find location into which to insert.
    // mCurPos is not guaranteed to be valid after this call due to possible list reallocation.
    // If an entry with the same name already exists, the fco is not added, an error is added to the error
    // bucket, and false is returned. Otherwise, true is always returned.

    void SeekToPeerByName(const TCHAR* pchName);
    // seeks the iter to the peer with this name.  Done() will be true if the function cannot find the peer
    // Uses binary search algorithm.

    FCOList::iterator UpperBound(const TCHAR* pchShortName);
    // same as std::upper_bound except it calls CompareForUpperBound() to
    // do its comparison.


    //
    // must override these functions
    //
    virtual void GetChildrenNames(const TSTRING& strParentName, std::vector<TSTRING>& vChildrenNames) = 0;
    // retrieves the names of all the children of the named parent
    // strParent name is of the form returned by cFCOName::AsString. Note that it is not very efficient
    // to do it this way for registry objects -- 4 Mar 99 mdb

    virtual iFCO* CreateObject(const cFCOName& name, bool bCreatingPeers) = 0;
    // this method is called when creating the heirarchy.  at minimum it needs to create an empty iFCO
    // with the correct name.  if NULL is returned, the iterator does not go any deeper into the FCO.
    // you may also use this call to set some properties of the FCO.  For instance,
    // FS will record the device number of a file if we are creating peers, and it's CreateObject will
    // fail if it's peers have a different device number.
    // Must set the object type in this function
    //
    // If bCreatingPeers is false, then this method needs to verify that the object actually exists, and
    // return 0 if it does not. (if bCreatingPeers is true, we got the name from the OS, so we assume it
    // exists)

    virtual bool InitializeTypeInfo(iFCO* pFCO) = 0;
    // initializes the minimal amout of information about the fco to know what "type" it is. If any OS calls
    // fail, the error bucket should be filled up and false should be returned.
    //
    // this method should check whether the properties are already valid and do nothing if they already are;
    // this has the potential to be called multiple times, so we need to be efficient.
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

inline int cFCODataSourceIterImpl::GetIterFlags() const
{
    return mFlags;
}

inline void cFCODataSourceIterImpl::SetIterFlags(int i)
{
    mFlags = i;
}

#endif //__FCODATASOURCEITERIMPL_H
