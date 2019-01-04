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
//
// Name....: twparserstrings.h
// Date....: 05/11/99
// Creator.: Brian McFeely (bmcfeely)
//

#ifndef __TWPARSERSTRINGS_H
#define __TWPARSERSTRINGS_H

#include "twparser.h" // for: STRINGTABLE syntax

//--Message Keys

TSS_BeginStringIds(twparser)

    STR_PARSER_INVALID_CHAR,
    STR_YYERROR_LINE_NUM_FMT, STR_PARSER_RULENAME, STR_PARSER_EMAILTO, STR_PARSER_RECURSE, STR_PARSER_SEVERITY,
    STR_PARSER_FALSE, STR_PARSER_TRUE, STR_PARSER_SECTION_GLOBAL, STR_LINE_NUMBER,
    STR_ERR2_PARSER_NO_RULES_IN_SECTION

    TSS_EndStringIds(twparser)


#endif //__PARSERSTRINGS_H
