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
// fcospechelper.h
//
// iFCOSpecHelper-- object that manages a spec's start point and abstracts its
//      stop criterion
#ifndef __FCOSPECHELPER_H
#define __FCOSPECHELPER_H

#ifndef __SREFCOUNTOBJ_H
#include "core/srefcountobj.h"
#endif
#ifndef __FCONAME_H
#include "fconame.h"
#endif
#ifndef __DEBUG_H
#include "core/debug.h"
#endif
#include <set>


class iSerializer;

///////////////////////////////////////////////////////////////////////////////
// iFCOSpecHelper -- interface for managing a spec's start point and stop
//      criterion
///////////////////////////////////////////////////////////////////////////////
class iFCOSpecHelper : public iTypedSerializable
{
public:
    virtual void            SetStartPoint(const cFCOName& startPoint); // throw (eError);
    virtual const cFCOName& GetStartPoint() const;
    // gets and sets the start point. SetStartPoint will throw an eError if the start point
    // is inappropriate

    virtual bool ContainsFCO(const cFCOName& name) const = 0;
    // returns true if the named fco is below the start point and "above" its terminating
    // criterion

    virtual iFCOSpecHelper* Clone() const = 0;
    // return a cloned copy of yourself; the clone will need to be deleted by the caller.

    virtual bool ShouldStopDescent(const cFCOName& name) const = 0;
    // used by the iFCODataSource interface of iFCOSpec, this is called for each fco that is
    // encountered. Really, ContainsFCO() could be used to reach the same ends, but
    // this might/should be faster.

    virtual void Read(iSerializer* pSerializer, int32_t version = 0);
    virtual void Write(iSerializer* pSerializer) const;
    // these just serialize the start point.

    enum CompareResult
    {
        CMP_LT = -1,
        CMP_EQ = 0,
        CMP_GT = 1
    };
    virtual CompareResult Compare(const iFCOSpecHelper* pRhs) const = 0;
    // these are needed to define an ordering of specs.

    virtual void TraceContents(int dl = -1) const
    {
    }

    virtual ~iFCOSpecHelper()
    {
    }

protected:
    cFCOName mStartPoint;
};

///////////////////////////////////////////////////////////////////////////////
// cFCOSpecStopPoints -- a spec helper that uses a set of stop points.
//      to determine where the spec ends.
//      NOTE -- stop points are stored internally in an order defined by cFCOName::operator<
//          All iterations over the stop points will be in this order.
//
//      This class also keeps track of a "maximum recurse depth", where all
//      calls to ShouldStopDescent() return true when the name is more than
//      the maximum depth below the start point.
///////////////////////////////////////////////////////////////////////////////
class cFCOSpecStopPointSet : public iFCOSpecHelper
{
public:
    cFCOSpecStopPointSet();

    virtual void            SetStartPoint(const cFCOName& startPoint);
    virtual bool            ContainsFCO(const cFCOName& name) const;
    virtual bool            ShouldStopDescent(const cFCOName& name) const;
    virtual void            TraceContents(int dl = -1) const;
    virtual iFCOSpecHelper* Clone() const;

    int GetSize() const;
    // returns the number of stop points this contains
    virtual void Clear();
    // clears the set of stop points.
    virtual void Add(const cFCOName& name); // throw(eError)
        // adds a stop point. Stop points may be removed and iterated through using
        // the iterator below.
        // If the stop point is above or below any existing stop points, the "lower" stop
        // point will be replaced with the higher one. Thus, at any given time,
        // all stop points are unrelated (retrun REL_UNRELATED from cFCOName::GetRelation)

    //
    // recurse depth
    //
    enum
    {
        RECURSE_INFINITE = -1
    };
    void SetRecurseDepth(int depth)
    {
        mRecurseDepth = depth;
    }
    int GetRecurseDepth() const
    {
        return mRecurseDepth;
    }
    // gets and sets the maximum recurse depth. If a name passed to ShouldStopDescent() is deeper than
    // the maximum recurse depth, true is returned. For example, if there are no stop points and the
    // start point is /etc, if the recurse depth is 1, then /etc/dog will be processed but /etc/dog/bark
    // will not. RECURSE_INFINITE can be passed in to indicate there is no maximum recurse depth.


    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0);
    virtual void Write(iSerializer* pSerializer) const;

    virtual CompareResult Compare(const iFCOSpecHelper* pRhs) const;

    DECLARE_TYPEDSERIALIZABLE()

    virtual ~cFCOSpecStopPointSet()
    {
    }

protected:
    std::set<cFCOName> mStopPoints;
    int                mRecurseDepth;

    friend class cFCOSpecStopPointIter;
    typedef iFCOSpecHelper inherited;
};

class cFCOSpecStopPointIter
{
public:
    explicit cFCOSpecStopPointIter(cFCOSpecStopPointSet& set);

    void SeekBegin() const;
    // seeks to the beginning of the stop point list
    bool Done() const;
    // returns true if the iterator has gone past the end of the set
    bool IsEmpty() const;
    // is the set empty?
    void Next() const;
    // seek to the next element in the set

    const cFCOName& StopPoint() const;
    const cFCOName& StopPoint();
    // methods for getting the StopPoint that the iterator is currently pointing at

    void Remove();
    // removes the fco pointed to by the iter. Behavior is undefined if the set is empty.
    // after the erase, the iterator points to the next element in the list
private:
    mutable std::set<cFCOName>::iterator mIter;
    std::set<cFCOName>&                  mSet;
};

///////////////////////////////////////////////////////////////////////////////
// cFCOSpecNoChildren -- an iFCOSpecHelper that maps to a single FCO -- the
//      start point
///////////////////////////////////////////////////////////////////////////////
class cFCOSpecNoChildren : public iFCOSpecHelper
{
public:
    virtual bool            ContainsFCO(const cFCOName& name) const;
    virtual iFCOSpecHelper* Clone() const;
    virtual bool            ShouldStopDescent(const cFCOName& name) const;
    virtual void            Read(iSerializer* pSerializer, int32_t version = 0);
    virtual void            Write(iSerializer* pSerializer) const;
    virtual CompareResult   Compare(const iFCOSpecHelper* pRhs) const;
    virtual void            TraceContents(int dl = -1) const;

    DECLARE_TYPEDSERIALIZABLE()

    typedef iFCOSpecHelper inherited;
};

//#############################################################################
// inline implementation
//#############################################################################

//////////////////////////
// iFCOSpecHelper
//////////////////////////
inline void iFCOSpecHelper::SetStartPoint(const cFCOName& startPoint)
{
    mStartPoint = startPoint;
}

inline const cFCOName& iFCOSpecHelper::GetStartPoint() const
{
    return mStartPoint;
}

//////////////////////////
// cFCOSpecStopPointSet
//////////////////////////
inline int cFCOSpecStopPointSet::GetSize() const
{
    return mStopPoints.size();
}

inline void cFCOSpecStopPointSet::Clear()
{
    mStopPoints.clear();
}

//////////////////////////
// cFCOSpecStopPointIter
//////////////////////////
inline cFCOSpecStopPointIter::cFCOSpecStopPointIter(cFCOSpecStopPointSet& set) : mSet(set.mStopPoints)
{
    mIter = mSet.begin();
}
inline void cFCOSpecStopPointIter::SeekBegin() const
{
    mIter = mSet.begin();
}

inline bool cFCOSpecStopPointIter::Done() const
{
    return (mIter == mSet.end());
}

inline bool cFCOSpecStopPointIter::IsEmpty() const
{
    return (mSet.size() == 0);
}

inline void cFCOSpecStopPointIter::Next() const
{
    ++mIter;
}

inline void cFCOSpecStopPointIter::Remove()
{
    ASSERT(!Done());
    mSet.erase(mIter++);
}

inline const cFCOName& cFCOSpecStopPointIter::StopPoint() const
{
    ASSERT(!Done());
    return (*mIter);
}

inline const cFCOName& cFCOSpecStopPointIter::StopPoint()
{
    ASSERT(!Done());
    return (*mIter);
}

#endif //__FCOSPECHELPER_H
