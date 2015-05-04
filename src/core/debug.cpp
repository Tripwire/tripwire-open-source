//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
// debug.cpp
#include "stdcore.h"
#include "debug.h"
#include "errorutil.h"


#ifdef DEBUG

#ifndef va_start
#include <cstdarg>
#endif
#include <cwchar>
#include <fstream>
#include <cstdio>

int		cDebug::mDebugLevel(10);
uint32	cDebug::mOutMask(cDebug::OUT_TRACE);
std::ofstream cDebug::logfile;
	//mDebugLevel default == 10, mOutMask default == OUT_TRACE.

///////////////////////////////////////////////////////////////////////////////
//  Constructors and Destructor
cDebug::cDebug(const char* label)
{
    int cnt = strlen(label);
    if (cnt > cDebug::MAX_LABEL)
        cnt = cDebug::MAX_LABEL - 1;

    memcpy(mLabel, label, cnt);
    mLabel[cnt] = '\0';
}

cDebug::cDebug(const cDebug &rhs)
{
	strcpy(mLabel, rhs.mLabel);
}

cDebug::~cDebug()
{
	if(logfile)
		logfile.close();
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Trace -- Outputs a format string only if the passed integer value is <=
// the "global debug level" (indicated by mDebugLevel).  
///////////////////////////////////////////////////////////////////////////////
void cDebug::Trace(int levelNum, const char* format, ...)
{
	if (levelNum > mDebugLevel)
		return;
    // create the output buffer 
    va_list args;
    va_start(args, format);
	DoTrace(format, args);
    va_end(args);

}

void cDebug::Trace(int levelNum, const wchar_t* format, ...)
{
	if (levelNum > mDebugLevel)
		return;
    // create the output buffer 
    va_list args;
    va_start(args, format);
	DoTrace(format, args);
    va_end(args);

}

///////////////////////////////////////////////////////////////////////////////
//  DoTrace()
//		internal helper function -- does the actual printing to logfile,
//		console, etc...
///////////////////////////////////////////////////////////////////////////////

void cDebug::DoTrace(const char *format, va_list &args)
{
    size_t guard1 = 0xBABABABA;
	char out[2048];
    size_t guard2 = 0xBABABABA;

    vsprintf(out, format, args);

    ASSERT(guard1 == 0xBABABABA && guard2 == 0xBABABABA); // string was too long
    ASSERT(strlen(out) < 1024);

	std::ostringstream ostr;
	ostr.setf(std::ios::left);
	ostr.width(40);
	ostr << mLabel;
	ostr.width(0);
	ostr << out;


    if ((mOutMask & OUT_STDOUT) != 0)
    {
        std::cout << ostr.str().c_str();
		std::cout.flush();
    }

    //
	//make it output to log file!
	//
	if ((mOutMask & OUT_FILE) != 0)
    {
		// the logfile is narrow chars only...
		logfile.setf(std::ios::left);
		logfile.width(40);
		logfile << mLabel;
		logfile.width(0);
		logfile << out;
		logfile.flush();
    }
}

void cDebug::DoTrace(const wchar_t *format, va_list &args)
{
#if IS_UNIX
    // we don't support vswprintf on UNIX
    ASSERT(false);
    THROW_INTERNAL("debug.cpp");
#else

    size_t guard1 = 0xBABABABA;
	wchar_t out[2048];
    size_t guard2 = 0xBABABABA;

    vswprintf(out, format, args);
    
    ASSERT(guard1 == 0xBABABABA && guard2 == 0xBABABABA); // string was too long
    char nout[1024];
	if (wcstombs(nout, out, 1024) == -1) 
        strcpy(nout, "XXX Unconvertable wide char detected in cDebug::DoTrace()\n");

    std::ostringstream ostr;
	ostr.setf(std::ios::left);
	ostr.width(40);
	ostr << mLabel;
	ostr.width(0);
	ostr << nout;


    if ((mOutMask & OUT_STDOUT) != 0)
    {
		std::cout << ostr.str().c_str();
		std::cout.flush();
    }

    //
	//make it output to log file!
	//
	if ((mOutMask & OUT_FILE) != 0)
    {
		// the logfile is narrow chars only...
		logfile.setf(std::ios::left);
		logfile.width(40);
		logfile << mLabel;
		logfile.width(0);
		logfile << out;
		logfile.flush();
    }
#endif // IS_UNIX
}

#ifdef DEBUG

//
// wrappers around Trace() that requires less typing
//		TODO: this is quick and dirty, but lets me check in all these files right away.  --ghk
//

void cDebug::TraceAlways(const char *format, ...)
{
	if (D_ALWAYS > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceError(const char *format, ...)
{
	if (D_ERROR > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceWarning(const char *format, ...)
{
	if (D_WARNING > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceDebug(const char *format, ...)
{
	if (D_DEBUG > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceDetail(const char *format, ...)
{
	if (D_DETAIL > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceNever(const char *format, ...)
{
	if (D_NEVER > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceAlways(const wchar_t *format, ...)
{
	if (D_ALWAYS > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceError(const wchar_t *format, ...)
{
	if (D_ERROR > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceWarning(const wchar_t *format, ...)
{
	if (D_WARNING > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceDebug(const wchar_t *format, ...)
{
	if (D_DEBUG > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceDetail(const wchar_t *format, ...)
{
	if (D_DETAIL > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceNever(const wchar_t *format, ...)
{
	if (D_NEVER > mDebugLevel)
		return;

	// fill up arglist, and pass to printing routine
    va_list args;
	va_start(args, format);
	DoTrace(format, args);
	va_end(args);
}

void cDebug::TraceVaArgs( int iDebugLevel, const char *format, va_list &args )
{
    if ( iDebugLevel <= mDebugLevel )
        DoTrace( format, args);
}

void cDebug::TraceVaArgs( int iDebugLevel, const wchar_t *format, va_list &args )
{
    if ( iDebugLevel <= mDebugLevel )
        DoTrace( format, args );
}

#endif // DEBUG

///////////////////////////////////////////////////////////////////////////////
//  AddOutTarget -- Attempts to add a new target for trace/debug output.  
//  FAILS ONLY IF caller attempts to SET OUT_FILE via this function.
///////////////////////////////////////////////////////////////////////////////
bool cDebug::AddOutTarget(OutTarget target)
{
	if (target == OUT_STDOUT)
		mOutMask |= OUT_STDOUT;
	if (target == OUT_TRACE)
		mOutMask |= OUT_TRACE;
	if (target == OUT_FILE) {
		mOutMask |= OUT_FILE;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// RemoveOutTarget -- Masks out from mOutMask the value passed.
///////////////////////////////////////////////////////////////////////////////
bool cDebug::RemoveOutTarget(OutTarget target)
{
	if (!HasOutTarget(target))
		return true;
	if (target == OUT_STDOUT)
		mOutMask ^= OUT_STDOUT;
	if (target == OUT_TRACE)
		mOutMask ^= OUT_TRACE;
	if (target == OUT_FILE)
		mOutMask ^= OUT_FILE;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// HasOutTarget -- Takes target - target must represent a single flagged bit
///////////////////////////////////////////////////////////////////////////////
bool cDebug::HasOutTarget(OutTarget target)
{
	
	if (target == OUT_STDOUT)
		return ((mOutMask & OUT_STDOUT) != 0);
	else if (target == OUT_TRACE)
		return ((mOutMask & OUT_TRACE) != 0);
	else if (target == OUT_FILE)
		return ((mOutMask & OUT_FILE) != 0);
	else //ambiguous input, or too many bits set in target
		return false;
}

///////////////////////////////////////////////////////////////////////////////
// SetOutputFile -- Attempts to set the output file for Logfile member to 
// the string passed in.
///////////////////////////////////////////////////////////////////////////////
bool cDebug::SetOutputFile(const char* filename)
{
	// TODO -- make sure this does the right thing if a log file is 
	// already open!
	// TODO -- make this work with wide chars	
	if (!logfile)
        logfile.open(filename, std::ios_base::out | std::ios_base::ate | std::ios_base::app);
	else
		logfile.setf(std::ios_base::hex, std::ios_base::basefield);
			//make sure info. will not be clobbered.

	//Should be open now- if not, abort.
	if (!logfile) {
		mOutMask ^= OUT_FILE;
		return false;
	} else
		mOutMask |= OUT_FILE;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
// DebugOut -- Works just like TRACE. note: there is an internal buffer size
// of 1024; traces larger than that will have unpredictable results.
//////////////////////////////////////////////////////////////////////////////
void cDebug::DebugOut( const char* lpOutputString, ... )
{
	char buf[2048];
    // create the output buffer 
    va_list args;
    va_start(args, lpOutputString);
    vsprintf(buf, lpOutputString, args);
    va_end(args);
    
#ifdef _UNICODE
    wchar_t wbuf[2048];
	if (mbstowcs(wbuf, buf, strlen(buf)+1) == -1) 
        wcscpy(wbuf, _T("XXX Unconvertable mb character detected in cDebug::DebugOut()\n") );

    #if !USE_OUTPUT_DEBUG_STRING 
    #ifdef _DEBUG
		TCERR << wbuf;
    #endif	//_DEBUG
    #else	// USE_OUTPUT_DEBUG_STRING
	::OutputDebugString(wbuf);
    #endif	// USE_OUTPUT_DEBUG_STRING
#else	// _UNICODE
    #if !USE_OUTPUT_DEBUG_STRING 
        #ifdef _DEBUG
		TCERR << buf;
        #endif	//_DEBUG
    #else	// USE_OUTPUT_DEBUG_STRING
	    ::OutputDebugString(buf);
    #endif	// USE_OUTPUT_DEBUG_STRING
#endif	// _UNICODE

    TCOUT.flush();
}

void cDebug::DebugOut( const wchar_t* lpOutputString, ... )
{
    va_list args;
    va_start(args, lpOutputString);

#if IS_UNIX
    char mbformatbuf[1024];
    char buf[1024];
//	if (wcstombs(mbformatbuf, lpOutputString, wcslen(lpOutputString)) == -1)
//      strcpy(mbformatbuf, "XXX Unconvertable wide char detected in cDebug::DebugOut()\n");

    vsprintf(buf, mbformatbuf, args);
#else

    wchar_t buf[1024];
    vswprintf(buf, lpOutputString, args);
#endif
    va_end(args);
    
#ifdef _UNICODE
    #if !USE_OUTPUT_DEBUG_STRING 
        #ifdef _DEBUG
        TCERR << buf;
        #endif	//_DEBUG
    #else	// USE_OUTPUT_DEBUG_STRING
    ::OutputDebugString(buf);
    #endif	// USE_OUTPUT_DEBUG_STRING
#else
    char nbuf[1024];
    #if IS_UNIX
	strcpy(nbuf, buf);
    #else
	if (wcstombs(nbuf, buf, wcslen(buf)+1) == -1)
        strcpy(nbuf, "XXX Unconvertable wide char detected in cDebug::DebugOut()\n");
#endif 

#if !USE_OUTPUT_DEBUG_STRING 
    #ifdef _DEBUG
    TCERR << nbuf;
    #endif	//_DEBUG
#else	// USE_OUTPUT_DEBUG_STRING
	::OutputDebugString(nbuf);
#endif	// USE_OUTPUT_DEBUG_STRING
    #endif

    TCOUT.flush();
}

#endif // DEBUG

//////////////////////////////////////////////////////////////////////////////////
// ASSERT macro support function
//////////////////////////////////////////////////////////////////////////////////

