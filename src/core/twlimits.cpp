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
// twlimits.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdcore.h"
#include "twlimits.h"
#include "debug.h"

//=========================================================================
// METHOD CODE
//=========================================================================

bool tss_isdigit(const TCHAR in)
{
#if ARCHAIC_STL
  return isdigit(in);
#else
  return std::isdigit<TCHAR>(in, std::locale());
#endif
}
  

bool cInterpretInt::InterpretInt(const TSTRING& str, int* pi)
{
    ASSERT(pi != 0);

    bool fFormatOK = true;

#if !ARCHAIC_STL    
    //
    // make sure string is not longer than a string representation of LIMIT_MAX
    //
    TOSTRINGSTREAM sstr;
    sstr << cInterpretInt::LIMIT_MAX;
    if (str.length() > sstr.str().length())
        fFormatOK = false;
#endif
    
    //
    // make sure string is not too short
    //
    if (fFormatOK && str.length() <= 0)
        fFormatOK = false;

    //
    // make sure first character is a digit or the minus sign
    //
    if (fFormatOK)
    {
        if (!(_T('-') == str[0] || tss_isdigit(str[0])))
            fFormatOK = false;
    }

    //
    // make sure all other characters are digits
    // NOTE:BAM -- this assumes that all digits in all locales are SB characters.
    // TODO:BAM -- check this...
    for (TSTRING::size_type j = 1; fFormatOK && j < str.length(); j++)
    {
        if (!tss_isdigit(str[j]))
            fFormatOK = false;
    }

    //
    // do _ttoi conversion and check that it is within allowable limits
    //
    if (fFormatOK)
    {
        *pi = _ttoi(str.c_str());

        if (*pi < GetMin() || *pi > GetMax())
            fFormatOK = false;
    }

    return fFormatOK;
}
