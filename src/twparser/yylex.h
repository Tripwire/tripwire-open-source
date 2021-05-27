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



#ifndef yy_state_t
#define yy_state_t unsigned char
#endif
#define YYNEWLINE 10

//#line 1 "..\\..\\mkslexyacc\\etc\\yylex.cpp"
// MKS LEX prototype scanner header
// Copyright 1991 by Mortice Kern Systems Inc.
// All rights reserved.

// You can define YY_PRESERVE to get System V UNIX lex compatibility,
//  if you need to change yytext[] in your user actions
// This is quite a bit slower, though, so the default is without

#include <stdio.h>      // uses printf(), et cetera
#ifdef HAVE_STDARG_H
#include <stdarg.h>     // uses va_list
#endif
#include <stdlib.h>     // uses exit()
#include <string.h>     // uses memmove()

#ifdef LEX_WINDOWS

// define, if not already defined
// the flag YYEXIT, which will allow
// graceful exits from yylex()
// without resorting to calling exit();

#ifndef YYEXIT
#define YYEXIT  1
#endif // YYEXIT

// include the windows specific prototypes, macros, etc

#include <windows.h>

// the following is the handle to the current
// instance of a windows program. The user
// program calling yylex must supply this!

#ifdef STRICT
extern HINSTANCE hInst; 
#else
extern HANDLE hInst;    
#endif

#endif /* LEX_WINDOWS */

class yy_scan {
protected:

#ifdef LEX_WINDOWS

    // protected member function for actual scanning 

    int win_yylex();

#endif /* LEX_WINDOWS */

    yy_state_t * state;     // state buffer
    int size;           // length of state buffer
    int mustfree;       // set if space is allocated
    int yy_end;         // end of pushback
    int yy_start;       // start state
    int yy_lastc;       // previous char
#ifdef YYEXIT
    int yyLexFatal;     // Lex Fatal Error Flag
#endif // YYEXIT
#ifndef YY_PRESERVE // efficient default push-back scheme
    char save;      // saved yytext[yyleng]
#else           // slower push-back for yytext mungers
    char *save;     // saved yytext[]
    char *push;
#endif

public:
    char   *yytext;     // yytext text buffer
    FILE   *yyin;           // input stream
    FILE   *yyout;          // output stream
    int yylineno;       // line number
    int yyleng;         // yytext token length

    yy_scan(int = 100); // constructor for this scanner
            // default token & pushback size is 100 bytes
    yy_scan(int, char*, char*, yy_state_t*);
                // constructor when tables are given

    virtual ~yy_scan();     // destructor

    int yylex();        // begin a scan

    virtual int yygetc() {  // scanner source of input characters
        return getc(yyin);
    }

    virtual int yywrap() { return 1; }  // EOF processing

    virtual void    yyerror(const char *,...);  // print error message

    virtual void    output(int c) { putc(c, yyout); }

#ifdef YYEXIT
    virtual void    YY_FATAL(const char * msg) {    // print message and set error flag
        yyerror(msg); yyLexFatal = 1;
    }
#else // YYEXIT
    virtual void    YY_FATAL(const char * msg) {    // print message and stop
        yyerror(msg); exit(1);
    }
#endif // YYEXIT
    virtual void    ECHO() {        // print matched input
        fputs((const char *) yytext, yyout);
    }
    int input();        // user-callable get-input
    int unput(int c);       // user-callable unput character
    void    yy_reset();     // reset scanner
    void    setinput(FILE * in) {       // switch input streams
        yyin = in;
    }
    void    setoutput(FILE * out) { // switch output
        yyout = out;
    }
    void    NLSTATE() { yy_lastc = YYNEWLINE; }
    void    YY_INIT() {
        yy_start = 0;
        yyleng = yy_end = 0;
        yy_lastc = YYNEWLINE;
    }
    void    YY_USER() {     // set up yytext for user
#ifndef YY_PRESERVE
        save = yytext[yyleng];
#else
        size_t n = yy_end - yyleng;
        push = save+size - n;
        if (n > 0)
            memmove(push, yytext+yyleng, n);
#endif
        yytext[yyleng] = 0;
    }
    void YY_SCANNER() {     // set up yytext for scanner
#ifndef YY_PRESERVE
        yytext[yyleng] = save;
#else
        size_t n = save+size - push;
        if (n > 0)
            memmove(yytext+yyleng, push, n);
        yy_end = yyleng + n;
#endif
    }
    void    yyless(int n) {     // trim input to 'n' bytes
        if (n >= 0 && n <= yy_end) {
            YY_SCANNER();
            yyleng = n;
            YY_USER();
        }
    }
    void    yycomment(char *const mat); // skip comment input
    int yymapch(int delim, int escape); // map C escapes
} ;

