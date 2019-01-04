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
// unixexcept.cpp
#include "stdcore.h"
#include "debug.h"
#include "unixexcept.h"
#include "debug.h"

#include <errno.h>

// TODO: JEB: make this look like eFileError
eUnix::eUnix(const TCHAR* szFunctionName, const TCHAR* szObjectName, bool fCallGetLastError) : eError(_T(""))
{
    ASSERT(szFunctionName || szObjectName || fCallGetLastError);
    //
    // construct the error message:
    //
    // it will be of the form: FuncName() failed for Object: <FormatMessage output>
    //
    if (szFunctionName)
    {
        mMsg = szFunctionName;
        mMsg += _T(" failed");

        if (szObjectName)
        {
            mMsg += _T(" for ");
            mMsg += szObjectName;
        }
    }
    else if (szObjectName)
    {
        mMsg = szObjectName;
    }
    else
    {
        mMsg = _T("Error");
    }

    if (fCallGetLastError)
    {
        TSTRING strErr = strerror(errno);

        if (!strErr.empty())
        {
            mMsg += _T(": ");
            mMsg += strErr;
        }
    }
}

// eof - unixexcept.cpp
