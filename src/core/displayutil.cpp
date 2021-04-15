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
// Name....: displayutil.cpp
// Date....: 7/12/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "stdcore.h"
#include "displayutil.h"
#include "ntmbs.h"
#include <iomanip>

TSTRING cDisplayUtil::FormatMultiLineString(const TSTRING& str, int nOffset, int nWidth)
{
    TOSTRINGSTREAM sstr;
    TSTRING        strT;
    bool           fFirstLine = true;
    for (TSTRING::const_iterator i = str.begin(); i != str.end(); i = *i ? i + 1 : i)
    {
        // return found -- add line to output string
        if (_T('\n') == *i)
        {
            // only do offset for strings after the first
            if (fFirstLine)
            {
                fFirstLine = false;
            }
            else
            {
                // add offset
                for (int j = 0; j < nOffset; j++)
                    sstr << _T(" ");

                // set width
                sstr << std::setw(nWidth);
            }

            // add to stringstream
            sstr << strT << std::endl;

            // erase temp string
            strT.erase();
        }
        else
        {
            // add char to string
            strT.append(i, (TSTRING::const_iterator)(*i ? i + 1 : i));
        }
    }

    // add last portion -- no endl
    // we want our client to be able to say "out << PropAsString() << endl;"

    // add offset
    if (!fFirstLine)
    {
        for (int j = 0; j < nOffset; j++)
            sstr << _T(" ");
    }

    // set width
    sstr << std::setw(nWidth);

    // now add last string
    sstr << strT;

    tss_return_stream(sstr, out);
}
