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
// parserobjects.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdtwparser.h"
#include "parserobjects.h"
#include "parserhelper.h"
#include "core/twlimits.h"

//=========================================================================
// METHOD CODE
//=========================================================================

//
// cParseRule
//

cParseRule::~cParseRule()
{
    delete mpAttrList;
    delete mpSpecMaskList;
}

const cFCOName& cParseRule::GetName() const
{
    return mName;
}

void cParseRule::SetName(const cFCOName& name)
{
    mName = name;
}

cParseNamedAttrList* cParseRule::GetAttrList() const
{
    return mpAttrList;
}

void cParseRule::SetAttrList(cParseNamedAttrList* pattr)
{
    mpAttrList = pattr;
}

///////////////////////////////////////////////////////////////////////////////
// SetSpecMaskList
//          attaches cParseSpecMaskList to this structure
///////////////////////////////////////////////////////////////////////////////
void cParseRule::SetSpecMaskList(cParseSpecMaskList* pmasks)
{
    mpSpecMaskList = pmasks;
}


///////////////////////////////////////////////////////////////////////////////
// GetNumSpecMasks
//          returns number of specmasks in list (excluding default)
///////////////////////////////////////////////////////////////////////////////
int cParseRule::GetNumSpecMasks() const
{
    // empty list?
    if (mpSpecMaskList == NULL)
        return 0;

    return mpSpecMaskList->mList.size();
}

///////////////////////////////////////////////////////////////////////////////
// GetNumNamedAttrs
//          returns number of named attributes in list
///////////////////////////////////////////////////////////////////////////////
int cParseRule::GetNumNamedAttrs() const
{
    // empty list?
    if (mpAttrList == NULL)
        return 0;

    return mpAttrList->mList.size();
}

///////////////////////////////////////////////////////////////////////////////
// Dump
//          dumps out all the contents of the parse node, including children
///////////////////////////////////////////////////////////////////////////////

void cParseRule::Dump()
{
    cDebug d("\tcParseRule::Dump()");

    d.TraceDebug("name=%s\n", mName.AsString().c_str());
    d.TraceDebug("defspecmask.prop_vector=%s\n", mDefSpecMask.GetPropVectorString().c_str());
    d.TraceDebug("defspecmask.condition=%s\n", mDefSpecMask.GetCondition().c_str());

    // specmasks
    d.TraceDebug("    numspecmasks=%d\n", GetNumSpecMasks());
    if (mpSpecMaskList)
        mpSpecMaskList->Dump(d);

    // named attributes
    d.TraceDebug("    numattrs=%d\n", GetNumNamedAttrs());
    if (mpAttrList)
        mpAttrList->Dump(d);
}

//
// cParseSpecMask
//

// constructor
cParseSpecMask::cParseSpecMask(TSTRING& condition, TSTRING& attr, iParserGenreUtil* pHelper) // throw( eParserHelper )
    : msCondition(condition), msPV(attr), mPropVector(), mpAttrList(NULL)
{
    cParserUtil::CreatePropVector(msPV, mPropVector, pHelper);
}

cParseSpecMask::~cParseSpecMask()
{
    delete mpAttrList;
}

int cParseSpecMask::GetNumAttrs() const
{
    if (mpAttrList == NULL)
        return 0;

    return mpAttrList->mList.size();
}

void cParseSpecMask::SetAttrList(cParseNamedAttrList* pattr)
{
    mpAttrList = pattr;
}

cParseNamedAttrList* cParseSpecMask::GetAttrList() const
{
    return mpAttrList;
}

const TSTRING& cParseSpecMask::GetPropVectorString() const
{
    return msPV;
}

const cFCOPropVector& cParseSpecMask::GetPropVector() const
{
    return mPropVector;
}

const TSTRING& cParseSpecMask::GetCondition() const
{
    return msCondition;
}

void cParseSpecMask::Dump(cDebug& d) const
{
    d.TraceDebug("    condition=(%s), propvector=:\n", msCondition.c_str(), msPV.c_str());

    // dump list of attributes
    if (mpAttrList)
        mpAttrList->Dump(d);
}

//
// cParseSpecMaskList
//

void cParseSpecMaskList::Dump(cDebug& d) const
{
    std::list<cParseSpecMask*>::const_iterator ispec;
    for (ispec = mList.begin(); ispec != mList.end(); ++ispec)
    {
        (*ispec)->Dump(d);
    }
}

//
// cParseNamedAttr
//

// constructor
cParseNamedAttr::cParseNamedAttr(const TSTRING& name, const TSTRING& value)
    : mstrName(name), mstrValue(value), mLineNum(cParserHelper::GetLineNumber())
{
    // convert name to lowercase
    std::transform(mstrName.begin(), mstrName.end(), mstrName.begin(), _totlower);
}

cParseNamedAttr* cParseNamedAttr::Clone() const
{
    cParseNamedAttr* pNew = new cParseNamedAttr(mstrName, mstrValue);
    pNew->mLineNum        = mLineNum;
    return pNew;
}

void cParseNamedAttr::Dump(cDebug& d) const
{
    d.TraceDebug("    name=(%s), value=(%s)\n", mstrName.c_str(), mstrValue.c_str());
}

void cParseNamedAttr::Validate() const
{
    if (NamesMatch(twparser::STR_PARSER_RULENAME, mstrName))
    {
        // no checking needed
    }
    else if (NamesMatch(twparser::STR_PARSER_EMAILTO, mstrName))
    {
        // no checking needed
    }
    else if (NamesMatch(twparser::STR_PARSER_SEVERITY, mstrName))
    {
        int             iIgnore;
        cSeverityLimits sl;
        if (!sl.InterpretInt(mstrValue, &iIgnore))
            throw eParserBadSevVal(mstrValue, mLineNum);
    }
    else if (NamesMatch(twparser::STR_PARSER_RECURSE, mstrName))
    {
        TSTRING str = mstrValue;
        std::transform(str.begin(), str.end(), str.begin(), _totlower);

        if (!NamesMatch(twparser::STR_PARSER_FALSE, str) && !NamesMatch(twparser::STR_PARSER_TRUE, str))
        {
            // must be number
            int                 iIgnore;
            cRecurseDepthLimits rdl;
            if (!rdl.InterpretInt(str, &iIgnore))
                throw eParserUnrecognizedAttrVal(mstrValue, mLineNum);
        }
    }
    else
    {
        throw eParserUnrecognizedAttr(mstrName, mLineNum);
    }
}

// case sensitive-matching
bool cParseNamedAttr::NamesMatch(int nString, const TSTRING& strName) const
{
    return (0 == strName.compare(TSS_GetString(cTWParser, nString)));
}

//
// cParseNamedAttrList
//


cParseNamedAttrList::~cParseNamedAttrList()
{
    Clear();
}

void cParseNamedAttrList::Clear()
{
    for (std::list<cParseNamedAttr*>::iterator iter = mList.begin(); iter != mList.end(); ++iter)
    {
        delete *iter;
    }
    mList.clear();
}

void cParseNamedAttrList::Dump(cDebug& d) const
{
    // dump out each named attribute
    std::list<cParseNamedAttr*>::const_iterator iattr;
    for (iattr = mList.begin(); iattr != mList.end(); ++iattr)
    {
        (*iattr)->Dump(d);
    }
}


void cParseNamedAttrList::Add(cParseNamedAttr* const pa)
{
    ASSERT(pa != 0);

    pa->Validate();

    // if not already in list, just put it in
    constListIter iter = Find(pa);
    if (iter == mList.end())
    {
        mList.push_back(pa);
    }
    else // set value of pa already in list
    {
        (*iter)->SetValue(pa->GetValue());
    }
}


cParseNamedAttrList::constListIter cParseNamedAttrList::Find(const cParseNamedAttr* const pa)
{
    ASSERT(pa != 0);

    for (std::list<cParseNamedAttr*>::const_iterator iter = mList.begin(); iter != mList.end(); ++iter)
    {
        if ((*iter)->GetName() == pa->GetName())
        {
            return iter;
        }
    }

    return mList.end();
}

void cParseNamedAttrList::MergeNoOverwrite(const cParseNamedAttrList* const pal)
{
    if (pal)
    {
        // foreach attribute in pal
        for (constListIter newIter = pal->mList.begin(); newIter != pal->mList.end(); ++newIter)
        {
            // look for this attribute in this list
            constListIter thisIter = Find(*newIter);

            // if not already in list, just put it in
            if (thisIter == mList.end())
            {
                mList.push_back((*newIter)->Clone());
            }
            else // do not overwrite value (except for emailto)
            {
                // special case for emailto: this can have multiple values, so add the new value to the old value
                if (0 == (*thisIter)->GetName().compare(TSS_GetString(cTWParser, twparser::STR_PARSER_EMAILTO)))
                {
                    TSTRING tstr = (*thisIter)->GetValue();
                    cParserUtil::MergeMailLists(tstr, (*newIter)->GetValue());
                    (*thisIter)->SetValue(tstr);
                }
            }
        }
    }
}

const cParseNamedAttr* cParseNamedAttrList::Lookup(const TSTRING& tstrAttrName) const
{
    constListIter i;

    for (i = mList.begin(); i != mList.end(); ++i)
    {
        if (0 == tstrAttrName.compare((*i)->GetName()))
        {
            return (*i);
        }
    }

    return (NULL);
}
