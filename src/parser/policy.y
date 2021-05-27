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
// policy.y -- yacc grammar for parsing policy language
//

    INTERESTING STUFF:
    ---------------------
    * all whitespace is removed from input
    * token "@@end" is taken as the logical end of policy file
    * "->" is now the fconame/specmask separator
    * attribute lists now contain comma-separated attributes
    * quoted strings use C++-style escaping (char (e.g. \n, \"), hex (e.g. \x8E230A), oct(e.g. \271) )
    * default spec mask has name _T("default")

    TODO: UNICODE
*/

/* 
 * NOTE:	prefix all token names with TWP_ to prevent #define name collisions
 *			(e.g., STRING is probably already defined by somebody...)
 */

/*
 * debugging hints:
 *
 *		- if you see an error like this: "policy.y: line 308:  Newline in string started at line 307"
 *				you are using a C++ style comment, that has a "n't" in it somewhere -- like this:
 *					// tell parent that we don't have any attributes
 *				either change it to C comment, or change "don't" to "do not".
 *				unbelievable, isn't it?  MKS Yacc is a piece of crap.
 *
 *				ghk 8/28/98
 *
 */

%token TWP_COMMA
%token TWP_LPAREN
%token TWP_RPAREN
%token TWP_COMMENT	
%token <mpString> TWP_STRING
%token TWP_SEMICOLON
%token TWP_LBRACE
%token TWP_RBRACE
%token TWP_DEFAULT
%token TWP_PIPE
%token TWP_DOLLAR
%token TWP_BANG
%token TWP_PLUS
%token TWP_MINUS
%token TWP_DQUOTE
%token TWP_RARROW
%token TWP_SECTION
%token TWP_IFHOST
%token TWP_ELSE
%token TWP_ENDIF
%token TWP_ECHO
%token TWP_ERROR

/* TODO: change TWP_XXX to TWP_ID_XXX for keyword */
%token TWP_DEFINE
%token TWP_UNDEF
%token TWP_ELSE
%token TWP_ENDIF
%token TWP_CONTENTS
%token TWP_DBASEVERSION
%token TWP_ANDAND
%token TWP_OROR
%token TWP_BSLASH
%token TWP_ESCCHAR
%token TWP_QSTRING
%token TWP_EQUALS
%token <mpString>TWP_GLOBAL_STRING

%{

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

%}

/* this defines the lval values that are returned by each grammar rule */

%union {
	class cParseString*           mpString;					// wrapper around TSTRING
    class cParseStringList*       mpStringList;

	class cParseRule*             mpNode;					// representation of FCOSpec

	class cParseSpecMask*         mpSpecMask;				// representation of specmask
	class cParseSpecMaskList*     mpSpecMaskList;			//		...and a collection 

	class cParseNamedAttr*        mpAttr;					// representation of an attribute
	class cParseNamedAttrList*    mpAttrList;				//		...and a collection 
};


%type <mpNode>          spec_masks

%type <mpAttr>		    attribute

%type <mpAttrList>		attribute_list
%type <mpAttrList>		attribute_list_with_opt_trailing_comma
%type <mpAttrList>		opt_spec_attributes

/*****************************************
TODO: implement this when everthing else is done and the details have been hammered out
%type <mpSpecMask>	    wildcard_spec_mask
%type <mpSpecMask>      default_spec_mask

%type <mpSpecMaskList>  opt_wildcard_spec_mask_list

%type <mpString>		fco_wildcard
*****************************************/


%type <mpStringList>    fco_name
%type <mpString>		attribute_name
%type <mpString>		attribute_value
%type <mpString>		string
%type <mpStringList>    multi_string
%type <mpStringList>	global_multi_string
%type <mpString>		variable
%type <mpString>		variable_name
%type <mpString>		global_string
%type <mpString>		host_name
%type <mpStringList>    host_name_list
%type <mpString>		prop_vector


%start policy
%%

/*-----------------------------------------------------------------------------
 * policy rules
 *
 * these productions are responsible for creating the list of rules.  each completed rule
 * adds a cParseRule to either the "omit list" or the "rule list"
 *
 */

policy      
            : opt_statement_list
            ;

opt_statement_list
            : statement_list
            | /* empty */
            ;
           
statement_list
            : statement_list statement
            | statement
            ;

statement   
            : variable_assignment TWP_SEMICOLON { cParserHelper::IncrementScopeStatementCount(); }
			| global_variable_assignment TWP_SEMICOLON { cParserHelper::IncrementScopeStatementCount(); }
            | rule TWP_SEMICOLON { cParserHelper::IncrementScopeStatementCount(); }
            | TWP_SEMICOLON
            | directive_block
            | scoped_block
            ;

scoped_block
            : TWP_LPAREN attribute_list_with_opt_trailing_comma TWP_RPAREN TWP_LBRACE              
            {
                cDebug d("Parse::scope");

                // create new scope
                cAttrScopeInfo* pScopeInfo = new cAttrScopeInfo;
                
                // init scope info
                pScopeInfo->SetAttributes( $2 );                

                // push onto scope stack
                cParserHelper::PushScope( pScopeInfo );
                d.TraceDebug("push\n");
            }
            opt_statement_list TWP_RBRACE 
            { 
                if( 0 == cParserHelper::GetScopeStatementCount() && iUserNotify::GetInstance()->GetVerboseLevel() == iUserNotify::V_VERBOSE )
                {
                    eParserNoStatementsInScope e;
                    e.SetFatality( false );
                    cParserHelper::GetErrorBucket()->AddError( e );
                }

                cDebug d("Parse::scope");
                cParserHelper::PopScope();
                d.TraceDebug("pop\n");
            }
            ;

directive_block
            : TWP_IFHOST host_name_list 
            {
                cDebug d("Parse::#ifhost");

                /* only look at this if we're not ignoring */
                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                {
                    if( cParserUtil::AnyOfTheseHostsExists( $2 ) )
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
            }
            opt_statement_list opt_else_host TWP_ENDIF
            { 
                cDebug d("Parse::#endif");
                cPreprocessor::PopState();

                delete $2;
            }            
            | TWP_SECTION string
            { 
                cDebug d("Parse::#section");

                if( ! ( cPreprocessor::AtTopLevel() && cParserHelper::AtTopScope() ) )
                    throw eParserSectionInsideBlock( *$2 );

                cParserHelper::SetSection( *$2 );

                delete $2;
            }
            | TWP_ERROR string
            {
                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT && !cParserHelper::ParseOnly() )
                {
					iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, $2->c_str());
                    iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, _T("\n") );
                    YYRETURN(-1);
                }

                delete $2;
            }
            | TWP_ECHO string
            {
                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT && !cParserHelper::ParseOnly() )
                {
					iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, $2->c_str());
                    iUserNotify::GetInstance()->Notify( iUserNotify::V_NORMAL, _T("\n") );
                }

                delete $2;
            }
            ;

host_name_list
            : host_name_list TWP_OROR host_name
            {
                $$ = $1;
                $$->push_back( *$3 );
            }
            | host_name
            {
                $$ = new cParseStringList;              
                $$->push_back( *$1 );
            }
            ;

opt_else_host
            : TWP_ELSE 
            { 
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
                    
            } 
            opt_statement_list
            | /* empty */
            ;

variable_assignment
            : variable_name TWP_EQUALS multi_string
            {
                cDebug d("Parse::variable_assignment");

                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring variable assignment\n"));
                }
                else
                {
                    TSTRING strVal = ConcatenateStrings( $3 );
				    d.TraceDebug("*** assigning <%s> to <%s>\n", $1->c_str(), strVal.c_str() );
                    cParserHelper::GetGenreInfo()->InsertVariable( *$1, strVal );
                }

                delete $1;
                delete $3;
            }
            ;

global_variable_assignment
			: global_string TWP_EQUALS global_multi_string
			{
				cDebug d("Parse::global variable_assignment");

                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring global variable assignment\n"));
                }
                else
                {
                    TSTRING strVal = ConcatenateStrings( $3 );
				    d.TraceDebug("*** assigning <%s> to <%s>\n", $1->c_str(), strVal.c_str() );
                    cParserHelper::InsertGlobalVariable( *$1, strVal );
                }

                delete $1;
                delete $3;
			}
			;

rule
            : fco_name TWP_RARROW spec_masks
            {
                cDebug d("Parse::rule(fco_name TWP_RARROW spec_masks)");
                if( cPreprocessor::GetState() == cPreprocessor::STATE_IGNORE )
                {   
                    d.TraceDebug(_T(" ignoring rule\n"));
                }             			 	    
                else
                {
				    // bind to children
				    cParseStringList    *pnamelist = $1;
				    cParseRule			*pnode = $3;
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
                }

                delete $1;
                /* do not delete $3 */
            }
            | TWP_BANG fco_name                         /* stop point */            
			{
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
                    pGU->InterpretFCOName( *$2, fcoName );

                    if( ! pGU->IsAbsolutePath( iTWFactory::GetInstance()->GetNameTranslator()->ToStringRaw( fcoName ) ) )
                        throw eParserRelativePath( iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( fcoName ) );
                    if( cParserHelper::GetGenreInfo()->RulePointAlreadyDefined( fcoName ) )
                        throw eParserPointAlreadyDefined( iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay( fcoName ) );
                    	    
				    d.TraceDebug( "*** omit(%s)\n", fcoName.AsString().c_str() );
                    
				    // add to stop list
				    cParserHelper::GetGenreInfo()->AddStopPoint( fcoName );			
                }

                delete $2;
			}
            ;

spec_masks
            : prop_vector opt_spec_attributes
            {
                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                {
                    iParserGenreUtil* pGU = iTWFactory::GetInstance()->CreateParserGenreUtil();
                    TSTRING strDefault = _T("default");
                    cParseRule* pNode = new cParseRule( cParseSpecMask( strDefault, *$1, pGU ) );
                        
				    pNode->SetAttrList( $2 );
                    $2->MergeNoOverwrite( cParserHelper::GetGlobalAttrList() );

                    $$ = pNode;
                }

                delete $1;
                /* do not delete $2 */
                /* do not delete pNode */
            }
/******************************************
*
            TODO: implement this when everthing else is done and the details have been hammered out
            | TWP_LBRACE opt_wildcard_spec_mask_list default_spec_mask TWP_RBRACE
            {
                cParseRule* pNode = new cParseRule( $3 );

                pNode->SetSpecMaskList( $2 );

                $$ = pNode;
            }
            | TWP_LBRACE opt_wildcard_spec_mask_list TWP_RBRACE
            {
                ASSERT( false );
            }
*
******************************************/
            ;

opt_spec_attributes
            : TWP_LPAREN attribute_list_with_opt_trailing_comma TWP_RPAREN
            {
                $$ = $2;
            }
            | /* empty */ 
            { 
                $$ = new cParseNamedAttrList;
            }
            ;

/******************************************
*
TODO: implement this when everthing else is done and the details have been hammered out
opt_wildcard_spec_mask_list
            : opt_wildcard_spec_mask_list wildcard_spec_mask
            {
                $$ = $1;
                if( $$ == NULL )
                {
                    $$ = new cParseSpecMaskList;  ASSERT( $$ );
                }

                $$->Add( $2 );
            }
            | { $$ = NULL; }
            ;
*
******************************************/

attribute_list_with_opt_trailing_comma
            : attribute_list opt_comma
            {
                $$ = $1;
            }
            ;

attribute_list
            : attribute_list TWP_COMMA attribute
            {
				cDebug d("Parse::attribute_list");
                ASSERT( $1 && $3 );

				// add attribute to list             
                $1->Add( $3 );
                d.TraceDebug("added attribute (%s=%s)\n", $3->GetName().c_str(), $3->GetValue().c_str());

				$$ = $1;
                
                /* do not delete $3 */
            }
            | attribute
            {
                cDebug d("Parse::attribute_list");

                cParseNamedAttrList *pAttrList = new cParseNamedAttrList;  ASSERT( pAttrList );
                
                pAttrList->Add( $1 );
                d.TraceDebug("added attribute (%s=%s)\n", $1->GetName().c_str(), $1->GetValue().c_str());

                $$ = pAttrList;
            }
            ;
attribute
            : attribute_name TWP_EQUALS attribute_value            
            {
                cDebug d("Parse::attribute");

            	cParseNamedAttr *pAttr = new cParseNamedAttr( *$1, *$3 );
				ASSERT(pAttr);

				d.TraceDebug("attribute (%s=%s)\n", $1->c_str(), $3->c_str());
				$$ = pAttr;

                /* don't need strings anymore */
                delete $1;
                delete $3;
            }
            ;

/******************************************
*
TODO: implement this when everthing else is done and the details have been hammered out

wildcard_spec_mask
            : fco_wildcard TWP_RARROW prop_vector opt_spec_attributes TWP_SEMICOLON
			{
				cDebug d("Parse::wildcard_spec_mask");

				// create node, and attach specmask string and attributes 
				//		the parent will merge this into the rest of the specmasks, and then delete it
				cParseSpecMask *pSpecMask = new cParseSpecMask( *$1, *$3 ); ASSERT( pSpecMask );

				// attach any named attributes
				pSpecMask->SetAttrList( $4 );


				d.TraceAlways("condition(%s), value(%s), attributes(%d)\n", 
									$1->c_str(), 
									$3->c_str(),
									pSpecMask->GetNumAttrs());
				
				$$ = pSpecMask;
			}
            ;

default_spec_mask
            : TWP_DEFAULT TWP_RARROW prop_vector opt_spec_attributes TWP_SEMICOLON            
			{
				cDebug d("Parse::default_spec_mask");

				// create node, and attach specmask string and attributes 
				//		the parent will merge this into the rest of the specmasks, and then delete it
				cParseSpecMask *pDefMask = new cParseSpecMask(TSTRING("default"), *$3); ASSERT(pDefMask);

				// attach any named attributes
				pDefMask->SetAttrList( $4 );

				d.TraceAlways("propvector(%s), num attributes(%d)\n", pDefMask->GetPropVector().c_str(),
									pDefMask->GetNumAttrs());
				
				$$ = pDefMask;
			}
            ;
*
******************************************/

opt_comma
            : TWP_COMMA
            | /* empty */
            ;


variable
            : TWP_DOLLAR TWP_LPAREN variable_name TWP_RPAREN 
			{
				cDebug d( "    parser::variable" );
                
                cParseString* pVarValue = new cParseString;

                if( cPreprocessor::GetState() == cPreprocessor::STATE_ACCEPT )
                {
                    // we want to replace the keyword with its value, (e.g. replacing <dollar><lparen>ReadOnly<rparen>
                    // with "pingus" (or whatever readonly means).  So it is a rote replacement.
                    if( ! cParserHelper::GetGenreInfo()->LookupVariable( *$3, *pVarValue ) )
                        throw eParserUseUndefVar( *$3 );

				    // return the lval
				    d.TraceDetail("--(STRING)--> got variable (%s) mapped to (%s)\n", 
								    $3->c_str(),
								    pVarValue->c_str());				    
                }

                $$ = pVarValue;
                
                /* don't need string anymore */
                delete $3;
			}
            ;


/******************************************
*
TODO: implement this when everthing else is done and the details have been hammered out
fco_wildcard
            : TWP_STRING 
            {            
                // like "*.exe"
                cDebug d("    parser::string(fco_wildcard)");
	            d.TraceNever("--(STRING)--> got string (%s)\n", $1);
                $$ = $1;
            }
            ;

*
*******************************************/

prop_vector
            : multi_string
            {
                $$ = new cParseString;
                *$$ = ConcatenateStrings( $1 );
                delete $1;
            }
            ;

attribute_name
            : string
            {             
                cDebug d("    parser::string(attribute_name)");
                $$ = $1;
            }
            ;

attribute_value
            : multi_string
            {                         
                $$ = new cParseString;
                cDebug d("    parser::multi_string(attribute_value)");
                *$$ = ConcatenateStrings( $1 );
                delete $1;
            }
            ;

fco_name    
            : multi_string
            { 
                cDebug d("    parser::multi_string(fco_name)");
                $$ = $1;
            }
            |
            multi_string TWP_PIPE multi_string
            {
            #if IS_WIN32
                ASSERT( 0 == _tcscmp( _T("NTREG"), cGenreSwitcher::GetInstance()->GenreToString( cGenreSwitcher::GetInstance()->CurrentGenre() ) ) );
            #endif

                $1->push_back( _T("|") );
                MergeIntoList( $1, $3 );
                $$ = $1;

                delete $3;
            }
            ;            

host_name
            : string 
            {        
                /*not multistring because no eol delimiter */        
                cDebug d("    parser::multi_string(host_name)");
                $$ = $1;
            }
            ;            

variable_name
            : TWP_STRING
            {             
                cDebug d("    parser::string(variable_name)");
                $$ = $1;
            }
            ;


multi_string
            : multi_string string
            {
                $$->push_back( *$2 );
                delete $2;
            }
            | string
            {
                $$ = new cParseStringList;
                $$->push_back( *$1 );
                delete $1;
            }
            ;

global_multi_string
			: global_multi_string global_string
			{
				$$->push_back( *$2 );
				delete $2;
			}
			| global_string
			{
			    $$ = new cParseStringList;
                $$->push_back( *$1 );
                delete $1;
			}
            ;
				

string
            : TWP_STRING
            {             
                cDebug d("    parser::string(normal)");
	            d.TraceNever("--(STRING)--> got string (%s)\n", $1);
                $$ = $1;
            }
            | variable
            {             
                cDebug d("    parser::string(normal)");
	            d.TraceNever("--(STRING)--> got string (%s)\n", $1);
                $$ = $1;
            }
            ;

global_string
			: TWP_GLOBAL_STRING
			{
			    cDebug d("    parser::string(normal)");
	            d.TraceNever("--(STRING)--> got string (%s)\n", $1);
                $$ = $1;
            }
			;	
%%

