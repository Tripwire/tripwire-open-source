//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
// tasktimer.h
//
// cTaskTimer -- a class that can be used to time a given task
// TODO -- make this cross-platform -- maybe a template class with the time-reaping 
// function as a parameter
#ifndef __TASKTIMER_H
#define __TASKTIMER_H

#ifndef __TYPES_H
#include "types.h"
#endif
#ifndef __DEBUG_H
#include "debug.h"
#endif

#include <string>

///////////////////////////////////////////////////////////////////////////////
// cTaskTimer -- 
///////////////////////////////////////////////////////////////////////////////
template<class TIME_FN, class TIME_TYPE>
class cTaskTimer
{
public:
    cTaskTimer(const TSTRING& name) : mName(name), mTotalTime(0), mStartTime(0), mNumStarts(0) {}
    ~cTaskTimer();

    void                Start();
    void                Stop();
    bool                IsRunning()             { return (mStartTime != 0); }
    void                Reset()                 { mNumStarts = mStartTime = mTotalTime = 0 }
    int32               GetTotalTime() const;
    int32               GetNumTimesStarted() const; // returns the number of times start() was called
    const std::string&  GetName() const;

private:
    TSTRING         mName;
    int32           mTotalTime;
    TIME_TYPE       mStartTime;
    int32           mNumStarts;
};


///////////////////////////////////////////////////////////////////////////////
// cUnixTimeFn -- Unix version, inserts proper function call and overloads
//      operator()
///////////////////////////////////////////////////////////////////////////////
#include <ctime>
class cUnixTimeFn
{
public:
    typedef uint32 DataType;

    uint32 operator()()
    {
        return time( &dummy_var );
    }
private:
    time_t dummy_var;
};

///////////////////////////////////////////////////////////////////////////////
// cUnixTaskTimer -- the Unix typedef...
///////////////////////////////////////////////////////////////////////////////
typedef cTaskTimer<cUnixTimeFn, cUnixTimeFn::DataType> cUnixTaskTimer;
typedef cUnixTaskTimer cGenericTaskTimer;



//-----------------------------------------------------------------------------
// inline implementation
//-----------------------------------------------------------------------------

template<class TIME_FN, class TIME_TYPE>
inline void cTaskTimer<TIME_FN, TIME_TYPE>::Start()
{
    ASSERT(! IsRunning());  
    TIME_FN GetTime;
    mStartTime = GetTime();
    mNumStarts++;
}

template<class TIME_FN, class TIME_TYPE>
inline void cTaskTimer<TIME_FN, TIME_TYPE>::Stop()
{
    ASSERT(IsRunning());    
    TIME_FN GetTime;
    mTotalTime += ( GetTime() - mStartTime );
    mStartTime = 0;
}

template<class TIME_FN, class TIME_TYPE>
inline int32 cTaskTimer<TIME_FN, TIME_TYPE>::GetTotalTime() const
{
    return mTotalTime;
}

template<class TIME_FN, class TIME_TYPE>
inline const std::string& cTaskTimer<TIME_FN, TIME_TYPE>::GetName() const
{
    return mName
}

template<class TIME_FN, class TIME_TYPE>
inline cTaskTimer<TIME_FN, TIME_TYPE>::~cTaskTimer()
{
    // stop the timer if it is currently running
    if(IsRunning())
        Stop();

    // trace out the time contents...
    cDebug d("cTaskTimer");
    d.TraceDebug("----- Time to execute %s: %d (started %d times)\n", mName.c_str(), mTotalTime, mNumStarts);
}


#endif

