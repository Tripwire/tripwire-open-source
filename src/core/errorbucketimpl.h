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
// errorbucketimpl.h
//
// This component contains classes derived from cErrorBucket and helper classes.
// They were broken out of errorbucket.h becuase many components will only need to
// know about cErrorBucket, so we reduce system dependencies by keeping these classes
// separate.
#ifndef __ERRORBUCKETIMPL_H
#define __ERRORBUCKETIMPL_H

#ifndef __ERRORBUCKET_H
#include "errorbucket.h"
#endif

#ifndef __DEBUG_H
#include "debug.h"
#endif

#ifndef __ERROR_H
#include "error.h"
#endif

#ifndef __ERRORUTIL_H
#include "errorutil.h"
#endif

#ifndef __SERIALIZABLE_H
#include "serializable.h"
#endif

//////////////////////////////////////////////////////
// cErrorReporter -- sends all error messages to
//      stderr
//////////////////////////////////////////////////////
class cErrorReporter : public cErrorBucket
{
public:
    static void PrintErrorMsg(const eError& error, const TSTRING& strExtra = _T(""));
    // function that HandleError() uses to print the error messages to stderr.
    // this function uses the current authoritative format for error reporting, so
    // other functions needing to display errors to the user should use this.
    //

    // NOTE:bam 5/7/99 -- I don't think the below is true anymore?
    // NOTE:mdb -- if the error has an ID of zero, nothing will be printed. This
    //      is a way to throw a fatal error where the error reporting has already
    //      occurred.

protected:
    virtual void HandleError(const eError& error);
};

///////////////////////////////////////////////////////
// cErrorTracer -- traces all errors with the D_ERROR debug
//      level
///////////////////////////////////////////////////////
class cErrorTracer : public cErrorBucket
{
protected:
    virtual void HandleError(const eError& error);
};


//////////////////////////////////////////////////////
// cErrorQueue -- keeps track of all the errors that
//      are reported to it, providing an interface for
//      retrieving them at a later time
//////////////////////////////////////////////////////
class cErrorQueue : public cErrorBucket, public iTypedSerializable
{
    friend class cErrorQueueIter;

public:
    typedef std::list<ePoly> ListType;
    
    void Clear();
    // remove all errors from the queue
    ListType::size_type GetNumErrors() const;
    // returns how many errors are in the queue

    //
    // iSerializable interface
    //
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    //
    // Debugging
    //
    void TraceContents(int dl = -1) const;

protected:
    virtual void HandleError(const eError& error);

private:
    ListType                 mList;

    DECLARE_TYPEDSERIALIZABLE()
};

class cErrorQueueIter
{
public:
    cErrorQueueIter(cErrorQueue& queue);
    cErrorQueueIter(const cErrorQueue& queue);
    ~cErrorQueueIter()
    {
    }

    // iteration methods
    void SeekBegin();
    void Next();
    bool Done() const;

    // access to the error
    const ePoly& GetError() const;
    // both of these return results are undefined if the iterator
    // is not valid (ie - IsDone() == true)
private:
    cErrorQueue::ListType&          mList;
    cErrorQueue::ListType::iterator mIter;
};

//////////////////////////////////////////////////////
// cErrorBucketNull -- an error bucket that plays the
//      role of /dev/null
//////////////////////////////////////////////////////
class cErrorBucketNull : public cErrorBucket
{
    virtual void AddError(const eError&)
    {
    }

protected:
    virtual void HandleError(const eError&)
    {
    }
};

//////////////////////////////////////////////////////
// cErrorBucketPassThru -- does nothing with errors;
//      just passes them on to its children
//////////////////////////////////////////////////////
class cErrorBucketPassThru : public cErrorBucket
{
protected:
    virtual void HandleError(const eError&)
    {
    }
};


#endif
