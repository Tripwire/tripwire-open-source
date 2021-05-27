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
// cmdlineparser.cpp
#include "stdcore.h"
#include "cmdlineparser.h"
#include "corestrings.h"


///////////////////////////////////////////////////////////////////////////////
// ctor, dotr
///////////////////////////////////////////////////////////////////////////////
cCmdLineParser::cCmdLineParser() : mArgTable(HASH_VERY_SMALL), mLastArgInfo(-1, PARAM_NONE)
{
}

cCmdLineParser::~cCmdLineParser()
{
}

///////////////////////////////////////////////////////////////////////////////
// AddArg
///////////////////////////////////////////////////////////////////////////////
void cCmdLineParser::AddArg(
    int argId, const TSTRING& arg, const TSTRING& alias, ParamCount numParams, bool multipleAllowed)
{
    if (arg.empty() && alias.empty())
    {
        // this refers to the list of parameters that comes after all the cmd line switches
        mLastArgInfo.mId        = argId;
        mLastArgInfo.mNumParams = numParams;
        return;
    }

    if (!arg.empty())
        mArgTable.Insert(arg, cArgInfo(argId, numParams));
    if (!alias.empty())
    {
        // put the alias in the table with a '-' prepended to it so it matches '--'
        TSTRING str(_T("-"));
        str += alias;
        mArgTable.Insert(str, cArgInfo(argId, numParams));
    }
    // This argument can appear more than once on the command line.
    if (multipleAllowed)
        mMultipleAllowed.insert(argId);
}

///////////////////////////////////////////////////////////////////////////////
// Clear
///////////////////////////////////////////////////////////////////////////////
/*void cCmdLineParser::Clear()
{
    mLastArgInfo.mId        = -1;
    mLastArgInfo.mNumParams = PARAM_INVALID;
    mArgTable.Clear();
    mArgData.clear();
    mMutExList.clear();
}*/

///////////////////////////////////////////////////////////////////////////////
// Parse
///////////////////////////////////////////////////////////////////////////////
void cCmdLineParser::Parse(int argc, const TCHAR* const* argv)
{
    // clear out any existing data
    mArgData.clear();

    const TCHAR* pCurArg       = 0;
    bool bProcessedFinalParams = false; // gets set to true when the parameters to the command line are processed

    // I assume argv[0] is the executable name...
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == _T('-'))
        {
            pCurArg = argv[i];

            // this is a switch; find it in the table...
            cArgInfo argInfo;
            if (!mArgTable.Lookup(TSTRING(&argv[i][1]), argInfo))
            {
                // unknown switch!
                throw eCmdLineInvalidArg(TSS_GetString(cCore, core::STR_ERR2_BAD_ARG_PARAMS) + pCurArg);
            }
            //
            // make sure this hasn't been specified yet...
            //
            if (ArgInList(argInfo.mId))
            {
                // Make sure it isn't okay for this one to appear more than once...
                std::set<int>::iterator it = mMultipleAllowed.find(argInfo.mId);
                if (it == mMultipleAllowed.end())
                {
                    // It wasn't in our list of allowed params, so error.
                    throw eCmdLineMultiArg(TSS_GetString(cCore, core::STR_ERR2_BAD_ARG_PARAMS) + argv[i]);
                }
            }
            //
            // add it to the list..
            //
            mArgData.push_back(cArgData(argInfo.mId, TSTRING(argv[i])));
            cArgData& curArg = mArgData.back();
            switch (argInfo.mNumParams)
            {
            case PARAM_NONE:
                // make sure there are no parameters to this, but be careful because
                // it is legal to start the parameters to the executable here.
                if ((i + 1 < argc) && (argv[i + 1][0] != _T('-')))
                {
                    // search for any more parameters
                    // TODO: In the future we may want to support a '--' switch that specifies the start
                    // of parameters to the executable.
                    for (int j = i + 2; j < argc; ++j)
                    {
                        if (argv[j][0] == _T('-'))
                        {
                            // >0 parameter passed !
                            throw eCmdLineBadParam(TSS_GetString(cCore, core::STR_ERR2_BAD_ARG_PARAMS) + pCurArg);
                        }
                    }
                }
                break;

            case PARAM_ONE:
                // get the next parameter...
                i++;
                if ((i >= argc) || (argv[i][0] == _T('-')))
                {
                    // zero parameters passed to something that needed one param
                    throw eCmdLineBadParam(TSS_GetString(cCore, core::STR_ERR2_BAD_ARG_PARAMS) + pCurArg);
                }

                curArg.mParams.push_back(TSTRING(argv[i]));
                break;

            case PARAM_MANY:
                i++;
                while ((i < argc) && (argv[i][0] != _T('-')))
                {
                    curArg.mParams.push_back(TSTRING(argv[i]));
                    i++;
                }
                i--; // since we have gone too far at this point
                break;

            default:
                ASSERTMSG(false, "Unknown number of arguments to parser");
            }
        }
        else
        {
            bProcessedFinalParams = true;
            // this must be the final "unnamed" arg
            // first, make sure it is consistent with the current info...
            bool bResult = true;
            switch (mLastArgInfo.mNumParams)
            {
            case PARAM_NONE:
                // this is an error; they didn't want any command line parameters...
                bResult = false;
                break;
            case PARAM_ONE:
                if (i + 1 != argc)
                    // there is >1 final parameter; it is an error
                    bResult = false;
                break;
            case PARAM_MANY:
                // we'll catch errors below
                break;
            default:
                ASSERT(false);
            }
            if (!bResult)
            {
                throw eCmdLineBadParam();
            }

            // ok, we can push the final parameter info onto the list...
            mArgData.push_back(cArgData(mLastArgInfo.mId));
            cArgData& curArg = mArgData.back();

            while (i < argc)
            {
                if (argv[i][0] == _T('-'))
                {
                    if (!pCurArg)
                    {
                        throw eCmdLineBadSwitchPos(TSS_GetString(cCore, core::STR_ERR2_BAD_ARG_PARAMS) + argv[i]);
                    }
                    else
                    {
                        // there was an extra parameter passed somewhere!
                        throw eCmdLineBadArgParam(TSS_GetString(cCore, core::STR_ERR2_BAD_ARG_PARAMS) + pCurArg);
                    }
                }

                // add this param to the list
                curArg.mParams.push_back(TSTRING(argv[i]));
                i++;
            }
        }
    }

    // it is possible not to process the final command line parameters in the "else" case above
    // (this only occurs if there are no command line parameters specified) so let's make sure that
    // is consistent with what we are configured with...
    // NOTE -- it is ok to have no cmd line parameters if they specified PARAM_NONE or PARAM_MANY
    if (!bProcessedFinalParams)
    {
        if (mLastArgInfo.mNumParams == PARAM_ONE)
        {
            throw eCmdLineBadParam();
        }
    }

    // Check for "relationship errors":
    TestMutEx();
    TestDependency();
}

///////////////////////////////////////////////////////////////////////////////
// TestMutEx
///////////////////////////////////////////////////////////////////////////////
void cCmdLineParser::TestMutEx()
{
    std::list<std::pair<int, int> >::const_iterator i;
    cCmdLineIter                                    iter1(*this), iter2(*this);
    for (i = mMutExList.begin(); i != mMutExList.end(); i++)
    {
        //TODO -- there is a much more efficent way to do this (using cFCOPropVector, for example)
        //      the command line is presumably small enough, tho, that it probably isn't a big
        //      deal to do it this way.
        iter1.SeekToArg(i->first);
        if (!iter1.Done())
        {
            iter2.SeekToArg(i->second);
            if (!iter2.Done())
            {
                // we have a mutual exclusion violation!
                throw eCmdLineMutEx(iter1.ActualParam() + _T(", ") + iter2.ActualParam());
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// TestDependency
///////////////////////////////////////////////////////////////////////////////
void cCmdLineParser::TestDependency()
{
    std::list<std::pair<std::pair<int, int>, bool> >::const_iterator i;
    cCmdLineIter                                                     iter1(*this), iter2(*this);

    for (i = mDependencyList.begin(); i != mDependencyList.end(); ++i)
    {
        iter1.SeekToArg(i->first.first);
        // was it on the command line?
        if (!iter1.Done())
        {
            // it was, is the corresponding arg on the command line?
            iter2.SeekToArg(i->first.second);
            if (iter2.Done()) // it wasn't, dependency error
            {
                TSTRING arg1, arg2, alias1, alias2;
                cCmdLineParser::LookupArgInfo(i->first.first, arg1, alias1);
                cCmdLineParser::LookupArgInfo(i->first.second, arg2, alias2);

                // determine in which form the user passed the arguments,
                // and construct the error message in the same form
                if (iter1.ActualParam().length() == 2)
                    throw eCmdLineDependency(_T("The switch -") + arg1 + _T(" requires -") + arg2 + _T("."));
                else
                    throw eCmdLineDependency(_T("The switch --") + alias1 + _T(" requires --") + alias2 + _T("."));
            }
        }
        else if (i->second)
        // only make this second check if the dependencies are MUTUAL,
        // as indicated (or not) by the bool value.
        {
            iter2.SeekToArg(i->first.second);
            // the first arg in the pair was not on the command line,
            // so just make sure the second isn't there...
            if (!iter2.Done())
            {
                // arg2 appeared without arg1, so dependency error.
                TSTRING arg1, arg2, alias1, alias2;
                cCmdLineParser::LookupArgInfo(i->first.first, arg1, alias1);
                cCmdLineParser::LookupArgInfo(i->first.second, arg2, alias2);

                // determine in which form the user passed the arguments,
                // and construct the error message in the same form
                if (iter1.ActualParam().length() == 2)
                    throw eCmdLineDependency(_T("The switch -") + arg2 + _T(" requires -") + arg1 + _T("."));
                else
                    throw eCmdLineDependency(_T("The switch --") + alias2 + _T(" requires --") + alias1 + _T("."));
            }
        }

    } //end for
}


///////////////////////////////////////////////////////////////////////////////
// AddMutEx
///////////////////////////////////////////////////////////////////////////////
void cCmdLineParser::AddMutEx(int argId1, int argId2)
{
    // note that I do no checking for duplicates here...
    std::pair<int, int> mutEx(argId1, argId2);
    ASSERT(argId1 != argId2);
    mMutExList.push_back(mutEx);
}

///////////////////////////////////////////////////////////////////////////////
// AddDependency
///////////////////////////////////////////////////////////////////////////////
void cCmdLineParser::AddDependency(int argId1, int argId2, bool mutual)
{
    // again, no checking for duplicates... would a set
    // prove to be a better container for this operation?
    std::pair<int, int>                  Args(argId1, argId2);
    std::pair<std::pair<int, int>, bool> Dep(Args, mutual);

    ASSERT(argId1 != argId2);
    mDependencyList.push_back(Dep);
}

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
void cCmdLineParser::TraceContents(int dl)
{
    cDebug d("cCmdLineParser::TraceContents");
    if (dl == -1)
        dl = cDebug::D_DEBUG;

    std::list<cArgData>::const_iterator i;
    for (i = mArgData.begin(); i != mArgData.end(); i++)
    {
        d.Trace(dl, "* Item id:%d\n", i->mId);
        for (std::vector<TSTRING>::const_iterator vi = i->mParams.begin(); vi != i->mParams.end(); vi++)
        {
            d.Trace(dl, "\t%s\n", vi->c_str());
        }
    }

    d.Trace(dl, "--- Switch id table ---\n");

    cHashTableIter<TSTRING, cArgInfo> iter(mArgTable);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        d.Trace(dl, "[%d] %s\n", iter.Val().mId, iter.Key().c_str());
    }

    d.Trace(dl, "[%d] Final Param List\n", mLastArgInfo.mId);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// LookupArgInfo
///////////////////////////////////////////////////////////////////////////////
bool cCmdLineParser::LookupArgInfo(int argId, TSTRING& arg, TSTRING& alias) const
{
    arg   = _T("");
    alias = _T("");

    cHashTableIter<TSTRING, cArgInfo> iter(mArgTable);
    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        if (iter.Val().mId == argId)
        {
            TSTRING str = iter.Key();
            if ((str.length() > 0) && (str[0] == _T('-')))
            {
                // this is the alias!
                alias = (str.c_str() + 1);
            }
            else
            {
                // this is the arg...
                arg = str;
            }
        }
    }
    return ((!arg.empty()) || (!alias.empty()));
}

///////////////////////////////////////////////////////////////////////////////
// ArgInList
///////////////////////////////////////////////////////////////////////////////
bool cCmdLineParser::ArgInList(int argId)
{
    std::list<cArgData>::iterator i;
    for (i = mArgData.begin(); i != mArgData.end(); i++)
    {
        if (i->mId == argId)
            return true;
    }
    return false;
}


//#############################################################################
// cCmdLineIter
//#############################################################################

///////////////////////////////////////////////////////////////////////////////
// SeekToArg
///////////////////////////////////////////////////////////////////////////////
bool cCmdLineIter::SeekToArg(int argId) const
{
    for (SeekBegin(); !Done(); Next())
    {
        if (ArgId() == argId)
            return true;
    }

    return false;
}
