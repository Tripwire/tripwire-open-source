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
// parserobjects.h
//

#ifndef __PARSEROBJECTS_H
#define __PARSEROBJECTS_H

//=========================================================================
// INCLUDES
//=========================================================================

#include "twparserstrings.h"

#ifndef __FCONAME_H
#include "fco/fconame.h"
#endif

#ifndef __FCOPROPVECTOR_H
#include "fco/fcopropvector.h"
#endif

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class iParserGenreUtil;

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

#define KILL_LIST(type, name)                                                              \
    do                                                                                     \
    {                                                                                      \
        for (std::list<type*>::iterator iter = mList.begin(); iter != mList.end(); ++iter) \
        {                                                                                  \
            delete *iter;                                                                  \
        }                                                                                  \
    } while (0)


///////////////////////////////////////////////////////////////////////////////
// parser representation of named attributes
//      these {name, value} pairs can be associated with either rules, or rule masks.
//      the implication is that each rule masks inherits the attributes of its parents,
//      but can override them.
//
//      Has knowledge of allowable attribute names and their allowable values
//      Validate() throws error if name is unrecognized or value is outside allowable range
//

class cParseNamedAttr
{
public:
    //
    // ctors and dtors
    //
    cParseNamedAttr(const TSTRING& strName, const TSTRING& strValue); // converts strName to lower before storing
    cParseNamedAttr* Clone() const;

    const TSTRING& GetName() const
    {
        return mstrName;
    };
    const TSTRING& GetValue() const
    {
        return mstrValue;
    };
    int GetLineNum() const
    {
        return mLineNum;
    };
    void SetValue(const TSTRING& str)
    {
        mstrValue = str;
    };

    void Validate() const;

    //
    // debug functions
    //
    void Dump(cDebug& d) const; // debugging routine

private:
    //
    // dont' let C++ create these
    //
    cParseNamedAttr(const cParseNamedAttr& rhs);
    cParseNamedAttr& operator=(const cParseNamedAttr& rhs);

    //
    // enums
    //
    enum ValidAttrNames
    {
        NAME_1   = twparser::STR_PARSER_RULENAME,
        NAME_2   = twparser::STR_PARSER_EMAILTO,
        NAME_3   = twparser::STR_PARSER_RECURSE,
        NAME_4   = twparser::STR_PARSER_SEVERITY,
        MIN_NAME = NAME_1,
        MAX_NAME = NAME_4
    };

    bool NamesMatch(int nString, const TSTRING& strName) const;

    //
    // data members
    //
    TSTRING mstrName;
    TSTRING mstrValue;
    int     mLineNum;
};


///////////////////////////////////////////////////////////////////////////////
// wrapper around list routines
class cParseNamedAttrList
{
public:
    //
    // ctors and dtors
    //
    cParseNamedAttrList(){};
    ~cParseNamedAttrList();

    //
    // typedefed iters
    //
    typedef std::list<cParseNamedAttr*>::const_iterator         constListIter;
    typedef std::list<cParseNamedAttr*>::const_reverse_iterator constListRevIter;

    //
    // general functionality
    //
    void                   Dump(cDebug& d) const; // dump entire list
    void                   Add(cParseNamedAttr* const pa);
    const cParseNamedAttr* Lookup(const TSTRING& tstrAttrName) const;
    void                   Clear();
    void                   MergeNoOverwrite(const cParseNamedAttrList* const pal);

private:
    //
    // private utils
    //
    constListIter Find(const cParseNamedAttr* const pa);

    //
    // dont' let C++ create these
    //
    cParseNamedAttrList(const cParseNamedAttrList& rhs);
    cParseNamedAttrList& operator=(const cParseNamedAttrList& rhs);


public:
    //
    // data members
    //
    std::list<cParseNamedAttr*> mList;
};


///////////////////////////////////////////////////////////////////////////////
// parser representation of fcospecmasks
//      specmasks consist of sets of {condition, propvector} pairs, and also have associated
//      with them a list of {attributes}.
class cParseSpecMask
{
public:
    //
    // ctor and dtor
    //
    cParseSpecMask(TSTRING& condition, TSTRING& vector, iParserGenreUtil* pHelper); // throw( eParserHelper )
    cParseSpecMask(const cParseSpecMask& sm)
    {
        *this = sm;
    };
    ~cParseSpecMask();

    //
    // data access
    //
    void                  SetAttrList(cParseNamedAttrList*); // attach list of named attributes
    cParseNamedAttrList*  GetAttrList() const;
    int                   GetNumAttrs() const;   // number of named attributes
    const cFCOPropVector& GetPropVector() const; // get prop vector (only used when parsing default specmasks)
    const TSTRING&        GetPropVectorString() const;
    const TSTRING&        GetCondition() const; // get condition

    //
    // debug functionality
    //
    void Dump(cDebug& d) const; // debugging routine

private:
    //
    // data members
    //
    TSTRING              msCondition; // TODO: this shouldn't be a string -- should be expression
    TSTRING              msPV;
    cFCOPropVector       mPropVector;
    cParseNamedAttrList* mpAttrList; // list of named attributes associated with specmask
};

///////////////////////////////////////////////////////////////////////////////
// wrapper around list routines
//      we do this so yacc can have something convenient to carry around
class cParseSpecMaskList
{
public:
    ~cParseSpecMaskList()
    {
        KILL_LIST(cParseSpecMask, mList);
    };

    void Dump(cDebug& d) const; // dump entire list
    void Add(cParseSpecMask* const pa)
    {
        mList.push_back(pa);
    };

    std::list<cParseSpecMask*> mList;
};


// parser representation of fcospecs, which is composed of...
//      a rule name, a set of specmasks, a default specmask (required), and a set of named attributes
class cParseRule
{
public:
    //
    // ctors and dtor
    //
    explicit cParseRule(const cParseSpecMask& defSpecMask)
        : mDefSpecMask(defSpecMask), mpAttrList(0), mpSpecMaskList(0), mName(_T(""))
    {
    }
    ~cParseRule();

    //
    // data access
    //
    const cFCOName&      GetName() const; // rule name
    void                 SetName(const cFCOName& name);
    cParseNamedAttrList* GetAttrList() const; // rule attributes
    void                 SetAttrList(cParseNamedAttrList*);
    void                 SetSpecMaskList(cParseSpecMaskList* pmask);
    // attach a list of specifiers to this rule (excluding default spec)
    int GetNumSpecMasks() const;
    // returns number of spec masks (excluding default rule)
    int GetNumNamedAttrs() const;
    // returns number of named attributes
    const cParseSpecMask& GetDefSpecMask() const
    {
        return mDefSpecMask;
    };

    //
    // debugging functions
    //
    void Dump();

private:
    //
    // debugging functions
    //
    cParseSpecMask       mDefSpecMask;   // default specifier
    cParseNamedAttrList* mpAttrList;     // list of named attributes
    cParseSpecMaskList*  mpSpecMaskList; // list of specifiers
    cFCOName             mName;          // name of fcospec
};


///////////////////////////////////////////////////////////////////////////////
// wrapper around TSTRING so yacc can have it inside the yylval union
//
class cParseString : public TSTRING
{
public:
    TSTRING& operator=(const TSTRING& str)
    {
        return TSTRING::operator=(str);
    };
};


class cParseStringList : public std::list<TSTRING>
{
private:
    std::list<TSTRING>& operator=(const std::list<TSTRING>& str);
};


// this class holds all info needed for an attribute scope
class cAttrScopeInfo
{
public:
    cAttrScopeInfo() : pAttrList(0), nStatements(0){};

    cParseNamedAttrList* GetAttrList()
    {
        return pAttrList;
    };
    void SetAttributes(cParseNamedAttrList* pL)
    {
        pAttrList = pL;
    };
    void IncrementStatementCount()
    {
        nStatements++;
    };
    int GetStatementCount()
    {
        return nStatements;
    };

private:
    cParseNamedAttrList* pAttrList;
    int                  nStatements;
};

#endif //__PARSEROBJECTS_H
