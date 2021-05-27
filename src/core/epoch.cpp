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
// epoch.h
#include "stdcore.h"
#include "epoch.h"
#include <time.h>
#include <iostream>
#include "timeconvert.h"
#include "corestrings.h"

///////////////////////////////////////////////////////////////////////////////
bool CheckEpoch()
{
#if SIZEOF_TIME_T == 4

    struct tm time_struct;

    // Many functions will fail as we approach the end of the epoch
    // Rather than crashing, we will exit with a nice message
    memset(&time_struct, 0, sizeof(time_struct));
    time_struct.tm_mday = 1;
    time_struct.tm_mon  = 0;
    time_struct.tm_year = 138;
    int64_t endoftime     = cTimeUtil::DateToTime(&time_struct);

    if (time(0) > endoftime)
    {
        TCERR << TSS_GetString(cCore, core::STR_ENDOFTIME) << std::endl;
        return true;
    }

    return false;
#else
    return false;
#endif
}
