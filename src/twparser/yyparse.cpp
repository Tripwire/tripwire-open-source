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
/* ..\. -LC -o ..\twparser\yyparse.cpp -P ..\..\mkslexyacc\etc\yyparse.cpp -D yyparse.h policy.y */
// #line 74 "policy.y"


#include "stdtwparser.h"

#include "core/debug.h"
#include "parserhelper.h"
#include "genreparseinfo.h"
#include "policyparser.h"
#include "fco/genreswitcher.h"
#include "fco/twfactory.h"
#include "fco/fconameinfo.h"
#include "fco/parsergenreutil.h"
#include "fco/fconametranslator.h"
#include "core/usernotify.h"

#ifndef YYNEWLINE
# include "parser/yylex.h"
# include "parser/yyparse.h"
#endif


// global variables
int yaccdebuglevel;

// in MKS Yacc in C++ mode, we need to explicitly declare yylavl
YYSTYPE yylval;


static TSTRING ConcatenateStrings( const std::list<TSTRING>* plist )
{
    TSTRING strRet;
    for( std::list<TSTRING>::const_iterator i = plist->begin(); i != plist->end(); i++ )
        strRet += *i;
    return strRet;
}

static std::list<TSTRING>* MergeIntoList( std::list<TSTRING>* plistLHS, const std::list<TSTRING>* plistRHS )
{
    for( std::list<TSTRING>::const_iterator i = plistRHS->begin(); i != plistRHS->end(); i++ )
        plistLHS->push_back( *i );
    return plistLHS;
}

extern YYSTYPE yylval;
#if YYDEBUG
enum YY_Types { YY_t_NoneDefined, YY_t_mpString, YY_t_mpNode, YY_t_mpAttr, YY_t_mpAttrList, YY_t_mpStringList
};
#endif
#if YYDEBUG
yyTypedRules yyRules[] = {
    { "&00: %18 &00",  0},
    { "%18: %19",  0},
    { "%19: %20",  0},
    { "%19:",  0},
    { "%20: %20 %21",  0},
    { "%20: %21",  0},
    { "%21: %22 &07",  0},
    { "%21: %23 &07",  0},
    { "%21: %24 &07",  0},
    { "%21: &07",  0},
    { "%21: %25",  0},
    { "%21: %26",  0},
    { "%27:",  0},
    { "%26: &03 %04 &04 &08 %27 %19 &09",  0},
    { "%28:",  0},
    { "%25: &19 %16 %28 %19 %29 &21",  0},
    { "%25: &18 %09",  0},
    { "%25: &23 %09",  0},
    { "%25: &22 %09",  0},
    { "%16: %16 &29 %15",  5},
    { "%16: %15",  5},
    { "%30:",  0},
    { "%29: &20 %30 %19",  0},
    { "%29:",  0},
    { "%22: %13 &33 %10",  0},
    { "%23: %14 &33 %11",  0},
    { "%24: %06 &17 %01",  0},
    { "%24: &13 %06",  0},
    { "%01: %17 %05",  2},
    { "%05: &03 %04 &04",  4},
    { "%05:",  4},
    { "%04: %03 %31",  4},
    { "%03: %03 &02 %02",  4},
    { "%03: %02",  4},
    { "%02: %07 &33 %08",  3},
    { "%31: &02",  0},
    { "%31:",  0},
    { "%12: &12 &03 %13 &04",  1},
    { "%17: %10",  1},
    { "%07: %09",  1},
    { "%08: %10",  1},
    { "%06: %10",  5},
    { "%06: %10 &11 %10",  5},
    { "%15: %09",  1},
    { "%13: &06",  1},
    { "%10: %10 %09",  5},
    { "%10: %09",  5},
    { "%11: %11 %14",  5},
    { "%11: %14",  5},
    { "%09: &06",  1},
    { "%09: %12",  1},
    { "%14: &34",  1},
{ "$accept",  0},{ "error",  0}
};
yyNamedType yyTokenTypes[] = {
    { "$end",  0,  0},
    { "error",  256,  0},
    { "TWP_COMMA",  257,  0},
    { "TWP_LPAREN",  258,  0},
    { "TWP_RPAREN",  259,  0},
    { "TWP_COMMENT",  260,  0},
    { "TWP_STRING",  261,  1},
    { "TWP_SEMICOLON",  262,  0},
    { "TWP_LBRACE",  263,  0},
    { "TWP_RBRACE",  264,  0},
    { "TWP_DEFAULT",  265,  0},
    { "TWP_PIPE",  266,  0},
    { "TWP_DOLLAR",  267,  0},
    { "TWP_BANG",  268,  0},
    { "TWP_PLUS",  269,  0},
    { "TWP_MINUS",  270,  0},
    { "TWP_DQUOTE",  271,  0},
    { "TWP_RARROW",  272,  0},
    { "TWP_SECTION",  273,  0},
    { "TWP_IFHOST",  274,  0},
    { "TWP_ELSE",  275,  0},
    { "TWP_ENDIF",  276,  0},
    { "TWP_ECHO",  277,  0},
    { "TWP_ERROR",  278,  0},
    { "TWP_DEFINE",  279,  0},
    { "TWP_UNDEF",  280,  0},
    { "TWP_CONTENTS",  281,  0},
    { "TWP_DBASEVERSION",  282,  0},
    { "TWP_ANDAND",  283,  0},
    { "TWP_OROR",  284,  0},
    { "TWP_BSLASH",  285,  0},
    { "TWP_ESCCHAR",  286,  0},
    { "TWP_QSTRING",  287,  0},
    { "TWP_EQUALS",  288,  0},
    { "TWP_GLOBAL_STRING",  289,  1}

};
#endif
static short yydef[] = {

    (short)65535, (short)65531,    9,   53, (short)65527,    3, (short)65523,   10,    7, (short)65519, 
       5,    4, (short)65515,   50, (short)65509,    8, (short)65505,  -35
};
static short yyex[] = {

       0,   52, (short)65535,    1,  288,   18, (short)65535,   13,    0,    0, 
    (short)65535,    1,  259,   49, (short)65535,    1,  262,    6, (short)65535,    1, 
     275,   52,  276,   52, (short)65535,    1,  276,   51, (short)65535,    1, 
     264,   52, (short)65535,    1,  276,   52, (short)65535,    1
};
static short yyact[] = {

    (short)65499, (short)65534, (short)65455, (short)65490, (short)65491, (short)65497, (short)65498, (short)65495, (short)65496, (short)65489, 
     289,  278,  277,  274,  273,  268,  267,  262,  261,  258, 
    (short)65487, (short)65503, (short)65490,  267,  266,  261, (short)65504,  258, (short)65487, (short)65490, 
     267,  261, (short)65505,  272, (short)65506,  288, (short)65507,  288, (short)65461,  262, 
    (short)65460,  262, (short)65459,  262, (short)65482,  261, (short)65489,  289, (short)65511,  284, 
    (short)65512,  288, (short)65522,  257, (short)65513,  259, (short)65479,  259, (short)65514,  258, 
    (short)65462,  263, (short)65470,  275, (short)65474,  259, (short)65464,  276, (short)65463,  264,   -1
};
static short yypact[] = {

      10,   20,   23,   10,   44,   49,   53,   30,   30,   59, 
      47,   30,   10,   30,   63,   30,   10,   10,   69,   67, 
      65,   30,   61,   30,   30,   57,   55,   51,   30,   47, 
      30,   45,   30,   43,   41,   39,   30,   30,   30,   30, 
      30,   37,   35,   33,   30,   27
};
static short yygo[] = {

    (short)65471, (short)65476, (short)65477,   13, (short)65529, (short)65515, (short)65509,   21, (short)65473, (short)65472, 
    (short)65492,   44, (short)65508, (short)65478, (short)65483, (short)65483, (short)65483, (short)65483, (short)65480, (short)65483, 
    (short)65480, (short)65481, (short)65480, (short)65481, (short)65465, (short)65466, (short)65467, (short)65484,   40,   39, 
      38,   37,   36,   24,   21,   15,   13,   11,    8,    7, 
       2, (short)65520, (short)65524, (short)65527, (short)65528, (short)65533,   32,   30,   28,   23, 
    (short)65525, (short)65488, (short)65510, (short)65494,   31, (short)65485, (short)65486, (short)65493,   29,   10, 
    (short)65468, (short)65469,   24, (short)65530, (short)65526, (short)65531, (short)65521, (short)65517, (short)65452, (short)65458, 
      17,   16,   12, (short)65532, (short)65457, (short)65456,    3, (short)65502, (short)65501, (short)65500, 
    (short)65454, (short)65453, (short)65519, (short)65523, (short)65516, (short)65518, (short)65475,   -1
};
static short yypgo[] = {

       0,    0,    0,   83,   77,   78,    8,   64,   13,   10, 
      10,   57,   27,   27,   50,   50,   45,   45,   53,   61, 
      12,   51,    2,    4,    4,    6,    8,    0,   79,   79, 
      85,   63,   63,   80,   80,   80,   80,   81,   82,   75, 
      75,   75,   65,   73,   73,   75,   75,   75,   84,   86, 
      86,   84,   69,   69,    0
};
static short yyrlen[] = {

       0,    0,    0,    0,    3,    3,    0,    1,    1,    1, 
       3,    1,    1,    1,    1,    2,    1,    2,    1,    1, 
       1,    4,    3,    1,    3,    2,    3,    2,    2,    3, 
       0,    1,    3,    2,    2,    2,    6,    7,    0,    2, 
       2,    2,    1,    2,    1,    1,    1,    1,    3,    0, 
       1,    0,    0,    1,    2
};
#define YYS0    0
#define YYDELTA 35
#define YYNPACT 46
#define YYNDEF  18

#define YYr52   0
#define YYr53   1
#define YYr54   2
#define YYr14   3
#define YYr24   4
#define YYr25   5
#define YYr30   6
#define YYr38   7
#define YYr40   8
#define YYr41   9
#define YYr42   10
#define YYr51   11
#define YYr50   12
#define YYr49   13
#define YYr48   14
#define YYr47   15
#define YYr46   16
#define YYr45   17
#define YYr44   18
#define YYr43   19
#define YYr39   20
#define YYr37   21
#define YYr34   22
#define YYr33   23
#define YYr32   24
#define YYr31   25
#define YYr29   26
#define YYr28   27
#define YYr27   28
#define YYr26   29
#define YYr21   30
#define YYr20   31
#define YYr19   32
#define YYr18   33
#define YYr17   34
#define YYr16   35
#define YYr15   36
#define YYr13   37
#define YYr12   38
#define YYr8    39
#define YYr7    40
#define YYr6    41
#define YYrACCEPT   YYr52
#define YYrERROR    YYr53
#define YYrLR2  YYr54
#if YYDEBUG
char * yysvar[] = {
    "$accept",
    "spec_masks",
    "attribute",
    "attribute_list",
    "attribute_list_with_opt_trailing_comma",
    "opt_spec_attributes",
    "fco_name",
    "attribute_name",
    "attribute_value",
    "string",
    "multi_string",
    "global_multi_string",
    "variable",
    "variable_name",
    "global_string",
    "host_name",
    "host_name_list",
    "prop_vector",
    "policy",
    "opt_statement_list",
    "statement_list",
    "statement",
    "variable_assignment",
    "global_variable_assignment",
    "rule",
    "directive_block",
    "scoped_block",
    "$12",
    "$14",
    "opt_else_host",
    "$21",
    "opt_comma",
    0
};
short yyrmap[] = {

      52,   53,   54,   14,   24,   25,   30,   38,   40,   41, 
      42,   51,   50,   49,   48,   47,   46,   45,   44,   43, 
      39,   37,   34,   33,   32,   31,   29,   28,   27,   26, 
      21,   20,   19,   18,   17,   16,   15,   13,   12,    8, 
       7,    6,    1,    4,    5,    9,   10,   11,   22,   36, 
      35,   23,    3,    2,    0
};
short yysmap[] = {

       0,    4,    5,   23,   25,   39,   43,   49,   52,   53, 
      56,   57,   59,   61,   69,   70,   77,   79,   81,   76, 
      74,   65,   63,   60,   58,   51,   44,   41,   33,   32, 
      31,   29,   28,   21,   20,   19,   15,   14,   13,   12, 
      11,   10,    9,    8,    7,    6,    1,    2,   26,   55, 
      67,    3,   27,   50,   37,   40,   64,   71,   42,   72, 
      62,   78,   66,   30,   54,   75,   38,   68,   34,   35, 
      36,   80,   83,   73,   45,   46,   47,   24,   48,   22, 
      18,   17,   16,   82
};
int yy_parse::yyntoken = 35;
int yy_parse::yynvar = 32;
int yy_parse::yynstate = 84;
int yy_parse::yynrule = 55;
#endif



// #line 2 "..\\..\\mkslexyacc\\etc\\yyparse.cpp"
// C++ YACC parser code
// Copyright 1991 by Mortice Kern Systems Inc.  All rights reserved.
//
// If YYDEBUG is defined as 1 and yy_parse::yydebug is set to 1,
// yyparse() will print a travelogue of its actions as it reads
// and parses input.
//
// YYSYNC can be defined to cause yyparse() to attempt to always
// hold a lookahead token

const int YY_MIN_STATE_NUM = 20;    // not useful to be too small!

#if YYDEBUG
#ifdef YYTRACE
long    * yy_parse::States  = yyStates;
#endif
yyTypedRules * yy_parse::Rules  = yyRules;
yyNamedType * yy_parse::TokenTypes = yyTokenTypes;

#define YY_TRACE(fn) { done = 0; fn(); if (done) YYRETURN(-1); }
#endif

// Constructor for yy_parse: user-provided tables
yy_parse::yy_parse(int sz, short * states, YYSTYPE * stack)
{
    mustfree = 0;
    if ((size = sz) < YY_MIN_STATE_NUM
     || (stateStack = states) == (short *) 0
     || (valueStack = stack) == (YYSTYPE *) 0) {
        fprintf(stderr,"Bad state/stack given");
        exit(1);
    }
    reset = 1;      // force reset
#if YYDEBUG
    yydebug = 0;
    typeStack = new short[size+1];
    if (typeStack == (short *) 0) {
        fprintf(stderr,"Cannot allocate typeStack");
        exit(1);
    }
#endif
}
// Constructor for yy_parse: allocate tables with new
yy_parse::yy_parse(int sz)
{
    size = sz;
    reset = 1;      // force reset
    mustfree = 1;       // delete space in deconstructor
#if YYDEBUG
    yydebug = 0;
    typeStack = new short[size+1];
#endif
    stateStack = new short[size+1];
    valueStack = new YYSTYPE[size+1];

    if (stateStack == (short *) 0 || valueStack == (YYSTYPE *) 0
#if YYDEBUG
        || typeStack == (short *) 0
#endif
        ) {
        fprintf(stderr,"Not enough space for parser stacks");
        exit(1);
    }
}
// Destructor for class yy_parse
//  Free up space
yy_parse::~yy_parse()
{
    if (mustfree) {
        delete [] stateStack;
        delete [] valueStack;
    }
    stateStack = (short *) 0;
#if YYDEBUG
    delete [] typeStack;
#endif
}

#if 0 //YACC_WINDOWS

// The initial portion of the yacc parser.
// In an windows environment, it will load the desired
// resources, obtain pointers to them, and then call
// the protected member win_yyparse() to acutally begin the
// parsing. When complete, win_yyparse() will return a
// value back to our new yyparse() function, which will 
// record that value temporarily, release the resources
// from global memory, and finally return the value
// back to the caller of yyparse().

int
yy_parse::yyparse(yy_scan* ps)
{
    int wReturnValue;
    HANDLE hRes_table;
    short far *old_yydef;       // the following are used for saving
    short far *old_yyex;        // the current pointers
    short far *old_yyact;
    short far *old_yypact;
    short far *old_yygo;
    short far *old_yypgo;
    short far *old_yyrlen;

    // the following code will load the required
    // resources for a Windows based parser.

    hRes_table = LoadResource (hInst,
        FindResource (hInst, "UD_RES_yyYACC", "yyYACCTBL"));
    
    // return an error code if any
    // of the resources did not load

    if (hRes_table == (HANDLE)NULL)
        return (1);
    
    // the following code will lock the resources
    // into fixed memory locations for the parser
    // (also, save away the old pointer values)

    old_yydef = yydef;
    old_yyex = yyex;
    old_yyact = yyact;
    old_yypact = yypact;
    old_yygo = yygo;
    old_yypgo = yypgo;
    old_yyrlen = yyrlen;

    yydef = (short far *)LockResource (hRes_table);
    yyex = (short far *)(yydef + Sizeof_yydef);
    yyact = (short far *)(yyex + Sizeof_yyex);
    yypact = (short far *)(yyact + Sizeof_yyact);
    yygo = (short far *)(yypact + Sizeof_yypact);
    yypgo = (short far *)(yygo + Sizeof_yygo);
    yyrlen = (short far *)(yypgo + Sizeof_yypgo);

    // call the official yyparse() function

    wReturnValue = win_yyparse (ps);

    // unlock the resources

    UnlockResource (hRes_table);

    // and now free the resource

    FreeResource (hRes_table);

    //
    // restore previous pointer values
    //

    yydef = old_yydef;
    yyex = old_yyex;
    yyact = old_yyact;
    yypact = old_yypact;
    yygo = old_yygo;
    yypgo = old_yypgo;
    yyrlen = old_yyrlen;

    return (wReturnValue);
}   // end yyparse()


// The parser proper.
//  Note that this code is reentrant; you can return a value
//  and then resume parsing by recalling yyparse().
//  Call yyreset() before yyparse() if you want a fresh start

int
yy_parse::win_yyparse(yy_scan* ps)

#else /* YACC_WINDOWS */

// The parser proper.
//  Note that this code is reentrant; you can return a value
//  and then resume parsing by recalling yyparse().
//  Call yyreset() before yyparse() if you want a fresh start
int
yy_parse::yyparse(yy_scan* ps)

#endif /* YACC_WINDOWS */

{
#if 0 //YACC_WINDOWS
    short far *yyp; // needed as res tables locked in far memory
    short far *yyq;
#else
    short   * yyp, * yyq;       // table lookup
#endif /* YACC_WINDOWS */
    int yyj;
#if YYDEBUG
    int yyruletype = 0;
#endif

    if ((scan = ps) == (yy_scan *) 0) { // scanner
        fprintf(stderr,"No scanner");
        exit(1);
    }

    if (reset) {            // start new parse
        yynerrs = 0;
        yyerrflag = 0;
        yyps = stateStack;
        yypv = valueStack;
#if YYDEBUG
        yytp = typeStack;
#endif
        yystate = YYS0;
        yyclearin();
        reset = 0;
    } else          // continue saved parse
        goto yyNext;            // after action

yyStack:
    if (++yyps > &stateStack[size]) {
        scan->yyerror("Parser stack overflow");
        YYABORT;
    }
    *yyps = yystate;    /* stack current state */
    *++yypv = yyval;    /* ... and value */
#if YYDEBUG
    if (yydebug) {
        *++yytp = yyruletype;   /* ... and type */
        YY_TRACE(yyShowState)
    }
#endif

    /*
     * Look up next action in action table.
     */
yyEncore:
#ifdef YYSYNC
    if (yychar < 0) {
        if ((yychar = scan->yylex()) < 0) {
            if (yychar == -2) YYABORT;
            yychar = 0;
        }   /* endif */
        yylval = ::yylval;
#if YYDEBUG
        if (yydebug)
            yyShowRead();   // show new input token
#endif
    }
#endif
#if 0 //YACC_WINDOWS
    if (yystate >= Sizeof_yypact)   /* simple state */
#else /* YACC_WINDOWS */
    if (yystate >= (int)sizeof yypact/(int)sizeof yypact[0])    /* simple state */
#endif /* YACC_WINDOWS */
        yyi = yystate - YYDELTA;    /* reduce in any case */
    else {
        if(*(yyp = &yyact[yypact[yystate]]) >= 0) {
            /* Look for a shift on yychar */
#ifndef YYSYNC
            if (yychar < 0) {
                if ((yychar = scan->yylex()) < 0) {
                    if (yychar == -2) YYABORT;
                    yychar = 0;
                }   /* endif */
                yylval = ::yylval;
#if YYDEBUG
                if (yydebug)
                    yyShowRead();   // show new input token
#endif
            }
#endif
            yyq = yyp;
            yyi = yychar;
            while (yyi < *yyp++)
                ;
            if (yyi == yyp[-1]) {
                yystate = yyneg(yyq[yyq-yyp]);
#if YYDEBUG
                if (yydebug) {
                    yyruletype = yyGetType(yychar);
                    YY_TRACE(yyShowShift)
                }
#endif
                yyval = yylval;     /* stack value */
                yyclearin();        /* clear token */
                if (yyerrflag)
                    yyerrflag--;    /* successful shift */
                goto yyStack;
            }
        }

        /*
         *  Fell through - take default action
         */

#if 0 //YACC_WINDOWS
        if (yystate >= Sizeof_yydef)    /* simple state */
#else /* YACC_WINDOWS */
        if (yystate >= (int)sizeof yydef /(int)sizeof yydef[0])
#endif /* YACC_WINDOWS */
            goto yyError;
        if ((yyi = yydef[yystate]) < 0)  { /* default == reduce? */

            /* Search exception table */
            yyp = &yyex[yyneg(yyi)];
#ifndef YYSYNC
            if (yychar < 0) {
                if ((yychar = scan->yylex()) < 0) {
                    if (yychar == -2) YYABORT;
                    yychar = 0;
                }   /* endif */
                yylval = ::yylval;
#if YYDEBUG
                if (yydebug)
                    yyShowRead();   // show new input token
#endif
            }
#endif
            while((yyi = *yyp) >= 0 && yyi != yychar)
                yyp += 2;
            yyi = yyp[1];
        }
    }

    yyj = yyrlen[yyi];

#if YYDEBUG
    if (yydebug) {
        npop = yyj; rule = yyi;
        YY_TRACE(yyShowReduce)
        yytp -= yyj;
    }
#endif
    yyps -= yyj;        /* pop stacks */
    yypvt = yypv;       /* save top */
    yypv -= yyj;
    yyval = yypv[1];    /* default action $ = $1 */
#if YYDEBUG
    if (yydebug)
        yyruletype = yyRules[yyrmap[yyi]].type;
#endif
    switch (yyi) {      /* perform semantic action */
        
case YYr6: {    /* statement :  variable_assignment TWP_SEMICOLON */
// #line 194 "policy.y"
 cParserHelper::IncrementScopeStatementCount(); 
} break;

case YYr7: {    /* statement :  global_variable_assignment TWP_SEMICOLON */
// #line 195 "policy.y"
 cParserHelper::IncrementScopeStatementCount(); 
} break;

case YYr8: {    /* statement :  rule TWP_SEMICOLON */
// #line 196 "policy.y"
 cParserHelper::IncrementScopeStatementCount(); 
} break;

case YYr12: {   /* scoped_block :  TWP_LPAREN attribute_list_with_opt_trailing_comma TWP_RPAREN TWP_LBRACE */
// #line 204 "policy.y"

                cDebug d("Parse::scope");

                // create new scope
                cAttrScopeInfo* pScopeInfo = new cAttrScopeInfo;
                
                // init scope info
                pScopeInfo->SetAttributes( yypvt[-2].mpAttrList );                

                // push onto scope stack
                cParserHelper::PushScope( pScopeInfo );
                d.TraceDebug("push\n");
            
} break;

case YYr13: {   /* scoped_block :  TWP_LPAREN attribute_list_with_opt_trailing_comma TWP_RPAREN TWP_LBRACE $12 opt_statement_list TWP_RBRACE */
// #line 218 "policy.y"
 
                if( 0 == cParserHelper::GetScopeStatementCount() && iUserNotify::GetInstance()->GetVerboseLevel() == iUserNotify::V_VERBOSE )
                {
                    eParserNoStatementsInScope e;
                    e.SetFatality( false );
                    cParserHelper::GetErrorBucket()->AddError( e );
                }

                cDebug d("Parse::scope");
                cParserHelper::PopScope();
                d.TraceDebug("pop\n");
            
} break;

case YYr14: {   /* directive_block :  TWP_IFHOST host_name_list */
// #line 234 "policy.y"

                cDebug d("Parse::#ifhost");

                
                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                {
                    if( cParserUtil::AnyOfTheseHostsExists( yypvt[0].mpStringList ) )
                    {
                        cPreprocessor::PushState( cPreprocessor::STATE_ACCEPT );
                    }
                    else
                    {
                        cPreprocessor::PushState( cPreprocessor::STATE_IGNORE );
                    }
                    d.TraceDebug(_T("\n"));
                }
                else
                {
                    cPreprocessor::PushState( cPreprocessor::STATE_IGNORE );
                    d.TraceDebug(_T("ignoring ifhost\n"));
                }
            
} break;

case YYr15: {   /* directive_block :  TWP_IFHOST host_name_list $14 opt_statement_list opt_else_host TWP_ENDIF */
// #line 257 "policy.y"
 
                cDebug d("Parse::#endif");
                cPreprocessor::PopState();

                delete yypvt[-4].mpStringList;
            
} break;

case YYr16: {   /* directive_block :  TWP_SECTION string */
// #line 264 "policy.y"
 
                cDebug d("Parse::#section");

                if( ! ( cPreprocessor::AtTopLevel() && cParserHelper::AtTopScope() ) )
                    throw eParserSectionInsideBlock( *yypvt[0].mpString );

                cParserHelper::SetSection( *yypvt[0].mpString );

                delete yypvt[0].mpString;
            
} break;

case YYr17: {   /* directive_block :  TWP_ERROR string */
// #line 275 "policy.y"

                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT && !cParserHelper::ParseOnly() )
                {
                    iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, yypvt[0].mpString->c_str());
                    iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, _T("\n") );
                    YYRETURN(-1);
                }

                delete yypvt[0].mpString;
            
} break;

case YYr18: {   /* directive_block :  TWP_ECHO string */
// #line 286 "policy.y"

                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT && !cParserHelper::ParseOnly() )
                {
                    iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, yypvt[0].mpString->c_str());
                    iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, _T("\n") );
                }

                delete yypvt[0].mpString;
            
} break;

case YYr19: {   /* host_name_list :  host_name_list TWP_OROR host_name */
// #line 299 "policy.y"

                yyval.mpStringList = yypvt[-2].mpStringList;
                yyval.mpStringList->push_back( *yypvt[0].mpString );
            
} break;

case YYr20: {   /* host_name_list :  host_name */
// #line 304 "policy.y"

                yyval.mpStringList = new cParseStringList;              
                yyval.mpStringList->push_back( *yypvt[0].mpString );
            
} break;

case YYr21: {   /* opt_else_host :  TWP_ELSE */
// #line 312 "policy.y"
 
                cDebug d("Parse::#else");

                if( cPreprocessor::GetLastState() == cPreprocessor::STATE_ACCEPT )
                {
                    cPreprocessor::ToggleTopState();
                    d.TraceDebug(_T("\n"));
                }
                else
                {
                    d.TraceDebug(_T("ignoring #else\n"));
                }
                    
            
} break;

case YYr24: {   /* variable_assignment :  variable_name TWP_EQUALS multi_string */
// #line 332 "policy.y"

                cDebug d("Parse::variable_assignment");

                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring variable assignment\n"));
                }
                else
                {
                    TSTRING strVal = ConcatenateStrings( yypvt[0].mpStringList );
                    d.TraceDebug("*** assigning <%s> to <%s>\n", yypvt[-2].mpString->c_str(), strVal.c_str() );
                    cParserHelper::GetGenreInfo()->InsertVariable( *yypvt[-2].mpString, strVal );
                }

                delete yypvt[-2].mpString;
                delete yypvt[0].mpStringList;
            
} break;

case YYr25: {   /* global_variable_assignment :  global_string TWP_EQUALS global_multi_string */
// #line 353 "policy.y"

                cDebug d("Parse::global variable_assignment");

                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring global variable assignment\n"));
                }
                else
                {
                    TSTRING strVal = ConcatenateStrings( yypvt[0].mpStringList );
                    d.TraceDebug("*** assigning <%s> to <%s>\n", yypvt[-2].mpString->c_str(), strVal.c_str() );
                    cParserHelper::InsertGlobalVariable( *yypvt[-2].mpString, strVal );
                }

                delete yypvt[-2].mpString;
                delete yypvt[0].mpStringList;
            
} break;

case YYr26: {   /* rule :  fco_name TWP_RARROW spec_masks */
// #line 374 "policy.y"

                cDebug d("Parse::rule(fco_name TWP_RARROW spec_masks)");
                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring rule\n"));
                }                               
                else
                {
                    // bind to children
                    cParseStringList    *pnamelist = yypvt[-2].mpStringList;
                    cParseRule          *pnode = yypvt[0].mpNode;
                    cFCOName             fcoName;

                    // create the fco name from string
                    iParserGenreUtil* pGU = iTWFactory::GetInstance()->CreateParserGenreUtil();
                    pGU->InterpretFCOName( *pnamelist, fcoName );
                    
                    if( ! pGU->IsAbsolutePath( iTWFactory::GetInstance()->GetNameTranslator()->ToStringRaw( fcoName ) ) )
                        throw eParserRelativePath( iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( fcoName ) );
                    else if( cParserHelper::GetGenreInfo()->RulePointAlreadyDefined( fcoName ) )
                        throw eParserPointAlreadyDefined( iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( fcoName ) );

                    // assign parse node the name, attributes
                    pnode->SetName( fcoName );

                    d.TraceDebug("*** rule(%s: %d specifiers, default=%s) (%d named attributes) (is%s case sensitive)\n", 
                                        pnode->GetName().AsString().c_str(), pnode->GetNumSpecMasks(),
                                        pnode->GetDefSpecMask().GetPropVectorString().c_str(),
                                        pnode->GetNumNamedAttrs(),
                                        iTWFactory::GetInstance()->GetNameInfo()->IsCaseSensitive() ? "" : " not" );

                    pnode->Dump();

                    // add to our lists
                    cParserHelper::GetGenreInfo()->AddStopPoint( fcoName );
                    cParserHelper::GetGenreInfo()->AddRule( pnode );
                    delete pGU;
                }

                delete yypvt[-2].mpStringList;
                
            
} break;

case YYr27: {   /* rule :  TWP_BANG fco_name */
// #line 416 "policy.y"

                cDebug d("Parse::rule(!fconame)");

                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring rule\n"));
                }                               
                else
                {
                    // create the fco name from string
                    cFCOName fcoName;
                    iParserGenreUtil* pGU = iTWFactory::GetInstance()->CreateParserGenreUtil();
                    pGU->InterpretFCOName( *yypvt[0].mpStringList, fcoName );

                    if( ! pGU->IsAbsolutePath( iTWFactory::GetInstance()->GetNameTranslator()->ToStringRaw( fcoName ) ) )
                        throw eParserRelativePath( iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( fcoName ) );
                    if( cParserHelper::GetGenreInfo()->RulePointAlreadyDefined( fcoName ) )
                        throw eParserPointAlreadyDefined( iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( fcoName ) );
                            
                    d.TraceDebug( "*** omit(%s)\n", fcoName.AsString().c_str() );
                    
                    // add to stop list
                    cParserHelper::GetGenreInfo()->AddStopPoint( fcoName );         
                    delete pGU;
                }

                delete yypvt[0].mpStringList;
            
} break;

case YYr28: {   /* spec_masks :  prop_vector opt_spec_attributes */
// #line 447 "policy.y"

                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                {
                    iParserGenreUtil* pGU = iTWFactory::GetInstance()->CreateParserGenreUtil();
                    TSTRING strDefault = _T("default");
                    cParseRule* pNode = new cParseRule( cParseSpecMask( strDefault, *yypvt[-1].mpString, pGU ) );
                        
                    pNode->SetAttrList( yypvt[0].mpAttrList );
                    yypvt[0].mpAttrList->MergeNoOverwrite( cParserHelper::GetGlobalAttrList() );

                    yyval.mpNode = pNode;
                    delete pGU;
                }

                delete yypvt[-1].mpString;
                
                
            
} break;

case YYr29: {   /* opt_spec_attributes :  TWP_LPAREN attribute_list_with_opt_trailing_comma TWP_RPAREN */
// #line 485 "policy.y"

                yyval.mpAttrList = yypvt[-1].mpAttrList;
            
} break;

case YYr30: {   /* opt_spec_attributes :  */
// #line 489 "policy.y"
 
                yyval.mpAttrList = new cParseNamedAttrList;
            
} break;

case YYr31: {   /* attribute_list_with_opt_trailing_comma :  attribute_list opt_comma */
// #line 515 "policy.y"

                yyval.mpAttrList = yypvt[-1].mpAttrList;
            
} break;

case YYr32: {   /* attribute_list :  attribute_list TWP_COMMA attribute */
// #line 522 "policy.y"

                cDebug d("Parse::attribute_list");
                ASSERT( yypvt[-2].mpAttrList && yypvt[0].mpAttr );

                // add attribute to list             
                yypvt[-2].mpAttrList->Add( yypvt[0].mpAttr );
                d.TraceDebug("added attribute (%s=%s)\n", yypvt[0].mpAttr->GetName().c_str(), yypvt[0].mpAttr->GetValue().c_str());

                yyval.mpAttrList = yypvt[-2].mpAttrList;
                
                
            
} break;

case YYr33: {   /* attribute_list :  attribute */
// #line 535 "policy.y"

                cDebug d("Parse::attribute_list");

                cParseNamedAttrList *pAttrList = new cParseNamedAttrList;  ASSERT( pAttrList );
                
                pAttrList->Add( yypvt[0].mpAttr );
                d.TraceDebug("added attribute (%s=%s)\n", yypvt[0].mpAttr->GetName().c_str(), yypvt[0].mpAttr->GetValue().c_str());

                yyval.mpAttrList = pAttrList;
            
} break;

case YYr34: {   /* attribute :  attribute_name TWP_EQUALS attribute_value */
// #line 548 "policy.y"

                cDebug d("Parse::attribute");

                cParseNamedAttr *pAttr = new cParseNamedAttr( *yypvt[-2].mpString, *yypvt[0].mpString );
                ASSERT(pAttr);

                d.TraceDebug("attribute (%s=%s)\n", yypvt[-2].mpString->c_str(), yypvt[0].mpString->c_str());
                yyval.mpAttr = pAttr;

                
                delete yypvt[-2].mpString;
                delete yypvt[0].mpString;
            
} break;

case YYr37: {   /* variable :  TWP_DOLLAR TWP_LPAREN variable_name TWP_RPAREN */
// #line 618 "policy.y"

                cDebug d( "    parser::variable" );
                
                cParseString* pVarValue = new cParseString;

                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                {
                    // we want to replace the keyword with its value, (e.g. replacing <dollar><lparen>ReadOnly<rparen>
                    // with "pingus" (or whatever readonly means).  So it is a rote replacement.
                    if( ! cParserHelper::GetGenreInfo()->LookupVariable( *yypvt[-1].mpString, *pVarValue ) )
                        throw eParserUseUndefVar( *yypvt[-1].mpString );

                    // return the lval
                    d.TraceDetail("--(STRING)--> got variable (%s) mapped to (%s)\n", 
                                    yypvt[-1].mpString->c_str(),
                                    pVarValue->c_str());                    
                }

                yyval.mpString = pVarValue;
                
                
                delete yypvt[-1].mpString;
            
} break;

case YYr38: {   /* prop_vector :  multi_string */
// #line 662 "policy.y"

                yyval.mpString = new cParseString;
                *yyval.mpString = ConcatenateStrings( yypvt[0].mpStringList );
                delete yypvt[0].mpStringList;
            
} break;

case YYr39: {   /* attribute_name :  string */
// #line 671 "policy.y"
             
                cDebug d("    parser::string(attribute_name)");
                yyval.mpString = yypvt[0].mpString;
            
} break;

case YYr40: {   /* attribute_value :  multi_string */
// #line 679 "policy.y"
                         
                yyval.mpString = new cParseString;
                cDebug d("    parser::multi_string(attribute_value)");
                *yyval.mpString = ConcatenateStrings( yypvt[0].mpStringList );
                delete yypvt[0].mpStringList;
            
} break;

case YYr41: {   /* fco_name :  multi_string */
// #line 689 "policy.y"
 
                cDebug d("    parser::multi_string(fco_name)");
                yyval.mpStringList = yypvt[0].mpStringList;
            
} break;

case YYr42: {   /* fco_name :  multi_string TWP_PIPE multi_string */
// #line 695 "policy.y"


                yypvt[-2].mpStringList->push_back( _T("|") );
                MergeIntoList( yypvt[-2].mpStringList, yypvt[0].mpStringList );
                yyval.mpStringList = yypvt[-2].mpStringList;

                delete yypvt[0].mpStringList;
            
} break;

case YYr43: {   /* host_name :  string */
// #line 710 "policy.y"
        
                        
                cDebug d("    parser::multi_string(host_name)");
                yyval.mpString = yypvt[0].mpString;
            
} break;

case YYr44: {   /* variable_name :  TWP_STRING */
// #line 719 "policy.y"
             
                cDebug d("    parser::string(variable_name)");
                yyval.mpString = yypvt[0].mpString;
            
} break;

case YYr45: {   /* multi_string :  multi_string string */
// #line 728 "policy.y"

                yyval.mpStringList->push_back( *yypvt[0].mpString );
                delete yypvt[0].mpString;
            
} break;

case YYr46: {   /* multi_string :  string */
// #line 733 "policy.y"

                yyval.mpStringList = new cParseStringList;
                yyval.mpStringList->push_back( *yypvt[0].mpString );
                delete yypvt[0].mpString;
            
} break;

case YYr47: {   /* global_multi_string :  global_multi_string global_string */
// #line 742 "policy.y"

                yyval.mpStringList->push_back( *yypvt[0].mpString );
                delete yypvt[0].mpString;
            
} break;

case YYr48: {   /* global_multi_string :  global_string */
// #line 747 "policy.y"

                yyval.mpStringList = new cParseStringList;
                yyval.mpStringList->push_back( *yypvt[0].mpString );
                delete yypvt[0].mpString;
            
} break;

case YYr49: {   /* string :  TWP_STRING */
// #line 757 "policy.y"
             
                cDebug d("    parser::string(normal)");
                d.TraceNever("--(STRING)--> got string (%s)\n", yypvt[0].mpString);
                yyval.mpString = yypvt[0].mpString;
            
} break;

case YYr50: {   /* string :  variable */
// #line 763 "policy.y"
             
                cDebug d("    parser::string(normal)");
                d.TraceNever("--(STRING)--> got string (%s)\n", yypvt[0].mpString);
                yyval.mpString = yypvt[0].mpString;
            
} break;

case YYr51: {   /* global_string :  TWP_GLOBAL_STRING */
// #line 772 "policy.y"

                cDebug d("    parser::string(normal)");
                d.TraceNever("--(STRING)--> got string (%s)\n", yypvt[0].mpString);
                yyval.mpString = yypvt[0].mpString;
            
} break;
// #line 343 "..\\..\\mkslexyacc\\etc\\yyparse.cpp"
    case YYrACCEPT:
        YYACCEPT;
    case YYrERROR:
        goto yyError;
    }
yyNext:
    /*
     *  Look up next state in goto table.
     */

    yyp = &yygo[yypgo[yyi]];
    yyq = yyp++;
    yyi = *yyps;
    while (yyi < *yyp++)        /* busy little loop */
        ;
    yystate = yyneg(yyi == *--yyp? yyq[yyq-yyp]: *yyq);
#if YYDEBUG
    if (yydebug)
        YY_TRACE(yyShowGoto)
#endif
    goto yyStack;

yyerrlabel: ;       /* come here from YYERROR   */
    yyerrflag = 1;
    if (yyi == YYrERROR) {
        yyps--, yypv--;
#if YYDEBUG
        if (yydebug) yytp--;
#endif
    }
    
yyError:
    switch (yyerrflag) {

    case 0:     /* new error */
        yynerrs++;
        yyi = yychar;
        scan->yyerror("Syntax error");
        if (yyi != yychar) {
            /* user has changed the current token */
            /* try again */
            yyerrflag++;    /* avoid loops */
            goto yyEncore;
        }

    case 1:     /* partially recovered */
    case 2:
        yyerrflag = 3;  /* need 3 valid shifts to recover */
            
        /*
         *  Pop states, looking for a
         *  shift on `error'.
         */

        for ( ; yyps > stateStack; yyps--, yypv--
#if YYDEBUG
                    , yytp--
#endif
        ) {
#if 0 //YACC_WINDOWS
            if (*yyps >= Sizeof_yypact)     /* simple state */
#else /* YACC_WINDOWS */
            if (*yyps >= (int)sizeof yypact/(int)sizeof yypact[0])
#endif /* YACC_WINDOWS */
                continue;
            yyp = &yyact[yypact[*yyps]];
            yyq = yyp;
            do
                ;
            while (YYERRCODE < *yyp++);
            if (YYERRCODE == yyp[-1]) {
                yystate = yyneg(yyq[yyq-yyp]);
                goto yyStack;
            }
                
            /* no shift in this state */
#if YYDEBUG
            if (yydebug && yyps > stateStack+1)
                YY_TRACE(yyShowErrRecovery)
#endif
            /* pop stacks; try again */
        }
        /* no shift on error - abort */
        break;

    case 3:
        /*
         *  Erroneous token after
         *  an error - discard it.
         */

        if (yychar == 0)  /* but not EOF */
            break;
#if YYDEBUG
        if (yydebug)
            YY_TRACE(yyShowErrDiscard)
#endif
        yyclearin();
        goto yyEncore;  /* try again in same state */
    }
    YYABORT;

}
#if YYDEBUG
/*
 * Return type of token
 */
int
yy_parse::yyGetType(int tok)
{
    yyNamedType * tp;
    for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
        if (tp->token == tok)
            return tp->type;
    return 0;
}

    
// Print a token legibly.
char *
yy_parse::yyptok(int tok)
{
    yyNamedType * tp;
    for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
        if (tp->token == tok)
            return tp->name;
    return "";
}
/*
 * Read state 'num' from YYStatesFile
 */
#ifdef YYTRACE

char *
yy_parse::yygetState(int num)
{
    int size;
    char    *cp;
    static FILE *yyStatesFile = (FILE *) 0;
    static char yyReadBuf[YYMAX_READ+1];

    if (yyStatesFile == (FILE *) 0
     && (yyStatesFile = fopen(YYStatesFile, "r")) == (FILE *) 0)
        return "yyExpandName: cannot open states file";

    if (num < yynstate - 1)
        size = (int)(States[num+1] - States[num]);
    else {
        /* length of last item is length of file - ptr(last-1) */
        if (fseek(yyStatesFile, 0L, 2) < 0)
            goto cannot_seek;
        size = (int) (ftell(yyStatesFile) - States[num]);
    }
    if (size < 0 || size > YYMAX_READ)
        return "yyExpandName: bad read size";
    if (fseek(yyStatesFile, States[num], 0) < 0) {
    cannot_seek:
        return "yyExpandName: cannot seek in states file";
    }

    (void) fread(yyReadBuf, 1, size, yyStatesFile);
    yyReadBuf[size] = '\0';
    return yyReadBuf;
}
#endif /* YYTRACE */
/*
 * Expand encoded string into printable representation
 * Used to decode yyStates and yyRules strings.
 * If the expansion of 's' fits in 'buf', return 1; otherwise, 0.
 */
int
yy_parse::yyExpandName(int num, int isrule, char * buf, int len)
{
    int i, n, cnt, type;
    char    * endp, * cp, * s;

    if (isrule)
        s = yyRules[num].name;
    else
#ifdef YYTRACE
        s = yygetState(num);
#else
        s = "*no states*";
#endif

    for (endp = buf + len - 8; *s; s++) {
        if (buf >= endp) {      /* too large: return 0 */
        full:   (void) strncpy(buf, " ...\n", len);
            return 0;
        } else if (*s == '%') {     /* nonterminal */
            type = 0;
            cnt = yynvar;
            goto getN;
        } else if (*s == '&') {     /* terminal */
            type = 1;
            cnt = yyntoken;
        getN:
            if (cnt < 100)
                i = 2;
            else if (cnt < 1000)
                i = 3;
            else
                i = 4;
            for (n = 0; i-- > 0; )
                n = (n * 10) + *++s - '0';
            if (type == 0) {
                if (n >= yynvar)
                    goto too_big;
                cp = yysvar[n];
            } else if (n >= yyntoken) {
                too_big:
                cp = "<range err>";
            } else
                cp = yyTokenTypes[n].name;

            if ((i = strlen(cp)) + buf > endp)
                goto full;
            (void) strncpy(buf, cp, len);
            buf += i;
        } else
            *buf++ = *s;
    }
    *buf = '\0';
    return 1;
}
#ifndef YYTRACE
/*
 * Show current state of yyparse
 */
void
yy_parse::yyShowState()
{
    (void) printf("state %d (%d), char %s (%d)\n%d stateStack entries\n",
        yysmap[yystate],yystate,yyptok(yychar),yychar,
        yypv - valueStack);
}
// show results of reduction: yyi is rule number
void
yy_parse::yyShowReduce()
{
    (void) printf("Reduce by rule %d (pop#=%d)\n", yyrmap[rule], npop);
}
// show read token
void
yy_parse::yyShowRead()
{
    (void) printf("read %s (%d)\n", yyptok(yychar), yychar);
}
// show Goto
void
yy_parse::yyShowGoto()
{
    (void) printf("goto %d (%d)\n", yysmap[yystate], yystate);
}
// show Shift
void
yy_parse::yyShowShift()
{
    (void) printf("shift %d (%d)\n", yysmap[yystate], yystate);
}
// show error recovery
void
yy_parse::yyShowErrRecovery()
{
    (void) printf("Error recovery pops state %d (%d), uncovers %d (%d)\n",
        yysmap[*(yyps-1)], *(yyps-1), yysmap[yystate], yystate);
}
// show token discards in error processing
void
yy_parse::yyShowErrDiscard()
{
    (void) printf("Error recovery discards %s (%d), ",
        yyptok(yychar), yychar);
}
#endif  /* ! YYTRACE */
#endif  /* YYDEBUG */


