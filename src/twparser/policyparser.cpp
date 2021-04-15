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
// policyparser.cpp -- intended to be used as interface to parser
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdtwparser.h"
#include "policyparser.h"
#include "core/twlocale.h"
#include "core/stringutil.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <iomanip>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#include "core/ntmbs.h"

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

static std::string util_ConvertMB(const std::string& sIn);
//static bool         util_ReadMBCharAsSBString( std::stringstream& ss, std::string& strCh );
static std::string convert_to_encoded_hex(char ch);
//static std::string  util_Output512Chars( const std::string& s );
// don't want to pass debug a string over 1024, so play it safe with 512

//=========================================================================
// PUBLIC METHOD CODE
//=========================================================================

//
// constructor
//
cPolicyParser::cPolicyParser(std::istream& in) : mParser(), mIn(in)
{
}

void tw_yy_scan::output(int c)
{
    TOSTRINGSTREAM sstr;
    TCHAR          sz[2];

    sstr << TSS_GetString(cTWParser, twparser::STR_PARSER_INVALID_CHAR);

    sz[0] = (unsigned char)c; // don't want to sign extend this
    sz[1] = 0;
    sstr << sz;

    tss_mkstr(errStr, sstr);
    
    throw eParseFailed(errStr);
}


///////////////////////////////////////////////////////////////////////////////
// Check -- only checks syntax and basic semantics.
//
//          on failure, it will throw an error
///////////////////////////////////////////////////////////////////////////////
void cPolicyParser::Check(cErrorBucket* pError) //throw(eError);
{
    cParserHelper::Init(pError);
    cParserHelper::SetParseOnly(true);

#if !ARCHAIC_STL
    TISTRINGSTREAM in(ConvertMultibyte(mIn));
#else    
    TSTRINGSTREAM in;
    in << ConvertMultibyte(mIn);
#endif

    tw_yy_scan     lexer(in);

    // start the parsing
    if (mParser.yyparse(&lexer) != 0)
        throw eParseFailed(_T(""));

    cParserHelper::Finit(NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Execute -- attempts to parse input to create list of fcospecs
//
//          on failure, it will throw an error (binding failed, or parse error)
//          on success, the list will be populated
///////////////////////////////////////////////////////////////////////////////
void cPolicyParser::Execute(cGenreSpecListVector& policy, cErrorBucket* pError) //throw( eParserHelper )
{
    cParserHelper::Init(pError);

#if !ARCHAIC_STL    
    TISTRINGSTREAM in(ConvertMultibyte(mIn));
#else
    TSTRINGSTREAM in;
    in << ConvertMultibyte(mIn);
#endif
    
    tw_yy_scan     lexer(in);

    // start the parsing
    if (mParser.yyparse(&lexer) != 0)
        throw eParseFailed(_T(""));

    cParserHelper::Finit(&policy);
}


// NOTE: this version ALWAYS IGNORES the va_list!!  Even though it says "...",
// it doesn't look at any args following pszErr
// Only call this with fully formatted message
// Parser will ALWAYS call the narrow-char version, so special case Unicode compile
void tw_yy_scan::yyerror(const char* pszErr, ...) //throw( eParserHelper )
{
#if 0  
    TOSTRINGSTREAM ssError; // final error string
    ssError << pszErr;
    throw eParseFailed(ssError.str());    
#endif
    
    TSTRING errText = pszErr ? pszErr : "";
    throw eParseFailed(errText);
}

// Throw this in the display encoder?
std::string cPolicyParser::ConvertMultibyte(std::istream& in)
{
    // get this file as a stringstream
    TOSTRINGSTREAM ss;

    while (true)
    {
        // get next character
        char ch;
        in.get(ch);

        // check stream state
        if (in.eof())
            break; // we're done!
        if (in.bad() || in.fail())
            throw eParserBadCharacter();

        // add next character to output buffer
        ss.put(ch);

        // check stream state
        if (ss.bad() || ss.fail())
            throw eParserBadCharacter();
    }

    tss_end(ss);
    
#ifdef DEBUG
    TCERR << _T("*** begin policy text ***") << std::endl;
    std::cerr << ss.str() << std::endl;
    TCERR << _T("*** end policy text ***") << std::endl << std::endl;
#endif

    // convert it to single byte escaped
    std::string str = util_ConvertMB(ss.str());
    tss_free(ss);
    
#ifdef DEBUG
    TCERR << _T("*** begin policy converted to ***") << std::endl;
    std::cerr << str << std::endl;
    TCERR << _T("*** end policy converted to  ***") << std::endl << std::endl;
#endif

    return str;
}

std::string convert_to_encoded_hex(char ch)
{
    TOSTRINGSTREAM sstr;

    // set up stringstream
    tss_classic_locale(sstr);
    sstr.setf(std::ios::hex, std::ios::basefield);

#if !ARCHAIC_STL        
    // get the num_put facet
    const std::num_put<char>*pnp = 0, &np = tss::GetFacet(sstr.getloc(), pnp);
    // output the char
    sstr << "\\x";
    
    np.put(sstr, sstr, sstr.fill(), (long)(unsigned char)ch);
#else
    sstr << "\\x" << ch;
#endif

    tss_return_stream(sstr, out);
}


std::string util_ConvertMB(const std::string& sIn)
{
#if !USES_MBLEN
    return sIn;
#else
    cDebug d("cPolicyParser::util_ConvertMB");

    std::string                 sOut;
    std::string::const_iterator at;

    for (at = sIn.begin(); at != sIn.end();
         // at gets incremented when used....
    )
    {
        int nBytes = ::mblen((char*)&at[0], MB_CUR_MAX);
        if (nBytes == -1)
        {
            d.TraceDebug("Unrecognized Character: %c\n", *at);
            if ((unsigned char)*at > 0x7F)
            {
                d.TraceDebug("Normalizing.\n");
                sOut += convert_to_encoded_hex(*(at++));
            }
            else
            {
                d.TraceDebug("Incorrectly Formed, Cannot Normalize!\n");
                std::string s(1, *at);
                TSTRING     ts = cStringUtil::StrToTstr(s);
                throw eParserBadCharacter(ts);
            }
        }
        else if (nBytes == 0)
        {
            break; // done with sIn
        }
        else if (nBytes == 1)
        {
            sOut += *(at++); // regular SB char
        }
        else // ( nBytes > 1 )
        {
            // mb char -- output it as encoded bytes
            while (nBytes-- > 0)
            {
                ASSERT(at != sIn.end());
                sOut += convert_to_encoded_hex(*(at++));
            }
        }
    }

    return sOut;
#endif
}
