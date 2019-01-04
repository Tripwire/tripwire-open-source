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
// policyparser.h -- wrapper around yacc generated classes.  intended to be used
//                  as interface to parser
//
// cPolicyParser:           object responsible for reading policy file, and
//                          generating list of FCOSpecs
//

#ifndef __POLICYPARSER_H
#define __POLICYPARSER_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __PARSERHELPER_H
#include "parserhelper.h"
#endif

#ifndef __GENREPARSEINFO_H
#include "genreparseinfo.h"
#endif

#ifndef __GENRESPECLIST_H
#include "fco/genrespeclist.h"
#endif

#ifndef YYNEWLINE
#include "twparser/yylex.h"
#include "twparser/yyparse.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

///////////////////////////////////////////////
// wrapper around yy_scan
//      we do this so we can overload member functions
class tw_yy_scan : public yy_scan
{
    enum
    {
        MAX_TOKEN_LENGTH = 1024
    };

public:
    explicit tw_yy_scan(std::istream& i)
        : yy_scan(MAX_TOKEN_LENGTH), mIn(i){}; // need to increase token length over mks default

    virtual int yygetc()
    {
        return mIn.get();
    };

    virtual void yyerror(const char* pszErr, ...); //throw( eParserHelper )
        // this is the MKS error function.  But, since some operating systems (e.g. like AIX)
        // don't offer a vnsprintf, so there's no way we can safely output the error
        // from the va_arg list to a string without possible buffer overflow.
        // So, only call this function with a fully formatted message.
        // NOTE: I repeat:  this version ALWAYS IGNORES the va_list!!  Even though it says "...",

    virtual void output(int c); //throw( eParserHelper )
                                // this is what lex calls when it gets to a char that it doesn't recognize.
                                // we throw an error.
private:
    tw_yy_scan(const tw_yy_scan&);
    tw_yy_scan& operator=(const tw_yy_scan&);

    // input stream
    std::istream& mIn;
};

///////////////////////////////////////////////
// wrapper around yy_parse
//      we do this so we can overload member functions
class tw_yy_parse : public yy_parse
{
    enum
    {
        PARSER_STATE_STACK_SIZE = 150
    };

public:
    tw_yy_parse() : yy_parse(PARSER_STATE_STACK_SIZE){}; // need to increase token length over mks default
private:
    tw_yy_parse(const tw_yy_parse&);
    tw_yy_parse& operator=(const tw_yy_parse&);
};

///////////////////////////////////////////////////////////////////////////////
// cPolicyParser
//      input:      istream& -- parser input
//      output:     list of (iFCOSpec *) -- caller is responsible for freeing
//
//      when parsing is done, this object can be destroyed, and no trace of the
//      parsing should exist
//

class cPolicyParser
{
public:
    explicit cPolicyParser(std::istream& in); // input source

    void Execute(cGenreSpecListVector& policy, cErrorBucket* pError); //throw(eError);
    void Check(cErrorBucket* pError);                                 //throw(eError);
                                                                      // do the parsing
private:
    cPolicyParser(const cPolicyParser&);
    cPolicyParser& operator=(const cPolicyParser&);

    std::string ConvertMultibyte(std::istream& in);
    // turns all mb chars > 1 byte or unrecognizable chars
    // into escaped hex (\xXX)

    tw_yy_parse   mParser;
    std::istream& mIn;
};


#endif
