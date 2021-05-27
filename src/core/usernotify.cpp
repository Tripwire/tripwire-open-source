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
// usernotify.cpp
#include "stdcore.h"
#include "usernotify.h"

// There was a problem with varargs on some combo of old OpenBSD & old compiler.
// Not needed with current, but it's here if you need it.
#ifdef TW_OLD_OPENBSD_VARARGS
#    undef va_start
#    ifdef __GNUC__
#        define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last))
#    else
#        define va_start(ap, last) ((ap) = (va_list) & (last) + __va_size(last))
#    endif
#endif // TW_OLD_OPENBSD_VARARGS

iUserNotify* iUserNotify::mpInstance = 0;

iUserNotify::iUserNotify(int verboseLevel) : mVerboseLevel(verboseLevel)
{
}

iUserNotify::~iUserNotify()
{
}

void iUserNotify::SetVerboseLevel(int level)
{
    mVerboseLevel = level;
}

int iUserNotify::GetVerboseLevel() const
{
    return mVerboseLevel;
}

///////////////////////////////////////////////////////////////////////////////
// NotifySilent
///////////////////////////////////////////////////////////////////////////////
void iUserNotify::NotifySilent(const TCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    HandleNotify(V_SILENT, format, args);
    va_end(args);
}

///////////////////////////////////////////////////////////////////////////////
// NotifyNormal
///////////////////////////////////////////////////////////////////////////////
void iUserNotify::NotifyNormal(const TCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    HandleNotify(V_NORMAL, format, args);
    va_end(args);
}

///////////////////////////////////////////////////////////////////////////////
// NotifyVerbose
///////////////////////////////////////////////////////////////////////////////
void iUserNotify::NotifyVerbose(const TCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    HandleNotify(V_VERBOSE, format, args);
    va_end(args);
}

///////////////////////////////////////////////////////////////////////////////
// Notify
///////////////////////////////////////////////////////////////////////////////
void iUserNotify::Notify(int verboseLevel, const TCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    HandleNotify(verboseLevel, format, args);
    va_end(args);
}
