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
// parserhelper.cpp -- helper classes that are called by yacc parser
//

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

#include "stdtwparser.h"

//=========================================================================
// INCLUDES
//=========================================================================

#include "parserhelper.h"
#include "core/fsservices.h"
#include "fco/fcopropvector.h"
#include "fco/parsergenreutil.h"
#include "fco/genreswitcher.h"
#include "fco/twfactory.h"
#include "fco/fcospechelper.h"
#include "core/twlimits.h"
#include "core/stringutil.h"

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

static int  util_ConvertHex(const char* psz, int* const pnCharsRead);
static int  util_ConvertUnicode(const char* psz, int* const pnCharsRead);
static int  util_ConvertOctal(const char* psz, int* const pnCharsRead);
static bool util_IsOctal(const char ch);
static int  util_GetEscapeValueOfChar(char ch);
static int  util_GetRecurseDepth(const cParseNamedAttrList* pList); //throw( eParserHelper )
static void util_LoseSurroundingWS(TSTRING& str);
#ifdef DEBUG
static bool util_AsciiCharsActLikeTheyShould();
#endif

// finds first '&', '+', '-'  on or after str[i]
static TSTRING::size_type util_FindNextDelim(const TSTRING& str, TSTRING::size_type i);


//=========================================================================
// STATIC DATA MEMBERS
//=========================================================================

cErrorBucket*              cParserHelper::mpError;
int                        cParserHelper::miLineNum;
cPreprocessor::AcceptStack cPreprocessor::mStateStack;
bool                       cPreprocessor::mfIgnoreSection;

cParserHelper::ScopedAttrContainer cParserHelper::mScopedAttrs;
cParserHelper::GenreContainer      cParserHelper::mAph;
cGenreParseInfo*                   cParserHelper::pCurrentGenreInfo;
cSymbolTable                       cParserHelper::mGlobalVarTable;
bool                               cParserHelper::mfParseOnly;


//=========================================================================
// METHOD CODE
//=========================================================================

eParserHelper::eParserHelper(const TSTRING& strMsg, int nLine /*= CURRENT_LINE */)
{
    TOSTRINGSTREAM strErr;

    strErr << strMsg;

    if (NO_LINE != nLine)
    {
        // separate the message from the line number
        if (!strMsg.empty())
            strErr << _T(": ");

        // output the line number
        strErr << TSS_GetString(cTWParser, twparser::STR_LINE_NUMBER);

        if (CURRENT_LINE == nLine)
            strErr << cParserHelper::GetLineNumber();
        else
            strErr << nLine;
    }

    tss_end(strErr);
    mMsg = strErr.str();
    tss_free(strErr);
}


void cParserHelper::Init(cErrorBucket* pE)
{
    // start off in default genre
    cGenreSwitcher::GetInstance()->SelectGenre(cGenreSwitcher::GetInstance()->GetDefaultGenre());

    mpError = pE;
    cPreprocessor::PushState(cPreprocessor::STATE_ACCEPT);
    miLineNum = 1;
    cPreprocessor::ReadSection();
    pCurrentGenreInfo = NULL;
    mfParseOnly       = false;
}

void cParserHelper::Finit(cGenreSpecListVector* pPolicy)
{
    //
    // don't build spec list if just checking syntax
    //
    if (!pPolicy)
    {
        CleanUp();
        return;
    }

    int nRulesInPolicy = 0;

    GenreContainer::iterator i;
    for (i = mAph.begin(); i != mAph.end(); ++i)
    {
        cGenreSpecListPair slp;
        cGenre::Genre      g    = i->first;
        cGenreParseInfo*   pgpi = i->second;

        //
        // create the specs from rules
        //
        cParserUtil::CreateFCOSpecs(g, pgpi, slp.GetSpecList());
        slp.SetGenre(g);

        //
        // get rule count for section
        //
        int nRulesInSection = slp.GetSpecList().Size();
        if (0 == nRulesInSection)
        {
            TSTRING str = TSS_GetString(cTWParser, twparser::STR_ERR2_PARSER_NO_RULES_IN_SECTION);
            str.append(cGenreSwitcher::GetInstance()->GenreToString(slp.GetGenre(), true));

            eParserNoRulesInSection e(str);
            e.SetFatality(false);
            cParserHelper::GetErrorBucket()->AddError(e);
        }

        //
        // increment rule count for policy file
        //
        nRulesInPolicy += slp.GetSpecList().Size();

        //
        // add to policy
        //
        pPolicy->push_back(slp);
    }

    CleanUp();

    //
    // check that we have some rules
    //
    if (0 == nRulesInPolicy)
        throw eParserNoRules(_T(""), eParserHelper::NO_LINE);
}

void cParserHelper::CleanUp()
{
    GenreContainer::iterator i;
    for (i = mAph.begin(); i != mAph.end(); ++i)
        delete i->second;
    while (!cPreprocessor::Empty())
        cPreprocessor::PopState();
    while (!cParserHelper::ScopeEmpty())
        cParserHelper::PopScope();
}

cPreprocessor::cPreprocessor()
{
    // start in the accept state
    mStateStack.push(STATE_ACCEPT);
}

void cPreprocessor::PushState(AcceptState state)
{
    mStateStack.push(state);

    cDebug d("cPreprocessor::PushState");

    if (state == STATE_ACCEPT)
        d.TraceDebug(_T("State == STATE_ACCEPT\n"));
    else
        d.TraceDebug(_T("State == STATE_IGNORE\n"));
}

void cPreprocessor::PopState()
{
    cDebug d("cPreprocessor::PopState");

    ASSERT(!mStateStack.empty());
    mStateStack.pop();

#ifdef DEBUG
    if (!Empty() && TopState() == STATE_ACCEPT)
        d.TraceDebug(_T("State == STATE_ACCEPT\n"));
    else
        d.TraceDebug(_T("State == STATE_IGNORE\n"));
#endif
}

cPreprocessor::AcceptState cPreprocessor::GetState()
{
    if (STATE_ACCEPT == TopState() && false == mfIgnoreSection)
        return STATE_ACCEPT;
    else
        return STATE_IGNORE;
}

cPreprocessor::AcceptState cPreprocessor::TopState()
{
    ASSERT(!mStateStack.empty());

    return mStateStack.top();
}

cPreprocessor::AcceptState cPreprocessor::UnderneathTopState()
{
    ASSERT(mStateStack.size() > 1);

    AcceptState stateRet, stateSave;

    stateSave = mStateStack.top();
    mStateStack.pop();
    stateRet = mStateStack.top();
    mStateStack.push(stateSave);

    return (stateRet);
}

void cPreprocessor::ToggleTopState()
{
    ASSERT(!mStateStack.empty());
    cDebug d("cPreprocessor::ToggleState");

    AcceptState state = TopState();

    mStateStack.pop();

    // toggle state
    mStateStack.push(GetOppositeState(state));

    if (GetOppositeState(state) == STATE_ACCEPT)
        d.TraceDebug(_T("State == STATE_ACCEPT\n"));
    else
        d.TraceDebug(_T("State == STATE_IGNORE\n"));
}


cPreprocessor::AcceptState cPreprocessor::GetOppositeState(AcceptState state)
{
    if (state == STATE_IGNORE)
        return STATE_ACCEPT;
    else
        return STATE_IGNORE;
}


bool cPreprocessor::AtTopLevel()
{
    ASSERT(!mStateStack.empty());

    bool        fAtTop;
    AcceptState stateSave;

    // save and pop current state
    stateSave = mStateStack.top();
    mStateStack.pop();

    // we're at top level if there was only one item in stack
    fAtTop = mStateStack.empty();

    // return item to stack
    mStateStack.push(stateSave);

    return (fAtTop);
}

// pushes cPreprocessor::STATE_IGNORE onto the state stack if
// strSection is not an acceptable genre for this OS

void cParserHelper::SetSection(TSTRING& strSection) // throw( eParserHelper )
{
    //
    // do we recognize the section string?
    //
    cGenre::Genre g = cGenreSwitcher::GetInstance()->StringToGenre(strSection.c_str());
    if (cGenre::GENRE_INVALID != g && cGenreSwitcher::GetInstance()->IsGenreAppropriate(g))
    {
        cGenreSwitcher::GetInstance()->SelectGenre(g);

        // set current genre info pointer
        GenreContainer::iterator i = mAph.find(cGenreSwitcher::GetInstance()->CurrentGenre());
        if (mAph.end() == i)
        {
            pCurrentGenreInfo = new cGenreParseInfo;
            mAph.insert(GenreContainer::value_type(cGenreSwitcher::GetInstance()->CurrentGenre(), pCurrentGenreInfo));

            cPreprocessor::ReadSection();
        }
        else
        {
            throw eParserSectionAlreadyDefined(strSection);
        }
    }
    else // ignore
    {
        if (!ParseOnly())
        {
            eParserIgnoringSection e(strSection);
            e.SetFatality(false);
            if (mpError)
                mpError->AddError(e);
        }

        cPreprocessor::IgnoreSection();
    }
}

// A function for inserting a global variable into the global variable table.  Throws
// an exception if they try to redefine one of the predefined variables.  Returns
// the return value of the hash table insert.
bool cParserHelper::InsertGlobalVariable(const TSTRING& var, const TSTRING& val)
{
    TSTRING dummy;
    /*  if( cParserHelper::GetGenreInfo()->GetPredefVarTable().Lookup(var, dummy) )
            throw eParserRedefineVar( var );
    */
    // TODO : Verify that there is no feasible way to search the predefines without
    // messing everything up, given our current architecture.

    return mGlobalVarTable.Insert(var, val);
}

cGenreParseInfo* cParserHelper::GetGenreInfo()
{
    if (!pCurrentGenreInfo)
    {
        pCurrentGenreInfo = new cGenreParseInfo;
        mAph.insert(GenreContainer::value_type(cGenreSwitcher::GetInstance()->CurrentGenre(), pCurrentGenreInfo));
    }

    return pCurrentGenreInfo;
}

bool cParserUtil::AnyOfTheseHostsExists(cParseStringList* pList)
{
    TSTRING strHostName;
    cDebug  d("cPreprocessor::AnyOfTheseHostsExists");

    // this throws an exception.  let it go on up, since we can't do any ifhosting
    // if we don't know what machine we're on
    iFSServices::GetInstance()->GetMachineName(strHostName);

    // want to do a case-insensitive compare
    std::transform(strHostName.begin(), strHostName.end(), strHostName.begin(), _totlower);

    // if the host name matches any in the list, return true
    for (std::list<TSTRING>::iterator iter = pList->begin(); iter != pList->end(); ++iter)
    {
        // want to do case-insensitive compare
        std::transform((*iter).begin(), (*iter).end(), (*iter).begin(), _totlower);

        if (0 == strHostName.compare((*iter)))
        {
            d.TraceDebug(_T("host exists\n"));
            return true;
        }
    }

    d.TraceDebug(_T("host doesn't exist\n"));
    return false;
}

// converts escaped character sequences in strEscapedString into their escape characters in strInterpretedString
//( e.g. turns "Hello World!\n" (literal backslash) into "Hello World!<return char>"
// all C++ escape sequences are recognized:
//          (1) octal numbers \012 (1, 2, or 3 octal digits),
//          (2) hex numbers \x2A ( 'x' followed by one or two hex digits ),
//          (3) unicode characters \uABCD ( 'u' followed by exactly 4 hex digits )
//          (4) chars: \t, \v, \b, \r, \f, \a, \\, \?, \', and \"
//          (5) all other escaped chars are treated as if not escaped
void cParserUtil::InterpretEscapedString(const std::string& strEscapedString, TSTRING& strInterpretedString)
{
    cDebug d("cParserHelper::InterpretEscapedString");

    ASSERT((void*)&strEscapedString != (void*)&strInterpretedString); // don't let us read and write to same string

    // The source string may contain literal multibyte characters, escaped MB characters, and escaped Unicode characters.
    // On Unix TCHAR == char always, therefore literal and esacped mb chars can be stored in strInterpretedString
    // directly, and everythign will be hunky dory (escased Unicode will cause an error).
    // But on Windows we need to build an intermediate narrow string because it needs to be passed through
    // cStringUtil::StrToTstr() to convert literal and escaped mb chars to TCHARs (which are wchar_t in this case).
    // Escaped Unicode chars are included in this intermediate string specially encoded so that StrToTstr()
    // leaves it alone.  After we have the wide string, we convert these encoded Unicode chars into true 16 bit
    // Unicode chars.


    // for unix we cheat a bit and do the initial interpretation
    // directly to the final string.
    typedef TCHAR INTERCHAR;
    TSTRING&      strIntermediateString = strInterpretedString;

    const char* pchCur = strEscapedString.c_str();

    for (strIntermediateString.erase(); *pchCur != 0; pchCur++) // only single byte in this part of the policy file
    {
        if (*pchCur != '\\') // just regular char
        {
            strIntermediateString += *pchCur;
        }
        else // deal with escaped character sequence
        {
            // make sure the '\' isn't the end of the string
            // ( if it is, the "for" will ++ pchCur, see it's 0, then break )
            if (*(pchCur + 1))
            {
                int nCharsRead;

                pchCur++; // go to char past '\'

                if (*pchCur == 'x' &&
#if !ARCHAIC_STL		    
                    std::isxdigit<TCHAR>(
                        *(pchCur + 1),
                        std::locale())) // deal with \xXXXX where 'x' is the character 'x', and 'X' is a hex number
#else
		    isxdigit(*(pchCur + 1)))
#endif		  		  
                {
                    pchCur++; // go to char past 'x'
                    char cEscapedChar = static_cast<char>(util_ConvertHex(pchCur, &nCharsRead));
                    pchCur += (nCharsRead == 0) ? 0 : nCharsRead - 1; //increment pointer to last char we read

                    if (cEscapedChar == 0) // null characters are not allowed
                    {
                        throw eParserBadHex(cStringUtil::StrToTstr(pchCur - ((nCharsRead == 0) ? 0 : nCharsRead - 1)));
                    }
                    else
                    {
                        strIntermediateString += static_cast<INTERCHAR>(cEscapedChar);
                    }
                }
                else if (*pchCur == 'u') // unicode escape
                {
                    pchCur++; // go to char past 'x'
                    wchar_t wcEscapedChar = static_cast<wchar_t>(util_ConvertUnicode(pchCur, &nCharsRead));
                    pchCur += (nCharsRead == 0) ? 0 : nCharsRead - 1; //increment pointer to last char we read

                    if (wcEscapedChar == 0) // null characters are not allowed
                    {
                        throw eParserBadHex(cStringUtil::StrToTstr(pchCur - ((nCharsRead == 0) ? 0 : nCharsRead - 1)));
                    }
                    else if (wcEscapedChar > 0xff)
                    {
                        throw eParserBadHex(cStringUtil::StrToTstr(pchCur - ((nCharsRead == 0) ? 0 : nCharsRead - 1)));
                    }
                    else
                    {
                        strIntermediateString += static_cast<INTERCHAR>(wcEscapedChar);
                    }
                }
                else if (util_IsOctal(*pchCur)) // deal with \xxx where 'x' is an octal digit
                {
                    strIntermediateString += static_cast<INTERCHAR>(util_ConvertOctal(pchCur, &nCharsRead));
                    pchCur += (nCharsRead == 0) ? 0 : nCharsRead - 1; //increment pointer to last char we read
                }
                else // deal with \x where 'x' is any other character
                {
                    strIntermediateString += static_cast<INTERCHAR>(util_GetEscapeValueOfChar(*pchCur));
                }
            }
        }
    }

#ifdef DEBUG
    std::string str;

    str = "before: <";
    str += strEscapedString;
    str += ">\n";
    d.TraceDebug(str.c_str());

    TSTRING tstr;
    tstr = _T("after: <");
    tstr += strInterpretedString;
    tstr += _T(">\n");
    d.TraceDebug(tstr.c_str());
#endif
}

///////////////////////////////////////////////////////////////////////////////
// CreateFCOSpecs -- foreach rule, create an FCOSpec.  then attach the stop
//          points.
///////////////////////////////////////////////////////////////////////////////
void cParserUtil::CreateFCOSpecs(cGenre::Genre    g,
                                 cGenreParseInfo* pgpi,
                                 cFCOSpecList&    fcospeclist) //throw( eParserHelper )
{
    cGenreSwitcher::GetInstance()->SelectGenre(g);
    iParserGenreUtil* pHelper = iTWFactory::GetInstance()->CreateParserGenreUtil();

    cDebug d("cGenreParseInfo::CreateFCOSpecs");

    // foreach rule
    std::list<const cParseRule*>::iterator rule;
    for (rule = pgpi->GetRules()->begin(); rule != pgpi->GetRules()->end(); ++rule)
    {
        //
        // create the spec with its the last element of its start point as its name.
        // if the attribute 'rulename' is specified, we'll set it as the spec name later
        // Need to set the start point after the helper is set
        //
        cFCOName  startpoint = (*rule)->GetName();
        iFCOSpec* pfcospec =
            iTWFactory::GetInstance()->CreateSpec(startpoint.GetShortName()); // we'll set the helper below

        //////////////////////////////////////////////////////////////
        // look to see if no recurse was specified.  if it is, set the helper to a cFCOSpecNoChildren,
        // otherwise set it to a cFCOSpecStopPointSet
        int depth = util_GetRecurseDepth((*rule)->GetAttrList());
        if (0 == depth)
        {
            // don't use stop point set.  say don't recurse
            cFCOSpecNoChildren* pNoKids = new cFCOSpecNoChildren;
            pfcospec->SetHelper(pNoKids);
        }
        else
        {
            cFCOSpecStopPointSet* pStopPtSet = new cFCOSpecStopPointSet;

            // set stop points
            cGenreParseInfo::StopListType::iterator stop;
            for (stop = pgpi->GetStopList()->begin(); stop != pgpi->GetStopList()->end(); ++stop)
            {
                // add stop point if below start point
                cFCOName::Relationship relate = startpoint.GetRelationship(*stop);

                if (relate == cFCOName::REL_ABOVE)
                {
                    pStopPtSet->Add(*stop);
                }
            }

            pStopPtSet->SetRecurseDepth(-1 == depth ? cFCOSpecStopPointSet::RECURSE_INFINITE : depth);
            pfcospec->SetHelper(pStopPtSet);
        }
        //
        //////////////////////////////////////////////////////////////

        //
        // set start point
        //
        pfcospec->SetStartPoint(startpoint);

        //
        // set default mask
        //
        const iFCOSpecMask* pdefspecmask = iFCOSpecMask::GetDefaultMask();
        ASSERT(pdefspecmask != 0);

        //
        // build property vector
        //
        cFCOPropVector v = (*rule)->GetDefSpecMask().GetPropVector();
        pHelper->AddSubTypeProps(v);
        pfcospec->SetPropVector(pdefspecmask, v);

        //
        // add attributes
        //
        TSTRING              strAttrVal;
        cParseNamedAttrList* pal   = (*rule)->GetAttrList();
        cFCOSpecAttr*        pAttr = new cFCOSpecAttr;
        if (pal != NULL)
        {
            // TODO: make storage place for these keywords
            // attribute names are stored in lowercase
            const cParseNamedAttr* pa;

            //
            // get rulename
            //
            pa = pal->Lookup(TSS_GetString(cTWParser, twparser::STR_PARSER_RULENAME));
            if (pa)
            {
                // TODO -- at some future date, remove the name in the spec
                // and only set it in pAttr
                pfcospec->SetName(pa->GetValue());
                pAttr->SetName(pa->GetValue());
            }

            //
            // get severity
            //
            pa = pal->Lookup(TSS_GetString(cTWParser, twparser::STR_PARSER_SEVERITY));
            if (pa)
            {
                int             iSev;
                cSeverityLimits sl;
                if (!sl.InterpretInt(pa->GetValue(), &iSev))
                    throw eParserBadSevVal(pa->GetValue(), pa->GetLineNum());

                pAttr->SetSeverity(iSev);
            }

            //
            // get emailto
            //
            pa = pal->Lookup(TSS_GetString(cTWParser, twparser::STR_PARSER_EMAILTO));
            if (pa)
            {
                TSTRING strAddressee;
                int     index = 0;
                while (cParserUtil::GetNextMailName(pa->GetValue(), strAddressee, index++))
                {
                    d.TraceDebug(_T("email to recip: <%s>\n"), strAddressee.c_str());
                    pAttr->AddEmail(strAddressee);
                }
            }
        }

        //
        // store rule in the spec list
        //
        fcospeclist.Add(pfcospec, pAttr);
        pfcospec->Release();
        pAttr->Release();
    }

    delete pHelper;
}
///////////////////////////////////////////////////////////////////////////////
// CreatePropVector -- given the string representation of the attribute vector,
//          create an FCOPropVector
///////////////////////////////////////////////////////////////////////////////
void cParserUtil::CreatePropVector(const TSTRING& strPropListC, class cFCOPropVector& v, iParserGenreUtil* pHelper)
{
    ASSERT(pHelper != NULL);

    // state: turning modes on or off
    enum Mode
    {
        MO_TURNON,
        MO_TURNOFF
    };
    Mode mode = MO_TURNON;

    // clear out all spaces in the string
    TSTRING strPropList = strPropListC;

// C++17 removes std::ptr_fun, so use a lambda where available
#if !USE_LAMBDAS
    strPropList.erase(std::remove_if(strPropList.begin(), strPropList.end(),
        std::ptr_fun<int, int>(std::isspace)), strPropList.end());
#else
    strPropList.erase(std::remove_if(strPropList.begin(), strPropList.end(),
        [](int c) {return std::isspace(c);}), strPropList.end());
#endif

    // zero it out
    v.Clear();

    TSTRING::const_iterator iter;
    TSTRING::size_type      i; // index into string
    for (iter = strPropList.begin(), i = 0; iter != strPropList.end(); ++iter, ++i)
    {
        int propIndex = -1; // index into propvector

        switch (*iter)
        {
        /////////////////////////////////////////////////////////////
        // parsing modes
        case '+':
            mode = MO_TURNON;
            continue;
        case '-':
            mode = MO_TURNOFF;
            continue;
        /////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////
        // long attribute names
        case '&':
        {
            //
            // collect string
            //

            // find next '&', '+', or '-'
            TSTRING::size_type next = util_FindNextDelim(strPropList, i + 1);

            // get attribute name
            TSTRING strProp;
            if (next == TSTRING::npos)
                strProp = strPropList.substr(i + 1);
            else
                strProp = strPropList.substr(i + 1, next - i - 1);

            // increment past string
            iter += strProp.length();
            i += strProp.length();

            // map attribute
            if (!pHelper->MapStringToProperty(strProp, propIndex))
                throw eParserPropChar(strProp);
        }
        break;
        /////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////
        // not '+' or '-' or '&' so map the char to a property
        default:
            if (!pHelper->MapStringToProperty(TSTRING(1, *iter), propIndex))
                throw eParserPropChar(TSTRING(1, *iter));
            break;
            //
            /////////////////////////////////////////////////////////////
        }

        // now turn on or turn off bit, according to mode
        ASSERT(propIndex != -1);
        if (mode == MO_TURNON)
            v.AddItemAndGrow(propIndex);
        else
            v.RemoveItem(propIndex);
    }

    return;
}

// gets semicolon-delimited words in a string. index starts at 0.
// strNameRet has surronding WS discarded
bool cParserUtil::GetNextMailName(const TSTRING& strNameList, TSTRING& strNameRet, int index)
{
    ASSERT(index >= 0);

    // go to nth name
    TSTRING::size_type nextSC, lastSC = (TSTRING::size_type)-1; // algorithm starts searching for next ';'
                                                                // at lastSC+1
    for (; index >= 0; index--)
    {
        nextSC = strNameList.find(_T(';'), (lastSC + 1));

        if (0 == index) // if the name # we're looking for
        {
            strNameRet =
                strNameList.substr((lastSC + 1), TSTRING::npos == nextSC ? TSTRING::npos : nextSC - (lastSC + 1));
            util_LoseSurroundingWS(strNameRet);
            return true;
        }
        else if (TSTRING::npos == nextSC) // didn't get to index'th naem: no names left
        {
            return false;
        }

        lastSC = nextSC;
    }

    // unreachable code
    ASSERT(false);
    return false; // just to placate the compiler
}

void cParserUtil::MergeMailLists(TSTRING& tstrMergeIntoMe, const TSTRING& tstrRHS)
{
    // foreach word in rhs
    TSTRING tstrWordToAdd;
    int     index = 0;
    while (cParserUtil::GetNextMailName(tstrRHS, tstrWordToAdd, index++))
    {
        // if it isn't in lhs, add it
        bool    fAlreadyInList = false;
        int     index2         = 0;
        TSTRING tstrWordInLHS;
        while (!fAlreadyInList && cParserUtil::GetNextMailName(tstrMergeIntoMe, tstrWordInLHS, index2++))
        {
            if (0 == tstrWordInLHS.compare(tstrWordToAdd))
                fAlreadyInList = true;
        }

        if (!fAlreadyInList)
        {
            tstrMergeIntoMe += _T(";");
            tstrMergeIntoMe += tstrWordToAdd;
        }
    }
}


bool cParserHelper::AtTopScope()
{
    return (mScopedAttrs.empty());
}

void cParserHelper::PopScope()
{
    ASSERT(!mScopedAttrs.empty());

    delete mScopedAttrs.back();
    mScopedAttrs.pop_back();
}

cParseNamedAttrList* cParserHelper::GetGlobalAttrList()
{
    if (!mScopedAttrs.empty())
        return mScopedAttrs.back()->GetAttrList();
    else
        return NULL;
}

void cParserHelper::IncrementScopeStatementCount()
{
    // must add count to ALL previous scopes.
    for (ScopedAttrContainer::iterator i = mScopedAttrs.begin(); i != mScopedAttrs.end(); ++i)
        (*i)->IncrementStatementCount();
}

int cParserHelper::GetScopeStatementCount()
{
    if (!mScopedAttrs.empty())
    {
        return mScopedAttrs.back()->GetStatementCount();
    }
    else
    {
        // should never ask for this when we aren't in a scope
        ASSERT(false);
        return 0;
    }
}


void cParserHelper::PushScope(cAttrScopeInfo* pSI)
{
    // add previous scope's info to this one
    if (!mScopedAttrs.empty())
    {
        pSI->GetAttrList()->MergeNoOverwrite(mScopedAttrs.back()->GetAttrList());
    }

    mScopedAttrs.push_back(pSI);
}


//=========================================================================
// UTIL FUNCTION CODE
//=========================================================================

int util_ConvertHex(const char* const cpsz, int* const pnCharsRead)
{
    ASSERT(util_AsciiCharsActLikeTheyShould());
    ASSERT(cpsz && pnCharsRead);

#if !ARCHAIC_STL    
    if (*cpsz == 0 || !std::isxdigit<TCHAR>(*cpsz, std::locale()))
#else
    if (*cpsz == 0 || !isxdigit(*cpsz))
#endif
      
        throw eParserBadHex(cStringUtil::StrToTstr(cpsz));

    int         iValue;
    const char* psz = cpsz;
    
#if !ARCHAIC_STL    
    for (*pnCharsRead = 0, iValue = 0; *psz && std::isxdigit<TCHAR>(*psz, std::locale()) && (*pnCharsRead < 2);
         psz++, (*pnCharsRead)++)
#else
    for (*pnCharsRead = 0, iValue = 0; *psz && isxdigit(*psz) && (*pnCharsRead < 2);
         psz++, (*pnCharsRead)++)
#endif      
    {
        iValue *= 0x10;

#if !ARCHAIC_STL	
        if (std::isdigit<TCHAR>(*psz, std::locale()))
#else
	if (isdigit(*psz))
#endif	  
        {
            iValue += (*psz - '0');
        }
        else
        {
#if !ARCHAIC_STL	  
            if (std::islower<TCHAR>(*psz, std::locale()))
#else
            if (islower(*psz))
#endif	      
                iValue += (*psz - 'a' + 10); // so that A=10, B=11, ..., F=15
            else                             // is uppercase
                iValue += (*psz - 'A' + 10); // so that a=10, a=11, ..., f=15
        }
    }

    return iValue;
}

int util_ConvertUnicode(const char* const cpsz, int* const pnCharsRead)
{
    ASSERT(util_AsciiCharsActLikeTheyShould());
    ASSERT(cpsz && pnCharsRead);

#if !ARCHAIC_STL    
    if (*cpsz == 0 || !std::isxdigit<TCHAR>(*cpsz, std::locale()))
#else
    if (*cpsz == 0 || !isxdigit(*cpsz))
#endif      
        throw eParserBadHex(cStringUtil::StrToTstr(cpsz));

    int         iValue;
    const char* psz = cpsz;
    for (*pnCharsRead = 0, iValue = 0; *pnCharsRead < 4; psz++, (*pnCharsRead)++)
    {
      
        // we require 4 chars for unicode escapes
#if !ARCHAIC_STL      
        if (*psz == 0 || !std::isxdigit<TCHAR>(*psz, std::locale()))
#else
	if (*psz == 0 || !isxdigit(*psz))
#endif	  
            throw eParserBadHex(cStringUtil::StrToTstr(cpsz));

        iValue *= 0x10;

#if !ARCHAIC_STL	
        if (std::isdigit<TCHAR>(*psz, std::locale()))
#else
        if (isdigit(*psz))
#endif	  
        {
            iValue += (*psz - '0');
        }
        else
        {
#if !ARCHAIC_STL	  
            if (std::islower<TCHAR>(*psz, std::locale()))
#else
            if (islower(*psz))
#endif	      
                iValue += (*psz - 'a' + 10); // so that A=10, B=11, ..., F=15
            else                             // is uppercase
                iValue += (*psz - 'A' + 10); // so that a=10, a=11, ..., f=15
        }
    }

    return iValue;
}

// only 3 octal chars allowed
int util_ConvertOctal(const char* psz, int* const pnCharsRead)
{
    ASSERT(util_AsciiCharsActLikeTheyShould());
    ASSERT(psz && pnCharsRead);
    ASSERT(util_IsOctal(*psz)); // at least one oct char

    int iValue;
    for (iValue = 0, *pnCharsRead = 0; *psz && util_IsOctal(*psz) && *pnCharsRead < 3; // limit of 3 octal chars
         psz++, (*pnCharsRead)++)
    {
        iValue *= 010;
        iValue += (*psz - '0');
    }

    return (iValue);
}

bool util_IsOctal(const char ch)
{
    ASSERT(util_AsciiCharsActLikeTheyShould());

    return (ch >= '0' && ch <= '7');
}

int util_GetEscapeValueOfChar(char ch)
{
    int iValue = -1;

    switch (ch)
    {
    case 'n':
        iValue = 10;
        break;
    case 't':
        iValue = 9;
        break;
    case 'v':
        iValue = 11;
        break;
    case 'b':
        iValue = 8;
        break;
    case 'r':
        iValue = 13;
        break;
    case 'f':
        iValue = 12;
        break;
    case 'a':
        iValue = 7;
        break;
    case '\\':
        iValue = 92;
        break;
    case '\?':
        iValue = 63;
        break;
    case '\'':
        iValue = 39;
        break;
    case '\"':
        iValue = 34;
        break;
    default:
    {
        // unrecognized escape char: just return character
        iValue = static_cast<int>(ch);
    }
    break;
    }

    ASSERT(iValue != -1); //should have *some* value at this point

    return (iValue);
}


// -1 means recurse all levels
int util_GetRecurseDepth(const cParseNamedAttrList* pList) //throw( eParserHelper )
{
    if (pList != NULL)
    {
        const cParseNamedAttr* pa = pList->Lookup(TSS_GetString(cTWParser, twparser::STR_PARSER_RECURSE));
        if (pa)
        {
            TSTRING str = pa->GetValue();

            std::transform(str.begin(), str.end(), str.begin(), _totlower);

            if (0 == str.compare(TSS_GetString(cTWParser, twparser::STR_PARSER_FALSE)))
            {
                return 0;
            }
            else if (0 == str.compare(TSS_GetString(cTWParser, twparser::STR_PARSER_TRUE)))
            {
                return -1;
            }
            else
            {
                // must be number
                int                 i;
                cRecurseDepthLimits rdl;
                if (!rdl.InterpretInt(str, &i))
                    throw eParserUnrecognizedAttrVal(str, pa->GetLineNum());
                return i;
            }
        }
    }

    return (-1); // defaults to recurse all levels
}

// finds first '&', '+', '-'  on or after str[i]
TSTRING::size_type util_FindNextDelim(const TSTRING& str, TSTRING::size_type i)
{
    TSTRING::size_type min   = TSTRING::npos;
    TSTRING::size_type amp   = str.find('&', i);
    TSTRING::size_type plus  = str.find('+', i);
    TSTRING::size_type minus = str.find('-', i);

    // now find minimum of the three
    if (amp != TSTRING::npos)
        min = amp;
    if (min != TSTRING::npos && plus != TSTRING::npos && plus < min)
        min = plus;
    if (min != TSTRING::npos && minus != TSTRING::npos && minus < min)
        min = minus;

    return min;
}

void util_LoseSurroundingWS(TSTRING& str)
{
    TSTRING::size_type nonWSStart = str.find_first_not_of(_T(" \t"), 0);
    TSTRING::size_type nonWSEnd   = str.find_last_not_of(_T(" \t"));

    if (TSTRING::npos == nonWSStart) // if only whitespace
    {
        ASSERT(TSTRING::npos == nonWSEnd);
        str = str.erase();
    }
    else // has non WS chars
    {
        ASSERT(TSTRING::npos != nonWSEnd);
        str = str.substr(nonWSStart, nonWSEnd - nonWSStart + 1);
    }
}


#ifdef DEBUG
bool util_AsciiCharsActLikeTheyShould()
{
    // we need numbers whose character
    // representation increase by one
    return (('0' < '1') && ('0' + 1 == '1') && ('a' < 'b') && ('a' + 1 == 'b') && ('A' < 'B') && ('A' + 1 == 'B'));
}
#endif
