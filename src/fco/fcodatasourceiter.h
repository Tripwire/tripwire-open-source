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
// fcodatasourceiter.h
//
// iFCODataSourceIter -- the data source iterator base class
//
#ifndef __FCODATASOURCEITER_H
#define __FCODATASOURCEITER_H

//=========================================================================
// INCLUDES
//=========================================================================


#ifndef __ERROR_H
#include "core/error.h"
#endif

#ifndef __FCONAME_H
#include "fconame.h"
#endif

#ifndef __DEBUG_H
#include "core/debug.h"
#endif

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class iFCO;
class cErrorBucket;

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

//-----------------------------------------------------------------------------
// eFCODataSourceIter
//-----------------------------------------------------------------------------
/*
class eFCODataSourceIter : public eError
{
public:
    // TODO -- this class should contain another exception as a member variable that
    //      represents the exception thrown from the underlying subsystem

    enum 
    {
        ERR_MIN                     = 1600,
        ERR_DUPE_FCO,
        ERR_MAX
    };

    eFCODataSourceIter(int errorNum, const TSTRING& msg) : eError( errorNum, msg ) {}
};
*/
TSS_EXCEPTION(eFCODataSourceIter, eError)
TSS_EXCEPTION(eFCODataSourceIterDupeFCO, eFCODataSourceIter)

//-----------------------------------------------------------------------------
// iFCODataSourceIter
//-----------------------------------------------------------------------------
class iFCODataSourceIter
{
public:
    //
    // constructor
    //
    iFCODataSourceIter()
    {
    }
    virtual ~iFCODataSourceIter()
    {
    }

    virtual iFCODataSourceIter* CreateCopy() const = 0;
    // this creates a new iterator that is an exact copy of myself; it is the caller's responsibility
    // to delete the return value of this method

    virtual const TCHAR* GetShortName() const  = 0;
    virtual cFCOName     GetName() const       = 0;
    virtual cFCOName     GetParentName() const = 0;
    // this returns the current node name, the fully qualified node name, and the node's parent's name
    // respectively. The first two methods return undefined strings if Done() is true.

    //
    // traverse up and down the hierarchy
    //
    virtual bool AtRoot() const     = 0; //throw (eError)
    virtual bool CanDescend() const = 0; //throw (eError)
    virtual void Descend()          = 0; //throw (eError) = 0
    virtual void Ascend()           = 0; //throw (eError) = 0

    //
    // traverse my peers (objects in the same directory as me)
    //
    virtual void SeekBegin()  = 0;
    virtual bool Done() const = 0;
    virtual void Next()       = 0;

    //
    // actually getting the fco
    //
    virtual iFCO* CreateFCO()                                               = 0; //throw (eError)                = 0
    virtual void  SeekToFCO(const cFCOName& name, bool bCreatePeers = true) = 0; //throw (eFCODataSourceIter)    = 0
        // I actually think that this method should just set Done() == true if the
        // object doesn't exist, besides adding something to the error bucket.
        // bCreatePeers is a hint to the iterator. Set it to false if you are only interested in the named
        //      FCO and/or its children. If it is set to false, the iterator may opt to not create the named
        //      FCO's peers, if that will speed up the operation.

    virtual void SetErrorBucket(cErrorBucket* pBucket) = 0;
    // call this to set the error queue; internal errors that occur while generating fcos
    // will have their messages posted to the error queue. To remove the error queue, call
    // this with NULL as the single parameter

    virtual bool SeekTo(const TCHAR* shortName) = 0;
    // this looks among my peers for an entry named shortName. It takes case-sensitiveness
    // into account. If the object is not found, Done() is true after this and false is returned.

    virtual bool IsCaseSensitive() const = 0;
    // override this to indicate whether the underlying data source is case sensitive

    enum IterFlags
    {
        DO_NOT_MODIFY_OBJECTS = 0x00000001 // reset any object properties that may have been altered due to iteration
    };
    virtual int  GetIterFlags() const = 0;
    virtual void SetIterFlags(int i)  = 0;
    // any flags needed for iteration.


    //TODO - should the iterator have insertion or deletion methods?
    //      It seems to me that insertion is necessary for rapid database creation, but deletion is
    //      probably not necessary (although nice for symmetry). It also seems that we would want the
    //

    //-------------------------------------------------------------------------
    // non-pure virtual methods
    //-------------------------------------------------------------------------

    //
    // getting my relationship to other data source iters...
    //
    enum Relationship
    {
        REL_LT, //less than
        REL_EQ, //equal
        REL_GT  //greater than
    };
    Relationship GetRelationship(const iFCODataSourceIter& rhs) const;
    // this returns the relationship of the currently pointed-at item in my list to
    // the current one in the rhs's list. It is assumed that the parent name is the same
    // for both sides, asserted that both sides are not Done(), and asserted that
    // the case-sensitiveness of both sides is the same.
    //
    // TODO -- If I implement this with string compares of the short name, is this
    //      going to be too slow? If so, I might need to add the GetShortHash() method
    //      to this interface that would return some simple hash of the short name.
    Relationship Compare(const TCHAR* str1, const TCHAR* str2) const;
    // this compares str1 to str2 and returns lt, eq, or gt. All iterators should use
    // this method to order the short names of the objects that they iterate over.
};


//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

inline iFCODataSourceIter::Relationship iFCODataSourceIter::GetRelationship(const iFCODataSourceIter& rhs) const
{
    ASSERT(IsCaseSensitive() == rhs.IsCaseSensitive());

    return (Compare(GetShortName(), rhs.GetShortName()));
}

#endif //__FCODATASOURCEITER_H
