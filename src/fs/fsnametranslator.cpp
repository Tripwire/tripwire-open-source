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
// fsnametranslator.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdfs.h"

#include "fsnametranslator.h"
#include "fco/fconame.h"
#include "util/stringencoder.h"
#include "core/displayencoder.h"
#include "core/debug.h"

//=========================================================================
// METHOD CODE
//=========================================================================


///////////////////////////////////////////////////////////////////////////////
// ToStringRaw
///////////////////////////////////////////////////////////////////////////////
TSTRING cFSNameTranslator::ToStringRaw(const cFCOName& name) const
{
    return name.AsString();
}

///////////////////////////////////////////////////////////////////////////////
// ToStringAPI
///////////////////////////////////////////////////////////////////////////////
TSTRING cFSNameTranslator::ToStringAPI(const cFCOName& name) const
{
    ASSERT(name.GetDelimiter() == _T('/'));

    return name.AsString();
}

///////////////////////////////////////////////////////////////////////////////
// ToStringDisplay
///////////////////////////////////////////////////////////////////////////////
TSTRING cFSNameTranslator::ToStringDisplay(const cFCOName& name, bool bUnique) const
{
    TSTRING strRet;

    if (bUnique)
    {
        cDisplayEncoder e(cDisplayEncoder::ROUNDTRIP);
        TSTRING         strUnencoded = name.AsString();
        e.Encode(strUnencoded);
        const TCHAR dq = _T('\"');

        strRet += dq;
        strRet += strUnencoded;
        strRet += dq;
    }
    else
    {
        cDisplayEncoder e(cDisplayEncoder::NON_ROUNDTRIP); // cDisplayEncoder
        strRet = name.AsString();
        e.Encode(strRet);
    }

    return strRet;
}

///////////////////////////////////////////////////////////////////////////////
// DisplayStringToFCOName
///////////////////////////////////////////////////////////////////////////////
bool cFSNameTranslator::DisplayStringToFCOName(const TSTRING& strC, cFCOName& name) const
{
    TSTRING     str = strC;
    const TCHAR dq  = _T('\"');

    // do a little error checking.  must have at least '""'
    if (str.size() < 1)
        return false;
    if (dq != str[0])
        return false;
    if (dq != str[str.size() - 1])
        return false;

    // get rid of beginning and trailing quote
    str = str.substr(1, str.size() - 2);

    //
    // undo funky wide char encoding
    //
    cDisplayEncoder e(cDisplayEncoder::ROUNDTRIP);
    if (!e.Decode(str))
        return false;

    // give to cFCOName
    name = str;

    return true;
}
