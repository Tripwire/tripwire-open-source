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
// cmdlineparser.h
//
// cCmdLineParser.h -- class that tokenizes the command line
#ifndef __CMDLINEPARSER_H
#define __CMDLINEPARSER_H

#ifndef __HASHTABLE_H
#include "hashtable.h"
#endif
#ifndef __DEBUG_H
#include "debug.h"
#endif
#ifndef __ERROR_H
#include "error.h"
#endif

//=============================================================================
// eCmdLine
//=============================================================================
TSS_EXCEPTION(eCmdLine, eError)
TSS_EXCEPTION(eCmdLineInvalidArg, eCmdLine)  // an arg on the command line is not recognized
TSS_EXCEPTION(eCmdLineBadArgParam, eCmdLine) // wrong number of parameters to an argument
TSS_EXCEPTION(eCmdLineBadParam,
              eCmdLine) // wrong number of paramters to the executable (not associated with any arguments)
TSS_EXCEPTION(eCmdLineBadSwitchPos, eCmdLine) // a '-' arg appeared after the final parameter list
TSS_EXCEPTION(eCmdLineMutEx, eCmdLine)        // a mutual exclusion error has occured
TSS_EXCEPTION(eCmdLineDependency, eCmdLine)   // a dependency error has occurred.
TSS_EXCEPTION(eCmdLineMultiArg, eCmdLine)     // an arg was found twice in the command line


/*
    // cCmdLineParser owns errors 600-699
    // these can be turned into a string by using cErrorTable
    enum ErrorType
    {
        ERR_NONE                    = 601,  // no error
        ERR_INVALID_ARG             = 602,  // an arg on the command line is not recognized
        ERR_BAD_ARG_PARAMS          = 603,  // wrong number of parameters to an argument
        ERR_BAD_PARAMS              = 604,  // wrong number of paramters to the executable (not associated with any arguments)
        ERR_SWITCH_AFTER_FINAL_LIST = 605,  // a '-' arg appeared after the final paramter list
        ERR_MUTUAL_EXCLUSION        = 606,  // a mutual exclusion error has occured
        ERR_MULTIPLE_ARG            = 607,  // an arg was found twice in the command line

        ERR_INVALID         // top of enum
    };

    // for storing error information
    ErrorType                       mCurError;
    TSTRING                         mCurErrorString;
    void GetErrorInfo(ErrorType& et, TSTRING& errorData) const;
        // returns information on the type of error that occured in a Parse() command. Only 
        // returns valid information if Parse() had just been called and returned false. A 
        // second call to Parse() might alter existing error info
*/

//=============================================================================
// cCmdLineParser
//=============================================================================
class cCmdLineParser
{
public:
    cCmdLineParser();
    ~cCmdLineParser();

    enum ParamCount
    {
        PARAM_NONE, // no parameters to arg
        PARAM_ONE,  // one parameter to arg
        PARAM_MANY, // zero or more paramters to arg

        PARAM_INVALID // top of enum
    };

    void
    AddArg(int argId, const TSTRING& arg, const TSTRING& alias, ParamCount numParams, bool multipleAllowed = false);
    // this method should be called for each argument that can appear on the
    // command line.
    // argId -- a number that uniquely identifies the argument; no two arguments
    //      may have the same id (ASSERT-enforced)
    // arg   -- string that comes after the '-'. can be _T("") if there is only
    //      a string representation
    // alias -- string that comes after '--' which has the same meaning. Can be _T("")
    //      if there is no alias. If both arg and alias are empty strings, then this arg
    //      represents the list of arguments that comes at the end of the command line
    // numParams -- number of parameters that this argument needs

    void AddMutEx(int argId1, int argId2);
    // this adds a mutual exclusion constraint. When Parse() is called, if argId1 and
    // argId2 both exist on the command line, then the parse will fail and the error
    // value ERR_MUTUAL_EXCLUSION will be set.

    void AddDependency(int argId1, int argId2, bool mutual = false);
    // This adds a dependency constraint.   When Parse() is called, if argId1
    // exists on the command line independent from argId2, then the parse will fail.
    // If the default param mutual is true, then the command parser will check for
    // argId1 if argId2 is passed.   We do this, since it is possible for one arg to
    // depend on another, but have the other arg alone on the command line, legally.

    void Parse(int argc, const TCHAR* const* argv); // throw eCmdLine
        // after AddArg() has been called for every argument that could be processed by the
        // command line, call this to tokenize argv. If the return value is false, then
        // the input was invalid in some way; the actual error can be determined by calling
        // GetErrorInfo() below.

    //    void Clear();
    // clear out all information that this class contains

    bool LookupArgInfo(int argId, TSTRING& arg, TSTRING& alias) const;
    // given an argId, fill out the strings with the argument and alias strings. Returns false
    // if the argId cannot be found. This method is not very fast, so don't use it often.

#ifdef DEBUG
    void TraceContents(int dl = -1);
#endif
private:
    void TestMutEx();
    // tests for mutual exclusion violations; if it fails, the current error is set and false
    // is returned.
    void TestDependency();
    // tests for all dependency violations.
    bool ArgInList(int argId);
    // returns true if an argument with the specified id already exists in the list; this is used
    // to make sure the same arg doesn't appear >1 time on the command line

    // for storing information on paramers
    struct cArgInfo
    {
        int        mId;
        ParamCount mNumParams;

        cArgInfo(int i = -1, ParamCount p = PARAM_INVALID) : mId(i), mNumParams(p)
        {
        }
    };
    // for storing parsed argv information
    struct cArgData
    {
        int                  mId;
        std::vector<TSTRING> mParams;
        TSTRING              mActualParam; // a string representation of what was actually on the command line

        cArgData(int id = -1, const TSTRING& actualParam = TSTRING(_T(""))) : mId(id), mActualParam(actualParam)
        {
        }
    };

    cHashTable<TSTRING, cArgInfo> mArgTable;
    cArgInfo
                                                     mLastArgInfo; // info on the argument that comes at the end of the command line (with no associated '-x' or '--x')
    std::list<cArgData>                              mArgData;
    std::list<std::pair<int, int> >                  mMutExList;      // all of the mutual exclusions
    std::list<std::pair<std::pair<int, int>, bool> > mDependencyList; // all of the dependencies
    std::set<int>                                    mMultipleAllowed;

    friend class cCmdLineIter;
};

///////////////////////////////////////////////////////////////////////////////
// cCmdLineIter -- used to iterate over the tokenized command line parameters;
//      is only useful after cCmdLineParser::Parse() has been called.
///////////////////////////////////////////////////////////////////////////////
class cCmdLineIter
{
public:
    cCmdLineIter(const cCmdLineParser& parser);

    // iteration
    void SeekBegin() const;
    bool Done() const;
    bool IsEmpty() const;
    void Next() const;

    bool SeekToArg(int argId) const;
    // seeks to the argument with the given argId. returns
    // false and Done() == true if it couldn't find it.

    // access to the argument data
    int ArgId() const;
    // returns the id of this arg; ASSERTs if Done() == true
    int NumParams() const;
    // returns the number of parameters this argument has
    const TSTRING& ActualParam() const;
    // returns exactly what was passed on the command line (ie -- what the user typed)
    const TSTRING& ParamAt(int index) const;
    // returns the parameter at the specified index. ASSERTs if
    // the index is out of range.

private:
    const std::list<cCmdLineParser::cArgData>&                  mList;
    mutable std::list<cCmdLineParser::cArgData>::const_iterator mIter;
};

//#############################################################################
// inline implementation
//#############################################################################
inline cCmdLineIter::cCmdLineIter(const cCmdLineParser& parser) : mList(parser.mArgData)
{
    SeekBegin();
}
inline void cCmdLineIter::SeekBegin() const
{
    mIter = mList.begin();
}
inline bool cCmdLineIter::Done() const
{
    return (mIter == mList.end());
}
inline bool cCmdLineIter::IsEmpty() const
{
    return (mList.size() == 0);
}
inline void cCmdLineIter::Next() const
{
    mIter++;
}
inline int cCmdLineIter::ArgId() const
{
    ASSERT(!Done());
    return mIter->mId;
}
inline int cCmdLineIter::NumParams() const
{
    ASSERT(!Done());
    return mIter->mParams.size();
}

inline const TSTRING& cCmdLineIter::ActualParam() const
{
    ASSERT(!Done());
    return mIter->mActualParam;
}

inline const TSTRING& cCmdLineIter::ParamAt(int index) const
{
    ASSERT((index >= 0) && (index < NumParams()));
    return mIter->mParams[index];
}


#endif
