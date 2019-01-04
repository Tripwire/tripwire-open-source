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
#include "stdtwparser.h"



const int TWP_COMMA = 257;
const int TWP_LPAREN = 258;
const int TWP_RPAREN = 259;
const int TWP_COMMENT = 260;
const int TWP_STRING = 261;
const int TWP_SEMICOLON = 262;
const int TWP_LBRACE = 263;
const int TWP_RBRACE = 264;
const int TWP_DEFAULT = 265;
const int TWP_PIPE = 266;
const int TWP_DOLLAR = 267;
const int TWP_BANG = 268;
const int TWP_PLUS = 269;
const int TWP_MINUS = 270;
const int TWP_DQUOTE = 271;
const int TWP_RARROW = 272;
const int TWP_SECTION = 273;
const int TWP_IFHOST = 274;
const int TWP_ELSE = 275;
const int TWP_ENDIF = 276;
const int TWP_ECHO = 277;
const int TWP_ERROR = 278;
const int TWP_DEFINE = 279;
const int TWP_UNDEF = 280;
const int TWP_CONTENTS = 281;
const int TWP_DBASEVERSION = 282;
const int TWP_ANDAND = 283;
const int TWP_OROR = 284;
const int TWP_BSLASH = 285;
const int TWP_ESCCHAR = 286;
const int TWP_QSTRING = 287;
const int TWP_EQUALS = 288;
const int TWP_GLOBAL_STRING = 289;
typedef union {
    class cParseString*           mpString;                 // wrapper around TSTRING
    class cParseStringList*       mpStringList;

    class cParseRule*             mpNode;                   // representation of FCOSpec

    class cParseSpecMask*         mpSpecMask;               // representation of specmask
    class cParseSpecMaskList*     mpSpecMaskList;           //      ...and a collection 

    class cParseNamedAttr*        mpAttr;                   // representation of an attribute
    class cParseNamedAttrList*    mpAttrList;               //      ...and a collection 
} YYSTYPE;
extern YYSTYPE yylval;

#ifdef YYTRACE
#define YYDEBUG 1
#else
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#endif
// C++ YACC parser header
// Copyright 1991 by Mortice Kern Systems Inc.  All rights reserved.
//
// yy_parse => class defining a parsing object
//  yy_parse needs a class yy_scan, which defines the scanner.
// %prefix or option -p xx determines name of this class; if not used,
// defaults to 'yy_scan'
//
// constructor fills in the tables for this grammar; give it a size
//    to determine size of state and value stacks. Default is 150 entries.
// destructor discards those state and value stacks
//
// int yy_parse::yyparse(yy_scan *) invokes parse; if this returns,
//  it can be recalled to continue parsing at last point.
// void yy_parse::yyreset() can be called to reset the parse;
//  call yyreset() before yy_parse::yyparse(yy_scan *)
#include <stdio.h>      // uses printf(), et cetera
#include <stdlib.h>     // uses exit()

const int YYERRCODE = 256;      // YACC 'error' value

// You can use these macros in your action code
#define YYERROR     goto yyerrlabel
#define YYACCEPT    YYRETURN(0)
#define YYABORT     YYRETURN(1)
#define YYRETURN(val)   return(val)

/*
 * Simulate bitwise negation as if it was done ona two's complement machine.
 * This makes the generated code portable to machines with different 
 * representations of integers (ie. signed magnitude).
 */
#define yyneg(s)    (-((s)+1))

#if YYDEBUG
typedef struct yyNamedType_tag {    /* Tokens */
    char    * name;     /* printable name */
    short   token;      /* token # */
    short   type;       /* token type */
} yyNamedType;
typedef struct yyTypedRules_tag {   /* Typed rule table */
    char    * name;     /* compressed rule string */
    short   type;       /* rule result type */
} yyTypedRules;
#endif

#if 0 // YACC_WINDOWS

// include all windows prototypes, macros, constants, etc.

#include <windows.h>

// the following is the handle to the current
// instance of a windows program. The user
// program calling yyparse must supply this!

#ifdef STRICT
extern HINSTANCE hInst; 
#else
extern HANDLE hInst;    
#endif

#endif  /* YACC_WINDOWS */


class yy_parse {
protected:

#if 0 // YACC_WINDOWS

    // protected member function for actual scanning 

    int win_yyparse(yy_scan * ps);  // parse with given scanner

#endif  /* YACC_WINDOWS */

    int mustfree;   // set if tables should be deleted
    int size;       // size of state and value stacks
    int reset;      // if set, reset state
    short   yyi;        // table index
    short   yystate;    // current state

    short   * stateStack;   // states stack
    YYSTYPE * valueStack;   // values stack
    short   * yyps;     // top of state stack
    YYSTYPE * yypv;     // top of value stack

    YYSTYPE yylval;     // saved yylval
    YYSTYPE yyval;      // $
    YYSTYPE * yypvt;    // $n
    int yychar;     // current token
    int yyerrflag;  // error flag
    int yynerrs;    // error count
#if YYDEBUG
    int done;       // set from trace to stop parse
    int rule, npop; // reduction rule # and length
    short   * typeStack;    // type stack to mirror valueStack[]
    short   * yytp;     // top of type stack
    char    * yygetState(int);  // read 'states.out'
#endif
public:
#if YYDEBUG
    // C++ has trouble with initialized arrays inside classes
    static long * States;       // pointer to yyStates[]
    static yyTypedRules * Rules;    // pointer to yyRules[]
    static yyNamedType * TokenTypes; // pointer to yyTokenTypes[]
    static int  yyntoken;   // number of tokens
    static int  yynvar;     // number of variables (nonterminals)
    static int  yynstate;   // number of YACC-generated states
    static int  yynrule;    // number of rules in grammar

    char*   yyptok(int);        // printable token string
    int yyExpandName(int, int, char *, int);
                        // expand encoded string
    virtual int yyGetType(int);     // return type of token
    virtual void    yyShowRead();       // see newly read token
    virtual void    yyShowState();      // see state, value stacks
    virtual void    yyShowReduce();     // see reduction
    virtual void    yyShowGoto();       // see goto
    virtual void    yyShowShift();      // see shift
    virtual void    yyShowErrRecovery();    // see error recovery
    virtual void    yyShowErrDiscard(); // see token discard in error
#endif
    yy_scan* scan;          // pointer to scanner
    int yydebug;    // if set, tracing if compiled with YYDEBUG=1

    yy_parse(int = 150);    // constructor for this grammar
    yy_parse(int, short *, YYSTYPE *);  // another constructor

    ~yy_parse();        // destructor

    int yyparse(yy_scan * ps);  // parse with given scanner

    void    yyreset() { reset = 1; } // restore state for next yyparse()

    void    setdebug(int y) { yydebug = y; }

// The following are useful in user actions:

    void    yyerrok() { yyerrflag = 0; }    // clear error
    void    yyclearin() { yychar = -1; }    // clear input
    int YYRECOVERING() { return yyerrflag != 0; }
};
// end of .hpp header

