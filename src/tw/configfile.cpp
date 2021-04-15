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
// configfile.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdtw.h"
#include "configfile.h"
#include "core/fsservices.h"
#include "core/hashtable.h"
#include "twcrypto/cryptoarchive.h"
#include "core/fileheader.h"
#include "core/timeconvert.h"
#include "tw/systeminfo.h"
#include "tw/twstrings.h"
#include "tw/twutil.h"
#include "core/charutil.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <fstream>
#include <iomanip>

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
// UTIL FUNCTION PROTOTYPES
//=========================================================================

static bool PopNextLine(TSTRING& fileIn, TSTRING& sLine, int& nLine);
static bool IsReturnChar(TCHAR tch);
static bool IsComment(const TSTRING& sLine);
//static void GetKeyValuePair( const TSTRING& sLine, TSTRING& sKey, TSTRING& sVal ); // throw( eConfigFile );
static TSTRING& util_MakeTripwireDateString(TSTRING& strBuf);

//=========================================================================
// UTIL FUNCTIONS
//=========================================================================

static bool IsReturnChar(TCHAR tch)
{
    return (tch == _T('\n') || tch == _T('\r'));
}

static bool IsWhiteSpace(TCHAR tch)
{
    return ((tch >= 0x09 && tch <= 0x0D) || tch == 0x20);
}

static bool IsPoundSymbol(TCHAR tch)
{
    return (tch == _T('#'));
}

static bool IsRightParen(TCHAR tch)
{
    return (tch == _T(')'));
}

static bool IsLeftParen(TCHAR tch)
{
    return (tch == _T('('));
}

static bool IsDollarSign(TCHAR tch)
{
    return (tch == _T('$'));
}

static bool IsEqualsSign(TCHAR tch)
{
    return (tch == _T('='));
}

static bool IsSingleTchar(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (first + 1 == last);
}

static bool IsReturnChar(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsReturnChar(*first));
}

static bool IsWhiteSpace(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsWhiteSpace(*first));
}


static bool IsPoundSymbol(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsPoundSymbol(*first));
}

static bool IsRightParen(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsRightParen(*first));
}

static bool IsLeftParen(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsLeftParen(*first));
}

static bool IsDollarSign(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsDollarSign(*first));
}

static bool IsEqualsSign(TSTRING::const_iterator first, TSTRING::const_iterator last)
{
    return (IsSingleTchar(first, last) && IsEqualsSign(*first));
}


//=========================================================================
// METHOD CODE
//=========================================================================

///////////////////////////////////////////////////////////////////////////////
// ctor, dtor
///////////////////////////////////////////////////////////////////////////////
cConfigFile::cConfigFile() : mStringHashTable(HASH_VERY_SMALL), mnLine(1)
{
    // we make the hash table very small (17 slots) because we don't expect very many things
    // in the config file -- mdb
}

cConfigFile::~cConfigFile()
{
}


void cConfigFile::WriteString(TSTRING& configText) // throw( eFSServices )
{
    TOSTRINGSTREAM out;

    TSTRING                          sKey, sVal;
    cHashTableIter<TSTRING, TSTRING> hashTableIter(mStringHashTable);

    // for all name-value pairs, print them out as "name=value\n"
    for (hashTableIter.SeekBegin(); !hashTableIter.Done(); hashTableIter.Next())
    {
        sKey = hashTableIter.Key();
        sVal = hashTableIter.Val();

        out << sKey << _T("=") << sVal << _T("\n");
    }

    tss_stream_to_string(out, configText);
}

void cConfigFile::ReadString(const TSTRING configText) // throw( eConfigFile );
{
#ifdef DEBUG
    // NOTE:BAM -- debug only code !
    TCERR << _T("*** begin config text ***") << std::endl;
    TCERR << configText << std::endl;
    TCERR << _T("*** end config text ***") << std::endl;
    TCERR << std::endl;
    // NOTE:BAM -- debug only code !
#endif

    TSTRING sLine;
    TSTRING sConfigBuf;
    for (sConfigBuf = configText; PopNextLine(sConfigBuf, sLine, mnLine); sLine.erase())
    {
        // ignore comments
        if (!IsComment(sLine))
        {
            TSTRING sKey, sVal;

            GetKeyValuePair(sLine, sKey, sVal);
            DoVarSubst(sVal);
            mStringHashTable.Insert(sKey, sVal);
        }
    }

    CheckThatAllMandatoryKeyWordsExists();

#ifdef DEBUG
    // NOTE:BAM -- debug only code !
    TSTRING sTemp;
    WriteString(sTemp);
    TCERR << _T("*** begin config read as ***") << std::endl;
    TCERR << sTemp << std::endl;
    TCERR << _T("*** end config read as ***") << std::endl;
    TCERR << std::endl;
    // NOTE:BAM -- debug only code !
#endif

    return;
}

bool cConfigFile::Lookup(const TSTRING& sKey, TSTRING& sVal) const
{
    bool fFound = false;

    if (mStringHashTable.Lookup(sKey, sVal))
    {
        // key was found
        fFound = true;
    }

    return (fFound);
}

void cConfigFile::Insert(const TSTRING& sKey, const TSTRING& sVal)
{
    mStringHashTable.Insert(sKey, sVal);
}

///////////////////////////////////////////////////////////////////////////////
// GetFileHeaderID()
///////////////////////////////////////////////////////////////////////////////

struct cConfigFileFHID
{
    cFileHeaderID* configID;

    cConfigFileFHID()
    {
        configID = 0;
    }
    ~cConfigFileFHID()
    {
        delete configID;
    }
} gConfigFileFHID;

const cFileHeaderID& cConfigFile::GetFileHeaderID()
{
    if (gConfigFileFHID.configID == 0)
        gConfigFileFHID.configID = new cFileHeaderID(_T("cConfigFile"));

    // sanity check
    ASSERT(*gConfigFileFHID.configID == cFileHeaderID(_T("cConfigFile")));

    return *gConfigFileFHID.configID;
}

//=========================================================================
// UTIL FUNCTION CODE
//=========================================================================

static bool PopNextLine(TSTRING& sIn, TSTRING& sLine, int& nLine)
{
    bool fGotNextLine = false;

    TSTRING::const_iterator       cur   = sIn.begin(); // pointer to working position in sIn
    const TSTRING::const_iterator end   = sIn.end();   // end of sIn
    TSTRING::const_iterator       first = end;         // identifies beginning of current character
    TSTRING::const_iterator       last  = end;         // identifies end of current character

    // eat up all return chars
    // while peek next char (does not update cur)
    while (cCharUtil::PeekNextChar(cur, end, first, last))
    {
        // is it a return char? (all return chars are single char)
        if (IsSingleTchar(first, last))
        {
            if (_T('\n') == *first)
            {
                // eat up return and increment line number
                nLine++;
                cCharUtil::PopNextChar(cur, end, first, last);
            }
            else if (_T('\r') == *first)
            {
                // eat up return but don't count it as another line
                cCharUtil::PopNextChar(cur, end, first, last);
            }
            else
            {
                // not a return char.  now get line
                break;
            }
        }
        else
        {
            // not a return char.  now get line
            break;
        }
    }

    // get line
    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        if (!IsReturnChar(first, last))
        {
            fGotNextLine = true; // we'll say we've got the next line if we get at least one char
            sLine.append(first, last);
        }
        else
        {
            // either eof or a return char found, exit loop
            break;
        }
    }
    // 'first' is either end of line or return character

    // Pop line of of input string
    TSTRING sTmp;
    sTmp.assign(first, end);
    sIn = sTmp;

    return (fGotNextLine);
}


static bool IsComment(const TSTRING& sLine)
{
    TSTRING::const_iterator       cur   = sLine.begin(); // pointer to working position in sLine
    const TSTRING::const_iterator end   = sLine.end();   // end of sLine
    TSTRING::const_iterator       first = end;           // identifies beginning of current character
    TSTRING::const_iterator       last  = end;           // identifies end of current character

    // while pop next char (updates cur)
    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        if (!IsWhiteSpace(first, last))
            break;
    }

    // it is a comment if it is an empty line or it starts with a #
    return (first == end || IsPoundSymbol(first, last));
}

///////////////////////////////////////////////////////////////////////////////
// TrimSpace -- trims leading and trailing whitespace from a string
///////////////////////////////////////////////////////////////////////////////
static void TrimSpace(TSTRING& s)
{
    TSTRING::const_iterator       cur     = s.begin(); // pointer to working position in s
    const TSTRING::const_iterator end     = s.end();   // end of s
    TSTRING::const_iterator       first   = end;       // identifies beginning of current character
    TSTRING::const_iterator       last    = end;       // identifies end of current character
    TSTRING::const_iterator       firstNW = end;       // identifies beginning of first non-whitespace character
    TSTRING::const_iterator       firstTW = end;       // identifies beginning of first trailing whitespace character

    // find first non-whitespace char
    while (cCharUtil::PeekNextChar(cur, end, first, last))
    {
        if (IsWhiteSpace(first, last))
        {
            cCharUtil::PopNextChar(cur, end, first, last);
        }
        else
        {
            break;
        }
    }
    if (first == end) // no non-whitespace char, so lose it all
    {
        s.erase();
        return;
    }

    // (first,last] now identify first non-whitespace character
    firstNW = first;

    // find first of trailing whitespace
    bool fInWhitespace = false;
    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        if (IsWhiteSpace(first, last))
        {
            if (!fInWhitespace)
            {
                firstTW = first;
            }
            // else we already know the beginning of the whitespace

            fInWhitespace = true;
        }
        else
        {
            fInWhitespace = false;
            firstTW       = end; // clear old trailing WS marker
        }
    }
    ASSERT(first == end);
    ASSERT(firstNW < firstTW);

    TSTRING sTmp;
    sTmp.assign(firstNW, firstTW);
    s = sTmp;
}

void cConfigFile::GetKeyValuePair(const TSTRING& sLine, TSTRING& sKey, TSTRING& sVal) // throw (eConfigFile)
{
    // erase old values
    sKey.erase();
    sVal.erase();

    TSTRING::const_iterator       cur   = sLine.begin(); // pointer to working position in sLine
    const TSTRING::const_iterator end   = sLine.end();   // end of sLine
    TSTRING::const_iterator       first = end;           // identifies beginning of current character
    TSTRING::const_iterator       last  = end;           // identifies end of current character

    while (cCharUtil::PopNextChar(cur, end, first, last))
    {
        if (IsEqualsSign(first, last))
        {
            break;
        }
    }

    if (first == end)
    {
        throw eConfigFileNoEq(MakeErrorString());
    }


    // get the two strings..
    sKey.assign(sLine.begin(), first);
    sVal.assign(last, sLine.end());

    // trim the white space from the beginning and end...
    TrimSpace(sKey);
    TrimSpace(sVal);

    //
    // find more errors
    //
    // 1. empty key string
    // 2. assignment to a predefined var
    //
    // allow empty values, but emit warning
    //
    TSTRING tstrDummy;

    if (sKey.empty())
    {
        throw eConfigFileNoKey(MakeErrorString());
    }
    else if (IsPredefinedVar(sKey, tstrDummy))
    {
        throw eConfigFileAssignToPredefVar(MakeErrorString(sKey));
    }
    else if (sVal.empty())
    {
        cTWUtil::PrintErrorMsg(eConfigFileEmptyVariable(MakeErrorString(sKey), eError::NON_FATAL));
    }

    // everything went ok.
}


///////////////////////////////////////////////////////////////////////////////
// DoVarSubst()
//      replaces any $(VAR) with either built-in functions, or in the cfg symbol table
//      fails if symbol isn't found
///////////////////////////////////////////////////////////////////////////////

void cConfigFile::DoVarSubst(TSTRING& s) const //throw( eConfigFile )
{
    cDebug d("cConfigFile::DoVarSubst()");
    d.TraceDebug(_T("ORIG: %s\n"), s.c_str());

    TSTRING sOut;

    // walk through string
    //      look for $(
    //          find matching )
    //          pick out subset
    //          look up in symbol table
    //          substitute subset
    //      continue iterating


    TSTRING::const_iterator       cur   = s.begin(); // pointer to working position in s
    const TSTRING::const_iterator end   = s.end();   // end of s
    TSTRING::const_iterator       first = end;       // identifies beginning of current character
    TSTRING::const_iterator       last  = end;       // identifies end of current character

    // look for '$('
    do
    {
        if (cCharUtil::PeekNextChar(cur, end, first, last) && IsDollarSign(first, last))
        {
            cCharUtil::PopNextChar(cur, end, first, last);

            if (cCharUtil::PeekNextChar(cur, end, first, last) && IsLeftParen(first, last))
            {
                TSTRING::const_iterator firstRP = end; // identifies beginning of rparen character
                TSTRING::const_iterator lastRP  = end; // identifies end of rparen character

                // [first,last) now identifies left paren
                while (cCharUtil::PeekNextChar(cur, end, firstRP, lastRP))
                {
                    if (IsRightParen(firstRP, lastRP))
                        break;
                    else
                        cCharUtil::PopNextChar(cur, end, firstRP, lastRP);
                }
                // [firstRP,lastRP) identifies right paren

                if (!IsRightParen(firstRP, lastRP))
                {
                    throw eConfigFileMissingRightParen(MakeErrorString(s));
                }

                // now get text between parens
                TSTRING sVarName;
                sVarName.assign(last, firstRP);
                d.TraceDebug(_T("symbol = %s\n"), sVarName.c_str());

                // look up in symbol table
                TSTRING sVarValue;
                if (!IsPredefinedVar(sVarName, sVarValue) && !mStringHashTable.Lookup(sVarName, sVarValue))
                {
                    throw eConfigFileUseUndefVar(MakeErrorString(sVarName));
                }

                sOut += sVarValue;
            }
            else
            {
                sOut.append(first, last);
            }
        }
        else
        {
            sOut.append(first, last);
        }
    } while (cCharUtil::PopNextChar(cur, end, first, last));

    d.TraceDebug(_T("DONE: %s\n"), sOut.c_str());
    s = sOut;
}


bool cConfigFile::IsPredefinedVar(const TSTRING& var, TSTRING& val) const
{
    bool fRecognizeVar = false;

    if (0 == var.compare(_T("HOSTNAME")))
    {
        fRecognizeVar = true;
        try
        {
            iFSServices::GetInstance()->GetMachineName(val);
            if (val.empty())
                val = _T("localhost");
        }
        catch (eFSServices&)
        {
            val = _T("localhost");
        }
    }
    else if (0 == var.compare(_T("DATE")))
    {
        util_MakeTripwireDateString(val);
        fRecognizeVar = true;
    }

    return (fRecognizeVar);
}

void cConfigFile::CheckThatAllMandatoryKeyWordsExists() // throw( eConfigFile )
{
    TSTRING astrMandatoryKeys[] = {
        // I don't think ROOT should be manditory, since we don't really use it
        // for anything -- mdb
        //_T("ROOT"),
        _T("POLFILE"),
        _T("DBFILE"),
        _T("REPORTFILE"),
        _T("SITEKEYFILE"),
        _T("LOCALKEYFILE"),
    };


    TSTRING strNotFound;
    bool    fAllFound = true;

    for (int i = 0; i < (int)countof(astrMandatoryKeys); i++)
    {
        TSTRING strDummy;
        if (!mStringHashTable.Lookup(astrMandatoryKeys[i], strDummy) || strDummy.empty())
        {
            strNotFound += astrMandatoryKeys[i];
            strNotFound += _T(" ");
            // key was found
            fAllFound = false;
        }
    }

    if (!fAllFound)
    {
        throw eConfigFileMissReqKey(MakeErrorString(strNotFound, false));
    }
}

///////////////////////////////////////////////////////////////////////////////
// MakeErrorString
///////////////////////////////////////////////////////////////////////////////
TSTRING cConfigFile::MakeErrorString(const TSTRING& strMsg, bool fShowLineNum) const
{
    TOSTRINGSTREAM strErr;

    strErr << strMsg;

    if (fShowLineNum)
    {
        // separate the message from the line number
        if (!strMsg.empty())
            strErr << _T(": ");

        // output the line number
        strErr << TSS_GetString(cTW, tw::STR_CUR_LINE) << mnLine;
    }

    tss_mkstr(out, strErr);
    return out;
}

TSTRING& util_MakeTripwireDateString(TSTRING& strBuf)
{
    struct tm*     ptmLocal = cTimeUtil::TimeToDateLocal(cSystemInfo::GetExeStartTime());
    TOSTRINGSTREAM ostr;
    tss_classic_locale(ostr);
    
    // format is YYYYMMDD-HHMMSS
    ostr.fill(_T('0'));
    ostr << std::setw(4) << ptmLocal->tm_year + 1900;
    ostr << std::setw(2) << ptmLocal->tm_mon + 1;
    ostr << std::setw(2) << ptmLocal->tm_mday;
    ostr << _T("-");
    ostr << std::setw(2) << ptmLocal->tm_hour;
    ostr << std::setw(2) << ptmLocal->tm_min;
    ostr << std::setw(2) << ptmLocal->tm_sec;

    tss_stream_to_string(ostr, strBuf);
    return strBuf;
}
