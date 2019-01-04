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
//////////////////////////////////////////////////////////////////////
// twparsererrors.cpp
//
// Registers all error strings in the parser package
//

#include "stdtwparser.h"
#include "twparsererrors.h"

#include "parserhelper.h"

TSS_BEGIN_ERROR_REGISTRATION(twparser)

//
// Parser Helper
//

TSS_REGISTER_ERROR(eParserHelper(), _T("Policy file parsing problem."));
TSS_REGISTER_ERROR(eParseFailed(), _T("Policy file parsing problem."));
TSS_REGISTER_ERROR(eParserRedefineVar(), _T("Read only variables cannot be assigned a value."));
TSS_REGISTER_ERROR(eParserUnrecognizedAttr(), _T("Unrecognized attribute name."));
TSS_REGISTER_ERROR(eParserUnrecognizedAttrVal(), _T("Unrecognized attribute value."));
TSS_REGISTER_ERROR(eParserPropChar(), _T("Unrecognized property mask in policy file."));
TSS_REGISTER_ERROR(eParserNoRules(), _T("No rules specified in policy file."));
TSS_REGISTER_ERROR(eParserNoRulesInSection(), _T("No rules specified in section."));
TSS_REGISTER_ERROR(eParserUseUndefVar(), _T("Undefined variable used."));
TSS_REGISTER_ERROR(eParserBadSevVal(), _T("Severity value outside of allowable limits."));
TSS_REGISTER_ERROR(eParserBadFCONameFmt(), _T("Bad objectname format."));
TSS_REGISTER_ERROR(eParserSectionAlreadyDefined(), _T("Section already declared."));
TSS_REGISTER_ERROR(eParserPointAlreadyDefined(), _T("Rule already defined as a start or stop point of another rule."));
TSS_REGISTER_ERROR(eParserSectionInsideBlock(), _T("Section declaration inside another block."));
TSS_REGISTER_ERROR(eParserRelativePath(), _T("Relative paths are not allowed in the policy file."));
TSS_REGISTER_ERROR(eParserIgnoringSection(), _T("Ignoring unknown section."));
TSS_REGISTER_ERROR(eParserNoStatementsInScope(), _T("Scoped attributes section has no statements."));
TSS_REGISTER_ERROR(eParserBadHex(), _T("Bad hex value in policy file."));
TSS_REGISTER_ERROR(eParserBadCharacter(), _T("Bad character found in policy file."));


TSS_END_ERROR_REGISTRATION()
