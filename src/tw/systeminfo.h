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
// systeminfo.h
//
// cSystemInfo -- a storage place for run time system information
#ifndef __SYSTEMINFO_H
#define __SYSTEMINFO_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

#ifndef __DEBUG_H
#include "core/debug.h"
#endif

class cSystemInfo
{
public:
    cSystemInfo();
    ~cSystemInfo();

    static TSTRING  GetExePath();
    static void     SetExePath(const TSTRING& path);
    // Get the full path to this executable

    static TSTRING  GetExeDir();
    static void     SetExeDir(const TSTRING& dir);
    // these methods get and set the executable's working directory ... it is asserted
    // that SetExeDir() has been called when GetExeDir() is called.

    static int64_t  GetExeStartTime();
    static void     SetExeStartTime(const int64_t& time);
    // Get and set the approximate time (time_t format) which the excecutable was started.
    // This will be used for all times having to do with this run of the executable.

private:
    static TSTRING mExePath;
    static bool    mbExePathSet;
    static TSTRING mExeDir;
    static bool    mbExeDirSet;
    static int64_t mExecuteStartTime;
};

//-----------------------------------------------------------------------------
// inline implementation
//-----------------------------------------------------------------------------
inline cSystemInfo::cSystemInfo()
{
}

inline cSystemInfo::~cSystemInfo()
{
}

inline TSTRING cSystemInfo::GetExePath()
{
    ASSERT(mbExePathSet);
    return mExePath;
}

inline void cSystemInfo::SetExePath(const TSTRING& path)
{
    ASSERT(!path.empty());
    mbExePathSet = true;

    mExePath = path;
}

inline TSTRING cSystemInfo::GetExeDir()
{
    ASSERT(mbExeDirSet);
    return mExeDir;
}

inline void cSystemInfo::SetExeDir(const TSTRING& dir)
{
    ASSERT(!dir.empty());
    mbExeDirSet = true;

    mExeDir = dir;
}

inline int64_t cSystemInfo::GetExeStartTime()
{
    ASSERT(mExecuteStartTime != 0);
    return mExecuteStartTime;
}

inline void cSystemInfo::SetExeStartTime(const int64_t& time)
{
    mExecuteStartTime = time;
}

#endif //__SYSTEMINFO_H
