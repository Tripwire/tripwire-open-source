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
// errorutil.cpp
//
#include "stdcore.h"
#include "errorutil.h"
#include "corestrings.h"

//=============================================================================
// class eInternal
//=============================================================================

namespace //unique
{
TCHAR* tw_itot(int value, TCHAR* string, int radix, int size)
{
    snprintf(string, size, "%d", value);
    return string;
}
} // namespace

eInternal::eInternal(TCHAR* sourceFile, int lineNum) : eError(_T(""))
{
    TCHAR buf[256];

    mMsg = _T("File: ");
    mMsg += sourceFile;
    mMsg += _T(" Line: ");
    mMsg += tw_itot(lineNum, buf, 10, 256);
}

//=============================================================================
// class cErrorUtil
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// MakeFileError
///////////////////////////////////////////////////////////////////////////////
TSTRING cErrorUtil::MakeFileError(const TSTRING& msg, const TSTRING& fileName)
{
    TSTRING ret;
    ret = TSS_GetString(cCore, core::STR_ERR2_FILENAME);
    ret.append(fileName);
    ret.append(1, _T('\n'));

    if (msg.length() > 0)
    {
        ret.append(msg);
    }

    return ret;
}
