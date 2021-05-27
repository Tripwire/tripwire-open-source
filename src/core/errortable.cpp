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
// errortable.cpp
//
#include "stdcore.h"
#include "errortable.h"

#ifdef DEBUG
#include "package.h"
#include "corestrings.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// GetInstance
///////////////////////////////////////////////////////////////////////////////
cErrorTable* cErrorTable::GetInstance()
{
    static cErrorTable gErrorTable;
    return &gErrorTable;
}

///////////////////////////////////////////////////////////////////////////////
// AssertMsgValidity
///////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
void cErrorTable::AssertMsgValidity(const TCHAR* msg)
{
    // Check to see that the first part of this error message is not too long.
    TSTRING errStr = msg;

    // We only need to check up until the first '\n' since error reporter moves everything
    // after that to the "second part".
    TSTRING::size_type errSize = errStr.find_first_of(_T('\n'));
    if (errSize == TSTRING::npos)
        errSize = errStr.length();

#    if 0 //TODO: Figure out how to do this properly.
    TSTRING::size_type errorSize = TSS_GetString( cCore, core::STR_ERROR_ERROR ).length();
    TSTRING::size_type warningSize = TSS_GetString( cCore, core::STR_ERROR_WARNING ).length();
    TSTRING::size_type maxHeaderSize = (errorSize > warningSize ? errorSize : warningSize) + 6; // Add 6 to account for "### ' and ': '
#    else
    // Sunpro got stuck in an infinite loop when we called GetString from this func;
    TSTRING::size_type errorSize   = 9;
    TSTRING::size_type warningSize = 10;
    TSTRING::size_type maxHeaderSize =
        (errorSize > warningSize ? errorSize : warningSize) + 6; // Add 6 to account for "### ' and ': '
#    endif

    ASSERT(maxHeaderSize + errSize < 80);
}
#endif
