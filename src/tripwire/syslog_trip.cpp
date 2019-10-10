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
// syslog.cpp
//

#include "stdtripwire.h"
#include "syslog_trip.h"

#if HAVE_SYSLOG_H
#   include <syslog.h>
#endif

#if HAVE_SYS_SYSLOG_H
#   include <sys/syslog.h>
#endif

// next three includes are for error reporting
#include "tw/twutil.h"
#include "tw/twerrors.h"
#include "tw/twstrings.h"

#if HAVE_PROTO_BSDSOCKET_H
#   include <proto/bsdsocket.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Syslog
///////////////////////////////////////////////////////////////////////////////
void cSyslog::Log(const TCHAR* programName, cSyslog::LogType logType, const TCHAR* message)
{
// SkyOS has syslog.h but doesn't actually implement the calls.
#if SUPPORTS_SYSLOG

    (void)logType; // logType is not used for Unix syslog

    ASSERT(sizeof(TCHAR) == sizeof(char));
    const char* ident = programName;
    const char* msg   = message;

#if HAVE_OPENLOG    
    openlog(ident, LOG_PID, LOG_USER);
#endif
    
    syslog(LOG_NOTICE, "%s", msg);
#if HAVE_CLOSELOG    
    closelog();
#endif
    
#endif
}
