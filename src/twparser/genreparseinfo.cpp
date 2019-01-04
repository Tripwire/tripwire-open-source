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
// genreparseinfo.cpp -- helper classes that are called by yacc parser
//

#include "stdtwparser.h"

//=========================================================================
// INCLUDES
//=========================================================================

#include "parserhelper.h"
#include "fco/twfactory.h"
#include "fco/fconameinfo.h"
#include "fco/genreswitcher.h"
#include "fco/parsergenreutil.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

//=========================================================================
// METHOD CODE
//=========================================================================


cGenreParseInfo::cGenreParseInfo()
{
    InitPredefinedVariables();
}

cGenreParseInfo::~cGenreParseInfo()
{
    RuleListType::iterator itr;
    for (itr = mRuleList.begin(); itr != mRuleList.end(); ++itr)
    {
        delete *itr;
    }
}


///////////////////////////////////////////////////////////////////////////////
// AddStopPoint
//          NOTE: the list copies the string, so it is safe for the parser to
//          free the lexeme
///////////////////////////////////////////////////////////////////////////////
void cGenreParseInfo::AddStopPoint(const cFCOName& name)
{
    mStopList.push_back(name);
}

///////////////////////////////////////////////////////////////////////////////
// AddRule -- adds the specified rule to our list.  we will eventually call
//          MakeFCOSpecs(), which will use this list as its source.
//
//          NOTE: the list is responsible for freeing the pointers
///////////////////////////////////////////////////////////////////////////////
void cGenreParseInfo::AddRule(const cParseRule* pnode)
{
    mRuleList.push_back(pnode);
}

///////////////////////////////////////////////////////////////////////////////
// RulePointAlreadyDefined -- returns true if the given name was already used
//                             as a start or stop point
///////////////////////////////////////////////////////////////////////////////
bool cGenreParseInfo::RulePointAlreadyDefined(const cFCOName& fcoName)
{
    bool fFound = false;

    // check start points
    for (RuleListType::iterator i = mRuleList.begin(); i != mRuleList.end(); ++i)
    {
        if ((*i)->GetName() == fcoName)
            fFound = true;
    }

    // check stop points
    if (!fFound)
    {
        for (StopListType::iterator j = mStopList.begin(); j != mStopList.end(); ++j)
        {
            if (*j == fcoName)
                fFound = true;
        }
    }

    return (fFound);
}


///////////////////////////////////////////////////////////////////////////////
// InitPredefinedVariables --
///////////////////////////////////////////////////////////////////////////////
void cGenreParseInfo::InitPredefinedVariables()
{
    iParserGenreUtil* pGU = iTWFactory::GetInstance()->CreateParserGenreUtil();

    TSTRING sVarName, sValue;
    int     index = 0;
    while (pGU->EnumPredefinedVariables(index++, sVarName, sValue))
    {
        mLocalPredefVarTable.Insert(sVarName, sValue);
    }

    delete pGU;
}


///////////////////////////////////////////////////////////////////////////////
// DoVarSubstitution()
//      replaces any $(VAR) in string with it's value, unless the $ is escaped with
//      a backslash.  Fails if symbol isn't found
///////////////////////////////////////////////////////////////////////////////

bool cGenreParseInfo::DoVarSubstitution(TSTRING& rval) //throw( eParserHelper )
{
    cDebug d("cConfigFile::DoVarSubst()");
    d.TraceDebug("ORIG: %s\n", rval.c_str());

    // walk through string
    //      look for $(
    //          find matching )
    //          pick out subset
    //          look up in symbol table
    //          substitute subset
    //      continue iterating

    // step through string
    // iterate to (slen-1), because we are looking for a two-character sentinel "$("
    bool fEscaping = false;
    for (TSTRING::size_type i = 0; i < rval.size(); i++)
    {
        TCHAR c = rval[i];

        // is it the "$(" sentinel? (an escaped '$' is not a variable)
        if (c == '$' && !fEscaping)
        {
            c = rval[i + 1];
            if (c == '(')
            {
                // ooh, wow!  it's a variable!  find the end
                bool               found = false;
                TSTRING::size_type j;

                for (j = i + 1; j < rval.size(); j++)
                {
                    if (rval[j] == ')')
                    {
                        found = true;
                        break;
                    }
                }

                // did we find it?
                if (!found)
                {
                    // TODO: throw error
                    return false;
                }

                // otherwise, cut out the variable name
                TSTRING::size_type begin = i + 2;
                TSTRING::size_type size  = j - i - 2;
                TSTRING            varname;
                varname = rval.substr(begin, size);

                d.TraceDebug("symbol = %s\n", varname.c_str());

                // look up in symbol table
                TSTRING varvalue;
                if (!LookupVariable(varname, varvalue))
                    throw eParserUseUndefVar(varname);

                // replace varname with varvalue
                rval.replace(begin - 2, size + 3, varvalue);

                d.TraceDebug("POST: %s\n", rval.c_str());

                // no no no
                // we should exit function, and get called again


                // update counters
                //      we should bump the cursor over by the length of the
                //          varvalue (minus one, to compensate for post-increment of index)
                i += varvalue.size() - 1;
                goto nextchar;
            }
        }
        else if (c == '\\')
        {
            fEscaping = !fEscaping;
        }
        else
        {
            fEscaping = false;
        }
    nextchar:;
    }


    d.TraceDebug("DONE: %s\n", rval.c_str());

    // switch around
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Insert -- add definition to symbol table
///////////////////////////////////////////////////////////////////////////////
bool cGenreParseInfo::InsertVariable(const TSTRING& var, const TSTRING& val) //throw( eParserHelper )
{
    TSTRING dummy;
    // don't let them redefine predefined vars
    if (mLocalPredefVarTable.Lookup(var, dummy))
        throw eParserRedefineVar(var);

    return mLocalVarTable.Insert(var, val);
}

///////////////////////////////////////////////////////////////////////////////
// Lookup -- returns value, or returns false
//      This method will return false only if the given variable is not defined
//      in any of the three symbol tables.
///////////////////////////////////////////////////////////////////////////////
bool cGenreParseInfo::LookupVariable(const TSTRING& var, TSTRING& val)
{
    if (mLocalPredefVarTable.Lookup(var, val) || mLocalVarTable.Lookup(var, val))
        return true;
    else
        return (cParserHelper::GetGlobalVarTable().Lookup(var, val));
}
