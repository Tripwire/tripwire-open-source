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
// genreparseinfo.h -- holds parsing information for each genre
//

#ifndef __GENREPARSEINFO_H
#define __GENREPARSEINFO_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __ERROR_H
#include "core/error.h"
#endif

#ifndef __HASHTABLE_H
#include "core/hashtable.h"
#endif

#ifndef __PARSEROBJECTS_H
#include "parserobjects.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

class cSymbolTable
{
public:
    bool Insert(const TSTRING& var, const TSTRING& val)
    {
        return mTable.Insert(var, val);
    }
    bool Lookup(const TSTRING& var, TSTRING& val)
    {
        return mTable.Lookup(var, val);
    }

private:
    cHashTable<TSTRING, TSTRING> mTable;
};

///////////////////////////////////////////////////////////////////////////////
//
// this class exists only to encapsulate the routines to help the parser
//      along -- the goal is to reduce the code that lives in the yacc
//

class cGenreParseInfo
{
public:
    cGenreParseInfo();
    ~cGenreParseInfo();

    void AddStopPoint(const cFCOName& name);
    // adds the specified path as a stop point -- for now, we just queue it up,
    //      since we don't know which rule to attach it to.  to do that, we call
    //      AttachStopPoints()

    bool RulePointAlreadyDefined(const cFCOName& fcoName);
    // checks if szName is in mSpecNameList

    void AddRule(const cParseRule* pnode);
    // adds the specified rule to our list.  we will eventually call CreateFCOSpecs(),
    //      which will use the list as its source.

    bool DoVarSubstitution(TSTRING& rval); //throw( eParserHelper )

    bool InsertVariable(const TSTRING& var, const TSTRING& val); //throw( eParserHelper )
    bool LookupVariable(const TSTRING& var, TSTRING& val);
    // This method now looks in the global variable table (defined in cParserHelper ),
    // if a variable is not found in the local or predefined tables.

    //
    // data retreval
    //
    typedef std::list<cFCOName>          StopListType;
    typedef std::list<const cParseRule*> RuleListType;
    RuleListType*                        GetRules()
    {
        return &mRuleList;
    };
    StopListType* GetStopList()
    {
        return &mStopList;
    };

    cSymbolTable& GetPredefVarTable(void)
    {
        return mLocalPredefVarTable;
    }

private:
    void InitPredefinedVariables();


    StopListType mStopList;
    // this is where all the stop points are stored at parse-time, since we
    // don't know enough to bind them to fcospecs yet.

    RuleListType mRuleList;
    // these are the list of parser generated rules

    cSymbolTable mLocalVarTable;
    // contains all the variable symbol definitions
    cSymbolTable mLocalPredefVarTable;
    // Contains the predefined variable definitions.
};

#endif // __GENREPARSEINFO_H
