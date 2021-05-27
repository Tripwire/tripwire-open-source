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
// parserhelper.h -- helper classes that are called by yacc parser
//

#ifndef __PARSERHELPER_H
#define __PARSERHELPER_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __PARSEROBJECT_H
#include "parserobjects.h"
#endif

#ifndef __GENREPARSEINFO_H
#include "genreparseinfo.h"
#endif

#ifndef __ERRORBUCKETIMPL_H
#include "core/errorbucketimpl.h"
#endif

#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif

#ifndef __GENRESPECLIST_H
#include "fco/genrespeclist.h"
#endif

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class iParserGenreUtil;
class cGenrePolicy;
class cFCOPropVector;

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

////////////////////////////////////////////////////////////
// Parser Base Exception
////////////////////////////////////////////////////////////
TSS_BEGIN_EXCEPTION_NO_CTOR(eParserHelper, eError)
enum
{
    CURRENT_LINE = -1,
    NO_LINE      = 0
};
// if ( nLine != NO_LINE ) appends the "Line number: <nLine>" to the
// end of the strMsg if fShowLineNum is true, then returns string
eParserHelper(const TSTRING& strMsg, int nLine = CURRENT_LINE);
TSS_END_EXCEPTION();

////////////////////////////////////////////////////////////
// Helper Macro For Parser Exceptions
////////////////////////////////////////////////////////////
#define TSS_PARSER_EXCEPTION(except)                                                          \
    TSS_BEGIN_EXCEPTION_NO_CTOR(except, eParserHelper)                                        \
    except(const TSTRING& strMsg, int nLine = CURRENT_LINE) : eParserHelper(strMsg, nLine){}; \
    TSS_END_EXCEPTION()

////////////////////////////////////////////////////////////
// Parser Exceptions
////////////////////////////////////////////////////////////
TSS_PARSER_EXCEPTION(eParseFailed);
TSS_PARSER_EXCEPTION(eParserRedefineVar);
TSS_PARSER_EXCEPTION(eParserUnrecognizedAttr);
TSS_PARSER_EXCEPTION(eParserUnrecognizedAttrVal);
TSS_PARSER_EXCEPTION(eParserPropChar);
TSS_PARSER_EXCEPTION(eParserNoRules);
TSS_PARSER_EXCEPTION(eParserNoRulesInSection);
TSS_PARSER_EXCEPTION(eParserUseUndefVar);
TSS_PARSER_EXCEPTION(eParserBadSevVal);
TSS_PARSER_EXCEPTION(eParserBadFCONameFmt);
TSS_PARSER_EXCEPTION(eParserSectionAlreadyDefined);
TSS_PARSER_EXCEPTION(eParserPointAlreadyDefined);
TSS_PARSER_EXCEPTION(eParserSectionInsideBlock);
TSS_PARSER_EXCEPTION(eParserRelativePath);
TSS_PARSER_EXCEPTION(eParserIgnoringSection);
TSS_PARSER_EXCEPTION(eParserNoStatementsInScope);
TSS_PARSER_EXCEPTION(eParserBadHex);
TSS_PARSER_EXCEPTION(eParserBadCharacter);


///////////////////////////////////////////////////////////////////////////////
// class cPreprocessor
//      used for directives #ifhost, to determine if parserhelper acts on input from parser
//      if current state is STATE_ACCEPT, it will create specs from rules, set variables, etc.
//      if current state is STATE_IGNORE, it will not.
//      -- assumes there is at least one state in stack at all times
class cPreprocessor
{
public:
    cPreprocessor();

    //
    // if we're in accept, then we process
    // input, otherwise we ignore it
    //
    enum AcceptState
    {
        STATE_ACCEPT,
        STATE_IGNORE
    };

    //
    // basic functionality
    //
    static void        PushState(AcceptState state);
    static void        PopState();
    static AcceptState TopState();
    static AcceptState GetState();
    static AcceptState UnderneathTopState(); // gets the item in the stack underneath top item
    static AcceptState GetLastState()
    {
        return UnderneathTopState();
    };
    static void        ToggleTopState();
    static AcceptState GetOppositeState(AcceptState state);
    static bool        AtTopLevel();
    static bool        Empty()
    {
        return mStateStack.empty();
    };
    // returns true if there is only one element in the stack

    static void IgnoreSection()
    {
        mfIgnoreSection = true;
    };
    static void ReadSection()
    {
        mfIgnoreSection = false;
    };

private:
    //
    // data members
    //
    typedef std::stack<AcceptState> AcceptStack;
    static AcceptStack              mStateStack;
    static bool                     mfIgnoreSection;
};

// encapsulates utility functions
class cParserUtil
{
public:
    //
    // general utility functions
    //
    static void InterpretEscapedString(const std::string& strEscapedString, TSTRING& strInterpretedString);
    // converts escaped character sequences in strEscapedString into their escape characters in strInterpretedString
    //( e.g. turns "Hello World!\n" (literal backslash) into "Hello World!<return char>"
    // all C++ escape sequences are recognized:
    //          (1) octal numbers \012 (1, 2, or 3 octal digits),
    //          (2) hex numbers \x2AFB1... ( 'x' followed by one or more hex digits ),
    //          (3) chars: \t, \v, \b, \r, \f, \a, \\, \?, \', and \"
    //          (4) all other escaped chars are treated as if not escaped
    static bool AnyOfTheseHostsExists(cParseStringList* pList);
    // is this machine is in pList?

    static bool GetNextMailName(const TSTRING& strNameList, TSTRING& strNameRet, int index);
    // gets semicolon-delimited words in a string.  index starts at 0.
    static void MergeMailLists(TSTRING& tstrMergeIntoMe, const TSTRING& tstrRHS);
    // merges semicolon-delimited mailnames.  will not add a duplicate name.

    static void
    CreatePropVector(const TSTRING& attr, cFCOPropVector& v, iParserGenreUtil* pHelper); // throw( eParserHelper )
        // takes a string of the form (+pinguam...) and turns it into a file system property vector; throws an error if
        // the string is in an invalid format.  Grows the prop vector if necessary.
    static void
    CreateFCOSpecs(cGenre::Genre g, cGenreParseInfo* pgpi, cFCOSpecList& fcospeclist); // throw( eParserHelper )
        // this uses the internal list of rules and stop points to generate the list of
        //      fco specs.
};

///////////////////////////////////////////////////////////////////////////////
//
// this class exists only to encapsulate the routines to help the parser
//      along -- the goal is to reduce the code that lives in the yacc
//
class cParserHelper
{
public:
    static void Init(cErrorBucket* pE);               // initializes static member vars
    static void Finit(cGenreSpecListVector* pPolicy); // creates policy and cleans up

    //
    // data accesss
    //
    static cErrorBucket* GetErrorBucket()
    {
        ASSERT(mpError != 0);
        return mpError;
    };
    static int GetLineNumber()
    {
        return miLineNum;
    };
    static void IncrementLineNumber()
    {
        ++miLineNum;
    };

    //
    // specify whether to parse entire file for correctness checking
    //
    static void SetParseOnly(bool f)
    {
        mfParseOnly = f;
    };
    static bool ParseOnly()
    {
        return mfParseOnly;
    };

    //
    // genre info
    //
    static cGenreParseInfo* GetGenreInfo();
    static void             SetSection(TSTRING& strSection); // throw( eParserHelper )

    //
    // scoped attribute info
    //
    static void                 PushScope(cAttrScopeInfo* pSI);
    static void                 PopScope();
    static bool                 AtTopScope();
    static cParseNamedAttrList* GetGlobalAttrList();
    static void                 IncrementScopeStatementCount();
    static int                  GetScopeStatementCount();
    static bool                 ScopeEmpty()
    {
        return mScopedAttrs.empty();
    };


    static bool InsertGlobalVariable(const TSTRING& var, const TSTRING& val); //throw( eParserHelper )
        // Insert a variable into mGlobalVarTable.

    static cSymbolTable& GetGlobalVarTable(void)
    {
        return mGlobalVarTable;
    }
    // Returns a reference to mGlobalVarTable.
private:
    static void CleanUp(); // frees memory

    static cSymbolTable mGlobalVarTable;
    // A table for all the global variable definitions.
    //
    // data members
    //
    static cErrorBucket* mpError;
    // holds errors that the parser creates
    static int miLineNum;
    // current line number (set in lexer)
    typedef std::map<cGenre::Genre, cGenreParseInfo*> GenreContainer;
    static GenreContainer                             mAph; // holds only genres that have been defined
        // genre helpers
    typedef std::list<cAttrScopeInfo*> ScopedAttrContainer;
    static ScopedAttrContainer         mScopedAttrs;
    // scoped attributes
    static cGenreParseInfo*    pCurrentGenreInfo;
    static const cGenre::Genre mDefaultGenre;      // default genre for this OS
    static const cGenre::Genre mAcceptedGenres[5]; // what genres we recognize for this OS. terminated with a -1
    static bool                mfParseOnly;
};


#endif
