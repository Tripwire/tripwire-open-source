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
/*
///////////////////////////////////////////////////////////////////////////////
// tokens.l -- describes all the tokens for the policy language
//
*/

/* increases number of states in DFA to 5000 */
%p 5000

%{
#include "stdtwparser.h"

//#ifndef YYNEWLINE
# include "twparser/yylex.h"
# include "twparser/yyparse.h"
//#endif

#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>

#include "core/debug.h" 

#include "policyparser.h"
#include "parserhelper.h"
#include "genreparseinfo.h"
#include "core/stringutil.h"
#include "core/displayencoder.h"



/* specify that the lexer exit on EOF */
int yywrap()
{
	return 1;
}

/* wrapper around cDebug tracing */
void lextrace(const TCHAR*str)
{
	cDebug d("\t\t\t\tlexer");
	d.TraceDetail( _T("%s\n"), str);
}

// since i've replaced this with a non-va_arg yyerror,
// call this when you hit a syntax error and then call
// the non-va_arg yyerror with the result
std::string FormatSyntaxError( char ch, const char* pszAdditionalMsg = NULL )
{
    // TODO:BAM MKS will call yyerror with narrow-char hard-coded string, so if 
    // we do it here as well, I suppose that's OK.  This should be eventually
    // corrected.

    std::ostringstream ssErr; 
    ssErr << "Syntax error: \'" << ch << "\'";
    if( pszAdditionalMsg )
        ssErr << ", " << pszAdditionalMsg;

    return ssErr.str();
}

// saves typing
#define TRACE_RETURN(x) lextrace(_T(#x)); return x

%}

/* Defining a start-condition for parsing a global definitions section */
/* in a special manner: */
%x globals

WS				[ \t\r]
COMMENT			#.*
BSLASH			\\
DBL_QUOTE       \"
DIRECTIVE		@@
EOL				\n
SEMICOLON		;
DOLLAR			\$
COMMA           \,
PIPE            \|
OROR			\|\|
EQUALS          =

LPAREN			\(
RPAREN			\)


/* normal strings -- anything but our reserved symbols */
STRING          ([^!\{\}>\(\)\n\r\t \,;=$#|\"]+)


/* quoted strings can contain escaped characters (including escaped quotes). Can also be empty string. */
QSTRING         (\"(([^\\\"\n])|(\\.))*\")

SECTION         {WS}*{DIRECTIVE}{WS}*section
IFHOST          {WS}*{DIRECTIVE}{WS}*ifhost
ELSE            {WS}*{DIRECTIVE}{WS}*else
ENDIF           {WS}*{DIRECTIVE}{WS}*endif
ERROR           {WS}*{DIRECTIVE}{WS}*error
ECHO            {WS}*{DIRECTIVE}{WS}*print
END             {WS}*{DIRECTIVE}{WS}*end

%%

/* Lexer state-switching code for implementing a "global definitions" section while enforcing */
/* the limited scope of such a construct.  The lexer will catch anything that cannot be  */
/* coerced into a variable definition, and output special warnings. */

^{SECTION}{WS}*((global)|(GLOBAL)) {
					BEGIN( globals );
				}

/* Are we done with the definitions section? */
<globals>^{SECTION} {
					BEGIN(INITIAL);
					return TWP_SECTION;
				}

<globals>^{ECHO}   { TRACE_RETURN(TWP_ECHO); }

/* We still need to eat white space, line continuations, and comments... */
<globals>{WS}+	{ 
					lextrace(_T("eating spaces..."));			 /* eat spaces */ 
				}
<globals>{BSLASH}{EOL}	{   
                    cParserHelper::IncrementLineNumber();
					lextrace(_T("eating line continuation...")); /* eat line continuations */ 
				}
<globals>{COMMENT}	{
					lextrace(_T("eating comment..."));
				}

/* We still need to increment the line #, and pass the two tokens needed to */
/* do variable definitions: */

/* We still need to treat strings the same way, just return a differnt ID. TODO: */
/* Maybe create some helper functions and get rid of all this repeat code? */
<globals>{STRING}	{ 
						// we must make copy of string, otherwise another lexeme will clobber it
						cDebug d("\t\t\t\tlexer::string");

						// convert LPCSTR to TSTRING
						cParseString *mpstring = new cParseString;
						ASSERT(mpstring);
						*mpstring = cStringUtil::StrToTstr( yytext );

						d.TraceDetail("--> <%s>\n", mpstring->c_str());

						// attach to lval
						yylval.mpString = mpstring;
						
                        return TWP_GLOBAL_STRING;
					}

<globals>{QSTRING}  {
						// we must make copy of string, otherwise another lexeme will clobber it
						cDebug d("\t\t\t\tlexer::qstring");

						// get rid of beginning and trailing quotes
                        std::string strWithQuotes = yytext;
                        std::string str = strWithQuotes.substr(1, strWithQuotes.size() - 2);
                        
                        // convert any escape characters
                        TSTRING strW;
                        cParserUtil::InterpretEscapedString( str, strW );
                        
                        d.TraceDetail(_T("read as        --> <%s>\n"), strW.c_str());

						// convert TSTRING to cParseString
						cParseString *mpstring = new cParseString;
						ASSERT(mpstring);
						*mpstring = strW;


						d.TraceDetail("interpreted as --> <%s>\n", mpstring->c_str());

						// attach to lval
						yylval.mpString = mpstring;
						return TWP_GLOBAL_STRING;
					}

<globals>{SEMICOLON}	{ TRACE_RETURN(TWP_SEMICOLON);		}
<globals>{EQUALS}		{ TRACE_RETURN(TWP_EQUALS);			}

<globals>\n             { cParserHelper::IncrementLineNumber(); }
<globals>.              {
                            std::string strError;
                            strError = FormatSyntaxError( yytext[0], "The global section only accepts statements of the form:\n variable = value;\n" );

                            // MKS defines yyerror with char*, for some stupid reason,
                            // so cast it away
                            yyerror( const_cast<char*>( strError.c_str() ) ); 
						} /* catches anything that cannot be deemed a variable definition and exits. */

/* consume spaces */
{WS}+		{ 
						lextrace(_T("eating spaces...")); /* eat spaces */ 
					}

{BSLASH}{EOL}	{   
                        cParserHelper::IncrementLineNumber();
						lextrace(_T("eating line continuation...")); /* eat line continuations */ 
					}

{COMMENT}     { 
						lextrace(_T("eating comment..."));
					}

"{"					{ TRACE_RETURN(TWP_LBRACE);			}
"}"					{ TRACE_RETURN(TWP_RBRACE);			}


"!"					{ TRACE_RETURN(TWP_BANG);			}

"->"				{ TRACE_RETURN(TWP_RARROW);			}
{EQUALS}		    { TRACE_RETURN(TWP_EQUALS);			}

{SEMICOLON}			{ TRACE_RETURN(TWP_SEMICOLON);		}
{LPAREN}			{ TRACE_RETURN(TWP_LPAREN);			}
{RPAREN}			{ TRACE_RETURN(TWP_RPAREN);			}
{COMMA}             { TRACE_RETURN(TWP_COMMA);			}
{PIPE}			    { TRACE_RETURN(TWP_PIPE); }


/* variables */
{DOLLAR}			{ TRACE_RETURN(TWP_DOLLAR);			}
{OROR}			    { TRACE_RETURN(TWP_OROR); }


^{SECTION}   { TRACE_RETURN(TWP_SECTION); }
^{IFHOST}    { TRACE_RETURN(TWP_IFHOST); }
^{ELSE}      { TRACE_RETURN(TWP_ELSE); }
^{ENDIF}     { TRACE_RETURN(TWP_ENDIF); }
^{ERROR}     { TRACE_RETURN(TWP_ERROR); }
^{ECHO}      { TRACE_RETURN(TWP_ECHO); }
^{END}       { lextrace( _T( "@@end" ) ); return 0; } /* logical end of file */


{STRING}    { 
						// we must make copy of string, otherwise another lexeme will clobber it
						cDebug d("\t\t\t\tlexer::string");
						
						// convert LPCSTR to TSTRING
						cParseString *mpstring = new cParseString;
						ASSERT(mpstring);
						*mpstring = cStringUtil::StrToTstr( yytext );

						d.TraceDetail("--> <%s>\n", mpstring->c_str());

						// attach to lval
						yylval.mpString = mpstring;
						
                        return TWP_STRING;
					}

{QSTRING}   {
                        // we must make copy of string, otherwise another lexeme will clobber it
						cDebug d("\t\t\t\tlexer::qstring");
                        
						// get rid of beginning and trailing quotes
                        std::string strWithQuotes = yytext;
                        std::string str = strWithQuotes.substr(1, strWithQuotes.size() - 2);
                        
                        // convert any escape characters
                        TSTRING strW;
                        cParserUtil::InterpretEscapedString( str, strW );
                        
                        d.TraceDetail(_T("read as        --> <%s>\n"), strW.c_str());

                        if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                        {
                            cParserHelper::GetGenreInfo()->DoVarSubstitution( strW );
                        }

						// convert TSTRING to cParseString
						cParseString *mpstring = new cParseString;
						ASSERT(mpstring);
						*mpstring = strW;


#ifdef DEBUG
                        TSTRING strDisplay = *mpstring;                        
                        cDisplayEncoder e;
                        e.Encode( strDisplay );
						d.TraceDetail("interpreted as --> <%s>\n", strDisplay.c_str());
#endif // DEBUG

						// attach to lval
						yylval.mpString = mpstring;
						return TWP_STRING;
					}

/* not implemented yet
^{DIRECTIVE}{WS}*include{WS}\"[^\"]\"  { 

                        // get include file name                                    
                        TCHAR szFilename[iFSServices::TW_MAX_PATH];

                        FILE* fpIncludeFile = _tfopen( szFilename, _T("r") );
                        if( fpIncludeFile )
                        {
                            lex_include_file( fpIncludeFile );

                            fclose( fpIncludeFile );
                        }
                    }
                    */

\n                  { cParserHelper::IncrementLineNumber(); }
.                   {
                        std::string strError;
                        strError = FormatSyntaxError( yytext[0] );
                        
                        // MKS defines yyerror with char*, for some stupid reason,
                        // so cast it away
                        yyerror( const_cast<char*>( strError.c_str() ) );
                    } /* catches anything else that's not in here and quits */


%%



