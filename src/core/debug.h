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
// debug.h
//
// definitions of debug macros that will vary across platforms

#ifndef __DEBUG_H
#define __DEBUG_H

#include <assert.h>


#include <iostream>
/* Do it in this order, because autoconf checks for <stdarg.h>
 * first  i.e. if HAVE_VARARGS_H is defined, it is only because
 * <stdarg.h> couldn't be found.
 */
#ifdef HAVE_VARARGS_H
#include <varargs.h>
#else
#    ifdef HAVE_STDARG_H
#        include <stdarg.h>
#    endif
#endif
#include "types.h"

//
// NOTE:
// When compiling with MFC, these are already defined and we get error msgs
// every time this file is included. Since these behave the same as the MFC
// version, it is OK to always undef them here....
//      -- 20 Aug 99 mdb
//
#undef ASSERT
#undef TRACE

//
// IMPORTANT:
//
// strings outputted as trace statements are printed as narrow characters.
// passing trace messages with wide characters will have odd results, since
// they will be used as arugments to sprintf(), etc...
//

// debug utility class
class cDebug
{
public:
    enum OutTarget
    {
        OUT_STDOUT = 1,
        OUT_TRACE  = 2,
        OUT_FILE   = 4
    };

    enum DebugLevel
    {
        D_ALWAYS  = 0,
        D_ERROR   = 1,
        D_WARNING = 4,
        D_DEBUG   = 8,
        D_DETAIL  = 16,
        D_NEVER   = 1000
    };

    cDebug(const char* pLabel);
    ~cDebug();
    cDebug(const cDebug& rhs);

    // These are the preferred tracing interfaces, because you don't need to know
    //  the DebugLevel enums.
    // Wide/Narrow Chars Issues: If you include a %s in your format string and you
    //  wish to print out a TCHAR (which might be a natural thing to do) you should
    //  encompas the format string with a _T("") macro, i.e. make it a TSTRING.
    //  The wide character overloads of these functions will expect wide strings
    //  for %s options.
    //
    void TraceAlways(const char* format, ...);
    void TraceError(const char* format, ...);
    void TraceWarning(const char* format, ...);
    void TraceDebug(const char* format, ...);
    void TraceDetail(const char* format, ...);
    void TraceNever(const char* format, ...);


    // these are of use if you are inside a function with a "..." as an argument
    // and you want to trace those args
    void TraceVaArgs(int iDebugLevel, const char* format, va_list& args);

    // ...but you can still choose to use this interface...

    void Trace(int levelNum, const char* format, ...);
    // Outputs based on levelnum.  If levelnum <= global debug, print.

public:
    static bool AddOutTarget(OutTarget target);
    static bool RemoveOutTarget(OutTarget target);
    // used to specify the out target....
    static bool HasOutTarget(OutTarget target);

    static bool SetOutputFile(const char* filename);
    // specifies the output file name used when OUT_FILE is set
    static void SetDebugLevel(int level);
    static int  GetDebugLevel(void);
    // gets and sets the global debug level. Trace output at or below this
    // level will be output.

    static void DebugOut(const char* lpOutputString, ...);
    // Works just like TRACE
    // note: there is an internal buffer size of 1024; traces larger
    // than that will have unpredictable and probably bad results
private:
#ifdef DEBUG
    enum
    {
        MAX_LABEL = 128
    };

    static int           mDebugLevel;
    static uint32_t      mOutMask;
    static std::ofstream logfile;
    char                 mLabel[MAX_LABEL];

    // helper functions
    void DoTrace(const char* format, va_list& args);
#endif
};

#ifdef DEBUG
#    define TRACE cDebug::DebugOut
#else
#    define TRACE 1 ? (void)0 : cDebug::DebugOut
#endif // DEBUG

//////////////////////////////////////////////////////////////////////////////////
// inline implementation
//////////////////////////////////////////////////////////////////////////////////
// Hopefully this class should do nothing in release mode

#ifdef DEBUG

inline void cDebug::SetDebugLevel(int level)
{
    mDebugLevel = level;
}

inline int cDebug::GetDebugLevel()
{
    return mDebugLevel;
}

#    else // DEBUG

inline cDebug::cDebug(const char*)
{
}
inline cDebug::~cDebug()
{
}
inline cDebug::cDebug(const cDebug&)
{
}
inline void cDebug::TraceAlways(const char*, ...)
{
}
inline void cDebug::TraceError(const char*, ...)
{
}
inline void cDebug::TraceWarning(const char*, ...)
{
}
inline void cDebug::TraceDebug(const char*, ...)
{
}
inline void cDebug::TraceDetail(const char*, ...)
{
}
inline void cDebug::TraceNever(const char*, ...)
{
}
inline void cDebug::TraceVaArgs(int, const char*, va_list&)
{
}
inline void cDebug::Trace(int, const char*, ...)
{
}
inline bool cDebug::AddOutTarget(OutTarget)
{
    return false;
}
inline bool cDebug::RemoveOutTarget(OutTarget)
{
    return false;
}
inline bool cDebug::HasOutTarget(OutTarget)
{
    return false;
}
inline bool cDebug::SetOutputFile(const char*)
{
    return false;
}
inline void cDebug::SetDebugLevel(int)
{
}
inline int cDebug::GetDebugLevel(void)
{
    return 0;
}
inline void cDebug::DebugOut(const char*, ...)
{
}

#    endif // DEBUG


//////////////////////////////////////////////////////////////////////////////////
// ASSERT macro
//////////////////////////////////////////////////////////////////////////////////

#    define ASSERTMSG(exp, s) assert((exp) != 0)
#    define ASSERT(exp) assert((exp) != 0)
    // if we are not windows we will just use the standard assert()
#    define TSS_DebugBreak() ASSERT(false);

#    ifndef ASSERT
#        error ASSERT did not get defined!!!
#    endif

#    ifndef ASSERTMSG
#        error ASSERTMSG did not get defined!!!
#    endif

#    ifndef TSS_DebugBreak
#        error TSS_DebugBreak did not get defined!!!
#    endif

#endif //__DEBUG_H
