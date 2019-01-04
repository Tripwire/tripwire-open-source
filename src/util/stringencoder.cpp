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
// stringencoder.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdutil.h"
#include "stringencoder.h"
#include "core/debug.h"

//=========================================================================
// METHOD CODE
//=========================================================================

//////////////////////////////////////////////////////////////////////////////
// cCharEncoder
//////////////////////////////////////////////////////////////////////////////

void cCharEncoder::EncodeChar(TSTRING::const_iterator& i, TSTRING& strToAppendTo)
{
    if (*i == mchEncodeMe)
    {
        strToAppendTo += mchEncoded1;
        strToAppendTo += mchEncoded2;
    }
    else if (*i == mchEncoded1)
    {
        strToAppendTo += mchEncoded1;
        strToAppendTo += mchEncoded1;
    }
    else
    {
        strToAppendTo += *i;
    }
}

TCHAR cCharEncoder::DecodeChar(TSTRING::const_iterator& i, TSTRING::const_iterator& end)
{
    ASSERT(i != end);

    if (*i == mchEncodeMe)
    {
        ASSERT(false);
        return _T('\0');
    }
    else if (*i == mchEncoded1)
    {
        if (++i == end)
        {
            ASSERT(false);
            return _T('\0');
        }
        else
        {
            if (*i == mchEncoded1)
                return mchEncoded1;
            else if (*i == mchEncoded2)
                return mchEncodeMe;
            else
            {
                ASSERT(false);
                return _T('\0');
            }
        }
    }
    else
        return (*i);
}

//////////////////////////////////////////////////////////////////////////////
// cStringEncoder
//////////////////////////////////////////////////////////////////////////////

TSTRING& cStringEncoder::Encode(TSTRING& inAndOut)
{
    TSTRING strOut;
    Encode(inAndOut, strOut);
    inAndOut = strOut;
    return inAndOut;
}


TSTRING& cStringEncoder::Encode(const TSTRING& in, TSTRING& out)
{
    out.erase();
    //
    // assume that out will approximately be the size of in
    //
    out.reserve(in.length());

    for (TSTRING::const_iterator i = in.begin(); i != in.end(); ++i)
        ce.EncodeChar(i, out);

    return out;
}

TSTRING& cStringEncoder::Unencode(TSTRING& inAndOut)
{
    TSTRING strOut;
    Unencode(inAndOut, strOut);
    inAndOut = strOut;
    return inAndOut;
}

TSTRING& cStringEncoder::Unencode(const TSTRING& in, TSTRING& out)
{
    out.erase();
    //
    // assume that out will approximately be the size of in
    //
    out.reserve(in.length());

    TSTRING::const_iterator end = in.end();
    for (TSTRING::const_iterator i = in.begin(); i != end; ++i)
        out += ce.DecodeChar(i, end);

    return out;
}
